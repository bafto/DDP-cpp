#pragma once

#include <vector>

//enum class for all operation codes that the VirtualMachine will process
enum class OpCode
{
	OP_RETURN
};

//holds byte-code and constant Values
class Chunk
{
public:
	std::vector<uint8_t> code; //the byte code
};

