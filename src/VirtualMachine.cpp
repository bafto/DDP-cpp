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
		auto tokens = scanner.scanTokens();
		for (auto& token : tokens)
			std::cout << "[Type] " << (int)token.type << "  [Line] " << token.line << "  [Depth] " << token.depth << "  [Literal] " << token.literal << "\n";
	}
	return InterpretResult::OK;
	/*chunk.write(OpCode::RETURN, 1);
	//run the byte-code in chunk
	ip = chunk.code.begin();
	stackTop = stack.begin();
	return run();*/
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

void VirtualMachine::addition()
{
	Value b = pop();
	ValueType bType = b.getType();
	Value a = pop();
	ValueType aType = a.getType();

	switch (aType)
	{
	case ValueType::INT:
		switch (bType)
		{
		case ValueType::INT:
			push(Value(a.asInt() + b.asInt()));
			return;
		case ValueType::DOUBLE:
			push(Value((double)(a.asInt() + b.asDouble())));
			return;
		case ValueType::CHAR:
			push(Value((a.asInt() + b.asChar())));
			return;
		case ValueType::STRING:
			push(Value(std::string(std::to_string(a.asInt()) + *b.asString())));
			return;
		}
	case ValueType::DOUBLE:
		switch (bType)
		{
		case ValueType::INT:
			push(Value((double)(a.asDouble() + b.asInt())));
			return;
		case ValueType::DOUBLE:
			push(Value(a.asDouble() + b.asDouble()));
			return;
		case ValueType::CHAR:
			push(Value(((int)a.asDouble() + (int)b.asChar())));
			return;
		case ValueType::STRING:
			std::string astr(std::to_string(a.asDouble()));
			astr.replace(astr.begin(), astr.end(), '.', ',');
			push(Value(astr + *b.asString()));
			return;
		}
	case ValueType::CHAR:
		switch (bType)
		{
		case ValueType::INT:
			push(Value((a.asChar() + b.asInt())));
			return;
		case ValueType::DOUBLE:
			push(Value((a.asChar() + (int)b.asDouble())));
			return;
		case ValueType::CHAR:
			push(Value(std::string(1, a.asChar()) + std::string(1, b.asChar())));
			return;
		case ValueType::STRING:
			push(Value(std::string(1, a.asChar()) + *b.asString()));
			return;
		}
	case ValueType::STRING:
		switch (bType)
		{
		case ValueType::INT:
			push(Value(*a.asString() + std::to_string(b.asInt())));
			return;
		case ValueType::DOUBLE:
		{
			std::string str(std::to_string(b.asDouble()));
			str.replace(str.begin(), str.end(), '.', ',');
			push(Value(*a.asString() + str));
			return;
		}
		case ValueType::CHAR:
			push(Value(*a.asString() + std::string(1, b.asChar())));
			return;
		case ValueType::STRING:
			push(Value(*a.asString() + *b.asString()));
			return;
		default:
			return;
		}
	}
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
		case (int)op::NEGATE:
		{
			Value constant = readConstant();
			switch (constant.getType())
			{
			case ValueType::INT: push(-constant.asInt()); break;
			case ValueType::DOUBLE: push(-constant.asDouble()); break;
			}
			break;
		}
		case (int)op::ADD: addition(); break;
		case (int)op::MULTIPLY:
		{
			Value b = pop();
			Value a = pop();
			switch (a.getType())
			{
			case ValueType::INT:
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asInt() * b.asInt())); break;
				case ValueType::DOUBLE: push(Value((double)(a.asInt() * b.asDouble()))); break;
				}
				break;
			case ValueType::DOUBLE:
				switch (b.getType())
				{
				case ValueType::INT: push(Value((double)(a.asDouble() * b.asInt()))); break;
				case ValueType::DOUBLE: push(Value(a.asDouble() * b.asDouble())); break;
				}
				break;
			}
			break;
		}
		case (int)op::MODULO:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push(Value(a % b));
			break;
		}
		case (int)op::SUBTRACT:
		{
			Value b = pop();
			Value a = pop();
			switch (a.getType())
			{
			case ValueType::INT:
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asInt() - b.asInt())); break;
				case ValueType::DOUBLE: push(Value((double)(a.asInt() - b.asDouble()))); break;
				}
				break;
			case ValueType::DOUBLE:
				switch (b.getType())
				{
				case ValueType::INT: push(Value((double)(a.asDouble() - b.asInt()))); break;
				case ValueType::DOUBLE: push(Value(a.asDouble() - b.asDouble())); break;
				}
				break;
			}
			break;
		}
		case (int)op::DIVIDE:
		{
			Value b = pop();
			Value a = pop();
			switch (a.getType())
			{
			case ValueType::INT:
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asInt() / b.asInt())); break;
				case ValueType::DOUBLE: push(Value((double)(a.asInt() / b.asDouble()))); break;
				}
				break;
			case ValueType::DOUBLE:
				switch (b.getType())
				{
				case ValueType::INT: push(Value((double)(a.asDouble() / b.asInt()))); break;
				case ValueType::DOUBLE: push(Value(a.asDouble() / b.asDouble())); break;
				}
				break;
			}
			break;
		}
		case (int)op::EXPONENT:
		{
			Value b = pop();
			Value a = pop();
			switch (a.getType())
			{
			case ValueType::INT:
				switch (b.getType())
				{
				case ValueType::INT: push(Value((int)pow(a.asInt(), b.asInt()))); break;
				case ValueType::DOUBLE: push(Value((double)pow(a.asInt(), b.asDouble()))); break;
				}
				break;
			case ValueType::DOUBLE:
				switch (b.getType())
				{
				case ValueType::INT: push(Value((double)pow(a.asDouble(), b.asInt()))); break;
				case ValueType::DOUBLE: push(Value(pow(a.asDouble(), b.asDouble()))); break;
				}
				break;
			}
			break;
		}
		case (int)op::ROOT: push(Value(pow((double)pop().asInt(), (double)(1.0 / (double)pop().asInt())))); break;
		case (int)op::LN:
		{
			Value val = pop();
			switch (val.getType())
			{
			case ValueType::INT: push(Value((double)log(val.asInt()))); break;
			case ValueType::DOUBLE: push(Value(log(val.asDouble()))); break;
			}
			break;
		}
		case (int)op::BETRAG:
		{
			Value val = pop();
			switch (val.getType())
			{
			case ValueType::INT: push(Value(abs(val.asInt()))); break;
			case ValueType::DOUBLE: push(Value(abs(val.asDouble()))); break;
			}
			break;
		}
		case (int)op::BITWISENOT: push(Value(~pop().asInt())); break;
		case (int)op::BITWISEAND:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push(Value(a & b));
			break;
		}
		case (int)op::BITWISEOR:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push(Value(a | b));
			break;
		}
		case (int)op::BITWISEXOR:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push(Value(a ^ b));
			break;
		}
		case (int)op::EQUAL:
		{
			Value b = pop();
			Value a = pop();
			switch (a.getType())
			{
			case ValueType::INT:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asInt() == b.asInt())); break;
				case ValueType::DOUBLE: push(Value((double)a.asInt() == b.asDouble())); break;
				}
				break;
			}
			case ValueType::DOUBLE:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asDouble() == (double)b.asInt())); break;
				case ValueType::DOUBLE: push(Value(a.asDouble() == b.asDouble())); break;
				}
				break;
			}
			case ValueType::BOOL: push(Value(a.asBool() == b.asBool())); break;
			case ValueType::CHAR: push(Value(a.asChar() == b.asChar())); break;
			case ValueType::STRING: push(Value(*a.asString() == *b.asString())); break;
			} 
			break;
		}
		case (int)op::UNEQUAL:
		{
			Value b = pop();
			Value a = pop();
			switch (a.getType())
			{
			case ValueType::INT:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asInt() != b.asInt())); break;
				case ValueType::DOUBLE: push(Value((double)a.asInt() != b.asDouble())); break;
				}
				break;
			}
			case ValueType::DOUBLE:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asDouble() != (double)b.asInt())); break;
				case ValueType::DOUBLE: push(Value(a.asDouble() != b.asDouble())); break;
				}
				break;
			}
			case ValueType::BOOL: push(Value(a.asBool() != b.asBool())); break;
			case ValueType::CHAR: push(Value(a.asChar() != b.asChar())); break;
			case ValueType::STRING: push(Value(*a.asString() != *b.asString())); break;
			}
			break;
		}
		case (int)op::GREATER:
		{
			Value b = pop();
			Value a = pop();
			switch (a.getType())
			{
			case ValueType::INT:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asInt() > b.asInt())); break;
				case ValueType::DOUBLE: push(Value((double)a.asInt() > b.asDouble())); break;
				}
				break;
			}
			case ValueType::DOUBLE:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asDouble() > (double)b.asInt())); break;
				case ValueType::DOUBLE: push(Value(a.asDouble() > b.asDouble())); break;
				}
				break;
			}
			}
			break;
		}
		case (int)op::GREATEREQUAL:
		{
			Value b = pop();
			Value a = pop();
			switch (a.getType())
			{
			case ValueType::INT:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asInt() >= b.asInt())); break;
				case ValueType::DOUBLE: push(Value((double)a.asInt() >= b.asDouble())); break;
				}
				break;
			}
			case ValueType::DOUBLE:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asDouble() >= (double)b.asInt())); break;
				case ValueType::DOUBLE: push(Value(a.asDouble() >= b.asDouble())); break;
				}
				break;
			}
			}
			break;
		}
		case (int)op::LESS:
		{
			Value b = pop();
			Value a = pop();
			switch (a.getType())
			{
			case ValueType::INT:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asInt() < b.asInt())); break;
				case ValueType::DOUBLE: push(Value((double)a.asInt() < b.asDouble())); break;
				}
				break;
			}
			case ValueType::DOUBLE:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asDouble() < (double)b.asInt())); break;
				case ValueType::DOUBLE: push(Value(a.asDouble() < b.asDouble())); break;
				}
				break;
			}
			}
			break;
		}
		case (int)op::LESSEQUAL:
		{
			Value b = pop();
			Value a = pop();
			switch (a.getType())
			{
			case ValueType::INT:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asInt() <= b.asInt())); break;
				case ValueType::DOUBLE: push(Value((double)a.asInt() <= b.asDouble())); break;
				}
				break;
			}
			case ValueType::DOUBLE:
			{
				switch (b.getType())
				{
				case ValueType::INT: push(Value(a.asDouble() <= (double)b.asInt())); break;
				case ValueType::DOUBLE: push(Value(a.asDouble() <= b.asDouble())); break;
				}
				break;
			}
			}
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
			break;
		}
		}
	}

	return InterpretResult::OK;
}
