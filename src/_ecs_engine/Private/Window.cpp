#include "Window.h"
#include "Public/Inputs.h"
#include <WindowsX.h>
#include <cassert>

Window* Window::sInstance = nullptr;

Window::~Window()
{
    sInstance = nullptr;
}

bool Window::Initialize(HINSTANCE hInstance, int width, int height, const std::wstring& caption)
{
    mhAppInst = hInstance;
    mClientWidth = width;
    mClientHeight = height;
    mMainWndCaption = caption;
    mhMainWnd = nullptr;

    assert(sInstance == nullptr && "Une seule Window autorisée pour le moment");
    sInstance = this;

    return InitMainWindow();
}

bool Window::ProcessMessages()
{
    MSG msg = { 0 };
    // On pompe tous les messages en attente
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false; // Demande de fermeture
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void Window::SetWindowTitle(const std::wstring& text)
{
    SetWindowText(mhMainWnd, text.c_str());
}

bool Window::InitMainWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = mhAppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"MainWnd";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    // Calculer la taille de la fenêtre avec les bordures
    RECT R = { 0, 0, mClientWidth, mClientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);

    if (!mhMainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(mhMainWnd, SW_SHOW);
    UpdateWindow(mhMainWnd);

    return true;
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Inputs::ProcessWindowsMessage(msg, wParam, lParam);

    Window* win = Window::GetInstance();

    if (!win) return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (Inputs::IsMouseLocked() || !Inputs::IsCursorVisible())
            {
                SetCursor(NULL);
                return TRUE;
            }
        }
        break;

    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            win->mAppPaused = true;
            if (win->OnFocusChange) win->OnFocusChange(false);
        }
        else
        {
            win->mAppPaused = false;
            if (win->OnFocusChange) win->OnFocusChange(true);
        }
        return 0;

    case WM_SIZE:
        win->mClientWidth = LOWORD(lParam);
        win->mClientHeight = HIWORD(lParam);

        if (wParam == SIZE_MINIMIZED)
        {
            win->mAppPaused = true;
            win->mMinimized = true;
            win->mMaximized = false;
        }
        else if (wParam == SIZE_MAXIMIZED)
        {
            win->mAppPaused = false;
            win->mMinimized = false;
            win->mMaximized = true;
            if (win->OnResize) win->OnResize(win->mClientWidth, win->mClientHeight);
        }
        else if (wParam == SIZE_RESTORED)
        {
            if (win->mMinimized)
            {
                win->mAppPaused = false;
                win->mMinimized = false;
                if (win->OnResize) win->OnResize(win->mClientWidth, win->mClientHeight);
            }
            else if (win->mMaximized)
            {
                win->mAppPaused = false;
                win->mMaximized = false;
                if (win->OnResize) win->OnResize(win->mClientWidth, win->mClientHeight);
            }
            else if (win->mResizing)
            {
                // Si on drag les bords, on ne fait rien tant que l'utilisateur n'a pas lâché (EXITSIZEMOVE)
                // pour éviter de spammer le GPU de reconstruction de SwapChain
            }
            else
            {
                if (win->OnResize) win->OnResize(win->mClientWidth, win->mClientHeight);
            }
        }
        return 0;

    case WM_ENTERSIZEMOVE:
        win->mAppPaused = true;
        win->mResizing = true;
        return 0;

    case WM_EXITSIZEMOVE:
        win->mAppPaused = false;
        win->mResizing = false;
        if (win->OnResize) win->OnResize(win->mClientWidth, win->mClientHeight);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_MENUCHAR:
        return MAKELRESULT(0, MNC_CLOSE);

    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}