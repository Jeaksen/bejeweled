// Pige_lab1.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "Pige_lab1.h"
#include <string>
#include <queue>
#include <ctime>

#define MAX_LOADSTRING 100
#define MAX_GRID_COUNT 12
#define TIMER_GAMEINIT 2
#define TIMER_HOVER 1

//enum GameSize
//{
//    Small, Medium, Big
//};
//GameSize gameSize = GameSize::Small;


// Global Variables:
HINSTANCE hInst;                                // current instance
int nCmdShow;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
const COLORREF colours[] = { 0xDB7093 , 0x7B68EE, 0xFFE4B5, 0xB22222, 0xBA55D3, 0x800080 };
std::pair<HWND, COLORREF> grid[MAX_GRID_COUNT][MAX_GRID_COUNT]{};
bool gemsToDestroy[MAX_GRID_COUNT][MAX_GRID_COUNT]{};
//stores handles to child windows, their colour and a flag indicating wether the window is empty after 'destroying' the gem


//std::tuple <HWND, COLORREF, bool> grid[MAX_GRID_COUNT][MAX_GRID_COUNT]{};
const unsigned int gridCount[] = { 8, 10, 12 };
//TODO change sizes
const unsigned int gridSizes[] = { 70, 60, 50 };
const unsigned int MARGIN = 5, HOVER_INCREASE = 4;

