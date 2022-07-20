#pragma once
#include <Windows.h>
#include <winnt.h>

typedef struct SECTION_INFO {
	const char* sectionName;
	int size;
	uintptr_t offset;
} SECTION_INFO;

uintptr_t getEntryPointOffset(HANDLE proc, uintptr_t baseAddr);

SECTION_INFO parseSections(HANDLE proc, uintptr_t baseAddr);
