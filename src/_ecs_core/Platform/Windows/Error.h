#pragma once

#include "win32.h"

namespace Platform::Win {
    enum class Level : UINT8 {
        Success,        // OK
        Recoverable,    // Can retry / continue
        Fatal           // Must terminate
    };

    enum class Type : HRESULT {
        OperationSuccessful     = S_OK,
        OperationAborted        = E_ABORT,
        AccessDenied            = E_ACCESSDENIED,
        UnspecifiedFailure      = E_FAIL,
        InvalidHandle           = E_HANDLE,
        InvalidArguments        = E_INVALIDARG,
        NoSuchInterface         = E_NOINTERFACE,
        NotImplemented          = E_NOTIMPL,
        OutOfMemory             = E_OUTOFMEMORY,
        InvalidPointer          = E_POINTER,
        UnexpectedFailure       = E_UNEXPECTED
    };

    struct Result {
        Level level;
        Type type;
        const bool succeeded() const;
    };

    constexpr Result success() { return Result{ Level::Success, Type::OperationSuccessful }; }
    constexpr Result recoverable(Type _type = Type::UnexpectedFailure) { return Result{ Level::Recoverable, _type }; }
    constexpr Result fatal(Type _type = Type::UnexpectedFailure) { return Result{ Level::Fatal, _type }; }
}