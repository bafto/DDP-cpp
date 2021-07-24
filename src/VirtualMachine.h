#pragma once

#include "Chunk.h"

enum class InterpretResult
{
	OK,
	RuntimeError,
	CompilationError,
};

class VirtualMachine
{
public:
	VirtualMachine();
	~VirtualMachine();

	InterpretResult interpret(const std::string& source); //interpret a string of source code (compile into byte-code + interpreting)
private:
	InterpretResult run(); //run the compiled byte code

	/**Functions only used inside run**/
	uint8_t readByte(); //return the next byte in chunk.code and advance ip
	Value readConstant(); //return the value in chunk.constants that the next byte in chunk.code indexes
public:
	Chunk chunk; //the byte code of the VM
	std::vector<uint8_t>::iterator ip; //instruction-pointer, iterator to the current instruction
};

