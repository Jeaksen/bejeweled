// Pige_lab1.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "Pige_lab1.h"
#include <string>
#include <queue>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
int nCmdShow;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
const int gridCount[] = { 8, 10, 12 };
//TODO change sizes
const int gridSizes[] = { 70, 60, 50 };
const int MARGIN = 5, HOVER_INCREASE = 4;
int gameSize = 0;




// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyRegisterChildClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                ResizeWindow(HWND mainHandle);
void                ChangeGameSize(HWND mainHandle, int size);
void                ResizeChild(HWND handle, RECT rect, int change);
BOOL CALLBACK       EnumKillChild(_In_ HWND   hwnd, _In_ LPARAM lParam);




int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PIGELAB1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    MyRegisterChildClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PIGELAB1));
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable
    ::nCmdShow = nCmdShow;

    HWND hWnd = CreateWindowExW(WS_EX_NOPARENTNOTIFY, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW - WS_MAXIMIZEBOX - WS_THICKFRAME, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) return FALSE;
    ResizeWindow(hWnd);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}


BOOL CALLBACK EnumKillChild(_In_ HWND   hwnd, _In_ LPARAM lParam)
{
    return DestroyWindow(hwnd);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HMENU menu;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_GAME_NEWGAME:
                InitInstance(hInst, nCmdShow);
                CloseWindow(hWnd);
                break;
            case ID_BOARDSIZE_SMALL:
                ChangeGameSize(hWnd, 0);
                break;
            case ID_BOARDSIZE_MEDIUM:
                ChangeGameSize(hWnd, 1);
                break;
            case ID_BOARDSIZE_BIG:
                ChangeGameSize(hWnd, 2);
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
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool hover = false;
    switch (message)
    {
        case WM_MOUSEMOVE:
            if (!hover)
            {
                hover = true;
                TRACKMOUSEEVENT trmev;
                trmev.cbSize = sizeof(TRACKMOUSEEVENT);
                trmev.dwFlags = TME_LEAVE;
                trmev.hwndTrack = hWnd;
                TrackMouseEvent(&trmev);
                RECT rect;

                GetClientRect(hWnd, &rect);
                int change = HOVER_INCREASE - (rect.right - rect.left - gridSizes[gameSize]) / 2;

                ResizeChild(hWnd, rect, change);
            }
            break;
        case WM_MOUSELEAVE:
        {
            hover = false;
            TRACKMOUSEEVENT trmev;
            trmev.cbSize = sizeof(TRACKMOUSEEVENT);
            trmev.dwFlags = TME_CANCEL | TME_LEAVE;
            trmev.hwndTrack = hWnd;
            TrackMouseEvent(&trmev);
            SetTimer(hWnd, 1, 50, NULL);
        }
            break;
        case WM_TIMER:
        {
            RECT rect;
            GetClientRect(hWnd, &rect);
            if (rect.right - rect.left == gridSizes[gameSize])
            {
                KillTimer(hWnd, 1);
                break;
            }
            ResizeChild(hWnd, rect, -1);
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
        case WM_DESTROY:
        {
            KillTimer(hWnd, 1);
            hover = false;
        }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


void ResizeWindow(HWND mainHandle)
{
    int maxWidth = GetSystemMetrics(SM_CXSCREEN);
    int maxHeight = GetSystemMetrics(SM_CYSCREEN);
    int size = gridCount[gameSize] * (gridSizes[gameSize] + 2 * MARGIN);
    RECT windowRec{ 0, 0, size, size };

    AdjustWindowRect(&windowRec, WS_OVERLAPPEDWINDOW, true);
    POINT WindowSize{ windowRec.right - windowRec.left, windowRec.bottom - windowRec.top };

    int dx = (maxWidth - WindowSize.x) / 2;
    int dy = (maxHeight - WindowSize.y) / 2;

    MoveWindow(mainHandle, dx, dy, WindowSize.x, WindowSize.y, TRUE);
    EnumChildWindows(mainHandle, EnumKillChild, NULL);
    UpdateWindow(mainHandle);
    HWND child;

    int x = 5, y = 5;
    for (size_t i = 0; i < gridCount[gameSize]; i++)
    {
        for (size_t j = 0; j < gridCount[gameSize]; j++)
        {
            child = CreateWindowW(L"Grid", L"", WS_CHILD | WS_VISIBLE, x, y, gridSizes[gameSize], gridSizes[gameSize], mainHandle, nullptr, hInst, nullptr);
            ShowWindow(child, nCmdShow);
            UpdateWindow(child);
            x += gridSizes[gameSize] + 10;
        }
        x = 5;
        y += gridSizes[gameSize] + 10;
    }
}


void ChangeGameSize(HWND mainHandle, int size)
{
    if (gameSize == size) return;
    HMENU menu = GetMenu(mainHandle);

    switch (gameSize)
    {
    case 0: CheckMenuItem(menu, ID_BOARDSIZE_SMALL, MF_UNCHECKED); break;
    case 1: CheckMenuItem(menu, ID_BOARDSIZE_MEDIUM, MF_UNCHECKED); break;
    case 2: CheckMenuItem(menu, ID_BOARDSIZE_BIG, MF_UNCHECKED); break;
    }
    switch (size)
    {
    case 0: CheckMenuItem(menu, ID_BOARDSIZE_SMALL, MF_CHECKED); break;
    case 1: CheckMenuItem(menu, ID_BOARDSIZE_MEDIUM, MF_CHECKED); break;
    case 2: CheckMenuItem(menu, ID_BOARDSIZE_BIG, MF_CHECKED); break;
    }
    gameSize = size;
    ResizeWindow(mainHandle);
}


void ResizeChild(HWND handle, RECT rect, int change)
{
    POINT spawn{ rect.left, rect.top };
    MapWindowPoints(handle, GetParent(handle), &spawn, 1);
    InflateRect(&rect, change, change);
    spawn.x -= change;
    spawn.y -= change;
    MoveWindow(handle, spawn.x, spawn.y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}


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


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = 0;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIGELAB1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PIGELAB1);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


ATOM MyRegisterChildClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = 0;
    wcex.lpfnWndProc = ChildWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIGELAB1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);;
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PIGELAB1);
    wcex.lpszClassName = L"Grid";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}