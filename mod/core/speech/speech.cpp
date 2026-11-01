#include "speech.h"

#include <windows.h>

#include <cstdio>
#include <deque>
#include <mutex>

#include "../util/log.h"
#include "../util/text.h"

namespace speech {
namespace {

std::wstring g_modDir;
std::mutex g_lock;
std::deque<std::string> g_queue;
HANDLE g_wake = nullptr;
HANDLE g_thread = nullptr;
volatile bool g_running = false;
HANDLE g_pipe = INVALID_HANDLE_VALUE;
ULONGLONG g_lastLaunch = 0;

std::wstring PipeName() {
    return L"\\\\.\\pipe\\SimsAccessSpeech-" + std::to_wstring(GetCurrentProcessId());
}

void LaunchHost() {
    ULONGLONG now = GetTickCount64();
    if (g_lastLaunch && now - g_lastLaunch < 5000) return;  // rate-limited restarts
    g_lastLaunch = now;
    std::wstring exe = g_modDir + L"\\SpeechHost.exe";
    std::wstring cmd = L"\"" + exe + L"\" --pid " + std::to_wstring(GetCurrentProcessId());
    STARTUPINFOW si = {sizeof si};
    PROCESS_INFORMATION pi = {};
    if (CreateProcessW(exe.c_str(), cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr,
                       g_modDir.c_str(), &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        Log("started SpeechHost (pid %lu)", pi.dwProcessId);
    } else {
        Log("cannot start %ls (error %lu)", exe.c_str(), GetLastError());
    }
}

// Connects to the host, starting it when no pipe exists. Returns false if not connected yet.
bool Connect() {
    if (g_pipe != INVALID_HANDLE_VALUE) return true;
    std::wstring name = PipeName();
    for (int attempt = 0; attempt < 2; attempt++) {
        if (WaitNamedPipeW(name.c_str(), 500)) {
            g_pipe = CreateFileW(name.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (g_pipe != INVALID_HANDLE_VALUE) return true;
        }
        if (GetLastError() == ERROR_FILE_NOT_FOUND || attempt == 0) {
            LaunchHost();
            for (int i = 0; i < 30 && !WaitNamedPipeW(name.c_str(), 100); i++) Sleep(100);
        }
    }
    return false;
}

bool WriteLine(const std::string& line) {
    if (!Connect()) return false;
    DWORD written = 0;
    if (!WriteFile(g_pipe, line.data(), (DWORD)line.size(), &written, nullptr)) {
        CloseHandle(g_pipe);
        g_pipe = INVALID_HANDLE_VALUE;
        return false;
    }
    return true;
}

DWORD WINAPI Worker(void*) {
    while (g_running) {
        WaitForSingleObject(g_wake, 250);
        for (;;) {
            std::string line;
            {
                std::lock_guard<std::mutex> lk(g_lock);
                if (g_queue.empty()) break;
                line = g_queue.front();
            }
            if (!WriteLine(line) && !WriteLine(line)) {
                Sleep(500);  // host missing; keep the line and retry later
                if (!g_running) break;
                continue;
            }
            std::lock_guard<std::mutex> lk(g_lock);
            if (!g_queue.empty()) g_queue.pop_front();
        }
    }
    if (g_pipe != INVALID_HANDLE_VALUE) CloseHandle(g_pipe);
    g_pipe = INVALID_HANDLE_VALUE;
    return 0;
}

void Enqueue(std::string line) {
    {
        std::lock_guard<std::mutex> lk(g_lock);
        if (g_queue.size() > 200) g_queue.pop_front();
        g_queue.push_back(std::move(line));
    }
    if (g_wake) SetEvent(g_wake);
}

}  // namespace

void Start(const wchar_t* modDir) {
    if (g_running) return;
    g_modDir = modDir;
    g_running = true;
    g_wake = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_thread = CreateThread(nullptr, 0, Worker, nullptr, 0, nullptr);
}

void Stop() {
    if (!g_running) return;
    g_running = false;
    SetEvent(g_wake);
    WaitForSingleObject(g_thread, 3000);
    CloseHandle(g_thread);
    CloseHandle(g_wake);
    g_thread = g_wake = nullptr;
}

void Say(const std::string& utf8, bool interrupt) {
    std::string text = util::CleanForSpeech(utf8);
    if (text.empty()) return;
    Log("Spoke: %s", text.c_str());
    Enqueue(std::string("say ") + (interrupt ? "1 " : "0 ") + text + "\n");
}

void Silence() { Enqueue("stop\n"); }

void Cue(const char* name, float pan, float pitch, float volume, int delayMs) {
    char line[128];
    snprintf(line, sizeof line, "play %s %.2f %.2f %.2f %d", name, pan, pitch, volume, delayMs);
    LogVerbose("Cue: %s", line);
    Enqueue(std::string(line) + "\n");
}

}  // namespace speech
