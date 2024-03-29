set "LLVMPath=C:\Program Files\LLVM"
set "VSPath=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
set "VSVersion=14.20.27508"
set "WinSDKVersion=10.0.17763.0"
set "WinSDKPath=C:\Program Files (x86)\Windows Kits\10"
set "VSBasePath=%VSPath%\VC\Tools\MSVC\%VSVersion%"
set "OLD_PATH=%PATH%"
set "PATH=%LLVMPath%\bin;%VSBasePath%\bin\Hostx64\x64;%PATH%"

cmake -H. -Bbin -G Ninja -DCMAKE_BUILD_TYPE=Release^
  -DCMAKE_C_COMPILER="C:/Program Files/LLVM/bin/clang-cl.exe"^
  -DCMAKE_CXX_COMPILER="C:/Program Files/LLVM/bin/clang-cl.exe"^
  -DCMAKE_LINKER:PATH="C:/Program Files/LLVM/bin/lld-link.exe"^
  -Dzfp_DIR="E:/Libraries/zfp/zfp/build/install/lib/cmake/zfp"^
  -Dstlab_DIR="E:/Libraries/stlab/libraries/build/install/share/cmake/stlab"^
  -DBOOST_ROOT="E:/Libraries/Boost"^
  -DVsBasePath="%VSBasePath%\lib\x64"^
  -DWinSdkCrtPath="%WinSDKPath%\Lib\%WinSDKVersion%\ucrt\x64"^
  -DWinSdkMPath="%WinSDKPath%\Lib\%WinSDKVersion%\um\x64"^
  -DLlvmPath="%LLVMPath%\lib"
