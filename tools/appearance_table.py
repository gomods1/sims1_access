"""Generate mod/core/nav/appearance_data.h from the appearance notes.

The notes (tools/appearance/heads.txt, bodies.txt) hold one line per mesh: the mesh without its
skin tone, then feature words, e.g. "c036ma_dave blond medium" or "b002mafit_casual shirt jeans
blue". They were written by looking at create-a-Sim's previews. Each feature word maps to a mod
string (the vocabulary below, English; translations in mod/core/lang/ like every string).

usage: python tools/appearance_table.py            write the table
       python tools/appearance_table.py --strings  print the vocabulary as addstr JSON
       python tools/appearance_table.py --from-index <names.json> <index notes> <heads|bodies>
             append index-numbered notes ("12 black short") as mesh notes
"""

import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
NOTES = {"heads": os.path.join(HERE, "appearance", "heads.txt"), "bodies": os.path.join(HERE, "appearance", "bodies.txt")}
OUT = os.path.join(ROOT, "mod", "core", "nav", "appearance_data.h")

# code -> (string id, English)
HEAD = {
    "black": ("LookHairBlack", "black hair"), "brown": ("LookHairBrown", "brown hair"),
    "blond": ("LookHairBlond", "blond hair"), "red": ("LookHairRed", "red hair"),
    "gray": ("LookHairGray", "gray hair"), "white": ("LookHairWhite", "white hair"),
    "dyed": ("LookHairDyed", "dyed hair"),
    "veryshort": ("LookHairVeryShort", "very short"), "short": ("LookHairShort", "short"),
    "medium": ("LookHairMedium", "medium length"), "long": ("LookHairLong", "long"),
    "spiky": ("LookSpiky", "spiky"), "curly": ("LookCurly", "curly"), "wavy": ("LookWavy", "wavy"),
    "slick": ("LookSlickedBack", "slicked back"), "bob": ("LookBob", "bob cut"), "bun": ("LookBun", "in a bun"),
    "afro": ("LookAfro", "afro"), "mohawk": ("LookMohawk", "mohawk"), "ponytail": ("LookPonytail", "ponytail"),
    "braids": ("LookBraids", "braids"), "pigtails": ("LookPigtails", "pigtails"), "bangs": ("LookBangs", "bangs"),
    "updo": ("LookUpdo", "hair up"),
    "bald": ("LookBald", "bald"), "beard": ("LookBeard", "beard"), "moustache": ("LookMoustache", "moustache"),
    "goatee": ("LookGoatee", "goatee"), "stubble": ("LookStubble", "stubble"), "sideburns": ("LookSideburns", "sideburns"),
    "glasses": ("LookGlasses", "glasses"), "sunglasses": ("LookSunglasses", "sunglasses"), "hat": ("LookHat", "hat"),
    "cap": ("LookCap", "cap"), "helmet": ("LookHelmet", "helmet"), "headscarf": ("LookHeadscarf", "headscarf"),
    "headband": ("LookHeadband", "headband"), "mask": ("LookMask", "mask"), "monster": ("LookMonster", "monster face"),
    "older": ("LookOlder", "older face"), "earrings": ("LookEarrings", "earrings"), "flower": ("LookHairFlower", "flower in the hair"),
}
BODY = {
    "armor": ("LookArmor", "armor"), "uniform": ("LookUniform", "uniform"), "tshirt": ("LookTShirt", "T-shirt"),
    "shirt": ("LookShirt", "shirt"), "tanktop": ("LookTankTop", "tank top"), "sweater": ("LookSweater", "sweater"),
    "hoodie": ("LookHoodie", "hoodie"), "jacket": ("LookJacket", "jacket"), "leatherjacket": ("LookLeatherJacket", "leather jacket"),
    "vest": ("LookVest", "vest"), "coat": ("LookCoat", "long coat"), "suit": ("LookSuit", "suit"), "tuxedo": ("LookTuxedo", "tuxedo"),
    "tie": ("LookTie", "tie"), "trousers": ("LookTrousers", "trousers"), "jeans": ("LookJeans", "jeans"), "shorts": ("LookShorts", "shorts"),
    "overalls": ("LookOveralls", "overalls"), "lederhosen": ("LookLederhosen", "lederhosen"), "kilt": ("LookKilt", "kilt"),
    "robe": ("LookRobe", "robe"), "toga": ("LookToga", "toga"), "kimono": ("LookKimono", "kimono"), "costume": ("LookCostume", "costume"),
    "sportswear": ("LookSportswear", "sports kit"), "apron": ("LookApron", "apron"), "shirtless": ("LookShirtless", "shirtless"),
    "barefoot": ("LookBarefoot", "barefoot"), "sandals": ("LookSandals", "sandals"), "boots": ("LookBoots", "boots"),
    "sneakers": ("LookSneakers", "sneakers"), "heels": ("LookHeels", "high heels"), "tattoos": ("LookTattoos", "tattoos"),
    "leather": ("LookLeather", "leather"), "dress": ("LookDress", "dress"), "longdress": ("LookLongDress", "long dress"),
    "skirt": ("LookSkirt", "skirt"), "blouse": ("LookBlouse", "blouse"), "swimsuit": ("LookSwimsuit", "swimsuit"),
    "bikini": ("LookBikini", "bikini"), "top": ("LookTop", "top"), "leggings": ("LookLeggings", "leggings"),
    "underwear": ("LookUnderwear", "underwear"),
    "black": ("LookColorBlack", "black"), "white": ("LookColorWhite", "white"), "gray": ("LookColorGray", "gray"),
    "red": ("LookColorRed", "red"), "blue": ("LookColorBlue", "blue"), "green": ("LookColorGreen", "green"),
    "yellow": ("LookColorYellow", "yellow"), "orange": ("LookColorOrange", "orange"), "purple": ("LookColorPurple", "purple"),
    "pink": ("LookColorPink", "pink"), "brown": ("LookColorBrown", "brown"), "beige": ("LookColorBeige", "beige"),
    "camouflage": ("LookCamouflage", "camouflage"), "checked": ("LookChecked", "checked"), "floral": ("LookFloral", "floral"),
    "striped": ("LookStriped", "striped"), "patterned": ("LookPatterned", "patterned"),
}
VOCAB = {"heads": HEAD, "bodies": BODY}


