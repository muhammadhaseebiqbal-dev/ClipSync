# ClipSync

ClipSync is a cross-platform clipboard synchronization project for **Windows** and **Android**.  
It syncs text clipboard content between your PC and phone over Bluetooth using the Serial Port Profile (SPP).

## What it does

- Monitors clipboard changes on Windows and Android.
- Sends new clipboard text from Windows to Android.
- Sends new clipboard text from Android to Windows.
- Runs as a background process on both platforms.
- Reconnects automatically on Android when connection drops.

## Project structure

- `/WindowsClient`
  - Native C++ Bluetooth clipboard server/client (`main.cpp`)
  - Build script (`build.bat`)
  - Prebuilt binary used by installer (`main.exe`)
- `/AndroidClient`
  - Android app (Kotlin + Gradle)
  - Foreground service for persistent Bluetooth sync
- `/install.ps1`
  - One-command installer for Windows background client

## How synchronization works

1. Windows app starts an RFCOMM Bluetooth server and registers an SPP service.
2. Android app scans paired devices and attempts SPP connection.
3. On successful connection, each side listens for incoming clipboard text.
4. Clipboard updates are written locally and forwarded to the connected device.

## Requirements

### Windows

- Windows with Bluetooth support
- A Bluetooth adapter enabled and available
- PowerShell (for installer flow)

### Android

- Android 7.0+ (minSdk 24)
- Bluetooth enabled
- App permissions granted (`BLUETOOTH_CONNECT`, `BLUETOOTH_SCAN` on Android 12+)
- Device paired with the target Windows machine

## Installation

### Windows quick install (recommended)

Run in an **Administrator PowerShell** terminal:

```powershell
irm https://raw.githubusercontent.com/muhammadhaseebiqbal-dev/ClipSync/master/install.ps1 | iex
```

The installer will:

- Create `%LOCALAPPDATA%\ClipSync`
- Download `WindowsClient/main.exe`
- Add startup entry at `HKCU:\Software\Microsoft\Windows\CurrentVersion\Run`
- Start the app in hidden/background mode

### Android install

1. Open `/AndroidClient` in Android Studio.
2. Build and run the app on your Android device.
3. Grant required Bluetooth permissions when prompted.

## First-time setup

1. Pair Android device with your Windows PC in Bluetooth settings.
2. Ensure Windows client is running (installed startup mode or manual run).
3. Open Android app and tap **Start Bluetooth Background Sync**.
4. Keep Bluetooth enabled on both devices.
5. Copy text on one device and confirm it appears on the other.

## Development

### Windows client

Source: `/WindowsClient/main.cpp`

Build on Windows with Visual Studio C++ tools:

```bat
cd WindowsClient
build.bat
```

Notes:

- The client is built as a Windows subsystem app (no console window in production).
- Uses Win32 clipboard listener (`WM_CLIPBOARDUPDATE`) and Winsock Bluetooth RFCOMM.

### Android client

Source: `/AndroidClient/app/src/main/java/com/ecosystem/client`

Build with Gradle:

```bash
cd AndroidClient
./gradlew assembleDebug
```

## Operational notes

- Current implementation targets **text clipboard** synchronization.
- Clipboard messages are UTF-8 over Bluetooth stream.
- Android side uses a foreground service to keep sync active.
- If connection drops, Android retries periodically.

## Troubleshooting

- **No connection found**: confirm devices are paired and Bluetooth is enabled on both sides.
- **Sync not starting on Android**: verify Bluetooth permissions are granted.
- **Windows not auto-starting**: verify the `ClipSync` Run registry value exists under current user.
- **No clipboard updates**: test plain text first; non-text clipboard data is not currently supported.
