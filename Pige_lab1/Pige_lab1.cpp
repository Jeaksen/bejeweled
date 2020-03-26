#include "pch.h"
#include "framework.h"
#include "Pige_lab1.h"
#include <string>
#include <ctime>
#include <vector>
#include <algorithm>

#define MAX_LOADSTRING 100
#define MAX_GRID_COUNT 12
#define RAND_FACTOR 4
#define TIMER_HOVER 1
#define TIMER_GAMEINIT 2
#define TIMER_CHANGE_GRID 3
#define TIMER_PARTICLES 4
#define INTERVAL_TIMER_HOVER 50
#define INTERVAL_TIMER_GAMEINIT 30
#define INTERVAL_TIMER_CHANGE_GRID 500
#define INTERVAL_TIMER_PARTICLES 20

// Struct for stoting data of a destruction particle 
typedef struct
{
    POINT position;
    std::pair<double, double> direction;
    COLORREF colour;
} Particle;

// Global Constants

const unsigned int gridCount[] = { 8, 10, 12 }; // Stores handles to child windows, their colour and a flag indicating wether the window is empty after 'destroying' the gem
const unsigned int gridSizes[] = { 70, 60, 50 }; // Posible sizes of elements
const unsigned int MARGIN = 5; // Size of margin
const unsigned int HOVER_INCREASE = 4; // Size by which windows increase when hovered
const unsigned int PARTICLE_MOVE_DISTANCE = 20; // distance by which the particles move on every tick of the animation clock
const unsigned int PARTICLES_COUNT = 100; // defines the number of particles which spawn from one 'destroed' grid element

// Global Variables:
HINSTANCE hInst;                                // current instance
int nCmdShow;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND overlayHandle = NULL;                      // Handle to the overlay with destruction animations

const COLORREF colours[] = { 0xFFD700, 0x32CD32, 0x0000CD, 0xFF4500, 0x00BFFF, 0xC71585 }; // Coulours of grid elements
std::pair<HWND, COLORREF> grid[MAX_GRID_COUNT][MAX_GRID_COUNT]{}; // Stores handles to child windows aliged in the grid as on the screen with their background colour
bool gemsToDestroy[MAX_GRID_COUNT][MAX_GRID_COUNT]{}; // Stores the state whether a given child should be 'destroyed' - background switched to hatch


short gameSize = 0; // Saves current game size: 0 - small, 1 - medium, 2 - big
bool gameStarted = false; // Flag indicating whether the game is in progress
bool initializingGame = false; // Flag indicating whether the game is in statup initalization 
bool changingGrid = false; // Flag indicating whether grid elements are moving
bool debug = false; // Flag indicating whether the debug mode is tuned on
int particleSize; // stroes the size of destruction animation particle
std::vector<Particle> particles; //vector which stores information about the position, velocity and colour of each particle


