// A room's layout by ear (Ctrl+L): what a sighted player takes in at a glance. The room of the
// build cursor, or else of the selected Sim: its size and where it lies, its doors and windows
// by wall and what they open onto, what stands along each wall and in the middle. The first
// lines are spoken; everything goes to the details buffer (Ctrl+Up/Down).
#pragma once

namespace nav::layout {

void SayRoomLayout();

}  // namespace nav::layout
