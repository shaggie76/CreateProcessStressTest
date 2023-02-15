#include <windows.h>
#include <tchar.h>

#define ARRAY_COUNT _countof

template<typename T>
inline void SafeCloseHandle(T& p)
{
    if(p)
    {
        CloseHandle(p);
        p = nullptr;
    }
}

int main()
{
    int argc = 0;
    TCHAR** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if(argc == 1)
    {
        return(0);
    }

    if(argc == 2)
    {
        size_t count = 0;
        if((_stscanf_s(argv[1], TEXT("%zd"), &count) != 1) || !count)
        {
            return(1);
        }

        // Single threaded fork loop:
        TCHAR cmdline[MAX_PATH];
        GetModuleFileName(GetModuleHandle(nullptr), cmdline, ARRAY_COUNT(cmdline));

        STARTUPINFO startupInfo = {};
        startupInfo.cb = sizeof(startupInfo);

        PROCESS_INFORMATION processInfo = {};

        for(size_t i = 0; i < count; ++i)
        {
            bool spawned = CreateProcess
            (
                cmdline,
                cmdline,
                nullptr,
                nullptr,
                FALSE,
                NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                nullptr,
                nullptr,
                &startupInfo,
                &processInfo
            );

            if(spawned && processInfo.hProcess)
            {
                WaitForSingleObject(processInfo.hProcess, INFINITE);
            }

            SafeCloseHandle(processInfo.hProcess);
            SafeCloseHandle(processInfo.hThread);

            if(!spawned)
            {
                return(1);
            }
        }

        return(0);
    }

    if((argc == 3) && !_tcsicmp(argv[1], TEXT("--fork")))
    {
        size_t count = 0;
        if((_stscanf_s(argv[2], TEXT("%zd"), &count) != 1) || !count)
        {
            return(1);
        }

        PROCESS_INFORMATION processInfo[32] = {};

        SYSTEM_INFO systemInfo = {};
        GetSystemInfo(&systemInfo);
        size_t forks = systemInfo.dwNumberOfProcessors / 2; // Assume HT
        
        if(forks > ARRAY_COUNT(processInfo))
        {
            forks = ARRAY_COUNT(processInfo);
        }
        
        const size_t countPerFork = count / forks;

        if(!forks || !countPerFork)
        {
            return(1);
        }

        // Single threaded fork loop:
        TCHAR executable[MAX_PATH];
        GetModuleFileName(GetModuleHandle(nullptr), executable, ARRAY_COUNT(executable));
        
        TCHAR cmdline[MAX_PATH *2];
        DWORD cmdlineLen = GetModuleFileName(GetModuleHandle(nullptr), cmdline, ARRAY_COUNT(cmdline));
        _stprintf_s(cmdline + cmdlineLen, ARRAY_COUNT(cmdline) - cmdlineLen, TEXT(" %zd"), countPerFork);

        STARTUPINFO startupInfo = {};
        startupInfo.cb = sizeof(startupInfo);

        for(size_t i = 0; i < forks; ++i)
        {
            bool spawned = CreateProcess
            (
                executable,
                cmdline,
                nullptr,
                nullptr,
                FALSE,
                NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                nullptr,
                nullptr,
                &startupInfo,
                &processInfo[i]
            );

            if(!spawned)
            {
                return(1);
            }
        }

        for(size_t i = 0; i < forks; ++i)
        {
            if(processInfo[i].hProcess)
            {
                WaitForSingleObject(processInfo[i].hProcess, INFINITE);
            }

            SafeCloseHandle(processInfo[i].hProcess);
            SafeCloseHandle(processInfo[i].hThread);
        }

        return(0);
    }

    return(1);
}
