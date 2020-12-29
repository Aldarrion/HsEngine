#pragma once

#include "Containers/Array.h"
#include "Containers/Hash.h"

#include "Common/Pointers.h"
#include "Common/Enums.h"

#include <unordered_map>

namespace hs
{

//------------------------------------------------------------------------------
class Texture;
class Material;

//------------------------------------------------------------------------------
extern class ResourceManager* g_ResourceManager;

//------------------------------------------------------------------------------
RESULT CreateResourceManager();

//------------------------------------------------------------------------------
void DestroyResourceManager();

//------------------------------------------------------------------------------
class ResourceManager
{
public:
    #if HS_WINDOWS
        RESULT InitWin32();
    #endif

    RESULT LoadTexture2D(const char* path, Texture** texture);

    void Free();

private:
    // TODO use custom hashmap
    std::unordered_map<const char*, Texture*, StrHash<const char*>, StrCmpEq<const char*>> textures_;
};

}
