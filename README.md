# Horizon Calculate

Horizon Calculate is a lightweight, classic Win32 calculator written in C. It is designed to be fast, portable, and compatible with a wide range of Windows versions — including legacy systems such as Windows 2000. The application uses only the traditional Win32 API, keeping the executable small and dependency‑free.

## Features
- Pure Win32 API (no frameworks or external libraries)
- Extremely small and fast executable
- Classic Windows user interface
- About dialog with system information
- Portable: runs without installation
- Designed for long‑term compatibility

## Compatibility
Horizon Calculate runs on every NT‑based Windows version:

- Windows 2000  
- Windows XP  
- Windows Vista  
- Windows 7  
- Windows 8 / 8.1  
- Windows 10  
- Windows 11  

## Download
Prebuilt binaries are available on the [Releases](../../releases) page.

## Build Instructions
The project is a single C source file and can be compiled with Clang or any modern Windows C compiler.
```clang calculator.c -o HorizonCalculate.exe -lgdi32 -luser32```
