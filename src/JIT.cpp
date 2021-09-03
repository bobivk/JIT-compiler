#include "JIT.h"
int main() {
	std::vector<byte>* byteCode = new std::vector<byte>();
	byteCode->push_back(OpCodes::Add);
	byteCode->push_back(1);
	byteCode->push_back(2);
	ASM* asmPtr = new ASM(*byteCode);
	asmPtr->run();
	return 0;
}