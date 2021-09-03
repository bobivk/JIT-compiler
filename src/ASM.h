#pragma once
#include<Windows.h>
#include<cstring>
#include<vector>
#include<iostream>
#include"OpCodes.h";

class ASM
{
    typedef unsigned (*asmFunc)(void);

public:
    ASM(std::vector<byte>& byteCode);
    void run();

private:
	void allocateMemory();
	void prolog();
	void epilog();
	void readByteCode();
    void execute();
    void writeToMemory(int64_t value);
    int64_t readInt(size_t size);

    void dup();
    void pop(size_t size);
    void constant(size_t size);
    void push(size_t size);
    void add();
    void sub();
    void mul();
    void div();
    void mod();
    void less();
    void lessEq();
    void greater();
    void greaterEq();
    void equal();
    void notEqual();
    void jump(size_t size);
    void jumpT(size_t size);
    void jumpF(size_t size);

	byte* m_memory; //array that holds the bytes of the instructions
	int m_memorySize;
	std::vector<byte> m_byteCode;
    unsigned m_PC; //program counter - index in m_memory
    unsigned m_byteCodeIndex;
};

