# Parameters
export COMPILER=clang++-7
export OUTPUT=main

# Compiler flags
# TODO: add different build configurations (release, debug, etc)
export CFLAGS="
  -std=c++17
  -fno-exceptions -fno-rtti -fopenmp -fopenmp-simd
  -Wall -Wextra -Wpedantic 
  -Wno-missing-braces
  -g -gno-column-info -rdynamic"

export CDEFS="
  -Dmg_Slow=1"

# Compiling
g++-8 build.cpp -o ${OUTPUT} ${CFLAGS} ${CDEFS}
g++-8 test_stacktrace.cpp mg_string.cpp mg_stacktrace_posix.cpp -o "test_trace" ${CFLAGS} ${CDEFS}

