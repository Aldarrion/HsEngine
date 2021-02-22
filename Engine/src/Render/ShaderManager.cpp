#include "Render/ShaderManager.h"

#include "Render/Render.h"
#include "Render/Shader.h"

#include "Common/Logging.h"

#include <cstdio>
#include <cstring>

namespace hs
{

//------------------------------------------------------------------------------
static constexpr const char* FRAG_EXT = "fs";
static constexpr const char* VERT_EXT = "vs";
static constexpr const char* PATH_PREFIX = "../Engine/Shaders/%s";
static constexpr const char* SHADER_BIN_DIR = "Shaders";

//------------------------------------------------------------------------------
RESULT ReadFile(const char* file, bool binary, char** buffer, size_t& size)
{
    const char* mode = "r";
    if (binary)
        mode = "rb";
    FILE* f = fopen(file, mode);
    if (!f)
    {
        Log(LogLevel::Error, "Failed to open file %s", file);
        return R_FAIL;
    }

    fseek(f , 0 , SEEK_END);
    auto fileSize = ftell(f);
    rewind(f);

    *buffer = (char*)malloc(fileSize);
    if (!*buffer)
    {
        fclose(f);
        free(*buffer);
        Log(LogLevel::Error, "Failed to alloc space for shader file");
        return R_FAIL;
    }

    size_t readRes = fread(*buffer, 1, fileSize, f);
    auto eof = feof(f);
    if (readRes != fileSize && !eof)
    {
        free(*buffer);
        fclose(f);
        Log(LogLevel::Error, "Failed to read the shader file, error %d", ferror(f));
        return R_FAIL;
    }
    fclose(f);

    size = readRes;

    return R_OK;
}

//------------------------------------------------------------------------------
RESULT ShaderManager::Init()
{
    return R_OK;
}

//------------------------------------------------------------------------------
ShaderManager::~ShaderManager()
{
    for (const auto& it : cache_)
        vkDestroyShaderModule(g_Render->GetDevice(), it.second->vkShader_, nullptr);
    cache_.clear();

    for (int i = 0; i < toDestroy_.Count(); ++i)
        vkDestroyShaderModule(g_Render->GetDevice(), toDestroy_[i], nullptr);
}

//------------------------------------------------------------------------------
Result<PipelineStage> GetTypeFromName(const char* name)
{
    const uint nameLen = (uint)strlen(name);
    if (nameLen < 4)
        Log(LogLevel::Error, "Invalid shader name: %s, name must end with valid pipeline stage extension such as _ps or _vs", name);

    const char* ext = name + nameLen - 2;

    PipelineStage stage;
    if (strncmp(ext, FRAG_EXT, 2) == 0)
    {
        stage = PS_FRAG;
    }
    else if (strncmp(ext, VERT_EXT, 2) == 0)
    {
        stage = PS_VERT;
    }
    else
    {
        Log(LogLevel::Error, "Invalid shader stage extension %s", ext);
        return Err<PipelineStage>();
    }

    return Ok(stage);
}

//------------------------------------------------------------------------------
Shader* ShaderManager::GetOrCreateShader(const char* name)
{
    auto val = cache_.find(name);
    if (val != cache_.end())
        return val->second;

    auto stageRes = GetTypeFromName(name);
    if (!stageRes)
        return nullptr;

    const PipelineStage stage = stageRes.GetValue();

    // Shader not found in cache, create it and add to cache
    Shader* shader = new Shader();
    if (LoadShader(name, stage, shader) != R_OK)
    {
        delete shader;
        return nullptr;
    }

    cache_.emplace(name, shader);

    return shader;
}

//------------------------------------------------------------------------------
RESULT ShaderManager::CompileShaders()
{
    LOG_DBG("Compiling shaders");
    #if HS_WINDOWS
        char dir[512];
        sprintf(dir, "%s", __FILE__);

        for (int i = 0; i < 4; ++i)
        {
            char* lastBs = strrchr(dir, '\\');
            *lastBs = 0;
        }

        char cmd[512];
        sprintf(cmd, "%s\\BuildShaders.bat Shaders %s\\Shaders", dir, dir);

        FILE* compileOut{};
        compileOut = _popen(cmd, "r");
        if (!compileOut)
        {
            LOG_ERR("Failed to run compile script");
            return R_FAIL;
        }

        char line[512];
        while (fgets(line, sizeof(line), compileOut))
            OutputDebugStringA(line);

        _pclose(compileOut);
    #else
        HS_NOT_IMPLEMENTED
    #endif

    LOG_DBG("Shader compile done");

    const RESULT reloadRes = ReloadShaders();
    return reloadRes;
}

//------------------------------------------------------------------------------
RESULT ShaderManager::ReloadShaders()
{
    // TODO check timestamps of files to avoid reloading all the shaders

    Log(LogLevel::Info, "Reloading shaders");

    bool reloadFailed = false;
    Shader s;
    for (const auto& it : cache_)
    {
        if (LoadShader(it.first, it.second->type_, &s) != R_OK)
        {
            reloadFailed = true;
            continue;
        }

        toDestroy_.Add(it.second->vkShader_);
        it.second->vkShader_ = s.vkShader_;
    }

    if (reloadFailed)
    {
        Log(LogLevel::Error, "Shader realod failed, see errors above");
        return R_FAIL;
    }
    else
    {
        Log(LogLevel::Info, "Shader realod done");
        return R_OK;
    }
}

//------------------------------------------------------------------------------
RESULT ShaderManager::LoadShader(const char* name, PipelineStage type, Shader* shader)
{
    char filePath[256];
    sprintf(filePath, "%s/%s.spv", SHADER_BIN_DIR, name);

    char* shaderBytecode;
    size_t fileSize;
    if (HS_FAILED(ReadFile(filePath, true, &shaderBytecode, fileSize)))
        return R_FAIL;

    VkShaderModuleCreateInfo shaderInfo{};
    shaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = fileSize;
    shaderInfo.pCode    = (uint*)shaderBytecode;

    if (HS_FAILED(vkCreateShaderModule(g_Render->GetDevice(), &shaderInfo, nullptr, &shader->vkShader_)))
    {
        free(shaderBytecode);
        LOG_ERR("Failed to create shader module");
        return R_FAIL;
    }

    shader->id_ = ++shaderId_[type]; // Pre increment to start with id 1
    shader->type_ = type;

    return R_OK;
}

}

