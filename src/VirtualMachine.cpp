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
	
	chunk.writeChunk(OpCode::CONSTANT, 1);
	chunk.writeChunk(chunk.addConstant(Value("Hello")), 1);
	chunk.writeChunk(OpCode::CONSTANT, 1);
	chunk.writeChunk(chunk.addConstant(Value(69)), 1);
	chunk.writeChunk(OpCode::CONSTANT, 1);
	chunk.writeChunk(chunk.addConstant(Value(true)), 1);
	chunk.writeChunk(OpCode::RETURN, 1);

	//run the byte-code in chunk
	ip = chunk.code.begin();
	return run();
}

uint8_t VirtualMachine::readByte()
{
	return *ip++;
}

Value VirtualMachine::readConstant()
{
	return chunk.constants[readByte()];
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
			Value constant = readConstant();
			constant.printValue();
			std::cout << "\n";
			break;
		}
		case (int)op::RETURN: return InterpretResult::OK;
			default: return InterpretResult::RuntimeError;
		}
	}

	return InterpretResult::OK;
}
