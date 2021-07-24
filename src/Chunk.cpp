#include "Chunk.h"

void Chunk::write(uint8_t byte, int line)
{
    code.push_back(byte);
    lines.push_back(line);
}

void Chunk::write(OpCode code, int line)
{
    writeChunk((uint8_t)code, line);
}

int Chunk::addConstant(Value val)
{
    constants.push_back(val);
    return constants.size() - 1;
}
