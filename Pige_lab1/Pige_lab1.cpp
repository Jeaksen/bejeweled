// Pige_lab1.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "Pige_lab1.h"
#include <string>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
int nCmdShow;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
const POINT Sizes[] = { {740, 720 + 60}, {820, 800 + 60}, {860, 840 + 60} };
const int gridCount[] = { 8, 10, 12 };
const int gridSizes[] = { 80, 70, 60 };
int gameSize = 0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyRegisterChildClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PIGELAB1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    MyRegisterChildClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

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


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = 0;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIGELAB1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PIGELAB1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable
    ::nCmdShow = nCmdShow;
    int maxWidth = GetSystemMetrics(SM_CXSCREEN);
    int maxHeight = GetSystemMetrics(SM_CYSCREEN);
    int dx = (maxWidth - Sizes[gameSize].x) / 2;
    int dy = (maxHeight - Sizes[gameSize].y) / 2;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW - WS_MAXIMIZEBOX - WS_THICKFRAME, dx, dy, Sizes[gameSize].x, Sizes[gameSize].y, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd) return FALSE;


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   HWND child;

   int x = 5, y = 5;
   for (size_t i = 0; i < gridCount[gameSize]; i++)
   {
       for (size_t j = 0; j < gridCount[gameSize]; j++)
       {
           child = CreateWindowW(L"Grid", L"", WS_CHILD | WS_VISIBLE, x, y, gridSizes[gameSize], gridSizes[gameSize], hWnd, nullptr, hInstance, nullptr);
           ShowWindow(child, nCmdShow);
           UpdateWindow(child);
           x += gridSizes[gameSize] + 10;
       }
       x = 5;
       y += gridSizes[gameSize] + 10;
   }

   return TRUE;
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
                gameSize = 0;
                menu = GetMenu(hWnd);
                
                CheckMenuItem(menu, ID_BOARDSIZE_SMALL, MF_CHECKED);
                CheckMenuItem(menu, ID_BOARDSIZE_MEDIUM, MF_UNCHECKED);
                CheckMenuItem(menu, ID_BOARDSIZE_BIG, MF_UNCHECKED);

                break;
            case ID_BOARDSIZE_MEDIUM:
                gameSize = 1;
                menu = GetMenu(hWnd);
                CheckMenuItem(menu, ID_BOARDSIZE_SMALL, MF_UNCHECKED);

                CheckMenuItem(menu, ID_BOARDSIZE_MEDIUM, MF_CHECKED);
                CheckMenuItem(menu, ID_BOARDSIZE_BIG, MF_UNCHECKED);

                break;
            case ID_BOARDSIZE_BIG:
                gameSize = 2;
                menu = GetMenu(hWnd);
                CheckMenuItem(menu, ID_BOARDSIZE_SMALL, MF_UNCHECKED);
                CheckMenuItem(menu, ID_BOARDSIZE_MEDIUM, MF_UNCHECKED);

                CheckMenuItem(menu, ID_BOARDSIZE_BIG, MF_CHECKED);


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
            // TODO: Add any drawing code that uses hdc here...
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

void ResizeWindow()
{

}

LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
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


// Message handler for about box.
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
