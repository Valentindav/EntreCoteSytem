#pragma once
#include "Private/NativeScript.h"

#define START_SCRIPT(ScriptName) \
class ScriptName : public NativeScript { \
public:

#define END_SCRIPT(ScriptName) \
};