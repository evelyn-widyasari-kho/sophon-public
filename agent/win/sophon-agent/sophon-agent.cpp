// SophonAgent.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "Sophon-agent.h"
#include <shellapi.h>                           // For system tray feature
#pragma comment(lib, "shell32.lib")
#include <curl/curl.h>                          // For GCP Storage JSON API
#pragma comment(lib, "libcurl-x64.lib")  

#define MAX_LOADSTRING 100
#define WM_TRAYICON (WM_USER + 1)               // For system tray feature
#define ID_TRAYICON 1                           // For system tray feature
#define IDC_BTN_LOGIN   1001
#define IDC_BTN_LOGOUT  1002
#define IDC_BTN_START   1003
#define IDC_BTN_PAUSE   1004

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
NOTIFYICONDATA nid;                             // For system tray feature
BOOL bAuthenticated;
BOOL bStarted;
WCHAR szUserName[MAX_LOADSTRING];
WCHAR szDeviceName[MAX_LOADSTRING];
WCHAR szIPAddr[MAX_LOADSTRING];
WCHAR szMacAddr[MAX_LOADSTRING];
WCHAR szRemoteStorageAddr[MAX_LOADSTRING];
HANDLE g_hScreenshotThread = NULL;              // Therad handler for the start button

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                SaveBitmapToFile(HBITMAP hBitmap, LPCWSTR filename);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SOPHONAGENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SOPHONAGENT));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SOPHONAGENT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SOPHONAGENT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        // Initialize tray icon
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hWnd;
        nid.uID = ID_TRAYICON;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SOPHONAGENT));
        wcscpy_s(nid.szTip, L"Sophon Agent"); // Tooltip text
        Shell_NotifyIcon(NIM_ADD, &nid);

        // Create buttons
        CreateWindow(L"BUTTON", L"Login",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 10, 100, 30,
            hWnd, (HMENU)IDC_BTN_LOGIN, hInst, NULL);
        CreateWindow(L"BUTTON", L"Logout",        // New button
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            120, 10, 100, 30,
            hWnd, (HMENU)IDC_BTN_LOGOUT, hInst, NULL);
        CreateWindow(L"BUTTON", L"Start",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            230, 10, 100, 30,                    // Changed x position
            hWnd, (HMENU)IDC_BTN_START, hInst, NULL);
        CreateWindow(L"BUTTON", L"Pause",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            340, 10, 100, 30,                    // Changed x position
            hWnd, (HMENU)IDC_BTN_PAUSE, hInst, NULL);

        // Initialize the flags
        bAuthenticated = FALSE;
        bStarted = FALSE;
    }
    break;
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
        {
            // Hide window
            ShowWindow(hWnd, SW_HIDE);
            return 0;
        }
    }
    break;
    case WM_TRAYICON:
    {
        if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
        {
            // Restore window when tray icon is clicked
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
        }
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDC_BTN_LOGIN:
            // Handle Login button click
            bAuthenticated = TRUE;
            InvalidateRect(hWnd, NULL, TRUE);  // Request screen update
            break;
        case IDC_BTN_LOGOUT:
            bAuthenticated = FALSE;
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        case IDC_BTN_START:
            // Set started flag to true
            bStarted = TRUE;
            InvalidateRect(hWnd, NULL, TRUE);

            // 이미 실행 중인 스레드가 있다면 무시
            if (g_hScreenshotThread != NULL) {
                break;
            }

            // Create thread for screenshot loop
            g_hScreenshotThread = CreateThread(
                NULL,
                0,
                [](LPVOID param) -> DWORD {
                    HWND hWnd = (HWND)param;

                    while (true) {
                        if (bAuthenticated && bStarted) {
                            // Capture screenshot
                            HDC hdcScreen = GetDC(NULL);
                            HDC hdcMemDC = CreateCompatibleDC(hdcScreen);

                            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                            int screenHeight = GetSystemMetrics(SM_CYSCREEN);

                            HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
                            SelectObject(hdcMemDC, hbmScreen);

                            // Capture screen content
                            BitBlt(hdcMemDC, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

                            // Save with ISO8601 timestamp
                            WCHAR szFilePath[MAX_PATH];
                            SYSTEMTIME st;
                            GetLocalTime(&st);

                            wsprintf(szFilePath, L"screenshot_%04d-%02d-%02dT%02d-%02d-%02dZ.bmp",
                                st.wYear, st.wMonth, st.wDay,
                                st.wHour, st.wMinute, st.wSecond);

                            // Save bitmap to file
                            SaveBitmapToFile(hbmScreen, szFilePath);

                            // TODO: Send to remote storage
                            // 

                            // Cleanup
                            DeleteObject(hbmScreen);
                            DeleteDC(hdcMemDC);
                            ReleaseDC(NULL, hdcScreen);

                            // Wait 10 seconds
                            Sleep(10000);
                        }
                        Sleep(100);  // Small delay to prevent CPU overuse
                    }
                    return 0;
                },
                hWnd,
                0,
                NULL
            );
            break;
        case IDC_BTN_PAUSE:
            // Handle Pause button click
            bStarted = FALSE;
            if (g_hScreenshotThread) {
                CloseHandle(g_hScreenshotThread);
                g_hScreenshotThread = NULL;
            }
            InvalidateRect(hWnd, NULL, TRUE);  // Request screen update
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...

        // Status indicator settings
        const int CIRCLE_RADIUS = 8;
        const int INDICATOR_LEFT = 100;          // Starting x position for circles
        const int FIRST_ROW_Y = 50;             // First indicator Y position
        const int SECOND_ROW_Y = 70;            // Second indicator Y position

        // Authentication status light
        if (bAuthenticated)
            SelectObject(hdc, CreateSolidBrush(RGB(0, 255, 0)));  // Green for TRUE
        else
            SelectObject(hdc, CreateSolidBrush(RGB(255, 0, 0)));  // Red for FALSE

        // Draw first circle (Auth status)
        Ellipse(hdc,
            INDICATOR_LEFT, FIRST_ROW_Y,           // Top-left x,y
            INDICATOR_LEFT + CIRCLE_RADIUS * 2,       // Bottom-right x
            FIRST_ROW_Y + CIRCLE_RADIUS * 2);        // Bottom-right y

        // Clean up first brush
        DeleteObject(SelectObject(hdc, GetStockObject(NULL_BRUSH)));

        // Started status light
        if (bStarted)
            SelectObject(hdc, CreateSolidBrush(RGB(0, 255, 0)));  // Green for TRUE
        else
            SelectObject(hdc, CreateSolidBrush(RGB(255, 0, 0)));  // Red for FALSE

        // Draw second circle (Started status)
        Ellipse(hdc,
            INDICATOR_LEFT, SECOND_ROW_Y,
            INDICATOR_LEFT + CIRCLE_RADIUS * 2,
            SECOND_ROW_Y + CIRCLE_RADIUS * 2);

        // Label text
        SetBkMode(hdc, TRANSPARENT);  // Make text background transparent
        TextOut(hdc, INDICATOR_LEFT - 80, FIRST_ROW_Y, L"Auth Status:", 11);
        TextOut(hdc, INDICATOR_LEFT - 80, SECOND_ROW_Y, L"Run Status:", 10);

        // Cleanup GDI resources
        DeleteObject(SelectObject(hdc, GetStockObject(NULL_BRUSH)));

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        // Remove tray icon when program exits
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void SaveBitmapToFile(HBITMAP hBitmap, LPCWSTR filename) {
    BITMAP bitmap;
    GetObject(hBitmap, sizeof(BITMAP), &bitmap);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bitmap.bmWidth;
    bi.biHeight = bitmap.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bitmap.bmWidth * bi.biBitCount + 31) / 32) * 4 * bitmap.bmHeight;

    HANDLE hFile = CreateFile(filename,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = dwSizeofDIB;
    bmfHeader.bfType = 0x4D42; // "BM"

    DWORD dwBytesWritten;
    WriteFile(hFile, (LPVOID)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPVOID)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);

    BYTE* lpbitmap = new BYTE[dwBmpSize];
    GetDIBits(GetDC(NULL), hBitmap, 0, (WORD)bitmap.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    delete[] lpbitmap;
    CloseHandle(hFile);
}