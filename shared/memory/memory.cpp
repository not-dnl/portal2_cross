#include <cstring>
#include <sstream>
#include <vector>
#include "memory.h"
#include "../log/log.h"

#ifdef _WIN32

#include <windows.h>
#include <winternl.h>
#include <algorithm>
#include "../hash/fnv-1a.h"

#elif __linux__

#include <link.h>

#endif

namespace memory
{
    struct s_module_info
    {
        size_t size{};
        uintptr_t address{};
        std::string module_name{};
    };

    // stores all info about modules of the process
    std::vector<s_module_info> modules{};

    // under windows full_path is actually just the module name, example: client.dll
    bool get_module_info(const char* module_name, uintptr_t* address, size_t* size, const char** full_path)
    {
        if (modules.empty()) // first time trying to get a module's info, time to grab everything
        {
#ifdef _WIN32
            // http://undocumented.ntinternals.net/index.html?page=UserMode%2FUndocumented%20Functions%2FNT%20Objects%2FProcess%2FPEB.html
            auto const peb = reinterpret_cast<PEB const* const>(__readfsdword(0x30));

            auto const list_head = &peb->Ldr->InMemoryOrderModuleList;
            auto current_module = reinterpret_cast<LDR_DATA_TABLE_ENTRY const*>(list_head->Flink);

            while (reinterpret_cast<std::ptrdiff_t const>(current_module) !=
                   reinterpret_cast<std::ptrdiff_t const>(list_head))
            {
                const auto mod_name = std::wstring_view{ current_module->FullDllName.Buffer };
                const auto name_buf = reinterpret_cast<char*>(_alloca(mod_name.size() + 1));

                if (!WideCharToMultiByte(CP_ACP, 0, mod_name.data(), static_cast<int>(mod_name.size()),
                        name_buf, static_cast<int>(mod_name.size()), nullptr, nullptr))
                    continue;

                name_buf[mod_name.size()] = '\0';

                auto module_address = reinterpret_cast<std::uintptr_t const>(current_module->Reserved2[0]);

                auto dos_header = PIMAGE_DOS_HEADER(module_address);
                auto nt_headers = PIMAGE_NT_HEADERS(
                        reinterpret_cast<std::uint8_t*>(module_address) + dos_header->e_lfanew);

                auto const module_size = nt_headers->OptionalHeader.SizeOfImage;

                s_module_info module_info = {};
                module_info.module_name = name_buf;
                module_info.address = module_address;
                module_info.size = module_size;

                modules.push_back(module_info);

                current_module = reinterpret_cast<LDR_DATA_TABLE_ENTRY const*>(
                        reinterpret_cast<LIST_ENTRY const* const>(current_module)->Flink);
            }
#elif __linux__
            // https://linux.die.net/man/3/dl_iterate_phdr
            dl_iterate_phdr(
                    [](struct dl_phdr_info* info, size_t, void*)
                    {
                        s_module_info module_info = {};
                        module_info.module_name = info->dlpi_name;
                        module_info.address = info->dlpi_addr + info->dlpi_phdr[0].p_vaddr;
                        module_info.size = info->dlpi_phdr[0].p_memsz;
                        modules.push_back(module_info);

                        return 0;
                    },
                    nullptr);
#endif
        }

        for (const auto& module: modules)
        {
#ifdef _WIN32
            if (module.module_name != module_name)
                continue;
#elif __linux__
            if (!strcasestr(module.module_name.c_str(),
                    module_name)) // on linux we get the full path, hence this compare
                continue;
#endif
            if (address)
                *address = module.address;

            if (size)
                *size = module.size;

            if (full_path)
                *full_path = module.module_name.c_str();

            return true;
        }

        LOG(std::string("couldn't find module: ") + module_name);

        return false;
    }

#ifdef _WIN32

