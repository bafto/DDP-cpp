#include "Chunk.h"

void Chunk::write(uint8_t byte)
{
	bytes.push_back(byte);
}

void Chunk::write(OpCode code)
{
	write((uint8_t)code);
}

size_t Chunk::addConstant(Value value)
{
	constants.push_back(std::move(value));
	return constants.size() - 1;
}
