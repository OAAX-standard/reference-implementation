@echo off
REM Disable command echoing for cleaner output

setlocal enabledelayedexpansion
REM Enable delayed environment variable expansion

REM Change to script directory
cd /d "%~dp0"

REM Map drive letter to the current directory to avoid long paths
subst T: "%~dp0"

REM Check if the drive letter is mapped
if not exist T:\ (
    echo Failed to map drive letter T:
    exit /b 1
)

REM Go to the mapped drive
T:

REM Go to onnxruntime-1.21.1 directory
cd onnxruntime-1.21.1 || exit /b 1

REM Remove and recreate build directory
if exist build rmdir /s /q build

REM Configure the build with CMake
.\build.bat --config Release --parallel --compile_no_warning_as_error --skip_submodule_sync --skip_tests

REM Go to root directory
cd ..

REM Remove and recreate output directories
if exist X86_64_WINDOWS rmdir /s /q X86_64_WINDOWS
mkdir X86_64_WINDOWS
mkdir X86_64_WINDOWS\include

REM Go to Release build output directory
cd .\onnxruntime-1.21.1\build\Windows\Release

REM Copy artifacts
@echo off
for /R %%f in (*.lib) do (
    REM Copy file to X86_64_WINDOWS
    copy "%%f" "..\..\..\..\X86_64_WINDOWS\"
)
REM Go back to the root of the dependency
cd ..\..\..\
REM Copy include files to output directory
xcopy .\include\onnxruntime ..\X86_64_WINDOWS\include\onnxruntime /E /I