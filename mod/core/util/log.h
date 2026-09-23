// Logging to SimsAccess\SimsAccess.log through the loader (which owns the file across reloads).
#pragma once

#include "../../loader/loader_api.h"

namespace util {

void InitLog(const SaLoaderApi* api, bool verbose);

// printf-style; one line per call.
void Log(const char* fmt, ...);

// Only written in development (SimsAccess\dev.txt present): key traces and similar noise.
void LogVerbose(const char* fmt, ...);
bool VerboseLogging();

}  // namespace util

using util::Log;
using util::LogVerbose;
