#ifndef PORTAL2_CROSS_LOG_H
#define PORTAL2_CROSS_LOG_H


#include <string>

namespace logs // can't call this "log" because of logarithm :(
{
    void print(const char* text);

    void print(const std::string& text);

#ifdef _WIN32

    void detach();

#endif
}

#define LOG(text) logs::print(text)


#endif //PORTAL2_CROSS_LOG_H
