#include "ASM.h"

ASM::ASM(std::vector<byte>& byteCode) : m_byteCode{ byteCode }, m_memorySize{ 0 }, m_memory{ nullptr }, m_PC{ 0 }, m_byteCodeIndex{ 0 } {
	allocateMemory();
	
}
void ASM::run() {
    prolog();
    readByteCode();
    epilog();
    execute();
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

int64_t ASM::readInt(size_t size) {
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
    return 0;
}

void ASM::dup()
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
}

void ASM::pop(size_t size)
{
    int64_t destination = readInt(size);
    m_memory[m_PC++] = 0x41;
    writeToMemory(destination);
}

void ASM::constant(size_t size)
{
    m_memory[m_PC++] = 0x41;
    m_memory[m_PC++] = 0x55; //push r13
}


void ASM::add()
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
}

void ASM::sub()
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
}

void ASM::mul()
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
}

void ASM::div()
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
}

void ASM::mod()
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
}

void ASM::less()
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
}

void ASM::lessEq()
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
}

void ASM::greater()
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
}

void ASM::greaterEq()
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
        movzx   eax, al
    */
}

void ASM::equal()
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
}

void ASM::notEqual()
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
}

void ASM::push(size_t size)
{
    int64_t value = readInt(size);
    m_memory[m_PC++] = 0x41;
    writeToMemory(value);
}

void ASM::jump(size_t size) {
    m_memory[m_PC++] = 0xe9; //jmp
    int64_t destination = readInt(size);
    writeToMemory(destination);
}

void ASM::jumpT(size_t size) {
    int64_t destination = readInt(size);
    int64_t condition = readInt(1);
    if (condition) {
        m_memory[m_PC++] = 0xe9; //jmp;
        writeToMemory(destination);
    }
}

void ASM::jumpF(size_t size) {
    int64_t destination = readInt(size);
    int64_t condition = readInt(1);
    if (!condition) {
        m_memory[m_PC++] = 0xe9; //jmp;
        writeToMemory(destination);
    }
}

void ASM::writeToMemory(int64_t value) {
    byte* hexNumbers = new byte[4]{ 0 };
    unsigned index = 0;
    while (value > 0) {
        byte tail = value % 16;
        hexNumbers[index++] = tail;
        value /= 16;
    }
    //write in correct order
    for (unsigned i = 3; i > 0; i--) {
        m_memory[m_PC++] = hexNumbers[i];
    }
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

void ASM::readByteCode() {
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
                dup();
                break;
            }
            case OpCodes::Pop: 
            {
                pop(size);
                break;
            }
            case OpCodes::Const:
            {
                constant(size);
                break;
            }
            case OpCodes::Add:
            {
                add();
                break;
            }
            case OpCodes::Mul:
            {
                mul();
                break;
            }
            case OpCodes::Div:
            {
                div();
                break;
            }
            case OpCodes::Mod:
            {
                mod();
                break;
            }
            case OpCodes::Sub:
            {
                sub();
                break;
            }
            case OpCodes::Greater:
            {
                greater();
                break;
            }
            case OpCodes::Equal:
            {
                equal();
                break;
            }
            case OpCodes::GreaterEq:
            {
                greaterEq();
                break;
            }
            case OpCodes::Less:
            {
                less();
                break;
            }
            case OpCodes::LessEq:
            {
                lessEq();
                break;
            }
            case OpCodes::NotEqual:
            {
                notEqual();
                break;
            }
            case OpCodes::Jump:
            {
                jump(size);
                break;
            }
            case OpCodes::JumpT:
            {
                jumpT(size);
                break;
            }
            case OpCodes::JumpF:
            {
                jumpF(size);
            }
            case OpCodes::Push:
            {
                push(size);
                break;
            }       
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