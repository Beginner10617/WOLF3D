#include "path_utils.h"

#include <string>
#include <limits.h>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif

std::string getExeDir()
{
    char buf[PATH_MAX];

#if defined(_WIN32)
    DWORD len = GetModuleFileNameA(NULL, buf, PATH_MAX);
    if (len == 0 || len == PATH_MAX)
        return "";

#elif defined(__APPLE__)
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) != 0)
        return "";

#elif defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len == -1)
        return "";
    buf[len] = '\0';
#endif

    std::string path(buf);
    return path.substr(0, path.find_last_of("/\\"));
}
