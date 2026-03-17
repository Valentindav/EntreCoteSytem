#pragma once

#include <Windows.h>
#include <string>
#include <functional>
#include <memory>

class Window
{
public:
    Window() = default;
    
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool Initialize(int width, int height, const std::wstring& title);

    bool ProcessMessages();

    HWND GetNativeHandle() const { return mHandle; }
    int GetClientWidth() const { return mClientWidth; }
    int GetClientHeight() const { return mClientHeight; }
    bool IsPaused() const { return mAppPaused; }
    std::wstring GetCaption() const { return mTitle; }
    float GetAspectRatio() const { return static_cast<float>(mClientWidth) / static_cast<float>(mClientHeight); }

    std::function<void(int, int)> OnResize;
    std::function<void(bool)> OnFocusChange;

    void SetWindowTitle(const std::wstring& text);

private:
    bool InitMainWindow();
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HWND mHandle = nullptr;
    
    std::wstring mTitle;
    int mClientWidth = 0;
    int mClientHeight = 0;

    bool mAppPaused = false;
    bool mResizing = false;
    bool mMaximized = false;
    bool mMinimized = false;
};