#pragma once
#include <Windows.h>
#include <vector>

typedef struct FUNCTION_INFO {
	int offset;
	int size;
} FUNCTION_INFO;

std::vector<FUNCTION_INFO> getFunctions(HANDLE proc, uintptr_t segmentStartAddr, int segmentSize);

void pruneFunctions(HANDLE proc, uintptr_t segmentStartAddr, int segmentSize, uintptr_t entryAddr, std::vector<FUNCTION_INFO>* funcList);

void mutateFunctions(HANDLE proc, uintptr_t segmentStartAddr, std::vector<FUNCTION_INFO> funcList);