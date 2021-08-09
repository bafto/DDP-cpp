#include "Function.h"

Function::Function(ValueType returnType, const std::vector<ValueType>& args)
	:
	returnType(returnType),
	args(args),
	functions(nullptr),
	globals(nullptr)
{}

void Function::push(Value value)
{
	*stackTop = std::move(value);
	stackTop++;
}

Value Function::pop()
{
	stackTop--;
	return *stackTop;
}

Value Function::peek(int distance)
{
	return stackTop[-1 - distance];
}

uint8_t Function::readByte()
{
	return *ip++;
}

uint16_t Function::readShort()
{
	return (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]));
}

Value Function::readConstant()
{
	return chunk.constants[readByte()];
}
