This program uses the zfp compression library to compress a data set using the
fixed-rate mode.

Get a recent version of CMake, GCC-8 or Clang-8, and (optionally) Ninja.

To compile:
- Get zfp (https://github.com/LLNL/zfp). Build and INSTALL using CMake.
- Get stlab (https://github.com/stlab/libraries). Build and INSTALL using CMake.
(The "build" actually does no compilation as stlab is header-only, but the INSTALL
step will generate a CMake file we need to use).
- Get Boost (https://www.boost.org/users/download/).

Look into cmake-command.bat for how to build on Windows. I have not tried
building on Linux, but it should roughly be the same.