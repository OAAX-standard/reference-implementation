@echo off
setlocal enabledelayedexpansion

REM Change to script directory
cd /d "%~dp0"

REM Go to onnxruntime-1.17.3 directory
cd onnxruntime-1.21.1 || exit /b 1

REM Remove and recreate build directory
if exist build rmdir /s /q build

REM Configure the build with CMake
.\build.bat --config Release --parallel --compile_no_warning_as_error --skip_submodule_sync

REM Go to root directory
cd ..

REM Remove and recreate output directories
if exist X86_64_WINDOWS rmdir /s /q X86_64_WINDOWS
mkdir X86_64_WINDOWS
mkdir X86_64_WINDOWS\include

cd .\onnxruntime-1.21.1\build\Windows\Release
REM Copy artifacts
@echo off
for /R %%f in (*.lib) do (
    REM copy file to X86_64_WINDOWS
    copy "%%f" "..\..\..\..\X86_64_WINDOWS\"
)
cd ..\..\..\
xcopy .\include\onnxruntime ..\X86_64_WINDOWS\include\onnxruntime /E /I