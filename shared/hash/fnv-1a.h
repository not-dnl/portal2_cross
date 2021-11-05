#ifndef PORTAL2_CROSS_FNV_1A_H
#define PORTAL2_CROSS_FNV_1A_H


#include <cstdint>
#include <cstring>

namespace hash
{
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function

    static constexpr uint64_t basis = 0x811c9dc5;
    static constexpr uint64_t prime = 0x1000193;

    constexpr uintptr_t get_const(const char* text, const uintptr_t value = basis) noexcept
    {
        // recursive
        return text[0] == '\0' ? value : get_const(&text[1], (value ^ uintptr_t(text[0])) * prime);
    }

    inline uintptr_t get(const char* text)
    {
        uintptr_t ret = basis;

        const auto length = strlen(text);
        for (size_t i = 0; i < length; ++i)
        {
            ret ^= text[i];
            ret *= prime;
        }

        return ret;
    }
}

template<typename Text>
constexpr auto ct_hash(Text text)
{
    return hash::get_const(text);
}

template<typename Text>
constexpr auto rt_hash(Text text)
{
    return hash::get(text);
}


#endif //PORTAL2_CROSS_FNV_1A_H
