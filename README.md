# portal2_cross

A Linux and Windows compatible Portal 2 Cheat, written as a fun project.

### Information
* Most interesting code parts should be commented.
* This project compiles to a shared library (.so on Linux, .dll on Windows).
* Tested on Windows 10 and Arch Linux using the Clang++ compiler.
* You need a DLL-Injector on Windows, to inject the shared library to the process.
* On Linux you can (ab)use GDB to call dlopen from the process.

### In action
* The features are controlled via the in game console, by registering our own console variables (cvars).
* You can watch a demo here: https://streamable.com/11vp9l
