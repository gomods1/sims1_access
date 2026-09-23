// Review buffers (the playbook's section 11): what is spoken stays concise, and everything else
// about it waits in a buffer the player reads line by line without moving focus.
//   Ctrl+Up / Ctrl+Down       next line deeper / back toward the top of the current buffer
//   Ctrl+Left / Ctrl+Right    previous / next buffer (empty ones are skipped)
// Buffers:
//   detalles  the details of what focus last spoke: a catalog product (every rating, the rooms it
//             is meant for, its description by sentences), a placement spot, an object of the lot
//             browser, any control. Replaced on every focus change; the position is kept when the
//             same thing is spoken again.
//   eventos   what happened (purchases, sales, game speed, dialogs, queued actions), newest first.
#pragma once

#include <string>
#include <vector>

namespace nav::buffers {

void Reset();

// Focus spoke about something new: these are its details (the first line usually repeats what was
// spoken). Empty lines are dropped.
void SetDetails(std::vector<std::string> lines);
void ClearDetails();  // the screen changed: old details would describe the wrong thing

// Says an event and keeps it in the events buffer.
void Event(const std::string& text);
// Keeps an event in the buffer without saying it (the narrator's lines below its verbosity).
void Record(const std::string& text);

void Deeper();       // Ctrl+Up
void TowardTop();    // Ctrl+Down
void Switch(int delta);  // Ctrl+Left / Ctrl+Right

// Splits a long text (a description) into sentences, one buffer line each.
std::vector<std::string> Sentences(const std::string& text);

}  // namespace nav::buffers
