#pragma once

#include "Chunk.h"
#include <array>

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
	void push(Value val); //push a value onto the stack
	Value pop(); //pop a value of the stack and return it

	void addition();
private:
	Chunk chunk; //the byte code of the VM

	/**Members only used inside run**/

	std::vector<uint8_t>::iterator ip; //instruction-pointer, iterator to the current instruction

	static constexpr int StackMax = 256; //the static size of the stack
	std::array<Value, StackMax> stack; //the Value stack
	std::array<Value, StackMax>::iterator stackTop; //pointer to the current stack top
};

