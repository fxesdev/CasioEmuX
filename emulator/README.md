## MSYS2
The `build.bat` was modified to work on MSYS2 MinGW32. You will need to install these packages:
- [`mingw-w64-i686-gcc`](https://packages.msys2.org/package/mingw-w64-i686-gcc)
- [`mingw-w64-i686-lua53`](https://packages.msys2.org/package/mingw-w64-i686-lua53)
- [`mingw-w64-i686-SDL2`](https://packages.msys2.org/package/mingw-w64-i686-SDL2)
- [`mingw-w64-i686-SDL2_image`](https://packages.msys2.org/package/mingw-w64-i686-SDL2_image)
- [`mingw-w64-i686-readline`](https://packages.msys2.org/package/mingw-w64-i686-readline)

You can use a single-line command to install all of them at once:
```shell
pacman -S mingw-w64-i686-gcc mingw-w64-i686-lua53 mingw-w64-i686-SDL2 mingw-w64-i686-SDL2_image mingw-w64-i686-readline
```

Since the build script uses g++, building may take some time.

You can modify `build.bat` as needed to make it work with different environments and compilers.

## Linux
For Ubuntu and similar distros, you will need to install these packages:
```shell
sudo apt install gcc liblua53-dev libsdl2-dev libsdl2image-dev
```

Then just run the `build.sh` script. Since it uses g++, building may take some time.
