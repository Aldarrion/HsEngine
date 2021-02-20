#pragma once
#include "Config.h"

namespace hs
{

struct Mat44;

//------------------------------------------------------------------------------
enum class LogLevel
{
    Info,
    Warning,
    Error
};

//------------------------------------------------------------------------------
void Log(LogLevel level, const char* formatString, ...);

//------------------------------------------------------------------------------
void Mat44ToString(const Mat44& m, char* buff);

//------------------------------------------------------------------------------
void LogMat44(const Mat44& m);

}

#define LOG_DBG(msg, ...) hs::Log(hs::LogLevel::Info, msg, ## __VA_ARGS__)
#define LOG_ERR(msg, ...) hs::Log(hs::LogLevel::Error, msg, ## __VA_ARGS__)

