#include "ASM.h"

ASM::ASM() : memorySize{ 0 }, memory{ nullptr } {
	allocateMemory();
	prolog();
}


void ASM::allocateMemory() {
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    auto const page_size = system_info.dwPageSize;

    // prepare the memory in which the machine code will be put (it's not executable yet):
    auto const buffer = VirtualAlloc(nullptr, page_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (buffer)
    {
        memory = (unsigned char*)buffer;
        
        // interpret the beginning of the (now) executable memory as the entry
        // point of a function taking no arguments and returning a 4-byte int:
        //auto const function_ptr = reinterpret_cast<std::int32_t(*)()>(buffer);
        // call the function and store the result in a local std::int32_t object:
        //auto const result = function_ptr();
        // use your std::int32_t:
        //std::cout << result << "\n";
    }
}

void ASM::prolog() {
    /*
    push all arguments passed through registers onto the stack
    push the number of arguments on to the stack
    set RDX to point to the top of the stack
    */


}

void ASM::epilog() {
    /*
    move from spasm result register to rax
    restore the stack frame
    ret
    */
    // mark the memory as executable:
    DWORD dummy;
    VirtualProtect(memory, memorySize, PAGE_EXECUTE_READ, &dummy);


    
}

void ASM::run() {
    // free the executable memory:
    VirtualFree(memory, 0, MEM_RELEASE);
}