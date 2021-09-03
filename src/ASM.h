#pragma once
#include<Windows.h>
#include<cstring>
#include<vector>
#include<iostream>
#include"E:\Bobby\Uni\Implementation of Programming Languages\IPL\spasm\src\spasm_impl.hpp"





class ASM
{
    typedef unsigned (*asmFunc)(void);

public:
    ASM(std::vector<byte>& byteCode);

private:
	void functionCall();
	void allocateMemory();
	void prolog();
	void epilog();
	void read_bytecode();
    void execute();

    int64_t read_int(size_t size);

	byte* m_memory; //array that holds the bytes of the instructions
	int m_memorySize;
	std::vector<byte> m_byteCode;
    unsigned m_PC; //program counter - index in m_memory
    unsigned m_byteCodeIndex;


    enum OpCodes : char
    {
        Halt,
        Dup,
        Pop,
        PopTo,
        PushFrom,
        Push,
        Print,
        Read,
        Call,
        Ret,
        Jump,
        JumpT,
        JumpF,
        Const,
        String,
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Less,
        LessEq,
        Greater,
        GreaterEq,
        Equal,
        NotEqual,
        LastIndex = NotEqual,
    };
   
};

