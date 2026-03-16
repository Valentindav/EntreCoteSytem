#pragma once

#include <Windows.h>
#include <string>
#include <functional>
#include <memory>

class Window
{
public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool Initialize(HINSTANCE hInstance, int width, int height, const std::wstring& caption);

    bool ProcessMessages();

    HWND GetHwnd() const { return mhMainWnd; }
    int GetClientWidth() const { return mClientWidth; }
    int GetClientHeight() const { return mClientHeight; }
    bool IsPaused() const { return mAppPaused; }
    std::wstring GetCaption() const { return mMainWndCaption; }
    float GetAspectRatio() const { return static_cast<float>(mClientWidth) / mClientHeight; }

    std::function<void(int, int)> OnResize;
    std::function<void(bool)> OnFocusChange;

    static Window* GetInstance() { return sInstance; }

    void SetWindowTitle(const std::wstring& text);

private:
    bool InitMainWindow();
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    static Window* sInstance;

    HINSTANCE mhAppInst = nullptr;
    HWND mhMainWnd = nullptr;
    std::wstring mMainWndCaption;

    int mClientWidth;
    int mClientHeight;

    bool mAppPaused = false;
    bool mResizing = false;
    bool mMaximized = false;
    bool mMinimized = false;
};