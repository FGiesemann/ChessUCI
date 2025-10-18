@echo off
setlocal enableDelayedExpansion

call D:\Programmierung\Projekte\dev-venv\Scripts\activate.bat

set "COMMON_CONAN_ARGS=-s compiler.cppstd=23 --build=missing"

set "CONAN_UPDATE_FLAG="
if /I "%1" == "-u" (
    set "CONAN_UPDATE_FLAG=--update"
)

conan install . -pr:b msvc194 -pr:h msvc194 -s build_type=Debug %COMMON_CONAN_ARGS% %CONAN_UPDATE_FLAG%
conan install . -pr:b msvc194 -pr:h msvc194 -s build_type=Release %COMMON_CONAN_ARGS% %CONAN_UPDATE_FLAG%
conan install . -pr clang-VS %COMMON_CONAN_ARGS% %CONAN_UPDATE_FLAG%

endlocal
