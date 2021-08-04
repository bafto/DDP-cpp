#include "Chunk.h"
#include "Value.h"

void Chunk::write(uint8_t byte, int line)
{
    code.push_back(byte);
    lines.push_back(line);
}

void Chunk::write(OpCode code, int line)
{
    write((uint8_t)code, line);
}

int Chunk::addConstant(Value val)
{
    constants.push_back(std::move(val));
    return constants.size() - 1;
}
