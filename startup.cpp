#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#else
#include <unistd.h>
#endif

std::filesystem::path getExecutableDir()
{
#if defined(_WIN32)
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();

#elif defined(__APPLE__)
    char path[1024];
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size);
    return std::filesystem::path(path).parent_path();

#else // Linux
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path));
    path[len] = '\0';
    return std::filesystem::path(path).parent_path();
#endif
}

/*Load all resources from the pkg file here*/