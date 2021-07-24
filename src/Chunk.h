#pragma once

#include <vector>
#include "Value.h"

//enum class for all operation codes that the VirtualMachine will process
enum class OpCode
{
	CONSTANT,
	RETURN
};

//holds byte-code and constant Values
class Chunk
{
public:
	void writeChunk(uint8_t byte, int line); //add a byte to the chunk
	void writeChunk(OpCode code, int line); //add a OpCode to the chunk (just takes away the casting)
	int addConstant(Value val); //add a value to constants and return its index

public:
	std::vector<uint8_t> code; //the byte code
	std::vector<Value> constants; //constant values
	std::vector<int> lines; //maps the code array. each byte in code has a line at the same index here
};

