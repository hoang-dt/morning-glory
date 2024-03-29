def Settings( **kwargs ):
  return {
    'flags': [ '-Xclang', '-flto-visibility-public-std', '-std=gnu++2a', '-fdiagnostics-absolute-paths', '-fno-exceptions', '-fno-rtti', '-fopenmp', '-fopenmp-simd', '-Wall', '-Wextra', '-pedantic', '-Wno-gnu-zero-variadic-macro-arguments', '-Wno-nested-anon-types', '-Wno-gnu-anonymous-struct', '-Wno-missing-braces', '-g', '-gcodeview', '-gno-column-info', '-O0' ]
  }