#pragma once

// TODO WINDOWS
// - move implementation to cc
// - redesign to minimise the amount of ifdefs

#include "types.hh"
#include "util.hh"

#include <cinttypes>

namespace nix {

class FileInfo {
public:
  FileInfo() {}

  FileType type() const {
#ifdef _WIN32
    // TODO WINDOWS: FileType::other, FileType::missing
    return data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT
               ? FileType::symlink
               : data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                     ? FileType::directory
                     : FileType::regular;
#else
    return S_ISREG(st.st_mode)
               ? FileType::regular
               : S_ISDIR(st.st_mode)
                     ? FileType::directory
                     : S_ISLNK(st.st_mode) ? FileType::symlink
                                           : st.st_mode == 0 ? FileType::missing
                                                             : FileType::other;
#endif
  }

  uint64_t size() const {
#ifdef _WIN32
    return 0; // TODO WINDOWS https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfilesize
#else
    return st.st_size;
#endif
  }

  bool is_symlink() const { return type() == FileType::symlink; }

  bool is_directory() const { return type() == FileType::directory; }

  bool is_regular() const { return type() == FileType::regular; }

  bool is_missing() const { return type() == FileType::missing; }

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
    return false; // TODO WINDOWS: perhaps check for .exe or .bat
#else
    return st.st_mode & S_IXUSR;
#endif
  }

    time_t modification_time() const {
#ifdef _WIN32
        // TODO WINDOWS
        return 0;
#else
        return st.st_mtime;
#endif
    }

    time_t accessTime() const {
#ifdef _WIN32
        // TODO WINDOWS
        return 0;
#else
        return st.st_atime;
#endif
    }

    Inode uniqueId() const {
        throw "TODO WINDOWS";
    }

  uint64_t size_on_disk() const {
#ifdef _WIN32
    // TODO WINDOWS https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getfileinformationbyhandleex
    return size();
#else
    return st.st_blocks * 512;
#endif
  }

  int hardlink_count() const {
#ifdef _WIN32
    // TODO WINDOWS https://docs.microsoft.com/en-us/windows/win32/api/fileapi/ns-fileapi-_by_handle_file_information
    return 1;
#else
    return st.st_nlink;
#endif
  }
    
private:
    friend FileInfo lstat(Path const &path, bool allowMissing);
    friend FileInfo stat(Path const &path);

#ifdef _WIN32
  WIN32_FILE_ATTRIBUTE_DATA data{};
#else
  struct stat st {};
#endif
};

#ifdef _WIN32
inline FileInfo lstat(Path const &path, bool allowMissing = false) {
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
inline FileInfo lstat(Path const &path, bool allowMissing = false) {
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

inline Path readlink(Path const& link) {
    // TODO WINDOWS
    return "";
}

#if _WIN32
inline FileInfo stat(Path const &path) {
    FileInfo ret = lstat(path);
    if (!ret.is_symlink()) return ret;

    HANDLE handle = CreateFileA(
        path.c_str(),
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        0,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        0);
    // TODO WINDOWS: close handle safely
    std::vector<char> finalPath;
    DWORD size = GetFinalPathNameByHandleA(handle, nullptr, 0, FILE_NAME_NORMALIZED);
    if (size) {
        finalPath.resize(size);
        size = GetFinalPathNameByHandleA(handle, finalPath.data(), size - 1, FILE_NAME_NORMALIZED);
        if (size != 0) {
            if (size != finalPath.size() - 1) {
                throw SysError("system returned inconsistent path size");
            }
            finalPath[size-1] = 0;
        }
    }
    if (size == 0) {
        throw SysError(format("GetFinalPathNameByHandleA failed: %1%") % GetLastError());
    }
    return lstat(finalPath.data());
}
#else
inline FileInfo stat(Path const &path) {
  FileInfo ret;
  if (stat(path.c_str(), &ret.st)) {
    throw SysError(format("getting status of '%1%'") % path);
  }
  return ret;
}
#endif

}
