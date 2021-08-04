#pragma once

#include "Value.h"
#include "Chunk.h"
#include <array>
#include <unordered_map>

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

	void runtimeError(std::string msg);

	uint8_t readByte(); //return the next byte in chunk.code and advance ip
	uint16_t readShort(); //return the next 2 bytes as short
	Value readConstant(); //return the value in chunk.constants that the next byte in chunk.code indexes
	void push(Value val); //push a value onto the stack
	Value pop(); //pop a value of the stack and return it
	Value peek(int distance);

	void addition();
private:
	static constexpr int FramesMax = 64;
	static constexpr int StackMax = FramesMax * (UINT8_MAX + 1); //the static size of the stack
	struct CallFrame
	{
		std::array<Value, StackMax>::iterator function;
		std::vector<uint8_t>::iterator ip;
		std::array<Value, StackMax>::iterator slots;
	};
private:
	Chunk chunk; //the byte code of the VM

	/**Members only used inside run**/
	std::array<CallFrame, FramesMax> frames;
	std::array<CallFrame, FramesMax>::iterator frame;

	std::array<Value, StackMax> stack; //the Value stack
	std::array<Value, StackMax>::iterator stackTop; //pointer to the current stack top

	std::unordered_map<std::string, Value> globals;
};

