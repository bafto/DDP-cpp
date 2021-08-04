#include "VirtualMachine.h"
#include "Compiler.h"

VirtualMachine::VirtualMachine()
{
}

VirtualMachine::~VirtualMachine()
{
}

InterpretResult VirtualMachine::interpret(const std::string& file)
{
	//compile the source file into chunk
	Function script;
	{
		Compiler compiler(file);
		script = compiler.compile();
		if (compiler.errored())
			return InterpretResult::CompilationError;
	}
	stackTop = stack.begin();
	push(Value(script));
	frame = frames.begin();
	frame->function = stack.begin();
	frame->ip = frame->function->asFunction()->chunk.code.begin();
	frame->slots = stack.begin();

	return run();
}

void VirtualMachine::runtimeError(std::string msg)
{
	std::cerr << msg;
}

//return the next byte in chunk.code and advance ip
uint8_t VirtualMachine::readByte()
{
	return *frame->ip++;
}

uint16_t VirtualMachine::readShort()
{
	return (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]));
}

//return the value in chunk.constants that the next byte in chunk.code indexes
Value VirtualMachine::readConstant()
{
	return frame->function->asFunction()->chunk.constants[readByte()];
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

Value VirtualMachine::peek(int distance)
{
	return stackTop[-1 - distance];
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
	bool forPrep = false;

	while (true)
	{
		switch (readByte())
		{
		case (int)op::CONSTANT: push(readConstant()); break;
		case (int)op::ARRAY:
		{
			int size = readConstant().asInt();
			ValueType type = (ValueType)readByte();
			switch (type)
			{
			case ValueType::INT:
			{
				std::vector<int> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(pop().asInt());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			case ValueType::DOUBLE:
			{
				std::vector<double> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(pop().asDouble());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			case ValueType::BOOL:
			{
				std::vector<bool> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(pop().asBool());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			case ValueType::CHAR:
			{
				std::vector<char> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(pop().asChar());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			case ValueType::STRING:
			{
				std::vector<std::string> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(*pop().asString());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			}
			break;
		}
		case (int)op::NOT: push(!pop().asBool()); break;
		case (int)op::NEGATE:
		{
			Value constant = pop();
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
		case (int)op::ROOT:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push(Value((int)pow((double)b, 1.0 / (double)a)));
			break;
		}
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
		case (int)op::LEFTBITSHIFT:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push(Value(a << b));
			break;
		}
		case (int)op::RIGHTBITSHIFT:
		{
			int b = pop().asInt();
			int a = pop().asInt();
			push(Value(a >> b));
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
				case ValueType::INT: 
				{
					if (forPrep)
					{
						OpCode code = a.asInt() <= b.asInt() ? op::GREATER : op::LESS;
						frame->ip[-1] = (uint8_t)code;
						if (code == op::GREATER) push(Value(a.asInt() > b.asInt()));
						else push(Value(a.asInt() < b.asInt()));
						break;
					}
					push(Value(a.asInt() > b.asInt()));
					break;
				}
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
		case (int)op::DEFINE_EMPTY_INTARR:
		{
			std::string name = *readConstant().asString();
			globals[name] = Value(std::vector<int>(pop().asInt(), 0));
			break;
		}
		case (int)op::DEFINE_EMPTY_DOUBLEARR:
		{
			std::string name = *readConstant().asString();
			globals[name] = Value(std::vector<double>(pop().asInt(), 0.0));
			break;
		}
		case (int)op::DEFINE_EMPTY_BOOLARR:
		{
			std::string name = *readConstant().asString();
			globals[name] = Value(std::vector<bool>(pop().asInt(), false));
			break;
		}
		case (int)op::DEFINE_EMPTY_CHARARR:
		{
			std::string name = *readConstant().asString();
			globals[name] = Value(std::vector<char>(pop().asInt(), 0));
			break;
		}
		case (int)op::DEFINE_EMPTY_STRINGARR:
		{
			std::string name = *readConstant().asString();
			globals[name] = Value(std::vector<std::string>(pop().asInt(), ""));
			break;
		}
		case (int)op::DEFINE_EMPTY_INTARR_LOCAL:
		{
			uint8_t slot = readByte();
			stack[slot] = Value(std::vector<int>(peek(0).asInt(), 0));
			break;
		}
		case (int)op::DEFINE_EMPTY_DOUBLEARR_LOCAL:
		{
			uint8_t slot = readByte();
			stack[slot] = Value(std::vector<double>(peek(0).asInt(), 0.0));
			break;
		}
		case (int)op::DEFINE_EMPTY_BOOLARR_LOCAL:
		{
			uint8_t slot = readByte();
			stack[slot] = Value(std::vector<bool>(peek(0).asInt(), false));
			break;
		}
		case (int)op::DEFINE_EMPTY_CHARARR_LOCAL:
		{
			uint8_t slot = readByte();
			stack[slot] = Value(std::vector<char>(peek(0).asInt(), 0));
			break;
		}
		case (int)op::DEFINE_EMPTY_STRINGARR_LOCAL:
		{
			uint8_t slot = readByte();
			stack[slot] = Value(std::vector<std::string>(peek(0).asInt(), ""));
			break;
		}
		case (int)op::GET_ARRAY_ELEMENT:
		{
			int index = pop().asInt();
			Value arr = globals.at(*pop().asString());
			try
			{
				switch (arr.getType())
				{
				case ValueType::INTARR: push(Value(arr.asIntArr()->at(index))); break;
				case ValueType::DOUBLEARR: push(Value(arr.asDoubleArr()->at(index))); break;
				case ValueType::BOOLARR: push(Value(arr.asBoolArr()->at(index))); break;
				case ValueType::CHARARR: push(Value(arr.asCharArr()->at(index))); break;
				case ValueType::STRINGARR: push(Value(arr.asStringArr()->at(index))); break;
				}
			}
			catch (std::out_of_range&)
			{
				runtimeError(u8"Der index ist außerhalb der Array Größe!");
				return InterpretResult::RuntimeError;
			}
			break;
		}
		case (int)op::SET_ARRAY_ELEMENT:
		{
			Value val = pop();
			int index = pop().asInt();
			Value& arr = globals.at(*pop().asString());
			switch (arr.getType())
			{
			case ValueType::INTARR: arr.asIntArr()->at(index) = val.asInt(); break;
			case ValueType::DOUBLEARR: arr.asDoubleArr()->at(index) = val.asDouble(); break;
			case ValueType::BOOLARR: arr.asBoolArr()->at(index) = val.asBool(); break;
			case ValueType::CHARARR: arr.asCharArr()->at(index) = val.asChar(); break;
			case ValueType::STRINGARR: arr.asStringArr()->at(index) = *val.asString(); break;
			}
			push(arr);
			break;
		}
		case (int)op::GET_ARRAY_ELEMENT_LOCAL:
		{
			int index = pop().asInt();
			Value arr = stack[readByte()];
			try
			{
				switch (arr.getType())
				{
				case ValueType::INTARR: push(Value(arr.asIntArr()->at(index))); break;
				case ValueType::DOUBLEARR: push(Value(arr.asDoubleArr()->at(index))); break;
				case ValueType::BOOLARR: push(Value(arr.asBoolArr()->at(index))); break;
				case ValueType::CHARARR: push(Value(arr.asCharArr()->at(index))); break;
				case ValueType::STRINGARR: push(Value(arr.asStringArr()->at(index))); break;
				}
			}
			catch (std::out_of_range&)
			{
				runtimeError(u8"Der index ist außerhalb der Array Größe!");
				return InterpretResult::RuntimeError;
			}
			break;
		}
		case (int)op::SET_ARRAY_ELEMENT_LOCAL:
		{
			Value val = pop();
			int index = pop().asInt();
			Value& arr = stack[readByte()];
			switch (arr.getType())
			{
			case ValueType::INTARR: arr.asIntArr()->at(index) = val.asInt(); break;
			case ValueType::DOUBLEARR: arr.asDoubleArr()->at(index) = val.asDouble(); break;
			case ValueType::BOOLARR: arr.asBoolArr()->at(index) = val.asBool(); break;
			case ValueType::CHARARR: arr.asCharArr()->at(index) = val.asChar(); break;
			case ValueType::STRINGARR: arr.asStringArr()->at(index) = *val.asString(); break;
			}
			push(arr);
			break;
		}
		case (int)op::DEFINE_GLOBAL:
		{
			std::string name = *readConstant().asString();
			globals[name] = peek(0);
			pop();
			break;
		}
		case (int)op::GET_GLOBAL:
		{
			std::string name = *readConstant().asString();
			Value val = globals.at(name);
			push(val);
			break;
		}
		case (int)op::SET_GLOBAL:
		{
			std::string name = *readConstant().asString();
			globals[name] = peek(0);
			break;
		}
		case (int)op::GET_LOCAL:
		{
			uint8_t slot = readByte();
			push(stack[slot]);
			break;
		}
		case (int)op::SET_LOCAL:
		{
			uint8_t slot = readByte();
			stack[slot] = peek(0);
			break;
		}
		case (int)op::JUMP:
		{
			uint16_t offset = readShort();
			frame->ip += offset;
			break;
		}
		case (int)op::JUMP_IF_FALSE:
		{
			uint16_t offset = readShort();
			if (!(peek(0).asBool())) frame->ip += offset;
			break;
		}
		case (int)op::LOOP:
		{
			uint16_t offset = readShort();
			frame->ip -= offset;
			break;
		}
		case (int)op::POP: pop(); break;
		case (int)op::FORPREP: forPrep = true; break;
		case (int)op::FORDONE: forPrep = false; break;
		case (int)op::RETURN: return InterpretResult::OK;
		default: return InterpretResult::RuntimeError;
#ifdef _MDEBUG_
		case (int)op::PRINT:
		{
			Value val = pop();
			switch (val.getType())
			{
			case ValueType::INT: std::cout << val.asInt() << "\n"; break;
			case ValueType::DOUBLE: std::cout << val.asDouble() << "\n"; break;
			case ValueType::BOOL: std::cout << (val.asBool() ? u8"wahr" : u8"falsch") << "\n"; break;
			case ValueType::CHAR: std::cout << val.asChar() << "\n"; break;
			case ValueType::STRING: std::cout << *val.asString() << "\n"; break;
			case ValueType::INTARR:
			case ValueType::DOUBLEARR:
			case ValueType::BOOLARR:
			case ValueType::CHARARR:
			case ValueType::STRINGARR: val.printValue(); std::cout << std::endl; break;
			default: std::cout << u8"Invalid Type\n"; break;
			}
			break;
		}
#endif
		}
	}

	return InterpretResult::OK;
}
