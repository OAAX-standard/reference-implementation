@echo off
setlocal enabledelayedexpansion

REM Change to script directory
cd /d %~dp0

set "BUILD_DIR=%cd%\build"
set "ARTIFACTS_DIR=%cd%\artifacts"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if exist "%ARTIFACTS_DIR%" rmdir /s /q "%ARTIFACTS_DIR%"
mkdir "%ARTIFACTS_DIR%"

REM Read platform from command line argument
if "%~1"=="" (
    set PLATFORMS=X86_64 AARCH64
    echo No platform specified. Building for both X86_64 and AARCH64.
    echo You can specify a platform by running: build-runtimes.bat ^<X86_64^|AARCH64^>
    timeout /t 1 >nul
) else (
    if /I not "%~1"=="X86_64" if /I not "%~1"=="AARCH64" (
        echo Invalid platform specified. Use X86_64 or AARCH64.
        exit /b 1
    )
    echo Platform specified: %~1
    set PLATFORMS=%~1
)

pushd "%BUILD_DIR%"
for %%P in (%PLATFORMS%) do (
    echo ---- Building for platform: %%P
    del /q * >nul 2>&1
    REM Generate the build files
    cmake .. -DPLATFORM=%%P
    if errorlevel 1 exit /b 1
    REM Make the build
    cmake --build . --config Debug
    if errorlevel 1 exit /b 1
    echo Build complete. The following shared libraries were created:
    dir /b *.so
    REM Copy the shared libraries to the artifacts directory
    echo Copying shared libraries to artifacts directory...
    if not exist "%ARTIFACTS_DIR%\%%P" mkdir "%ARTIFACTS_DIR%\%%P"
    copy *.so "%ARTIFACTS_DIR%\%%P\"
    REM Bundle the shared libraries into a tarball
    tar czf "%ARTIFACTS_DIR%\runtime-library-%%P.tar.gz" -C "%ARTIFACTS_DIR%\%%P" *.so
    echo Shared libraries for %%P have been copied to "%ARTIFACTS_DIR%\%%P\"
)
popd

endlocal
