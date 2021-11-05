#ifdef _WIN32

#include <Windows.h>
#include <chrono>

#elif __linux__

#include <fstream>

#endif

#include <string>
#include "log.h"

namespace logs
{
#ifdef _WIN32
    bool init = false;
#endif

    void print(const char* text)
    {
#ifdef _WIN32
        if (!init) // did we already allocate a console?
        {
            // allocate console
            AllocConsole();
            AttachConsole(GetCurrentProcessId());

            // redirect I/O to console
            freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
            freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);

            SetConsoleTitleA("portal2_cross");

            init = true;
        }

        std::puts(text); // could add a timestamp some day
#elif __linux__
        auto stream = std::ofstream("/dev/pts/1"); // has to be set accordingly
        if (!stream.is_open())
            return; // if you got here, open a terminal.

        stream << text << std::endl;

        stream.close();
#endif
    }

    void print(const std::string& text)
    {
        print(text.c_str());
    }

#ifdef _WIN32

    void detach()
    {
        if (!init) // did we even allocate a console?
            return;

        // handles
        fclose(stdout);
        fclose(stdin);

        // console
        FreeConsole();
        PostMessageW(GetConsoleWindow(), WM_CLOSE, 0, 0);
    }

#endif
}