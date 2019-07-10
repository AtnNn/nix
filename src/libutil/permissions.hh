#pragma once

#include "windows.hh"

bool userIsRoot() {
#ifdef _WIN32
    // TODO WINDOWS
    return true;
#else
    return getuid() == 0
#endif
}
