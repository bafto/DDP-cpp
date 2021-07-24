#include "VirtualMachine.h"

VirtualMachine::VirtualMachine()
{
}

VirtualMachine::~VirtualMachine()
{
}

InterpretResult VirtualMachine::interpret(const std::string& source)
{
	//compile the source string into chunk
	
	chunk.write(OpCode::CONSTANT, 1);
	chunk.write(chunk.addConstant(Value("Hello")), 1);
	chunk.write(OpCode::CONSTANT, 1);
	chunk.write(chunk.addConstant(Value(69)), 1);
	chunk.write(OpCode::CONSTANT, 1);
	chunk.write(chunk.addConstant(Value(true)), 1);
	chunk.write(OpCode::RETURN, 1);

	//run the byte-code in chunk
	ip = chunk.code.begin();
	stackTop = stack.begin();
	return run();
}

//return the next byte in chunk.code and advance ip
uint8_t VirtualMachine::readByte()
{
	return *ip++;
}

//return the value in chunk.constants that the next byte in chunk.code indexes
Value VirtualMachine::readConstant()
{
	return chunk.constants[readByte()];
}

void VirtualMachine::push(Value val)
{
	*stackTop = val;
	stackTop++;
}

Value VirtualMachine::pop()
{
	stackTop--;
	return *stackTop;
}

InterpretResult VirtualMachine::run()
{
	using op = OpCode;

	while (true)
	{
		switch (readByte())
		{
		case (int)op::CONSTANT:
		{
			push(readConstant());
			break;
		}
		case (int)op::RETURN: return InterpretResult::OK;
			default: return InterpretResult::RuntimeError;
		}
	}

	return InterpretResult::OK;
}