short gameSize = 0;
bool gameStarted = false;
bool initializingGame = false;
bool changingGrid = false;


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
void                InitGame(HWND mainHandle);
bool                ValidateMove(HWND selectedWindow, HWND validatedWindow);
bool                AnalyseGrid();
bool                ShouldDestroyGem(HWND hwnd);
COLORREF            FindColour(HWND hwnd);
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

    HWND hWnd = CreateWindowExW(WS_EX_COMPOSITED, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW - WS_MAXIMIZEBOX - WS_THICKFRAME, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);
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
        {
            InitGame(hWnd);
            break;
        }
        case ID_BOARDSIZE_SMALL:
            if (!initializingGame) ChangeGameSize(hWnd, 0);
            break;
        case ID_BOARDSIZE_MEDIUM:
            if (!initializingGame) ChangeGameSize(hWnd, 1);
            break;
        case ID_BOARDSIZE_BIG:
            if (!initializingGame) ChangeGameSize(hWnd, 2);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_TIMER:
    {
        static int count = 0;
        if (wParam == TIMER_GAMEINIT)
        {
            if (initializingGame && count == pow(gridCount[gameSize], 2))
            {
                initializingGame = false;
                count = 0;
                KillTimer(hWnd, TIMER_GAMEINIT);
                SendMessage(hWnd, WM_APP, 0, 0);
            }
            else
            {
                int idx = count / gridCount[gameSize];
                int idy = count % gridCount[gameSize];
                grid[idx][idy].second = colours[rand() % (sizeof(colours) / sizeof(COLORREF))];
                InvalidateRect(grid[idx][idy].first, NULL, TRUE);
                UpdateWindow(grid[idx][idy].first);
                count++;
            }
        }
        break;
    }
    case WM_APP:
    {
        if (!AnalyseGrid()) break;
        InvalidateRect(hWnd, NULL, TRUE);
    }
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
    static bool selected = false;
    static HWND selectedWindowHwnd = NULL;
    switch (message)
    {
        //when mouse enters the region of a sqare tracking of leave is triggered and the client rectangle is increase b HOVER_INCREASE
        case WM_MOUSEMOVE:
        {
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
        }
        //when mouse leaves the region of a square the traccking of leave is turned off and a timer with id TIMER_HOVER is started
        case WM_MOUSELEAVE:
        {
            hover = false;
            TRACKMOUSEEVENT trmev;
            trmev.cbSize = sizeof(TRACKMOUSEEVENT);
            trmev.dwFlags = TME_CANCEL | TME_LEAVE;
            trmev.hwndTrack = hWnd;
            TrackMouseEvent(&trmev);
            SetTimer(hWnd, TIMER_HOVER, 50, NULL);
            break;
        }
        case WM_LBUTTONDOWN:
        {
            if (initializingGame || !gameStarted) break;
            if (!selected)
            {
                selected = true;
                selectedWindowHwnd = hWnd;
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
            }
            else if (hWnd == selectedWindowHwnd)
            {
                selected = false;
                selectedWindowHwnd = NULL;
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
            }
            else 
            {
                if (ValidateMove(selectedWindowHwnd, hWnd))
                {
                    SendMessage(GetParent(hWnd), WM_APP, 0, 0);
                    InvalidateRect(hWnd, NULL, TRUE);
                    UpdateWindow(hWnd);
                }

                HWND temp = selectedWindowHwnd;
                selected = false;
                selectedWindowHwnd = NULL;
                InvalidateRect(temp, NULL, TRUE);
                UpdateWindow(temp);
            }
            break;
        }
        //for timer id TIMER_HOVER decreases the size of the square as long as it's bigger than the default one, then kills the timer
        case WM_TIMER:
        {
            if (wParam == TIMER_HOVER)
            {
                RECT rect;
                GetClientRect(hWnd, &rect);
                if (rect.right - rect.left == gridSizes[gameSize])
                {
                    KillTimer(hWnd, TIMER_HOVER);
                    break;
                }
                ResizeChild(hWnd, rect, -1);
            }
            break;
        }
        case WM_PAINT:
            {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            COLORREF colour = FindColour(hWnd);
            HBRUSH brush;
            
            if (!gameStarted || colour == NULL)
            {
                brush = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
            }
            else
            {
                if (ShouldDestroyGem(hWnd))
                    brush = CreateHatchBrush(HS_CROSS, colour);
                else 
                    brush = CreateSolidBrush(colour);
            }
            RECT clRect;
            GetClientRect(hWnd, &clRect);
            HBRUSH oldBrush = (HBRUSH) SelectObject(hdc, brush);
            FillRect(hdc,&clRect , brush);
            brush = (HBRUSH)SelectObject(hdc, oldBrush);
            if (!gameStarted || colour == NULL) DeleteObject(brush);
            if (selected && selectedWindowHwnd == hWnd)
            {
                HPEN pen = CreatePen(PS_SOLID, 2*MARGIN, 0x0);
                HPEN oldPen = (HPEN)SelectObject(hdc, pen);
                MoveToEx(hdc, 0, 0, NULL);
                LineTo(hdc, clRect.right, clRect.top);
                LineTo(hdc, clRect.right, clRect.bottom);
                LineTo(hdc, clRect.left, clRect.bottom);
                LineTo(hdc, clRect.left, clRect.top);
            }
            EndPaint(hWnd, &ps);
            break;
            }
        case WM_DESTROY:
            {
            KillTimer(hWnd, TIMER_HOVER);
            selected = hover = false;
            break;
            }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


void ResizeWindow(HWND mainHandle)
{
    gameStarted = initializingGame = false;
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
    srand((unsigned int)time(NULL));
    int x = MARGIN, y = MARGIN;
    for (size_t i = 0; i < gridCount[gameSize]; i++)
    {
        for (size_t j = 0; j < gridCount[gameSize]; j++)
        {
            child = CreateWindowW(L"Grid", L"", WS_CHILD | WS_VISIBLE, x, y, gridSizes[gameSize], gridSizes[gameSize], mainHandle, nullptr, hInst, nullptr);
            ShowWindow(child, nCmdShow);
            UpdateWindow(child);
            x += gridSizes[gameSize] + 10;
            grid[i][j] = std::make_pair(child, NULL);
            gemsToDestroy[i][j] = false;
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


void InitGame(HWND mainHandle)
{
    if (initializingGame) return;
    for (size_t i = 0; i < gridCount[gameSize]; i++)
    {
        for (size_t j = 0; j < gridCount[gameSize]; j++)
        {
            grid[i][j].second = NULL;
            gemsToDestroy[i][j] = false;
            InvalidateRect(mainHandle, NULL, TRUE);
        }
    }
    SetTimer(mainHandle, TIMER_GAMEINIT, 50, NULL);
    gameStarted = initializingGame  = true;
}


bool ValidateMove(HWND selectedWindow, HWND validatedWindow)
{
    POINT selectedCoord{-1, -1}, validatedCoord{-1, -1};
    for (int i = 0; i < gridCount[gameSize]; i++)
        for (int j = 0; j < gridCount[gameSize]; j++)
        {
            if (grid[i][j].first == selectedWindow)
                selectedCoord = POINT{ i ,j };
            if (grid[i][j].first == validatedWindow)
                validatedCoord = POINT{ i ,j };
        }
    //elements are not next to each other in the same column/row
    if (abs(selectedCoord.x - validatedCoord.x) + abs(selectedCoord.y - validatedCoord.y) != 1) return false;
    int countVertical = 0, countHorizontal = 0;
    //count all elements with the colour as selected element in the same column and row as the validated element 
    //we skip the direction of the selected element
    if (selectedCoord.x - validatedCoord.x >= 0) for (int i = validatedCoord.x - 1;
        i >= 0 && grid[i][validatedCoord.y].second == grid[selectedCoord.x][selectedCoord.y].second;
        i--, countVertical++);
    if (selectedCoord.x - validatedCoord.x <= 0) for (int i = validatedCoord.x + 1;
        i < gridCount[gameSize] && grid[i][validatedCoord.y].second == grid[selectedCoord.x][selectedCoord.y].second;
        i++, countVertical++);
    if (selectedCoord.y - validatedCoord.y >= 0) for (int i = validatedCoord.y - 1;
        i >= 0 && grid[validatedCoord.x][i].second == grid[selectedCoord.x][selectedCoord.y].second;
        i--, countHorizontal++);
    if (selectedCoord.y - validatedCoord.y <= 0) for (int i = validatedCoord.y + 1;
        i < gridCount[gameSize] && grid[validatedCoord.x][i].second == grid[selectedCoord.x][selectedCoord.y].second;
        i++, countHorizontal++);

    if (countHorizontal >= 2 || countVertical >= 2)
    {
        COLORREF temp = grid[selectedCoord.x][selectedCoord.y].second;
        grid[selectedCoord.x][selectedCoord.y].second = grid[validatedCoord.x][validatedCoord.y].second;
        grid[validatedCoord.x][validatedCoord.y].second = temp;
        return true;
    }
    
    countVertical = 0, countHorizontal = 0;
    //count all elements with the colour as validated element in the same column and row as the selected element 
    //we skip the direction of the validated element
    if (selectedCoord.x - validatedCoord.x <= 0) for (int i = selectedCoord.x - 1;
        i >= 0 && grid[i][selectedCoord.y].second == grid[validatedCoord.x][validatedCoord.y].second;
        i--, countVertical++);
    if (selectedCoord.x - validatedCoord.x >= 0) for (int i = selectedCoord.x + 1;
        i < gridCount[gameSize] && grid[i][selectedCoord.y].second == grid[validatedCoord.x][validatedCoord.y].second;
        i++, countVertical++);
    if (selectedCoord.y - validatedCoord.y <= 0) for (int i = selectedCoord.y - 1;
        i >= 0 && grid[selectedCoord.x][i].second == grid[validatedCoord.x][validatedCoord.y].second;
        i--, countHorizontal++);
    if (selectedCoord.y - validatedCoord.y >= 0) for (int i = selectedCoord.y + 1;
        i < gridCount[gameSize] && grid[selectedCoord.x][i].second == grid[validatedCoord.x][validatedCoord.y].second;
        i++, countHorizontal++);

    if (countHorizontal >= 2 || countVertical >= 2)
    {
        COLORREF temp = grid[selectedCoord.x][selectedCoord.y].second;
        grid[selectedCoord.x][selectedCoord.y].second = grid[validatedCoord.x][validatedCoord.y].second;
        grid[validatedCoord.x][validatedCoord.y].second = temp;
        return true;
    }
    return false;
}


bool AnalyseGrid()
{
    bool foundAny = false;
    for (size_t i = 0; i < gridCount[gameSize]; i++)
        for (size_t j = 0; j < gridCount[gameSize]; j++)
        {
            if (i > 0 && i < gridCount[gameSize] - 1 && grid[i][j].second == grid[i - 1][j].second && grid[i][j].second == grid[i + 1][j].second)
            {
                gemsToDestroy[i - 1][j] = gemsToDestroy[i][j] = gemsToDestroy[i + 1][j] = true;
                foundAny = true;
            }
            if (j > 0 && j < gridCount[gameSize] - 1 && grid[i][j].second == grid[i][j - 1].second && grid[i][j].second == grid[i][j + 1].second)
            {
                gemsToDestroy[i][j - 1] = gemsToDestroy[i][j] = gemsToDestroy[i][j + 1] = true;
                foundAny = true;
            }
        }

    return foundAny;
}


bool ShouldDestroyGem(HWND hwnd)
{
    for (size_t i = 0; i < gridCount[gameSize]; i++)
    {
        for (size_t j = 0; j < gridCount[gameSize]; j++)
        {
            if (grid[i][j].first == hwnd) return gemsToDestroy[i][j];
        }

    }
    return false;
}


COLORREF FindColour(HWND hwnd)
{
    for (size_t i = 0; i < gridCount[gameSize]; i++)
    {
        for (size_t j = 0; j < gridCount[gameSize]; j++)
        {
            if (grid[i][j].first == hwnd) return grid[i][j].second;
        }

    }
    return (COLORREF)0;
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
    wcex.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PIGELAB1);
    wcex.lpszClassName = L"Grid";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
