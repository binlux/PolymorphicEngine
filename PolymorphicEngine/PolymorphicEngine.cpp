// PolymorphicEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <windows.h>
#include <tlhelp32.h>
#include "PEParser.h"
#include "Memory.h"

HANDLE getHandleByName(const wchar_t* name, int* procID) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (_wcsicmp(entry.szExeFile, name) == 0)
            {
                std::wcout << "Process " << name << " found. Acquired handle\n";
                *procID = entry.th32ProcessID;
                CloseHandle(snapshot);
                return OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
            }
        }
    }

    return 0;
}

uintptr_t getBaseAddress(int procID) {
    MODULEENTRY32 entry;
    entry.dwSize = sizeof(MODULEENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procID);
    if (Module32First(snapshot, &entry)) {
        std::cout << std::hex << "Base address of module is: " << (uintptr_t)entry.modBaseAddr << std::endl;
        return (uintptr_t)entry.modBaseAddr;
    }

    return 0;
}


int main()
{
    int procID;
    HANDLE proc = getHandleByName(L"test.exe", &procID);
    uintptr_t baseAddr = getBaseAddress(procID);

    uintptr_t entryAddrOffset = getEntryPointOffset(proc, baseAddr);
    SECTION_INFO codeSegment = parseSections(proc, baseAddr);

    std::vector<FUNCTION_INFO> funcList = getFunctions(proc, baseAddr + codeSegment.offset, codeSegment.size);
    int origCount = funcList.size();
    std::cout << std::dec << funcList.size() << " functions identified." << std::endl;

    pruneFunctions(proc, baseAddr + codeSegment.offset, codeSegment.size, baseAddr + entryAddrOffset, &funcList);
    std::cout << origCount - funcList.size() << " functions pruned. Remaining " << funcList.size() << " functions to be mutated." << std::endl;

    mutateFunctions(proc, baseAddr + codeSegment.offset, funcList);
    std::cout << "Mutated " << std::dec << funcList.size() << " functions." << std::endl;

    CloseHandle(proc);
}
