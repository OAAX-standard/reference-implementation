@echo off
setlocal enabledelayedexpansion

REM Change to script directory
cd /d %~dp0

set "BUILD_DIR=%cd%\build"
set "ARTIFACTS_DIR=%cd%\artifacts"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if exist "%ARTIFACTS_DIR%" rmdir /s /q "%ARTIFACTS_DIR%"
mkdir "%ARTIFACTS_DIR%"

pushd "%BUILD_DIR%"
del /q * >nul 2>&1
REM Generate the build files
cmake ..
if errorlevel 1 exit /b 1
REM Make the build
cmake --build . --config Release
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

popd

endlocal
