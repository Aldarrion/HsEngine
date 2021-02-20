#include "Render/ShaderManager.h"

#include "Render/Render.h"
#include "Common/Logging.h"
#include "Render/Shader.h"

#include "Render/hs_Shaderc.h"


#pragma warning(push)
#pragma warning(disable : 4530)
#include <fstream>
#pragma warning(pop)
#include <cstdio>

namespace hs
{

//------------------------------------------------------------------------------
static constexpr const char* FRAG_EXT = "fs";
static constexpr const char* VERT_EXT = "vs";
static constexpr const char* PATH_PREFIX = "../Engine/Shaders/%s";
static constexpr const char* SHADER_BIN_DIR = "Shaders";

//------------------------------------------------------------------------------
const char* ShadercStatusToString(shaderc_compilation_status status)
{
    switch(status)
    {
        case shaderc_compilation_status_success: return "shaderc_compilation_status_success";
        case shaderc_compilation_status_invalid_stage: return "shaderc_compilation_status_invalid_stage";
        case shaderc_compilation_status_compilation_error: return "shaderc_compilation_status_compilation_error";
        case shaderc_compilation_status_internal_error: return "shaderc_compilation_status_internal_error";
        case shaderc_compilation_status_null_result_object: return "shaderc_compilation_status_null_result_object";
        case shaderc_compilation_status_invalid_assembly: return "shaderc_compilation_status_invalid_assembly";
        case shaderc_compilation_status_validation_error: return "shaderc_compilation_status_validation_error";
        case shaderc_compilation_status_transformation_error: return "shaderc_compilation_status_transformation_error";
        case shaderc_compilation_status_configuration_error: return "shaderc_compilation_status_configuration_error";
        default: return "UNKNOWN CODE";
    }
}

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
shaderc_include_result* ShaderIncludeResolver(
    void* userData, const char* requestedSource, int type,
    const char* requestingSource, size_t includeDepth)
{
    auto result = new shaderc_include_result();
    result->source_name = requestedSource;
    result->source_name_length = strlen(requestedSource);

    char filePath[128];
    sprintf(filePath, PATH_PREFIX, requestedSource);

    char* buffer{};
    size_t size{};
    if (HS_FAILED(ReadFile(filePath, false, &buffer, size)))
        return nullptr;

    result->content = buffer;
    result->content_length = size;
    result->user_data = buffer;

    return result;
}

//------------------------------------------------------------------------------
void ShaderIncludeReleaser(void* userData, shaderc_include_result* includeResult)
{
    free(userData);
    delete includeResult;
}

//------------------------------------------------------------------------------
RESULT ShaderManager::Init()
{
    shadercCompiler_ = shaderc_compiler_initialize();
    if (!shadercCompiler_)
    {
        Log(LogLevel::Error, "Could not create shaderc compiler");
        return R_FAIL;
    }

    opts_ = shaderc_compile_options_initialize();
    shaderc_compile_options_set_source_language(opts_, shaderc_source_language_hlsl);
    shaderc_compile_options_set_optimization_level(opts_, shaderc_optimization_level_performance);
    shaderc_compile_options_set_include_callbacks(opts_, &ShaderIncludeResolver, &ShaderIncludeReleaser, nullptr);

    return R_OK;
}

//------------------------------------------------------------------------------
ShaderManager::~ShaderManager()
{
    shaderc_compiler_release(shadercCompiler_);
    shaderc_compile_options_release(opts_);

    for (const auto& it : cache_)
        vkDestroyShaderModule(g_Render->GetDevice(), it.second->vkShader_, nullptr);
    cache_.clear();

    for (int i = 0; i < toDestroy_.Count(); ++i)
        vkDestroyShaderModule(g_Render->GetDevice(), toDestroy_[i], nullptr);
}

//------------------------------------------------------------------------------
RESULT ShaderManager::CompileShader(const char* file, PipelineStage type, Shader& shader) const
{
    Log(LogLevel::Info, "---- Compiling shader %s ----", file);

    char* buffer{};
    size_t size{};
    if (HS_FAILED(ReadFile(file, false, &buffer, size)))
        return R_FAIL;

    shaderc_shader_kind kind = type == PipelineStage::PS_VERT ? shaderc_glsl_vertex_shader : shaderc_glsl_fragment_shader;

    const char* values[2] = { "0", "0" };
    if (type == PipelineStage::PS_VERT)
        values[0] = "1";
    else
        values[1] = "1";

    shaderc_compile_options_add_macro_definition(opts_, "VS", 2, values[0], 1);
    shaderc_compile_options_add_macro_definition(opts_, "PS", 2, values[1], 1);
    shaderc_compilation_result_t result = shaderc_compile_into_spv(shadercCompiler_, buffer, size, kind, file, "main", opts_);
    free(buffer);

    shaderc_compilation_status status = shaderc_result_get_compilation_status(result);

    const char* msg = shaderc_result_get_error_message(result);
    auto warningCount = shaderc_result_get_num_warnings(result);
    auto errorCount = shaderc_result_get_num_errors(result);
    LogLevel resultLevel = LogLevel::Info;
    if (errorCount > 0)
        resultLevel = LogLevel::Error;
    else if (warningCount > 0)
        resultLevel = LogLevel::Warning;

    if (msg && *msg)
        Log(resultLevel, msg);
    Log(resultLevel, "Done with %d errors, %d warnings", errorCount, warningCount);

    if (status != shaderc_compilation_status_success)
    {
        shaderc_result_release(result);
        Log(LogLevel::Error, "Shader creation failed, %s", ShadercStatusToString(status));
        return R_FAIL;
    }

    VkShaderModuleCreateInfo shaderInfo{};
    shaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = shaderc_result_get_length(result);
    shaderInfo.pCode    = (uint*)shaderc_result_get_bytes(result);

    if (VKR_FAILED(vkCreateShaderModule(g_Render->GetDevice(), &shaderInfo, nullptr, &shader.vkShader_)))
    {
        shaderc_result_release(result);
        return R_FAIL;
    }

    shaderc_result_release(result);
    return R_OK;
}

//------------------------------------------------------------------------------
RESULT ShaderManager::CreateShader(const char* name, PipelineStage type, Shader* shader)
{
    hs_assert(shader);

    constexpr uint BUFF_LEN = 1024;
    static char path[BUFF_LEN];
    int written = snprintf(path, BUFF_LEN, PATH_PREFIX, name);

    hs_assert(written > 0);

    if (CompileShader(path, type, *shader) != R_OK)
        return R_FAIL;

    return R_OK;
}

//------------------------------------------------------------------------------
Shader* ShaderManager::GetOrCreateShader(const char* name)
{
    auto val = cache_.find(name);
    if (val != cache_.end())
        return val->second;

    const uint nameLen = (uint)strlen(name);
    if (nameLen < 4)
        Log(LogLevel::Error, "Invalid shader name: %s, name must end with valid pipeline stage extension such as _ps or _vs", name);

    const char* ext = name + nameLen - 2;

    PipelineStage stage;
    if (strncmp(ext, FRAG_EXT, 2) == 0)
    {
        stage = PipelineStage::PS_FRAG;
    }
    else if (strncmp(ext, VERT_EXT, 2) == 0)
    {
        stage = PipelineStage::PS_VERT;
    }
    else
    {
        Log(LogLevel::Error, "Invalid shader stage extension %s", ext);
        return nullptr;
    }

    // Shader not found in cache, create it and add to cache
    Shader* shader = new Shader();
    if (LoadShader(name, shader) != R_OK)
    {
        char srcFileName[256];
        sprintf(srcFileName, "%s.hlsl", name);

        if (CreateShader(srcFileName, stage, shader) != R_OK)
        {
            delete shader;
            return nullptr;
        }
    }

    shader->type_ = stage;
    shader->id_ = ++shaderId_[stage]; // Pre increment to start with id 1
    cache_.emplace(name, shader);

    return shader;
}

//------------------------------------------------------------------------------
RESULT ShaderManager::ReloadShaders()
{
    // TODO check timestamps of files to avoid reloading of all shaders

    bool reloadFailed = false;
    Shader s;
    for (const auto& it : cache_)
    {
        if (CreateShader(it.first, it.second->type_, &s) != R_OK)
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
RESULT ShaderManager::LoadShader(const char* name, Shader* shader)
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

    return R_OK;
}

}

