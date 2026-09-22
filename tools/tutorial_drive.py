"""Replay the game's tutorial (house 7) by keyboard through the mod, checking each step in the mod's log.

  python tools/tutorial_drive.py [from_step]     steps: reset, enter, zoom, scroll, rotate, portrait,
                                                 gohere, interact, shower, needs, buy, chair, place, turn, live2, personality, isabel,
                                                 isabel_object, select_first, social, relationship, newspaper, job, options
Each step presses keys (tools/keys.py) and waits for a line the mod speaks or logs ("tutorial step:").
Prints what it waited for and stops at the first step that does not arrive within its timeout.
"""
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
LOG = r"S:\steam\steamapps\common\The Sims Legacy Collection\SimsAccess\SimsAccess.log"


def lines():
    with open(LOG, encoding="utf-8", errors="replace") as f:
        return f.read().splitlines()


def keys(*ks, delay=None):
    cmd = [sys.executable, os.path.join(HERE, "keys.py"), *ks]
    if delay is not None:
        cmd += ["--delay", str(delay)]
    subprocess.run(cmd, check=True)


def last_spoken():
    """The last focus announcement (the tutorial watcher's lines are skipped)."""
    for l in reversed(lines()):
        if "Spoke:" in l:
            text = l.split("Spoke:", 1)[1].strip()
            if text.startswith(("Tutorial:", "Con el teclado", "la flecha", "hay un consejo", "Keyboard:", "the arrow")):
                continue
            return text
    return ""


def nav(key, text, limit=30):
    # The item focused now counts (a category jump lands on its first object).
    spoken = last_spoken()
    if key.endswith("pgdn") or key.endswith("pgup"):  # "categoria, N elementos. Objeto, sala, ..." -> Objeto
        label = spoken.split(". ", 1)[-1].split(", ", 1)[0] if " elementos. " in spoken or " elemento. " in spoken else spoken.split(", ", 1)[0]
    else:
        label = ""  # groups: always search (their summary is only spoken on entry)
    if text.lower() in label.lower():
        print("  already on: " + last_spoken()[:100])
        return True
    r = subprocess.run([sys.executable, os.path.join(HERE, "nav_to.py"), key, text, str(limit)],
                       capture_output=True, text=True, encoding="utf-8")
    ok = r.returncode == 0
    print(("  found: " if ok else "  NOT FOUND: ") + r.stdout.strip().splitlines()[-1] if r.stdout.strip() else "")
    return ok


def wait_any(texts, timeout=45, since=None):
    """Waits for the first of several texts; returns (text, line count)."""
    start = len(lines()) if since is None else since
    deadline = time.time() + timeout
    while time.time() < deadline:
        cur = lines()
        for i in range(start, len(cur)):
            for t in texts:
                if t.lower() in cur[i].lower():
                    print(f"  ok: {t}")
                    return t, len(cur)
        time.sleep(0.5)
    print(f"  TIMEOUT waiting for any of {texts!r}")
    raise SystemExit(1)


def wait_for(text, timeout=45, since=None, answer_help=True):
    """Waits until a line after `since` (line count) contains text; returns the new line count."""
    start = len(lines()) if since is None else since
    deadline = time.time() + timeout
    while time.time() < deadline:
        cur = lines()
        for i in range(start, len(cur)):
            l = cur[i]
            if answer_help and "activar el sistema de ayuda" in l:
                print("  help system prompt: yes")
                keys("enter")
                start = i + 1
                break
            if text.lower() in l.lower():
                print(f"  ok: {l[l.find('Spoke:') + 6 if 'Spoke:' in l else 0:][:110].strip()}")
                return len(cur)
        time.sleep(0.5)
    print(f"  TIMEOUT waiting for {text!r}")
    raise SystemExit(1)


def step_reset():
    keys("f5")
    time.sleep(2)
    nav("tab", "opciones", 8)
    nav("right", "juego", 8)
    keys("enter")
    time.sleep(1.5)
    nav("down", "tutorial", 12)
    n = len(lines())
    keys("enter")
    wait_for("Seguro que quieres reiniciar", since=n)
    n = len(lines())
    keys("enter")
    wait_for("guardar la partida", since=n)
    n = len(lines())
    keys("right")
    time.sleep(0.8)
    keys("enter")
    wait_for("Barrio", 60, since=n)


def step_enter():
    time.sleep(2)
    keys("tab")
    nav("right", "Calle Sim,7", 45)
    n = len(lines())
    keys("enter")
    wait_for("Bienvenido al tutorial", 90, since=n)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Echa un vistazo", 60, since=n)
    keys("3")  # ultra speed: the Sim's own steps (shower) take game time


