#include "error.h"

namespace Platform::Win {
    const bool Result::succeeded() const {
        return SUCCEEDED(HRESULT(type));
    }
}