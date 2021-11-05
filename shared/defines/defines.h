#ifndef PORTAL2_CROSS_DEFINES_H
#define PORTAL2_CROSS_DEFINES_H


// (ab)using macros, to define values based on the OS on compile time

#ifdef _WIN32
#define HOOK_ARGS void* this_ptr, void* edx // used to define correct calling convention of our hooks
#define C_HOOK_ARGS this_ptr, edx // used when calling
#define L_W(Linux, Windows) Windows // convenience macro
#elif __linux__
#define HOOK_ARGS void* this_ptr
#define C_HOOK_ARGS this_ptr
#define L_W(Linux, Windows) Linux

#define __fastcall // we need the calling convention on Windows, but it's useless on Linux
#define __thiscall
#endif


#endif //PORTAL2_CROSS_DEFINES_H
