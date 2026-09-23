// The narrator's watch over the family (part of nav/narrator): what a sighted player sees the
// Sims do without asking, spoken as it changes.
//   Rooms: the selected Sim's room, once the Sim has stayed there ("Isabel, ahora en cocina");
//     rooms only crossed on the way are not spoken.
//   Actions: the selected Sim's running interaction starting ("Isabel: Comer"; "Isabel, por su
//     cuenta: Comer" when free will chose it) and one the player chose finishing with nothing
//     after it ("Isabel terminó: Comer").
//   Needs: a need falling low (the selected Sim) or to urgent (anyone in the family), once per
//     crossing; the mood meter turning red. Recoveries only go to the events buffer.
// Switching the selected Sim or the lot starts from what is there, so nothing stale is said.
#pragma once

#include <string>

namespace nav::simwatch {

void Reset();
void Tick();

// The selected Sim's state in one line, for the status key: "Isabel, en cocina, Comer, luego
// Ducharse; necesidades bajas: hambre; humor, 2 de 5 barras en verde".
std::string Status();

}  // namespace nav::simwatch
