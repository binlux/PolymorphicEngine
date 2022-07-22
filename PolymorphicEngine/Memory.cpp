#include <Windows.h>
#include <iostream>
#include "Memory.h"
#include <Zydis/Zydis.h>
#include <vector>


BYTE* readSegment(HANDLE proc, uintptr_t startAddr, int size) {
	BYTE* segment = new BYTE[size];

	ReadProcessMemory(proc, (LPCVOID)startAddr, segment, size, NULL);

	return segment;
}

std::vector<FUNCTION_INFO> getFunctions(HANDLE proc, uintptr_t segmentStartAddr, int segmentSize) {
	BYTE* segment = readSegment(proc, segmentStartAddr, segmentSize);
	
    std::vector<FUNCTION_INFO> functionList;

    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);

    ZyanUSize offset = 0;
    ZydisDecodedInstruction instruction;
    bool isFunc = false;
    int funcStart = 0;
    FUNCTION_INFO currFunc;

    while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, segment + offset, segmentSize - offset,
        &instruction)))
    {
        const char* opcode = ZydisMnemonicGetString(instruction.mnemonic);
        if (strcmp("int3", opcode) == 0 || strcmp("ret", opcode) == 0) {
            if (isFunc) {
                isFunc = false;
                currFunc.size = offset - funcStart;
                functionList.push_back(currFunc);
                currFunc = {};
            }
        }else{
            if (!isFunc && strcmp("jmp", opcode) != 0 && strcmp("nop", opcode) != 0 ) {
                // JMP and NOP cannot be first byte of function.
                isFunc = true;
                funcStart = offset;
                currFunc.offset = offset;
            }
        }
        offset += instruction.length;
    }

    return functionList;
}

int findFunc(std::vector<FUNCTION_INFO> funcList, int offset) {
    for (int i = 0; i < funcList.size(); i++) {
        if (funcList[i].offset == offset) {
            return i;
        }
    }

    return -1;
}

void recursivePrune(BYTE* segment, int offset, std::vector<FUNCTION_INFO>* funcList) {
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
    ZydisDecodedInstruction instruction;

    int index = findFunc(*funcList, offset);

    if (index == -1) {
        // Test for landing in jump table
        ZydisDecoderDecodeBuffer(&decoder, segment + offset, 5, &instruction);
        if (strcmp("jmp", ZydisMnemonicGetString(instruction.mnemonic)) == 0) {
            int newOffset = offset + 5 + (int)instruction.operands->imm.value.u;
            return recursivePrune(segment, newOffset, funcList);
        }
        else {
            return;
        }
    };

    FUNCTION_INFO funcInfo =  (*funcList)[index];
    (*funcList).erase((*funcList).begin() + index);

    int i = 0;
    while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&decoder, segment + offset + i, funcInfo.size - i, 
        &instruction))) {
        
        const char* opcode = ZydisMnemonicGetString(instruction.mnemonic);
        if (strcmp(opcode, "call") == 0 || strcmp(opcode, "jmp") == 0) {
            int newOffset = offset + i + 5 + (int)instruction.operands->imm.value.u;
            recursivePrune(segment, newOffset, funcList);
        }

        i += instruction.length;
    }
}

void pruneFunctions(HANDLE proc, uintptr_t segmentStartAddr, int segmentSize, uintptr_t entryAddr, std::vector<FUNCTION_INFO>* funcList) {
    BYTE* segment = readSegment(proc, segmentStartAddr, segmentSize);

    int offset = entryAddr - segmentStartAddr;

    if (findFunc((*funcList), offset) == -1) {
        // Resolve JMP
        ZydisDecoder decoder;
        ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
        ZydisDecodedInstruction instruction;

        ZydisDecoderDecodeBuffer(&decoder, segment + offset,  5, &instruction);
        offset = offset + 5 + (int)instruction.operands->imm.value.u;
        recursivePrune(segment, offset, funcList);
    }
    else {
        recursivePrune(segment, offset, funcList);
    }
}

void mutateFunctions(HANDLE proc, uintptr_t segmentStartAddr, std::vector<FUNCTION_INFO> funcList) {
    for (int i = 0; i < funcList.size(); i++) {
        BYTE* nops = new BYTE[funcList[i].size];
        for (int j = 0; j < funcList[i].size; j++) {
            nops[j] = (BYTE)(rand() % 255);
        }

        WriteProcessMemory(proc, (LPVOID)(segmentStartAddr + funcList[i].offset), nops, funcList[i].size, NULL);
    }
}