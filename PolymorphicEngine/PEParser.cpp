#include "PEParser.h"
#include <windows.h>
#include <winnt.h>
#include <iostream>

IMAGE_DOS_HEADER getDosHeader(HANDLE proc, uintptr_t baseAddr) {
	IMAGE_DOS_HEADER dosHeader;

	ReadProcessMemory(proc, (LPCVOID)baseAddr, &dosHeader, sizeof(IMAGE_DOS_HEADER), NULL);

	return dosHeader;
}

uintptr_t getEntryPointOffset(HANDLE proc, uintptr_t baseAddr) {
	IMAGE_DOS_HEADER dosHeader = getDosHeader(proc, baseAddr);
	IMAGE_NT_HEADERS ntHeader;

	ReadProcessMemory(proc, (LPCVOID)(baseAddr + dosHeader.e_lfanew), &ntHeader, sizeof(IMAGE_NT_HEADERS), NULL);

	IMAGE_OPTIONAL_HEADER optionHeader = ntHeader.OptionalHeader;

	std::cout << std::hex << "Process entry address offset: " << optionHeader.AddressOfEntryPoint << std::endl;

	return optionHeader.AddressOfEntryPoint;
}

SECTION_INFO parseSections(HANDLE proc, uintptr_t baseAddr) {
	uintptr_t sectionHeaderStartAddr = baseAddr + getDosHeader(proc, baseAddr).e_lfanew + sizeof(IMAGE_NT_HEADERS);

	IMAGE_SECTION_HEADER curr;
	ReadProcessMemory(proc, (LPCVOID)sectionHeaderStartAddr, &curr, sizeof(IMAGE_SECTION_HEADER), NULL);

	char* str = new char[sizeof(curr.Name) + 1];
	memcpy(str, curr.Name, sizeof(curr.Name));
	str[sizeof(curr.Name)] = 0;

	while (strcmp(str, ".text") != 0) {
		sectionHeaderStartAddr += sizeof(IMAGE_SECTION_HEADER);
		ReadProcessMemory(proc, (LPCVOID)sectionHeaderStartAddr, &curr, sizeof(IMAGE_SECTION_HEADER), NULL);

		str = new char[sizeof(curr.Name) + 1];
		memcpy(str, curr.Name, sizeof(curr.Name));
		str[sizeof(curr.Name)] = 0;
	}

	SECTION_INFO result;
	result.sectionName = str;
	result.offset = curr.VirtualAddress;
	result.size = curr.Misc.VirtualSize;

	std::cout << "Located " << result.sectionName << " section with offset " << std::hex << result.offset << " and size of " << std::dec << result.size << " bytes" << std::endl;

	return result;
}

