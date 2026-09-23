#include "buffers.h"

#include "../speech/speech.h"
#include "../util/strings.h"
#include "../util/text.h"

namespace nav::buffers {
namespace {

constexpr size_t kMaxEvents = 200;

struct Buffer {
    Str name;
    std::vector<std::string> items;  // review order: index 0 is the top (first detail, newest event)
    int cursor = -1;                 // -1: before the top, so the first Ctrl+Up reads the top item
    bool followLatest = false;       // coming back to it starts again at the newest item
};

Buffer g_buffers[] = {
    {Str::BufferDetails, {}, -1, false},
    {Str::BufferEvents, {}, -1, true},
};
constexpr int kCount = sizeof(g_buffers) / sizeof(g_buffers[0]);
Buffer& Details() { return g_buffers[0]; }
Buffer& Events() { return g_buffers[1]; }
int g_current = 0;

Buffer& Current() { return g_buffers[g_current]; }

std::string Summary(const Buffer& b) {
    const char* name = S(b.name);
    return b.items.size() == 1 ? util::Format(S(Str::GroupSummaryOne), name)
                               : util::Format(S(Str::GroupSummary), name, (int)b.items.size());
}

// Moves the cursor and reads the item; at either end the item repeats (no wrap).
void Step(int delta) {
    Buffer& b = Current();
    if (b.items.empty()) {
        speech::Say(util::Format(S(Str::BufferEmpty), S(b.name)));
        return;
    }
    int last = (int)b.items.size() - 1;
    int next = b.cursor + delta;
    if (next < 0) next = 0;
    if (next > last) next = last;
    b.cursor = next;
    speech::Say(b.items[next]);
}

}  // namespace

void Reset() {
    for (Buffer& b : g_buffers) {
        b.items.clear();
        b.cursor = -1;
    }
    g_current = 0;
}

void SetDetails(std::vector<std::string> lines) {
    std::vector<std::string> clean;
    for (std::string& line : lines) {
        std::string text = util::CleanForSpeech(line);
        if (!text.empty()) clean.push_back(std::move(text));
    }
    Buffer& b = Details();
    if (clean == b.items) return;  // the same thing again: the player keeps their place
    b.items = std::move(clean);
    b.cursor = -1;
}

void ClearDetails() {
    Details().items.clear();
    Details().cursor = -1;
}

void Event(const std::string& text) {
    speech::Say(text);
    Record(text);
}

void Record(const std::string& text) {
    std::string line = util::CleanForSpeech(text);
    if (line.empty()) return;
    Buffer& b = Events();
    b.items.insert(b.items.begin(), line);
    if (b.cursor >= 0) b.cursor++;  // a player reviewing stays on the same event
    if (b.items.size() > kMaxEvents) {
        b.items.resize(kMaxEvents);
        if (b.cursor >= (int)kMaxEvents) b.cursor = (int)kMaxEvents - 1;
    }
}

void Deeper() { Step(+1); }

void TowardTop() { Step(-1); }

void Switch(int delta) {
    for (int step = 1; step <= kCount; step++) {
        int next = ((g_current + delta * step) % kCount + kCount) % kCount;
        if (g_buffers[next].items.empty()) continue;
        g_current = next;
        if (Current().followLatest) Current().cursor = -1;
        speech::Say(Summary(Current()));
        return;
    }
    speech::Say(util::Format(S(Str::BufferEmpty), S(Current().name)));
}

std::vector<std::string> Sentences(const std::string& text) {
    std::vector<std::string> out;
    std::string clean = util::CleanForSpeech(text);
    size_t start = 0;
    for (size_t i = 0; i < clean.size(); i++) {
        char c = clean[i];
        bool end = (c == '.' || c == '!' || c == '?') && (i + 1 == clean.size() || clean[i + 1] == ' ');
        if (!end) continue;
        std::string sentence = clean.substr(start, i + 1 - start);
        size_t first = sentence.find_first_not_of(' ');
        if (first != std::string::npos) out.push_back(sentence.substr(first));
        start = i + 1;
    }
    if (start < clean.size()) {
        std::string rest = clean.substr(start);
        size_t first = rest.find_first_not_of(' ');
        if (first != std::string::npos) out.push_back(rest.substr(first));
    }
    return out;
}

}  // namespace nav::buffers
