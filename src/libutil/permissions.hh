#pragma once

#include "windows.hh"

namespace nix {

struct Identity {
#ifdef _WIN32
    // TODO WINDOWS
#else
    uid_t uid;
    gid_t gid;
#endif
};

inline bool userIsRoot() {
#ifdef _WIN32
    // TODO WINDOWS
    return true;
#else
    return getuid() == 0
#endif
}

inline uid_t currentUser() {
#ifdef _WIN32
    // TODO WINDOWS
    return 0;
#else
    return geteuid();
#endif
}

inline Identity currentIdentity() {
#ifdef _WIN32
    // TODO WINDOWS
    return Identity{};
#else
    return Identity{geteuid(), getegid()};
#endif
}

inline void changeOwner(Path const& path, Identity const& id) {
#ifdef _WIN32
    // TODO WINDOWS
#else
    if (chown(path.c_str(), id.uid, id.gid) == -1)
        throw SysError(format("changing owner of '%1%' to %2%") % path % geteuid());
#endif
}

}
