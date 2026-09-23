#include "log.h"

#include <cstdarg>
#include <cstdio>

namespace util {
namespace {

const SaLoaderApi* g_api = nullptr;
bool g_verbose = false;

void Write(const char* fmt, va_list ap) {
    if (!g_api) return;
    char buf[2048];
    vsnprintf(buf, sizeof buf, fmt, ap);
    g_api->log(buf);
}

}  // namespace

void InitLog(const SaLoaderApi* api, bool verbose) {
    g_api = api;
    g_verbose = verbose;
}

void Log(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    Write(fmt, ap);
    va_end(ap);
}

bool VerboseLogging() { return g_verbose; }

void LogVerbose(const char* fmt, ...) {
    if (!g_verbose) return;
    va_list ap;
    va_start(ap, fmt);
    Write(fmt, ap);
    va_end(ap);
}

}  // namespace util
