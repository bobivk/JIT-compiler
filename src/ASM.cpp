#include "ASM.h"

ASM::ASM(std::vector<byte>& byteCode) : m_byteCode{ byteCode }, m_memorySize{ 0 }, m_memory{ nullptr }, m_PC{ 0 }, m_byteCodeIndex{ 0 } {
	allocateMemory();
	
}


void ASM::allocateMemory() {
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    auto const page_size = system_info.dwPageSize;

    // prepare the memory in which the machine code will be put (it's not executable yet):
    auto const buffer = VirtualAlloc(nullptr, page_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (buffer)
    {
        m_memory = (unsigned char*)buffer;
        m_memorySize = page_size;
    }
    else std::cerr << "Could not allocate executable memory.";
}

int64_t ASM::read_int(size_t size) {
    switch (size)
    {
    case 0:
        return m_byteCode[m_byteCodeIndex++];
    case 1:
    {
        auto result = *reinterpret_cast<int16_t*>(&m_byteCode[0] + m_byteCodeIndex);
        m_byteCodeIndex += 2;
        return result;
    }
    case 2:
    {
        auto result = *reinterpret_cast<int32_t*>(&m_byteCode[0] + m_byteCodeIndex);
        m_byteCodeIndex += 4;
        return result;
    }
    case 3:
    {
        auto result = *reinterpret_cast<int64_t*>(&m_byteCode[0] + m_byteCodeIndex);
        m_byteCodeIndex += 8;
        return result;
    }
    }
    assert(false && "not reached");
    return 0;
}

void ASM::functionCall() {
    prolog();




    epilog();
}



void ASM::prolog() {

    m_memory[m_PC++] = 0x48;
    m_memory[m_PC++] = 0x89;
    m_memory[m_PC++] = 0x4c;
    m_memory[m_PC++] = 0x24;
    m_memory[m_PC++] = 0x08;
    m_memory[m_PC++] = 0x41;
    m_memory[m_PC++] = 0x56;
    m_memory[m_PC++] = 0x41;
    m_memory[m_PC++] = 0x55;

    m_memory[m_PC++] = 0x48;
    m_memory[m_PC++] = 0x81;
    m_memory[m_PC++] = 0xec;
    m_memory[m_PC++] = 0x00;
    m_memory[m_PC++] = 0x01;
    m_memory[m_PC++] = 0x00;
    m_memory[m_PC++] = 0x00;
    m_memory[m_PC++] = 0x4c;
    m_memory[m_PC++] = 0x8d;
    m_memory[m_PC++] = 0xac;
    m_memory[m_PC++] = 0x24;
    m_memory[m_PC++] = 0x80;
    m_memory[m_PC++] = 0x00;
    m_memory[m_PC++] = 0x00;
    m_memory[m_PC++] = 0x00;
    m_memory[m_PC++] = 0x00;
/*
mov    [RSP + 8], RCX
push   R15
push   R14
push   R13
sub    RSP, 256
lea    R13, 128[RSP]

256 is the fixed allocation size
establish frame pointer 128 B into the fixed allocation
to allow more of the fixed area to be addressed by one-byte offsets
*/
}

void ASM::epilog() {
    m_memory[m_PC++] = 0x49;
    m_memory[m_PC++] = 0x8d;
    m_memory[m_PC++] = 0xa5;
    m_memory[m_PC++] = 0x80;
    m_memory[m_PC++] = 0x00;
    m_memory[m_PC++] = 0x00; //undo changes to rsp from prolog
    m_memory[m_PC++] = 0x00; //lea RSP, 256 - 128[R13]

    m_memory[m_PC++] = 0x41; 
    m_memory[m_PC++] = 0x5d;
    m_memory[m_PC++] = 0x41;
    m_memory[m_PC++] = 0x5e;
    m_memory[m_PC++] = 0x41;
    m_memory[m_PC++] = 0x5f; //pop registers r13, r14, r15
    m_memory[m_PC++] = 0xc3; //ret
}

void ASM::read_bytecode() {
    const auto codeSize = m_byteCode.size();
    while (m_byteCodeIndex < codeSize)
    {
        const auto instruction = m_byteCode[m_byteCodeIndex];
        const auto opcode = OpCodes(instruction & 0x3f); //get six rightmost bits of byte
        const auto size = instruction >> 6;              //first two bits are the size
        switch (opcode) {
            case OpCodes::Halt:
                m_memory[m_PC++] = 0xf4; //hlt
            case OpCodes::Ret:
            {
                return; //go to epilog
            }
            case OpCodes::Dup:
            {
                m_memory[m_PC++] = 0x41;
                m_memory[m_PC++] = 0x4d;
                m_memory[m_PC++] = 0x89;
                m_memory[m_PC++] = 0xc1;
                m_memory[m_PC++] = 0x41;
                m_memory[m_PC++] = 0x50;
                m_memory[m_PC++] = 0x41;
                m_memory[m_PC++] = 0x51;
                /*
                pop r8
                mov r9, r8
                push r8
                push r9
                */
                break;
            }
            case OpCodes::Pop: 
            {
                m_memory[m_PC++] = 0x41; //pop
                break;
            }
            case OpCodes::PopTo:
            {
                //read pointer?
                int64_t destination = read_int(size);
                m_memory[m_PC++] = 0x41; //pop
                break;
            }
            case OpCodes::Const:
            {
                m_memory[m_PC++] = 0x41; //push - why do push and pop have the same opcodes in hex?
                m_memory[m_PC++] = 0x55; //r13
                break;
            }
            case OpCodes::Add:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x55;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xf8;
                m_memory[m_PC++] = 0x01;
                m_memory[m_PC++] = 0xd0;
                /*
                    mov     edx, DWORD PTR [rbp-4]
                    mov     eax, DWORD PTR [rbp-8]
                    add     eax, edx
                */
                break;
            }
            case OpCodes::Mul:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x55;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0xaf;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xf8;
                /*        
                    mov     eax, DWORD PTR [rbp-4]
                    imul    eax, DWORD PTR [rbp-8]
                */
                break;
            }
            case OpCodes::Div:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x99;
                m_memory[m_PC++] = 0xf7;
                m_memory[m_PC++] = 0x7d;
                m_memory[m_PC++] = 0xf8;
                /*
                mov     eax, DWORD PTR [rbp-4]
                cdq
                idiv    DWORD PTR [rbp-8]
                */
                break;
            }
            case OpCodes::Mod:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x99;
                m_memory[m_PC++] = 0xf7;
                m_memory[m_PC++] = 0x7d;
                m_memory[m_PC++] = 0xf8;
                m_memory[m_PC++] = 0x89;
                m_memory[m_PC++] = 0xd0;
                /*
                mov     eax, DWORD PTR [rbp-4]
                cdq
                idiv    DWORD PTR [rbp-8]
                mov     eax, edx
                */
                break;
            }
            case OpCodes::Sub:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x2b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xf8;
                /*
                mov     eax, DWORD PTR [rbp-4]
                sub     eax, DWORD PTR [rbp-8]
                */
                break;
            }
            case OpCodes::Greater:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x3b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xf8;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0x9f;
                m_memory[m_PC++] = 0xc0;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0xb6;
                m_memory[m_PC++] = 0xc0;
                /*
                mov     eax, DWORD PTR [rbp-4]
                cmp     eax, DWORD PTR [rbp-8]
                setg    al
                movzx   eax, al
                */
                break;
            }
            case OpCodes::Equal:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x3b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xf8;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0x94;
                m_memory[m_PC++] = 0xc0;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0xb6;
                m_memory[m_PC++] = 0xc0;
                /*
                mov     eax, DWORD PTR [rbp-4]
                cmp     eax, DWORD PTR [rbp-8]
                sete    al
                movzx   eax, al
                */
                break;
            }
            case OpCodes::GreaterEq:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x3b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xf8;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0x9d;
                m_memory[m_PC++] = 0xc0;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0xb6;
                m_memory[m_PC++] = 0xc0;
                /*
                mov     eax, DWORD PTR [rbp-4]
                cmp     eax, DWORD PTR [rbp-8]
                setge   al
                movzx   eax, al*/
                break;
            }
            case OpCodes::Less:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x3b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xf8;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0x9c;
                m_memory[m_PC++] = 0xc0;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0xb6;
                m_memory[m_PC++] = 0xc0;
                /*
                mov     eax, DWORD PTR [rbp-4]
                cmp     eax, DWORD PTR [rbp-8]
                setl    al
                movzx   eax, al
                */
                break;
            }
            case OpCodes::LessEq:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x3b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xf8;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0x9e;
                m_memory[m_PC++] = 0xc0;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0xb6;
                m_memory[m_PC++] = 0xc0;
                /*
                mov     eax, DWORD PTR [rbp-4]
                cmp     eax, DWORD PTR [rbp-8]
                setle   al
                movzx   eax, al
                */
                break;
            }
            case OpCodes::NotEqual:
            {
                m_memory[m_PC++] = 0x8b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xfc;
                m_memory[m_PC++] = 0x3b;
                m_memory[m_PC++] = 0x45;
                m_memory[m_PC++] = 0xf8;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0x95;
                m_memory[m_PC++] = 0xc0;
                m_memory[m_PC++] = 0x0f;
                m_memory[m_PC++] = 0xb6;
                m_memory[m_PC++] = 0xc0;
                /*
                mov     eax, DWORD PTR [rbp-4]
                cmp     eax, DWORD PTR [rbp-8]
                setne   al
                movzx   eax, al
                */
                break;
            }

            /*
            opcodes:
        +Halt,
        +Dup,
        Pop,
        PopTo,
        PushFrom,
        Push,
        Print,
        Read,
        Call,
        +Ret,
        Jump,
        JumpT,
        JumpF,
        +Const,
        String,
        +Add,
        +Sub,
        +Mul,
        +Div,
        +Mod,
        +Less,
        +LessEq,
        +Greater,
        +GreaterEq,
        +Equal,
        +NotEqual,
        LastIndex = NotEqual
            */
        }

        m_byteCodeIndex++;
    }
}

void ASM::execute()
{  
    // mark the memory as executable:
    DWORD dummy;
    VirtualProtect(m_memory, m_memorySize, PAGE_EXECUTE_READ, &dummy);
   
    //call the constructed function via its function pointer
    ((asmFunc)(m_memory))();
    
    // free the executable memory:
    VirtualFree(m_memory, 0, MEM_RELEASE);
}