// Forward declarations of functions included in this code module:

ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyRegisterChildClass(HINSTANCE hInstance);
ATOM                MyRegisterOverlayClass(HINSTANCE hInstance);
// Creates the main window and overlay window
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    ChildWndProc(HWND , UINT , WPARAM , LPARAM );
LRESULT CALLBACK    OverlayWndProc(HWND , UINT , WPARAM , LPARAM );
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
// Resizes the window after changing the game size and initializes/resets requried properties
void                ResizeWindow(HWND mainHandle);
// Changes the variable storing the game size and the menu for choosing game size
void                ChangeGameSize(HWND mainHandle, int size);
// Increases the size of a grid element by _change_ to each side (effective increase of 2 x change)
void                ResizeChild(HWND handle, RECT rect, int change);
// Called when a new game is started, resets all needed game varaibles and starts initialization timer
void                InitGame(HWND mainHandle);
// Checks if player can swap the two grid elements
bool                ValidateMove(HWND selectedWindow, HWND validatedWindow);
// Sets the state of gemsToDestory matrix and return a bool indicating whether any gems should be 'destroyed'
bool                AnalyseGrid();
// Checks the value in gemsToDestory for a given grid element
bool                ShouldDestroyGem(HWND hwnd);
// Moves grid elements down in columns in which grid elements were 'drestoyed', return a flag indicating whether the grid was moved
bool                MoveGrid(HWND hwnd);
// Adds to the particles vector new particles for each 'destoryed' grid element
void                GenerateParticles(HWND mainHandle);
// Removes all particles which are not in the screen region
void                RemoveParticles();
// Returns a colour of a grid element
COLORREF            FindColour(HWND hwnd);
// Used for changing the game size, destroyes a grid element
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
    MyRegisterOverlayClass(hInstance);

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

    HWND hWnd = CreateWindowExW(WS_EX_COMPOSITED, szWindowClass, szTitle, WS_CLIPCHILDREN | (WS_OVERLAPPEDWINDOW - WS_MAXIMIZEBOX - WS_THICKFRAME), 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) return FALSE;
    ResizeWindow(hWnd);
    ShowWindow(hWnd, nCmdShow);

    if (overlayHandle == NULL)
    {
        overlayHandle = CreateWindowExW(WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TOPMOST, L"Overlay", L"", WS_POPUP | WS_VISIBLE, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, hInst, nullptr);
        if (!overlayHandle)
        {
            DWORD s = GetLastError();
            return FALSE;
        }
        SetLayeredWindowAttributes(overlayHandle, 0x000000, 0, LWA_COLORKEY);
        ShowWindow(overlayHandle, nCmdShow);
        UpdateWindow(overlayHandle);
    }

    SetActiveWindow(hWnd);
    UpdateWindow(hWnd);
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
        case ID_HELP_DEBUG:
        {
            HMENU menu = GetMenu(hWnd);
            if (debug) CheckMenuItem(menu, ID_HELP_DEBUG, MF_UNCHECKED);
            else CheckMenuItem(menu, ID_HELP_DEBUG, MF_CHECKED);
            debug = !debug;
            InvalidateRect(overlayHandle, NULL, TRUE);
            UpdateWindow(overlayHandle);
            break;
        }
        case ID_BOARDSIZE_SMALL:
            if (!initializingGame && !changingGrid) ChangeGameSize(hWnd, 0);
            break;
        case ID_BOARDSIZE_MEDIUM:
            if (!initializingGame && !changingGrid) ChangeGameSize(hWnd, 1);
            break;
        case ID_BOARDSIZE_BIG:
            if (!initializingGame && !changingGrid) ChangeGameSize(hWnd, 2);
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
            // Creates the animation for game initialization
            // At each tick of the timer chooses a rand colour for the next grid element and displays it
            // once all grid elements were coloured kills the timer
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
        if (wParam == TIMER_CHANGE_GRID)
        {
            // Calls the MoveGrid functions as log as there are any moves to be made, then kills the timer
            if (!MoveGrid(hWnd))
            {
                KillTimer(hWnd, TIMER_CHANGE_GRID);
                SendMessage(hWnd, WM_APP, 1, 0);
                break;
            }
        }
        break;
    }
    case WM_SIZE:
    {
        // minimizes and restores the particle overlay with the main window
        if (wParam == SIZE_RESTORED)
        {
            ShowWindow(overlayHandle, SW_RESTORE);
        }
        if (wParam == SIZE_MINIMIZED)
            ShowWindow(overlayHandle, SW_MINIMIZE);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    // this message is used to transfer information that some elements should be 'destroyed' and the grid moved
    // wParam set to 1 incidates than the call is recursive, and it's done to check whether after the previous movement of grid there are new elemetns to 'destroy'
    case WM_APP:
    {
        if (!AnalyseGrid())
        {
            // the main window has to be redrawn to change the colour indicating that no moves can be made
            if (wParam == 1)
            {
                changingGrid = false;
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
            }
            break;
        }
        changingGrid = true;
        // New particles are generated and the timer for particles is started
        GenerateParticles(hWnd);
        SetTimer(overlayHandle, TIMER_PARTICLES, INTERVAL_TIMER_PARTICLES, NULL);
        // Main window is redraw to change the background coulour indicating that no moves can be made
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
        // Timer for grid movement is started
        SetTimer(hWnd, TIMER_CHANGE_GRID, INTERVAL_TIMER_CHANGE_GRID, 0);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (changingGrid)
        {
            HBRUSH brush = (HBRUSH)GetStockObject(GRAY_BRUSH);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
            RECT clRect;
            GetClientRect(hWnd, &clRect);
            FillRect(hdc, &clRect, brush);
            brush = (HBRUSH)SelectObject(hdc, oldBrush);
            DeleteObject(brush);
        }
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
    static bool hover = false; // flag indicating whether any grid element is current hovered
    static bool selected = false; // flag indicating whether any grid element is current selected and has a boarder
    static HWND selectedWindowHwnd = NULL; // handle to a grid element which is selected
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
            SetTimer(hWnd, TIMER_HOVER, INTERVAL_TIMER_HOVER, NULL);
            break;
        }
        case WM_LBUTTONDOWN:
        {
            // Handles selecting grid elements
            // selecting is invalid in those scenarios
            if (initializingGame || !gameStarted || changingGrid) break;
            // if no other element is selected then select it and update to draw a border
            if (!selected)
            {
                selected = true;
                selectedWindowHwnd = hWnd;
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
            }
            // if a window which was selected is selected again then mark it as not selected and update to remove the border
            else if (hWnd == selectedWindowHwnd)
            {
                selected = false;
                selectedWindowHwnd = NULL;
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
            }
            // when a element is selected and the user clicks on a different element
            else 
            {
                // if the two elements can be swapped start the procedure of moving them 
                if (ValidateMove(selectedWindowHwnd, hWnd))
                {
                    SendMessage(GetParent(hWnd), WM_APP, 0, 0);
                    //InvalidateRect(hWnd, NULL, TRUE);
                    //UpdateWindow(hWnd);
                }
                // regardless of the valid move of the move the selection should be removed and element updatede to remove the border
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
            // After a grid element was increased by hovering and the mouse is no longer on it, it decreases with every tick of the timer
            // once the size goes back to standart the timer is killed 
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

            // if the game hasn't started yet the defaoult dark gray brush is selected
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
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
            FillRect(hdc, &clRect, brush);
            brush = (HBRUSH)SelectObject(hdc, oldBrush);
            DeleteObject(brush);

            // Drawing the selection border
            if (selected && selectedWindowHwnd == hWnd)
            {
                HPEN pen = CreatePen(PS_SOLID, 2 * MARGIN, 0x0);
                HPEN oldPen = (HPEN)SelectObject(hdc, pen);
                MoveToEx(hdc, 0, 0, NULL);
                LineTo(hdc, clRect.right, clRect.top);
                LineTo(hdc, clRect.right, clRect.bottom);
                LineTo(hdc, clRect.left, clRect.bottom);
                LineTo(hdc, clRect.left, clRect.top);
                pen = (HPEN)SelectObject(hdc, oldPen);
                DeleteObject(pen);
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


LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)         
    {
    case WM_TIMER:
    {
        // Redraws the particles at each tick of the clock and removes the ones which should no longer be printed
        if (wParam == TIMER_PARTICLES)
        {
            if (particles.size() == 0)
                KillTimer(hWnd, TIMER_PARTICLES);
            RemoveParticles();
            InvalidateRect(overlayHandle, NULL, FALSE);
            UpdateWindow(overlayHandle);
        }
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        RECT rc;
        HBRUSH brush, oldbrush;
        HBITMAP cBmp, oldBmp;
        HDC hdc = BeginPaint(hWnd, &ps);
        HDC cHdc = CreateCompatibleDC(hdc);

        cBmp = CreateCompatibleBitmap(hdc, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        oldBmp = (HBITMAP)SelectObject(cHdc, cBmp);

        brush = (HBRUSH)GetStockObject(DC_BRUSH);
        oldbrush = (HBRUSH)SelectObject(cHdc, brush);

        GetWindowRect(hWnd, &rc);
        SetDCBrushColor(cHdc, 0x000000);
        FillRect(cHdc, &rc, (HBRUSH)GetCurrentObject(cHdc, OBJ_BRUSH));

        if (debug)
        {
            HFONT font = CreateFontW(100, 0, 0, 0, 700, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, TEXT("Times New Roman") );
            HFONT oldfont = (HFONT)SelectObject(cHdc, font);

            std::wstring text{ L"Particles:" };
            text += std::to_wstring(particles.size());
            SetBkColor(cHdc, 0x000000);
            SetTextColor(cHdc, 0x0000FF);
            DrawTextW(cHdc, text.c_str(), -1, &rc, DT_CENTER | DT_SINGLELINE);

            font = (HFONT)SelectObject(cHdc, font);
            DeleteObject(font);
        }

        COLORREF prevColour{ 0 };
        for (auto& particle : particles)
        {
            /*if (particle.colour != prevColour) SetDCBrushColor(cHdc, particle.colour);
            particle.position.x += PARTICLE_MOVE_DISTANCE * particle.direction.first;
            particle.position.y += PARTICLE_MOVE_DISTANCE * particle.direction.second;
            Rectangle(cHdc, particle.position.x, particle.position.y, particle.position.x + particleSize, particle.position.y + particleSize);*/
            particle.position.x += PARTICLE_MOVE_DISTANCE * particle.direction.first;
            particle.position.y += PARTICLE_MOVE_DISTANCE * particle.direction.second;
            SetDCBrushColor(cHdc, 0xC0C0C0);
            int size = 2;
            Rectangle(cHdc, particle.position.x - size, particle.position.y - size, particle.position.x + particleSize + size, particle.position.y + particleSize + size);
            SetDCBrushColor(cHdc, particle.colour);
            Rectangle(cHdc, particle.position.x, particle.position.y, particle.position.x + particleSize, particle.position.y + particleSize);

        }

        BitBlt(hdc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), cHdc, 0, 0, SRCCOPY);
        brush = (HBRUSH)SelectObject(cHdc, oldbrush);
        cBmp = (HBITMAP)SelectObject(cHdc, oldBmp);
        DeleteObject(brush);
        DeleteObject(cBmp);
        DeleteDC(cHdc);

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_GETMINMAXINFO: {
        DefWindowProc(hWnd, message, wParam, lParam);
        MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
        pmmi->ptMaxTrackSize.x = 2 * GetSystemMetrics(SM_CXSCREEN);
        pmmi->ptMaxTrackSize.y = 2 * GetSystemMetrics(SM_CYSCREEN);
        return 0;
    }
    case WM_DESTROY:
    {
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


void ResizeWindow(HWND mainHandle)
{
    // Reset params of the game
    gameStarted = initializingGame = changingGrid = false;
    //particleSize = gridSizes[gameSize] / pow(PARTICLES_COUNT, 0.5);
    particleSize = 8;
    particles.clear();

    // Set the size of the main window to store all grid elements + menu
    int size = gridCount[gameSize] * (gridSizes[gameSize] + 2 * MARGIN);
    RECT windowRec{ 0, 0, size, size };
    AdjustWindowRect(&windowRec, WS_OVERLAPPEDWINDOW, true);
    POINT WindowSize{ windowRec.right - windowRec.left, windowRec.bottom - windowRec.top };

    //window is centered on the screen
    int maxWidth = GetSystemMetrics(SM_CXSCREEN);
    int maxHeight = GetSystemMetrics(SM_CYSCREEN);
    int dx = (maxWidth - WindowSize.x) / 2;
    int dy = (maxHeight - WindowSize.y) / 2;

    //Remove all children windows (if present) and then redraw the main window for the specific size
    EnumChildWindows(mainHandle, EnumKillChild, NULL);
    MoveWindow(mainHandle, dx, dy, WindowSize.x, WindowSize.y, TRUE);
    UpdateWindow(mainHandle);

    HWND child;
    srand(time(NULL));
    int x = MARGIN, y = MARGIN;
    for (size_t i = 0; i < gridCount[gameSize]; i++)
    {
        for (size_t j = 0; j < gridCount[gameSize]; j++)
        {
            child = CreateWindowW(L"Grid", L"", WS_CHILD | WS_VISIBLE, x, y, gridSizes[gameSize], gridSizes[gameSize], mainHandle, nullptr, hInst, nullptr);
            ShowWindow(child, nCmdShow);
            x += gridSizes[gameSize] + 10;
            // the coulour at start is NULL
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
    //move the rectangle to preserve the symmetry
    spawn.x -= change;
    spawn.y -= change;
    MoveWindow(handle, spawn.x, spawn.y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}


void InitGame(HWND mainHandle)
{
    if (initializingGame || changingGrid) return;
    for (size_t i = 0; i < gridCount[gameSize]; i++)
    {
        for (size_t j = 0; j < gridCount[gameSize]; j++)
        {
            grid[i][j].second = NULL;
            gemsToDestroy[i][j] = false;
        }
    }
    InvalidateRect(mainHandle, NULL, TRUE);
    SetTimer(mainHandle, TIMER_GAMEINIT, INTERVAL_TIMER_GAMEINIT, NULL);
    gameStarted = initializingGame  = true;
}


bool ValidateMove(HWND selectedWindow, HWND validatedWindow)
{
    POINT selectedCoord{-1, -1}, validatedCoord{-1, -1};
    // Finds the postions of elements
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


bool MoveGrid(HWND hwnd)
{
    bool foundAny = false;

    
    for (int i = 0; i < gridCount[gameSize]; i++)
    {
        int j = 0;
        //for each column finds the first grid element which was 'destroyed'
        for (; j < gridCount[gameSize] && !gemsToDestroy[j][i]; j++);
        //if there was none continues
        if (j == gridCount[gameSize]) continue;
        foundAny = true;
        // resets the flag for the found grid element
        gemsToDestroy[j][i] = false;
        // swaps the colours of elements one by one from bottom to top, and randomly chooses a colour for the top most element
        // only the elements for which the colour was changed are reprinted
        for (; j > 0; j--)
        {
            grid[j][i].second = grid[j - 1][i].second;
            InvalidateRect(grid[j][i].first, NULL, TRUE);
            UpdateWindow(grid[j][i].first);
        }
        grid[0][i].second = colours[rand() % (sizeof(colours) / sizeof(COLORREF))];
        InvalidateRect(grid[0][i].first, NULL, TRUE);
        UpdateWindow(grid[0][i].first);
    }
    return foundAny;
}

// returns the position of the left-top edge of a grid element in the main window client coordinates
POINT GetElementPosition(int idx, int idy)
{
    int x, y;
    x = MARGIN + idy * (gridSizes[gameSize] + 2 * MARGIN);
    y = MARGIN + idx * (gridSizes[gameSize] + 2 * MARGIN);
    return POINT{ x, y };
}


void GenerateParticles(HWND mainHandle)
{
    srand(time(NULL));
    bool foundAny = false;
    for (size_t i = 0; i < gridCount[gameSize]; i++)
        for (size_t j = 0; j < gridCount[gameSize]; j++)
        {
            if (gemsToDestroy[i][j])
            {
                //finds the position and translates it to screen coord
                POINT elmPos = GetElementPosition(i, j);
                MapWindowPoints(mainHandle, overlayHandle, &elmPos, 1);
                //spawns PARTICLE_COUNT particles in the place of an grid element
                for (int k = 0; k < pow(PARTICLES_COUNT, 0.5); k++)
                {
                    for (int l = 0; l < pow(PARTICLES_COUNT, 0.5); l++)
                    {
                        // the direction depends on the position of the particle + random factor
                        double x = -1 + 2 * (l / (double)gridCount[gameSize]) - (0.5 / RAND_FACTOR) + rand() / (double)(RAND_FACTOR * RAND_MAX);
                        double y = -1 + 2 * (k / (double)gridCount[gameSize]) - (0.5 / RAND_FACTOR) + rand() / (double)(RAND_FACTOR * RAND_MAX);
                        // then it's normalized for the vector to have a lenght of one
                        double lenght = pow(pow(x, 2) + pow(y, 2), 0.5);
                        x /= lenght;
                        y /= lenght;
                        particles.push_back(Particle
                            {
                                POINT{elmPos.x + l * particleSize, elmPos.y + k * particleSize},
                                std::make_pair(x, y),
                                grid[i][j].second
                            });
                    }
                }
            }
        }
}


void RemoveParticles()
{
    int maxWidth = GetSystemMetrics(SM_CXSCREEN);
    int maxHeight = GetSystemMetrics(SM_CYSCREEN);
    auto endIt = std::remove_if(particles.begin(), particles.end(), [&](Particle particle) 
        {
            return particle.position.x < 0 || particle.position.y < 0 ||
                particle.position.x > maxWidth || particle.position.y > maxHeight;
        });
    particles.resize(endIt - particles.begin());
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


ATOM MyRegisterOverlayClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = 0;
    wcex.lpfnWndProc = OverlayWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIGELAB1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"Overlay";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
