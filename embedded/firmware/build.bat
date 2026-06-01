@echo off
echo [FogGuard] Checking cmake file...

findstr /c:"icm42688p.c" cmake_gcc\icc-26-fogguard.cmake >nul
if errorlevel 1 (
    powershell -Command "(gc 'cmake_gcc\icc-26-fogguard.cmake') -replace '\"../app_bm.c\"', '\"../app_bm.c\"\n    \"../icm42688p.c\"\n    \"../ldma_buffer.c\"\n    \"../sliding_window.c\"\n    \"../gait_fsm.c\"\n    \"../feedback.c\"' | sc 'cmake_gcc\icc-26-fogguard.cmake'"
    echo [FogGuard] Custom source files added
) else (
    echo [FogGuard] Source files already present
)

echo [FogGuard] Building...
cd cmake_gcc
cmake --preset project
cmake --build build
cd ..

if errorlevel 0 (
    echo [FogGuard] BUILD SUCCESSFUL
) else (
    echo [FogGuard] BUILD FAILED
)