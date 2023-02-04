@echo off

  mkdir _build_release_clang
  cd _build_release_clang
  cmake -A x64 -DYOURGAME_PLATFORM=desktop -DCMAKE_BUILD_TYPE=RELEASE -TLLVM-MSVC_v143 -DCPACK_GENERATOR="ZIP" ../..
  cd ..



