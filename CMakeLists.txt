cmake_minimum_required(VERSION 3.0)
project(portal2_cross)

set(CMAKE_CXX_STANDARD 20)

add_library(portal2_cross SHARED
        portal2/main.cpp
        portal2/helper/interfaces.h portal2/helper/interfaces.cpp
        shared/memory/memory.h
        shared/memory/memory.cpp shared/memory/winapi.h
        shared/log/log.cpp shared/log/log.h
        shared/hook/hooking_methods.cpp
        shared/hook/hooking_methods.h
        portal2/hook/manager/hooking_manager.cpp
        portal2/hook/manager/hooking_manager.h
        portal2/source_engine/other/user_cmd.h
        portal2/hook/client_mode/create_move.cpp
        shared/hash/fnv-1a.h portal2/source_engine/interfaces/entity_list.h
        shared/defines/defines.h)

set_target_properties(portal2_cross PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")