def step_zoom():
    nav("tab", "controles", 8)
    nav("right", "Ver de cerca", 25)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Éste es el zoom", since=n)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Hay muchas formas", since=n)


def step_scroll():
    n = len(lines())
    keys("pgdn")
    wait_for("tutorial step: Bien. Otra buena manera", since=n)
    n = len(lines())
    keys("pgdn")
    wait_for("tutorial step: Ahora vamos a rotar", since=n)


def step_rotate():
    nav("tab", "controles", 8)
    nav("right", "Girar a la derecha", 25)
    n = len(lines())
    keys("enter")
    got, n = wait_any(["VIVIR activa", "icono VIVIR"], since=n)
    if got == "icono VIVIR":  # live mode is on already: the step needs a mode change back to live
        nav("tab", "controles", 8)
        nav("right", "Modo Comprar", 25)
        keys("enter")
        time.sleep(3)
        step_live()
        return
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Los miembros de la familia", since=n)


def step_live():
    # from buy mode: the Live tab is the first control
    nav("tab", "controles", 8)
    keys("home")
    time.sleep(0.8)
    n = len(lines())
    keys("enter")
    wait_for("VIVIR activa", since=n)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Los miembros de la familia", since=n)


def step_portrait():
    n = len(lines())
    keys("ctrl+1")
    wait_for("tutorial step: Ahora vas a aprender a controlar", since=n)


def step_gohere():
    n = len(lines())
    keys("shift+pgup", "shift+pgdn", "pgdn")
    keys("ctrl+enter")
    wait_for("Bien, mira a Roberto", since=n)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Puedes dar", since=n)


def step_interact():
    nav("shift+pgdn", "electrodom", 8)
    nav("pgdn", "Nevera", 10)
    n = len(lines())
    keys("enter")
    wait_for("interacciones", since=n)
    n = len(lines())
    keys("enter")
    wait_for("Las necesidades, abajo", 60, since=n)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Vamos a mejorar", since=n)


def step_shower():
    nav("shift+pgdn", "fontaner", 8)
    nav("pgdn", "Ducha", 12)
    n = len(lines())
    keys("enter")
    wait_for("interacciones", since=n)
    nav("down", "Duchar", 6)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Haz clic en las las diferentes", 240, since=n)


def step_needs():
    if not nav("tab", "Necesidades", 8):  # the mood tab toggles the needs display
        nav("tab", "paneles del Sim", 8)
        nav("right", "Estado de", 3)
        keys("enter")
        time.sleep(2)
        nav("tab", "Necesidades", 8)
    keys("right", "right")
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Si logras mantener", since=n)


def step_buy():
    nav("tab", "controles", 8)
    nav("right", "Modo Comprar", 25)
    n = len(lines())
    keys("enter")
    wait_for("Ahora estás en el modo COMPRAR", since=n)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Echa un vistazo al catálogo", since=n)


def step_chair():
    nav("tab", "catálogo", 8)
    keys("enter")
    time.sleep(1.5)
    for _ in range(14):  # right until a priced product (the sub-tabs come first)
        keys("right")
        time.sleep(0.7)
        if ", $" in last_spoken():
            print("  product: " + last_spoken()[:80])
            break
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Para comprar este objeto", since=n)


def step_place():
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Estupendo, ahora vamos a rotarlo", since=n)


def step_turn():
    if "otro en la mano" in " ".join(lines()[-6:]):  # another chair in hand: put it away
        keys("esc")
        time.sleep(1)
    nav("shift+pgup", "asientos", 10)
    for _ in range(30):  # the chair bought a moment ago
        if "comprado" in last_spoken():
            break
        keys("pgdn")
        time.sleep(0.7)
    print("  chair: " + last_spoken()[:80])
    n = len(lines())
    keys("enter")
    wait_for("moviendo", since=n)
    time.sleep(1)
    keys("shift+pgdn")
    time.sleep(1.5)
    n = len(lines())
    keys("enter")
    wait_for("colocado", since=n)
    wait_for("tutorial step:", 60, since=n)


def step_live2():
    # Dialog25: back to live mode (the tutorial waits for the Live button)
    nav("tab", "controles", 8)
    keys("home")
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Si no das ninguna orden", 60, since=n)


def step_personality():
    nav("tab", "paneles del Sim", 8)
    nav("right", "Personalidad", 8)
    n = len(lines())
    keys("enter")
    got, n = wait_any(["tutorial step: Haz clic en la etiqueta", "tutorial step: Ahora vamos a agregar"], since=n)
    if got.endswith("etiqueta"):
        n = len(lines())
        keys("enter")
        wait_for("tutorial step: Ahora vamos a agregar", since=n)


