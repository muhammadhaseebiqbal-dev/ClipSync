#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// Hide console window in production
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")

// Global Socket state
SOCKET clientSocket = INVALID_SOCKET;
bool ignoreNextClipboardUpdate = false;

// Convert UTF-16 (Windows Clipboard) to UTF-8 (Network/Android)
std::string utf16_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert UTF-8 to UTF-16
std::wstring utf8_to_utf16(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

void SendClipboardDataToAndroid(const std::wstring& wText) {
    if (clientSocket != INVALID_SOCKET) {
        std::string utf8Text = utf16_to_utf8(wText);
        // Prepend with length or just send bytes (for now just send and close with a newline or custom delimiter)
        // Adding a newline as a simple delimiter for the stream
        utf8Text += "\n"; 
        send(clientSocket, utf8Text.c_str(), (int)utf8Text.length(), 0);
        std::cout << "Sent " << utf8Text.length() << " bytes to Android." << std::endl;
    }
}

void SetLocalClipboard(const std::wstring& wText) {
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        ignoreNextClipboardUpdate = true; // Prevent infinite loop when we set it ourselves

        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (wText.length() + 1) * sizeof(wchar_t));
        if (hMem) {
            memcpy(GlobalLock(hMem), wText.c_str(), (wText.length() + 1) * sizeof(wchar_t));
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
        CloseClipboard();
        std::cout << "Updated local PC clipboard." << std::endl;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CLIPBOARDUPDATE) {
        if (ignoreNextClipboardUpdate) {
            ignoreNextClipboardUpdate = false;
            return 0;
        }

        if (OpenClipboard(hwnd)) {
            if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
                HANDLE hData = GetClipboardData(CF_UNICODETEXT);
                if (hData != nullptr) {
                    wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
                    if (pszText != nullptr) {
                        std::wstring text(pszText);
                        GlobalUnlock(hData);
                        std::cout << "Clipboard updated locally. Sending to Android..." << std::endl;
                        SendClipboardDataToAndroid(text);
                    }
                }
            }
            CloseClipboard();
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void BluetoothServerThread() {
    WSADATA wsd;
    WSAStartup(MAKEWORD(2, 2), &wsd);

    SOCKET serverSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

    SOCKADDR_BTH sab;
    memset(&sab, 0, sizeof(sab));
    sab.addressFamily = AF_BTH;
    sab.btAddr = 0;
    sab.port = BT_PORT_ANY;

    bind(serverSocket, (SOCKADDR*)&sab, sizeof(sab));
    listen(serverSocket, 1);

    int size = sizeof(sab);
    getsockname(serverSocket, (SOCKADDR*)&sab, &size);
    std::cout << "Bluetooth Server started. Listening on RFCOMM Channel: " << sab.port << std::endl;

    // Register SDP Service Record so Android can find this specific socket using the SPP UUID
    CSADDR_INFO csa = {0};
    csa.LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
    csa.LocalAddr.lpSockaddr = (LPSOCKADDR)&sab;
    csa.iSocketType = SOCK_STREAM;
    csa.iProtocol = BTHPROTO_RFCOMM;

    WSAQUERYSET wsaq = {0};
    wsaq.dwSize = sizeof(wsaq);
    wsaq.lpszServiceInstanceName = (LPWSTR)L"Ecosystem Clipboard Sync";
    wsaq.lpszComment = (LPWSTR)L"Clipboard Sync Profile";
    
    // Standard SPP UUID exactly matching Android: 00001101-0000-1000-8000-00805F9B34FB
    GUID serviceID = { 0x00001101, 0x0000, 0x1000, { 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } };
    wsaq.lpServiceClassId = &serviceID;
    wsaq.dwNameSpace = NS_BTH;
    wsaq.dwNumberOfCsAddrs = 1;
    wsaq.lpcsaBuffer = &csa;

    if (WSASetService(&wsaq, RNRSERVICE_REGISTER, 0) != 0) {
        std::cerr << "Failed to register Bluetooth SDP record: " << WSAGetLastError() << std::endl;
    } else {
        std::cout << "SDP Service registered. PC is now discoverable to the App!" << std::endl;
    }

    while (true) {
        std::cout << "Waiting for Android to connect..." << std::endl;
        SOCKADDR_BTH clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET newClient = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
        
        if (newClient != INVALID_SOCKET) {
            std::cout << "Android device connected!" << std::endl;
            clientSocket = newClient;

            char buffer[4096];
            int bytesRead;
            while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
                buffer[bytesRead] = '\0'; // Null terminate
                std::string receivedData(buffer);
                std::cout << "Received data from Android (" << bytesRead << " bytes)." << std::endl;
                
                // Set the clipboard locally
                std::wstring wText = utf8_to_utf16(receivedData);
                SetLocalClipboard(wText);
            }
            
            std::cout << "Android disconnected." << std::endl;
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        } else {
            // If accept fails (e.g. no Bluetooth adapter active), sleep to prevent CPU spiking
            std::cerr << "Bluetooth accept failed. Error: " << WSAGetLastError() << ". Retrying in 3 seconds..." << std::endl;
            Sleep(3000);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    std::cout << "--- Clipboard Bluetooth Sync (C++) ---" << std::endl;

    // Start Bluetooth Server Thread
    std::thread btThread(BluetoothServerThread);
    btThread.detach();

    // Create a hidden Message-Only window to receive WM_CLIPBOARDUPDATE
    WNDCLASSEX wx = {};
    wx.cbSize = sizeof(WNDCLASSEX);
    wx.lpfnWndProc = WndProc;
    wx.hInstance = GetModuleHandle(nullptr);
    wx.lpszClassName = L"ClipboardSyncHiddenWindow";
    RegisterClassEx(&wx);

    HWND hwnd = CreateWindowEx(0, wx.lpszClassName, L"SyncWindow", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wx.hInstance, nullptr);
    if (!hwnd) {
        std::cerr << "Failed to create hidden window." << std::endl;
        return 1;
    }

    // Register to receive clipboard updates natively (0% CPU idle)
    AddClipboardFormatListener(hwnd);
    std::cout << "Monitoring clipboard..." << std::endl;

    // Standard Windows Message Loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    RemoveClipboardFormatListener(hwnd);
    return 0;
}
