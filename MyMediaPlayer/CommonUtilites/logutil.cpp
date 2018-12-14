#include "logutil.h"
#include <unordered_map>

const static std::unordered_map<int, std::string> g_logTypeStringMap=
{
    {logutil::trace,    "trace"},
    {logutil::debug,    "debug"},
    {logutil::info,     "info "},
    {logutil::warn,     "warn "},
    {logutil::err,      "err  "},
    {logutil::fatal,    "fatal "},
};

std::string logutil::LogTypeToString(logutil::MyLogType type)
{
    return g_logTypeStringMap.at(type);
}
