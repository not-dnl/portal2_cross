#include <vector>
#include <unordered_map>
#include <string>
#include "interfaces.h"
#include "../../shared/log/log.h"

#ifdef __linux__

#include <dlfcn.h>

#endif

namespace interfaces
{
    typedef void* (* instantiate_interface_fn)();

    struct s_interface_node
    {
        instantiate_interface_fn create_fn;
        const char* name;
        s_interface_node* next;
    };

    struct s_interfaces_head
    {
        s_interface_node* head_node;
    };

    // have to wrap this in a function, otherwise we crash due to https://en.cppreference.com/w/cpp/language/siof
    // accessing the map throws a: 'Thread 1 "portal2_linux" received signal SIGFPE, Arithmetic exception.'
    // furthermore the division happens at:
    // /usr/bin/../lib64/gcc/x86_64-pc-linux-gnu/11.1.0/../../../../include/c++/11.1.0/bits/hashtable_policy.h:429
    // { return __num % __den; } due to __den being 0, since m._M_bucket_count is 0
    // the map is used before the constructor was run hence bucket count will be zero
    std::unordered_map<const char*, std::vector<std::pair<uintptr_t*, const char*>>>& get_interfaces()
    {
        ///                     - module_name                     - interface_ptr, interface_name
        static std::unordered_map<const char*, std::vector<std::pair<uintptr_t*, const char*>>> interfaces{};

        return interfaces;
    }

    bool get_interface_list(const char* module_name)
    {
        auto& entry = get_interfaces()[module_name];
        if (!entry.empty()) // already added the module's interface list to the map
            return true;
#ifdef _WIN32
        uintptr_t address{};
        memory::get_module_info(module_name, &address);

        auto create_interface_fn = reinterpret_cast<uintptr_t*>(memory::get_proc_address(address, "CreateInterface"));

        // find the jump to the main block of code
        create_interface_fn = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(create_interface_fn) + 0x4);

        // later on replace this + 4 with a search for the first jmp(0xE9)
        const auto jmp_conv = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(create_interface_fn) + 0x1);

        // + 1 to skip the opcode, jmp is relative to the end of this line (hence the +5 bytes)
        create_interface_fn = reinterpret_cast<uintptr_t*>(
                0x5 + reinterpret_cast<uintptr_t>(create_interface_fn) + jmp_conv);

        auto* const list = *reinterpret_cast<s_interfaces_head**>(reinterpret_cast<uintptr_t>(create_interface_fn) +
                                                                  0x6);
        if (!list)
        {
            LOG("Unable to get the InterfaceList in: " + std::string(module_name));

            return false;
        }

        for (auto* node = list->head_node; node; node = node->next)
            entry.emplace_back(std::make_pair(reinterpret_cast<uintptr_t*>(node->create_fn()), node->name));

        return true;


#elif __linux__
        uintptr_t address{}; // unused
        size_t size{};       // unused
        const char* full_module_path{};

        // we need the full path to get a handle
        memory::get_module_info(module_name, &address, &size, &full_module_path);

        // get a module handle
        auto h_module = dlopen(full_module_path, RTLD_NOLOAD | RTLD_NOW | RTLD_LOCAL);
        if (!h_module)
        {
            // could get more info why it failed by calling dlerror(), but it's not needed for now.
            LOG("Unable to get handle to: " + std::string(module_name));

            return false;
        }

        auto interface_regs = dlsym(h_module, "s_pInterfaceRegs"); // the game's linked list
        if (!interface_regs)
        {
            // same here, could get more info by calling dlerror().
            LOG("Unable to get s_pInterfaceRegs in: " + std::string(module_name));

            dlclose(h_module); // need to close the handle too.

            return false;
        }

        dlclose(h_module); // we're done, close the handle.

        // iterate over the list and store each name and base address to our map.
        auto interfaces = *reinterpret_cast<s_interface_node**>(interface_regs);
        for (auto interface = interfaces; interface; interface = interface->next)
            entry.emplace_back(std::make_pair(
                    reinterpret_cast<uintptr_t*>(interface->create_fn()), interface->name));

        return true;
#endif // _WIN32
    }

    uintptr_t* get_interface(const char* module_name, const char* interface_name)
    {
        if (!get_interface_list(
                module_name)) // for some reason we couldn't find the list. did you mistype the module name?
        {
            LOG("Unable to find Interface-List in: " + std::string(module_name));

            return nullptr;
        }

        for (auto& e: get_interfaces()[module_name]) // go over our map
        {
            auto e_interface_name = std::string(e.second);
            if (e_interface_name.find(interface_name) != std::string::npos) // found something
            {
                LOG("Found Interface: " + e_interface_name + " in Module: " + std::string(module_name));

                return e.first; // return the pointer to the interface
            }
        }

        LOG("Unable to find Interface: " + std::string(interface_name) + " in Module: " + std::string(module_name));

        return nullptr; // we couldn't find what we were looking for
    }

    s_interfaces& get()
    {
        static auto interfaces = s_interfaces();

        return interfaces;
    }
}