    uintptr_t get_proc_address(const uintptr_t& module, const char* function, const bool& in_memory)
    {
        const auto relative_virtual_address_to_offset = [](const uintptr_t& relative_virtual_address,
                const PIMAGE_NT_HEADERS& nt_headers,
                const bool& in_memory = false) -> uintptr_t
        {
            if (relative_virtual_address == 0 || !in_memory)
                return relative_virtual_address;

            auto section = IMAGE_FIRST_SECTION(nt_headers);
            for (size_t i = 0; i < nt_headers->FileHeader.NumberOfSections; i++)
            {
                if (relative_virtual_address >= section->VirtualAddress
                    && relative_virtual_address < section->VirtualAddress + section->Misc.VirtualSize)
                    break;

                section++;
            }

            return relative_virtual_address - section->VirtualAddress + section->PointerToRawData;
        };

        const auto dos_headers = reinterpret_cast<IMAGE_DOS_HEADER*>(module);
        if (dos_headers->e_magic != IMAGE_DOS_SIGNATURE)
            return {};

        const auto nt_headers = reinterpret_cast<IMAGE_NT_HEADERS*>(dos_headers->e_lfanew + module);
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
            return {};

        const auto exports = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(relative_virtual_address_to_offset(
                nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, nt_headers,
                in_memory) + module);

        const auto names = reinterpret_cast<uint32_t*>(relative_virtual_address_to_offset(
                exports->AddressOfNames, nt_headers, in_memory) + module);

        auto ordinal_index = static_cast<uint32_t>(-1);
        for (size_t i = 0; i < exports->NumberOfFunctions; i++)
        {
            const auto function_name = reinterpret_cast<const char*>(relative_virtual_address_to_offset(
                    names[i], nt_headers, in_memory) + module);
            if (rt_hash(function_name) == rt_hash(function))
            {
                ordinal_index = i;

                break;
            }
        }

        if (ordinal_index > exports->NumberOfFunctions)
            return {};

        const auto ordinals = reinterpret_cast<uint16_t*>(relative_virtual_address_to_offset(
                exports->AddressOfNameOrdinals, nt_headers, in_memory) + module);
        const auto addresses = reinterpret_cast<uint32_t*>(relative_virtual_address_to_offset(
                exports->AddressOfFunctions, nt_headers, in_memory) + module);

        return relative_virtual_address_to_offset(
                addresses[ordinals[ordinal_index]], nt_headers, in_memory) + module;
    }

#endif

    std::string format_address(const uintptr_t& address)
    {
        auto ss = std::stringstream();
        ss << "0x" << std::uppercase << std::hex << address;

        return ss.str();
    }

    // pattern scanning for locations in memory, since not all locations are static, but the bytes are
    // ida refers to hex ray's tool that uses those type of patterns: "E8 ? ? ? ? 83 ..."
    uintptr_t find_pattern(const char* module_name, const char* ida_pattern,
            const int& offset, const std::string& pattern_name)
    {
        auto bytes = std::vector<int16_t>{};
        for (auto* i = const_cast<char*>(ida_pattern);
             i < const_cast<char*>(ida_pattern) + strlen(ida_pattern); ++i)
        {
            if (*i == '?')
            {
                ++i;
                bytes.push_back(-1);
            }
            else
            {
                bytes.push_back(static_cast<short>(strtoul(i, &i, 16)));
            }
        }

        uintptr_t module_address{};
        size_t module_size{};
        get_module_info(module_name, &module_address, &module_size);

        if (module_size == 0) // this should never happen
        {
            LOG("Module size was zero, pattern name: " + std::string(pattern_name));

            return {};
        }

        const auto module_bytes = reinterpret_cast<uint8_t*>(module_address);
        const auto pattern_size = bytes.size();
        const auto pattern_bytes = bytes.data();

        for (size_t i = 0; i < module_size - pattern_size; ++i)
        {
            auto found = true;

            for (size_t j = 0; j < pattern_size; ++j)
            {
                if (pattern_bytes[j] != -1 && module_bytes[i + j] != pattern_bytes[j])
                {
                    found = false;

                    break;
                }
            }

            if (found)
            {
                const auto final_address = reinterpret_cast<uintptr_t>(&module_bytes[i]) + offset;

                if (!pattern_name.empty())
                {
                    LOG("Found pattern: " + std::string(pattern_name) + " at: " +
                        memory::format_address(final_address));

                    return final_address;
                }
            }
        }

        LOG("Unable to get a pattern: " + std::string(pattern_name));

        return {};
    }

    uintptr_t get_absolute_address(uintptr_t instruction, int offset, int size)
    {
        return instruction + *reinterpret_cast<uint32_t*>(instruction + offset) + size;
    }
} // namespace memory
