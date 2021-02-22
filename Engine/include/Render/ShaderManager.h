#pragma once

#include "Config.h"

#include "Render/VkTypes.h"

#include "String/String.h"
#include "Containers/Array.h"
#include "Containers/Hash.h"

#include "Common/Enums.h"

#include <unordered_map>

struct shaderc_compiler;
struct shaderc_compile_options;


namespace hs
{

class Shader;

//------------------------------------------------------------------------------
class ShaderManager
{
public:
    RESULT Init();
    ~ShaderManager();

    Shader* GetOrCreateShader(const char* name);
    RESULT CompileShaders();
    RESULT ReloadShaders();

private:
    // TODO use custom hashmap
    std::unordered_map<const char*, Shader*, StrHash<const char*>, StrCmpEq<const char*>> cache_;

    Array<VkShaderModule>       toDestroy_;
    shaderc_compiler*           shadercCompiler_{};
    shaderc_compile_options*    opts_{};

    uint16 shaderId_[PS_COUNT]{};

    RESULT LoadShader(const char* name, PipelineStage type, Shader* shader);
};

}
