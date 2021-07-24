#include "Chunk.h"

void Chunk::writeChunk(uint8_t byte, int line)
{
    code.push_back(byte);
    lines.push_back(line);
}

int Chunk::addConstant(Value val)
{
    constants.push_back(val);
    return constants.size() - 1;
}
