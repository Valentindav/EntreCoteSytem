#include "win.h"

namespace Platform::Win {
    Result RegisterWindowClass(WNDCLASS* _class, HMODULE _inst, const wchar_t* _name, WNDPROC _proc) {
        if (_class == nullptr)
            return fatal(Type::InvalidPointer);

        if (_name == nullptr)
            return fatal(Type::InvalidArguments);

        _class->lpfnWndProc = _proc;
        _class->hInstance = _inst;
        _class->lpszClassName = _name;

        if (!RegisterClass(_class))
            return fatal(Type::UnexpectedFailure);

        return success();
    }

    Result CreateWindowFromClass(HWND* _handle, const WNDCLASS* _class, HMODULE _inst, WindowDescriptor _desc, LPVOID _appdata) {
        if(_handle == nullptr)
            return fatal(Type::InvalidPointer);

        if (_class == nullptr)
            return fatal(Type::InvalidPointer);
		
        *_handle = CreateWindowEx(
            0,                      // Optional window styles.
            _class->lpszClassName,  // Window class
            _desc.title,			// Window text
            _desc.style,    // Window style

            // Position and size
            _desc.x, _desc.y, _desc.width, _desc.height,

            nullptr,		// Parent window    
            nullptr,		// Menu
            _inst,		// Instance handle
            _appdata	// Additional application data
        );

        if (*_handle == nullptr)
            return fatal(Type::UnexpectedFailure);

        return success();
    }
}