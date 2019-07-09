#pragma once

#include "types.hh"

#include <cinttypes>

namespace nix {

enum class FileType { missing, regular, directory, symlink, other };

class FileInfo {
public:
    FileInfo() {}

  FileType type() const {
#ifdef _WIN32
      // TODO WINDOWS: FileType::other, FileType::missing
      return data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ? FileType::symlink
          : data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? FileType::directory
          : FileType::regular;
#else
    return S_ISREG(st.st_mode) ? FileType::regular
        : S_ISDIR(st.st_mode) ? FileType::directory
        : S_ISLNK(st.st_mode) ? FileType::symlink
        : st.st_mode == 0 ? FileType::missing
        : FileType::other;
#endif
  }

  uint64_t size() const {
#ifdef _WIN32
      return 0; // TODO WINDOWS
#else
    return st.st_size;
#endif
  }

    bool is_symlink() const {
        return type() == FileType::symlink;
    }

    bool is_directory() const {
        return type() == FileType::directory;
    }

    bool is_regular() const {
        return type() == FileType::regular;
    }

    bool is_missing() const {
        return type() == FileType::missing;
    }

    int mode() const {
#ifdef _WIN32
        return 0; // TODO WINDOWS
#else
        return st.st_mode;
#endif
    }

    
    uid_t owner() const {
#ifdef _WIN32
        return 0; // TODO WINDOWS
#else
        return st.st_uid;
#endif
    }

    bool is_executable() const {
#ifdef _WIN32
        return false; // TODO WINDOWS
#else
        return st.st_mode & S_IXUSR;
#endif
    }

    uint64_t hardlink_count() const {
#ifdef _WIN32
        // TODO WINDOWS
        return size();
#else
        return st.st_blocks * 512;
#endif
    }

    int hardlink_count() const {
#ifdef _WIN32
        // TODO WINDOWS
        return 1;
#else
        return st.st_nlink;
#endif
    }

private:
    friend FileInfo lstat(Path const& path, bool allowMissing);

#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA data{};
#else
    struct stat st{};
#endif
};

#ifdef _WIN32
inline FileInfo lstat(Path const& path, bool allowMissing = false) {
    FileInfo ret;
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &ret.data)) {
            if (allowMissing) {
                // TODO WINDOWS
            }
            throw SysError(format("GetFileAttributesA failed: %1%") % GetLastError());
    }
    return ret;
}
#else
inline FileInfo lstat(Path const& path, bool allowMissing = false) {
    FileInfo ret;
    if (lstat(path.c_str(), &ret.st)) {
        if (allowMissing && (errno == ENOENT || errno == ENOTDIR)) {
            return FileInfo();
      }
      throw SysError(format("getting status of '%1%'") % path);
    }
    return ret;
  }
#endif
