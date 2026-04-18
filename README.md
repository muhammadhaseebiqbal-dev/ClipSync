# ClipSync

ClipSync is a cross-platform clipboard synchronization ecosystem, currently supporting Windows and Android. It allows you to seamlessly share your clipboard across your devices.

## Installation

### Windows Client

The Windows client runs invisibly in the background and automatically starts when you turn on your PC.

**To install:**
1. Open **PowerShell** as an **Administrator**.
2. Run the following command to download and execute the installation script directly from this repository:

```powershell
irm https://raw.githubusercontent.com/muhammadhaseebiqbal-dev/ClipSync/master/install.ps1 | iex
```

This script will:
- Create an installation directory in your local app data.
- Download the built `main.exe` client.
- Configure Windows Auto-Startup so the client runs automatically.
- Start the clipboard service in the background.

### Android Client

The Android client code is located in the `AndroidClient` directory. You can open this folder in **Android Studio** to build and install the APK on your Android device.

## Development

- **Windows Client**: Written in C++. You can use the `build.bat` script provided in the `WindowsClient` folder to compile `main.cpp` (requires MSVC or MinGW). Ensure that `main.exe` is pushed to the repository for the install script to fetch it.
- **Android Client**: Developed using Android Studio and Gradle.
