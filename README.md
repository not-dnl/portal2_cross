# portal2_cross

A Linux and Windows compatible Portal 2 memory hack, written as a fun project.

### Information
* Most interesting code parts should be commented.
* This project compiles to a shared library (.so on Linux, .dll on Windows).
* Tested and developed on Windows 10 and Arch Linux with CLion, using the Clang++ compiler with CMake.
* You need a DLL-Injector on Windows, to inject the shared library to the process.
* On Linux you can (ab)use GDB to call dlopen from the process.

### In action
* The features are controlled via the in game console, by registering our own console variables (cvars).
* Demo:
<video width="630" height="300" src="https://github.com/user-attachments/assets/46fdf432-b81b-4d95-8d21-3da131ca1222"></video>
