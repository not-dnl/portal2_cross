#include "../shared/log/log.h"
#include "../shared/memory/memory.h"
#include "../portal2/helper/interfaces.h"
#include "hook/manager/hooking_manager.h"

// could move this somewhere else
void cvar_constructor(c_con_var* cvar, const char* name,
        const char* default_value, int flags,
        const char* help_string, bool has_min,
        float min, bool has_max, float max)
{
    static uintptr_t cvar_constructor_fn{};
    if (!cvar_constructor_fn)
    {
        // we're getting the address of the cvar constructor from memory
        cvar_constructor_fn = memory::find_pattern(
                L_W("engine.so", "engine.dll"),
                L_W("55 89 E5 57 56 E8 ? ? ? ? 81 C6 ? ? ? ? 53 83 EC 20 8B 45 1C 8B 5D 08 89 45 E4 8B 45 24 8D 96 ? ? ? ?",
                        "55 8B EC F3 0F 10 45 ? 8B 55 14"), 0, "cvar_constructor");
        if (!cvar_constructor_fn)
        {
            LOG("Unable to get cvar_constructor pattern, it returned 0");

            return;
        }
    }

    // so that we can call it with our own parameters and create our own cvar.
    reinterpret_cast<void (__thiscall*)(
            c_con_var*, const char*, const char*, int, const char*, bool, float, bool, float)>(
            cvar_constructor_fn)(cvar, name, default_value, flags, help_string, has_min, min, has_max, max);
}

// this con_var needs to be global, if it goes out of scope the pointer becomes invalid
// we could put them in a container, but in this example now we only have one feature anyways
c_con_var auto_bhop;

#ifdef _WIN32

#include <thread>
#include <Windows.h>

BOOL WINAPI detach()
{
    hooking_manager::restore();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    logs::detach();

    return TRUE;
}

DWORD WINAPI entry(LPVOID lpThreadParameter)
{
    LOG("Attached!");

    interfaces::get();
    hooking_manager::initialize();

    cvar_constructor(&auto_bhop, "auto_bhop", "0", 0,
            "Player jumps automatically once jump button is held and on ground.", true, 0, true, 1);
    interfaces::get().con_var->register_con_command(&auto_bhop);

    while (!(GetAsyncKeyState(VK_F12) & 1))
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

    LOG("Detached!");

    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    detach();

    FreeLibraryAndExitThread(static_cast<HMODULE>(lpThreadParameter), EXIT_SUCCESS);
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstDLL);

        if (auto handle = CreateThread(nullptr, 0, entry, hinstDLL, 0, nullptr))
            CloseHandle(handle);
    }
    else if (fdwReason == DLL_PROCESS_DETACH && !lpvReserved)
    {
        return detach();
    }

    return TRUE;
}

#elif __linux__

void __attribute__((constructor)) portal2_cross_attach()
{
    LOG("Attached!");

    interfaces::get();

    hooking_manager::initialize();

    cvar_constructor(&auto_bhop, "auto_bhop", "0", 0,
            "Player jumps automatically once jump button is held and on ground.", true, 0, true, 1);
    interfaces::get().con_var->register_con_command(&auto_bhop);
}

void __attribute__((destructor)) portal2_cross_detach()
{
    LOG("Detached!");

    hooking_manager::restore();
}

#endif
