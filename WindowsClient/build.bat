@echo off
echo Locating Visual Studio Environment...
set "vswhere=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%vswhere%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "vsPath=%%i"

if "%vsPath%"=="" (
    echo Could not find Visual Studio with C++ tools installed.
    pause
    exit /b 1
)

call "%vsPath%\VC\Auxiliary\Build\vcvars64.bat"

echo Building C++ Bluetooth Clipboard Client...
cl.exe /EHsc /std:c++17 /D UNICODE /D _UNICODE main.cpp ws2_32.lib user32.lib bthprops.lib

if %ERRORLEVEL% EQU 0 (
    echo Build Successful! Run main.exe to start the Windows Client.
) else (
    echo Build Failed.
)
pause
