#pragma once
#include<Windows.h>
#include<cstring>
#include<vector>
#include<iostream>
class ASM
{

	ASM();


	void allocateMemory();
	void prolog();
	void epilog();
	void run();

	unsigned char* memory;
	int memorySize;
};

