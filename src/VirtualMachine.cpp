#include "VirtualMachine.h"
#include "Scanner.h" //for testing

VirtualMachine::VirtualMachine()
{
}

VirtualMachine::~VirtualMachine()
{
}

InterpretResult VirtualMachine::interpret(const std::string& file)
{
	//compile the source file into chunk
	{
		Scanner scanner(file);
		for (Token token = scanner.scanToken(); token.type != TokenType::END; token = scanner.scanToken())
		{
			std::cout << "[Type] " << (int)token.type << "\n[Line] " << token.line << "\n[Depth] " << token.depth << "\n[Literal] " << token.literal << "\n\n";
		}
	}
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
	*stackTop = std::move(val);
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
		case (int)op::CONSTANT: push(readConstant()); break;
		case (int)op::NOT: push(!readConstant().asBool()); break;
		case (int)op::INEGATE: push(-readConstant().asInt()); break;
		case (int)op::DNEGATE: push(-readConstant().asDouble()); break;
		case (int)op::IADD: push(pop().asInt() + pop().asInt()); break;
		case (int)op::DADD: push(pop().asDouble() + pop().asDouble()); break;
		case (int)op::SADD:
		{
			std::string b = *pop().asString();
			std::string a = *pop().asString();
			push(a + b);
			break;
		}
		case (int)op::IMULTIPLY: push(pop().asInt() * pop().asInt()); break;
		case (int)op::DMULTIPLY: push(pop().asDouble() * pop().asDouble()); break;
		case (int)op::MODULO:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push(a % b);
			break;
		}
		case (int)op::ISUBTRACT:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push(a - b);
			break;
		}
		case (int)op::DSUBTRACT:
		{
			double b = pop().asDouble();
			double a = pop().asDouble();
			push(a - b);
			break;
		}
		case (int)op::IDIVIDE:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push(a / b);
			break;
		}
		case (int)op::DDIVIDE:
		{
			double b = pop().asDouble();
			double a = pop().asDouble();
			push(a / b);
			break;
		}
		case (int)op::IEXPONENT:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push((int)pow(a, b));
			break;
		}
		case (int)op::DEXPONENT:
		{
			double b = pop().asDouble();
			double a = pop().asDouble();
			push(pow(a, b));
			break;
		}
		case (int)op::RETURN: return InterpretResult::OK;
		default: return InterpretResult::RuntimeError;
		case (int)op::PRINT:
		{
			Value val = pop();
			switch (val.getType())
			{
			case ValueType::INT: std::cout << val.asInt() << "\n"; break;
			case ValueType::DOUBLE: std::cout << val.asDouble() << "\n"; break;
			case ValueType::BOOL: std::cout << val.asBool() << "\n"; break;
			case ValueType::CHAR: std::cout << val.asChar() << "\n"; break;
			case ValueType::STRING: std::cout << *val.asString() << "\n"; break;
			default: std::cout << "Invalid Type\n"; break;
			}
		}
		}
	}

	return InterpretResult::OK;
}