def step_isabel():
    n = len(lines())
    keys("ctrl+2")
    time.sleep(2)
    keys("ctrl+2")
    wait_for("Perfecto", since=n)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Haz clic sobre un objeto para que Isabel", since=n)


def step_isabel_object():
    nav("shift+pgdn", "electrodom", 9)
    nav("pgdn", "Nevera", 10)
    n = len(lines())
    keys("enter")
    wait_for("interacciones", since=n)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Hay varias maneras", 60, since=n)


def step_select_first():
    n = len(lines())
    keys("ctrl+1")
    wait_for("tutorial step: Muy bien, ya has seleccionado a Roberto", since=n)
    n = len(lines())
    keys("space")
    wait_for("tutorial step: Bien. Los sims interact", since=n)


def step_social():
    nav("shift+pgup", "Sims", 9)
    nav("pgdn", "Roberto", 6)
    n = len(lines())
    keys("enter")
    wait_for("interacciones", since=n)
    n = len(lines())
    keys("enter")  # the first social (or its sub-level)
    time.sleep(2)
    cur = lines()
    if any("interacciones" in l for l in cur[n:]):  # a sub-level opened
        keys("enter")
    wait_for("tutorial step: Para saber qué sienten", 90, since=n)
    n = len(lines())
    nav("shift+pgup", "Sims", 9)
    nav("pgdn", "Roberto", 6)
    keys("enter")
    wait_for("interacciones", since=n)
    n = len(lines())
    keys("down", "enter")
    time.sleep(2)
    cur = lines()
    if any("interacciones" in l for l in cur[n:]):
        keys("enter")
    wait_for("tutorial step: Si haces clic en el bot", 90, since=n)


def step_relationship():
    nav("tab", "paneles del Sim", 8)
    nav("right", "Relaciones", 8)
    keys("enter")
    time.sleep(2)
    # then the other Sim's card on the relationships panel
    nav("tab", "Relaciones", 8)
    nav("right", "Roberto", 8)
    n = len(lines())
    keys("enter")
    wait_for("Cuando tus sims conocen", since=n)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Los sims ganan dinero", since=n)


def step_newspaper():
    # bring the paper in first, then find a job in it; the offer is accepted
    if not nav("shift+pgdn", "otros objetos", 10):
        nav("shift+pgup", "otros objetos", 10)
    nav("pgdn", "Peri", 25) or nav("pgup", "Peri", 25)
    n = len(lines())
    keys("enter")
    wait_for("interacciones", since=n)
    nav("down", "Recoger", 5) or nav("up", "Recoger", 5)
    keys("enter")
    for _ in range(10):  # until the paper is inside and offers the job search
        time.sleep(6)
        nav("pgup", "Peri", 6) or nav("pgdn", "Peri", 6)
        n = len(lines())
        keys("enter")
        time.sleep(2)
        if nav("down", "empleo", 5) or nav("up", "empleo", 5):
            keys("enter")
            break
        keys("esc")
    wait_for("El único empleo", 120, since=n)
    keys("enter")  # yes
    wait_for("El coche pasará", since=n)
    keys("enter")
    wait_for("tutorial step: Felicidades", since=n)


def step_job():
    nav("tab", "paneles del Sim", 8)
    nav("right", "Empleo", 8)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Este panel muestra", since=n)
    nav("tab", "empleo", 8)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: Las habilidades", since=n)
    nav("right", "Cocina", 8)
    n = len(lines())
    keys("enter")
    wait_for("tutorial step: El modo opciones", since=n)


def step_options():
    nav("tab", "controles", 8)
    nav("right", "Modo Opciones", 25)
    n = len(lines())
    keys("enter")
    wait_for("termina el tutorial", since=n)
    keys("enter")

STEPS = [("reset", step_reset), ("enter", step_enter), ("zoom", step_zoom), ("scroll", step_scroll),
         ("rotate", step_rotate), ("live", step_live), ("portrait", step_portrait), ("gohere", step_gohere),
         ("interact", step_interact), ("shower", step_shower), ("needs", step_needs), ("buy", step_buy),
         ("chair", step_chair), ("place", step_place), ("turn", step_turn),
         ("live2", step_live2), ("personality", step_personality), ("isabel", step_isabel),
         ("isabel_object", step_isabel_object), ("select_first", step_select_first), ("social", step_social),
         ("relationship", step_relationship), ("newspaper", step_newspaper), ("job", step_job), ("options", step_options)]


def main(argv):
    sys.stdout.reconfigure(encoding="utf-8")
    start = argv[0] if argv else "reset"
    names = [n for n, _ in STEPS]
    for name, fn in STEPS[names.index(start):]:
        print(f"== {name}")
        fn()
    print("done")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
