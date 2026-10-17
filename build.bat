@echo off
rem Builds Windows License Manager (x64 and x86) with MSVC.
rem Requirements: Visual Studio or "Build Tools for Visual Studio" with the
rem "Desktop development with C++" workload and a Windows SDK.
setlocal
cd /d "%~dp0"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo vswhere.exe not found. Install Visual Studio / Build Tools with the C++ workload.
    exit /b 1
)
set "VSROOT="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%i"
if not defined VSROOT (
    echo No Visual Studio installation with the MSVC x86/x64 tools was found.
    exit /b 1
)
set "VC=%VSROOT%\VC\Auxiliary\Build"

set OPTS=/nologo /EHsc /O2 /W3 /utf-8 /MT /I src
set LNK=/link /SUBSYSTEM:WINDOWS /MANIFEST:EMBED /MANIFESTUAC:NO /MANIFESTINPUT:res\app.manifest

if not exist build\x64 mkdir build\x64
if not exist build\x86 mkdir build\x86

echo === x64 ===
call "%VC%\vcvars64.bat" >nul || exit /b 1
rc /nologo /c65001 /fo build\x64\app.res res\app.rc || exit /b 1
cl %OPTS% src\main.cpp build\x64\app.res /Fo:build\x64\ /Fe:build\x64\WindowsLicenseManager.exe %LNK% || exit /b 1

echo === x86 ===
call "%VC%\vcvars32.bat" >nul || exit /b 1
rc /nologo /c65001 /fo build\x86\app.res res\app.rc || exit /b 1
cl %OPTS% src\main.cpp build\x86\app.res /Fo:build\x86\ /Fe:build\x86\WindowsLicenseManager_x86.exe %LNK% || exit /b 1

echo.
echo Done:
echo   build\x64\WindowsLicenseManager.exe      (64-bit Windows 10/11)
echo   build\x86\WindowsLicenseManager_x86.exe  (32-bit Windows 10)
