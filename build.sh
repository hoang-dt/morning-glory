# Parameters
export COMPILER=g++-8
export OUTPUT=bin/main

# Compiler flags
export CFLAGS="Please-provide-a-build-configuration:-Debug/FastDebug/Release"

if [ "$1" == "Debug" ]
  then
    export CFLAGS="
      -std=gnu++2a
      -fno-exceptions -fno-rtti -fopenmp -fopenmp-simd
      -Wall -Wextra -Wpedantic
      -Wno-missing-braces
      -g -rdynamic -O0"
    export CDEFS="-Dmg_Slow=1"
fi

if [ "$1" == "FastDebug" ]
  then
    export CFLAGS="
      -std=gnu++2a
      -fno-exceptions -fno-rtti -fopenmp -fopenmp-simd
      -Wall -Wextra -Wpedantic
      -Wno-missing-braces
      -g -rdynamic -Og -DNDEBUG -ftree-vectorize -march=native"
fi

if [ "$1" == "Release" ]
  then
    export CFLAGS="
      -std=gnu++2a
      -fno-exceptions -fno-rtti -fopenmp -fopenmp-simd
      -Wall -Wextra -Wpedantic
      -Wno-missing-braces
      -g -gno-column-info -rdynamic -O2 -DNDEBUG -ftree-vectorize -march=native"
fi

# Compiling
${COMPILER} src/build.cpp -o ${OUTPUT} ${CFLAGS} ${CDEFS}