def key_of(mesh: str) -> str:
    prefix, _, rest = mesh.partition("_")
    for skin in ("lgt", "med", "drk"):
        if skin in prefix:
            prefix = prefix.replace(skin, "", 1)
            break
    return (prefix + ("_" + rest if rest else "")).lower()


def read_notes(kind):
    out = {}
    if not os.path.exists(NOTES[kind]):
        return out
    for line in open(NOTES[kind], encoding="utf-8"):
        parts = line.split("#")[0].split()
        if not parts:
            continue
        key, words = parts[0], parts[1:]
        for w in words:
            if w not in VOCAB[kind]:
                sys.exit(f"{kind}: unknown word {w!r} for {key}")
        out.setdefault(key, words)
    return out


def main(argv):
    if argv[:1] == ["--strings"]:
        seen = {}
        for kind in ("heads", "bodies"):
            for sid, en in VOCAB[kind].values():
                seen.setdefault(sid, en)
        print(json.dumps({"group": "Create-a-Sim appearance", "strings": [[k, v] for k, v in seen.items()]}, ensure_ascii=False))
        return 0
    if argv[:1] == ["--from-index"]:
        names = json.load(open(argv[1], encoding="utf-8"))[argv[3]]
        known = read_notes(argv[3])
        os.makedirs(os.path.dirname(NOTES[argv[3]]), exist_ok=True)
        with open(NOTES[argv[3]], "a", encoding="utf-8") as f:
            for line in open(argv[2], encoding="utf-8"):
                parts = line.split()
                if not parts:
                    continue
                key = key_of(names[int(parts[0])])
                if key in known:
                    continue
                known[key] = parts[1:]
                f.write(key + " " + " ".join(parts[1:]) + "\n")
        return 0
    lines = ["// Generated by tools/appearance_table.py from tools/appearance/*.txt: do not edit.",
             "const Entry kEntries[] = {"]
    for kind in ("heads", "bodies"):
        for key, words in sorted(read_notes(kind).items()):
            ids = ", ".join("Str::" + VOCAB[kind][w][0] for w in words)
            lines.append(f'    {{"{key}", {{{ids}}}}},')
    lines.append("};")
    open(OUT, "w", encoding="utf-8", newline="\n").write("\n".join(lines) + "\n")
    print("wrote", OUT)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
