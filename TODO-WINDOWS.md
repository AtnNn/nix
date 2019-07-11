# Native Windows Port

TODO WINDOWS:

- Symbolic links
- Inodes and hard links
- File locking
- Users and permissions
- Processes and pid
- Networking
- Unicode paths
- Translate hardcoded paths
- Runtime roots
- Add mingw64 to release.nix
- Update documentation

# Building with MSYS2

```
pacman -S git mingw-w64-x86_64-toolchain base-devel autoconf-archive
```

```bash
sh booststrap.sh
./configure --disable-doc-gen EDITLINE_LIBS=wineditline
make -j `nproc`
```
