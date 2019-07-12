#pragma once

// TODO WINDOWS: this file is badly named and should be split

#if _WIN32



// Limit the amount of transitive Windows headers
#define WIN32_LEAN_AND_MEAN

// Don't leak these top-level typedefs used by other headers
#define INT INT_
#define FLOAT FLOAT_

#include <Windows.h>

#undef INT
#undef FLOAT

// Don't leak macros used by other headers
#undef IN

#include <sys/types.h>
#include <sys/stat.h>

#include <cstdlib>
#include <io.h>

#include <boost/format.hpp>

#undef stat

using uid_t = int;
using gid_t = int;

const int O_CLOEXEC = 0;


inline unsetenv(char const* name) {
    if (!SetEnvironmentVariableA(name, nullptr)) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

inline std::string get_current_user_name() {
    DWORD size;
    if (GetUserNameA(nullptr, &size)) {
        std::vector<char> ret;
        ret.resize(size);
        if (GetUserNameA(ret.data(), &size)) {
            return std::string(ret.begin(), ret.end() - 1);
        }
    }
    throw nix::SysError(boost::format("GetUserNameA error: %1%") % GetLastError());
}

inline int setenv(char const* name, char const* val, int overwrite) {
    if (!overwrite) {
        bool res = GetEnvironmentVariableA(name, nullptr, 0);
        if (!res && GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
            return 0;
        }
    }
    if (!SetEnvironmentVariableA(name, val)) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

inline int mkdir(char const* path, mode_t) {
    // TODO WINDOWS: mkdir mode
    if (!CreateDirectoryA(path, nullptr)) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

inline int futimens(int fd, std::nullptr_t) {
    SYSTEMTIME stime;
    GetSystemTime(&stime);
    FILETIME ftime;
    if (!SystemTimeToFileTime(&stime, &ftime)) {
        errno = EINVAL;
        return -1;
    }
    HANDLE handle = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
    if (!SetFileTime(handle, nullptr, &ftime, &ftime)) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

inline void* localtime_r(time_t const* in, tm* out) {
    errno_t res = localtime_s(out, in);
    if (!res) {
        errno = res;
        return nullptr;
    }
    return out;
}

#else

#include <string>

inline std::string get_current_user_name() {
    auto pw = getpwuid(geteuid());
    return pw ? pw->pw_name : getEnv("USER", "");
}

#endif

#ifndef _WIN32
#include <sys/resource.h>
#endif

inline float getCPUTime() {
#ifdef _WIN32
    FILETIME utime;
    GetProcessTimes(GetCurrentProcess(), nullptr, nullptr, nullptr, &utime);
    ULARGE_INTEGER time;
    time.LowPart = utime.dwLowDateTime;
    time.HighPart = utime.dwHighDateTime;
    return time.QuadPart / (double)10000000.0; // TODO WINDOWS: test
#else
    struct rusage buf;
    getrusage(RUSAGE_SELF, &buf);
    return buf.ru_utime.tv_sec + ((float) buf.ru_utime.tv_usec / 1000000);
#endif
}
