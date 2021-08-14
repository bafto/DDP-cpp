#include "Function.h"
#include <iostream>

Function::Function()
	:
	returnType(ValueType::None),
	args(),
	functions(nullptr),
	globals(nullptr),
	argUnit(0),
	returned(false)
{}

Value Function::run(std::unordered_map<std::string, Value>* globals, std::unordered_map<std::string, Function>* functions)
{
	using op = OpCode;

	this->globals = globals;
	this->functions = functions;

	stackTop = stack.begin();
	ip = chunk.bytes.begin();

	bool forPrep = false;

	while (true)
	{
		switch ((OpCode)readByte())
		{
		case op::CONSTANT: push(readConstant()); break;
		case op::ARRAY:
		{
			int size = readConstant().Int();
			ValueType type = (ValueType)readByte();
			switch (type)
			{
			case ValueType::IntArr:
			{
				std::vector<int> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(pop().Int());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			case ValueType::DoubleArr:
			{
				std::vector<double> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(pop().Double());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			case ValueType::BoolArr:
			{
				std::vector<bool> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(pop().Bool());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			case ValueType::CharArr:
			{
				std::vector<char> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(pop().Char());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			case ValueType::StringArr:
			{
				std::vector<std::string> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(*pop().String());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			}
			break;
		}
		case op::NOT: push(!pop().Bool()); break;
		case op::NEGATE:
		{
			Value val = pop();
			switch (val.Type())
			{
			case ValueType::Int: push(-val.Int()); break;
			case ValueType::Double: push(-val.Double()); break;
			}
			break;
		}
		case op::ADD: addition(); break;
		case op::MULTIPLY:
		{
			Value b = pop();
			Value a = pop();
			switch (a.Type())
			{
			case ValueType::Int:
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Int() * b.Int())); break;
				case ValueType::Double: push(Value((double)(a.Int() * b.Double()))); break;
				}
				break;
			case ValueType::Double:
				switch (b.Type())
				{
				case ValueType::Int: push(Value((double)(a.Double() * b.Int()))); break;
				case ValueType::Double: push(Value(a.Double() * b.Double())); break;
				}
				break;
			}
			break;
		}
		case op::DIVIDE:
		{
			Value b = pop();
			Value a = pop();
			switch (a.Type())
			{
			case ValueType::Int:
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Int() / b.Int())); break;
				case ValueType::Double: push(Value((double)(a.Int() / b.Double()))); break;
				}
				break;
			case ValueType::Double:
				switch (b.Type())
				{
				case ValueType::Int: push(Value((double)(a.Double() / b.Int()))); break;
				case ValueType::Double: push(Value(a.Double() / b.Double())); break;
				}
				break;
			}
			break;
		}
		case op::MODULO:
		{
			int b = pop().Int();
			int a = pop().Int();
			push(Value(a % b));
			break;
		}
		case op::SUBTRACT:
		{
			Value b = pop();
			Value a = pop();
			switch (a.Type())
			{
			case ValueType::Int:
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Int() - b.Int())); break;
				case ValueType::Double: push(Value((double)(a.Int() - b.Double()))); break;
				}
				break;
			case ValueType::Double:
				switch (b.Type())
				{
				case ValueType::Int: push(Value((double)(a.Double() - b.Int()))); break;
				case ValueType::Double: push(Value(a.Double() - b.Double())); break;
				}
				break;
			}
			break;
		}
		case op::EXPONENT:
		{
			Value b = pop();
			Value a = pop();
			switch (a.Type())
			{
			case ValueType::Int:
				switch (b.Type())
				{
				case ValueType::Int: push(Value((int)pow(a.Int(), b.Int()))); break;
				case ValueType::Double: push(Value((double)pow(a.Int(), b.Double()))); break;
				}
				break;
			case ValueType::Double:
				switch (b.Type())
				{
				case ValueType::Int: push(Value((double)pow(a.Double(), b.Int()))); break;
				case ValueType::Double: push(Value(pow(a.Double(), b.Double()))); break;
				}
				break;
			}
			break;
		}
		case op::ROOT:
		{
			int b = pop().Int();
			int a = pop().Int();
			push(Value(pow((double)b, 1.0 / (double)a)));
			break;
		}
		case op::LN:
		{
			Value val = pop();
			switch (val.Type())
			{
			case ValueType::Int: push(Value((double)log(val.Int()))); break;
			case ValueType::Double: push(Value(log(val.Double()))); break;
			}
			break;
		}
		case op::BETRAG:
		{
			Value val = pop();
			switch (val.Type())
			{
			case ValueType::Int: push(Value(abs(val.Int()))); break;
			case ValueType::Double: push(Value(abs(val.Double()))); break;
			}
			break;
		}
		case op::BITWISENOT: push(Value(~pop().Int())); break;
		case op::BITWISEAND:
		{
			int b = pop().Int();
			int a = pop().Int();
			push(Value(a & b));
			break;
		}
		case op::BITWISEOR:
		{
			int b = pop().Int();
			int a = pop().Int();
			push(Value(a | b));
			break;
		}
		case op::BITWISEXOR:
		{
			int b = pop().Int();
			int a = pop().Int();
			push(Value(a ^ b));
			break;
		}
		case op::LEFTBITSHIFT:
		{
			int b = pop().Int();
			int a = pop().Int();
			push(Value(a << b));
			break;
		}
		case op::RIGHTBITSHIFT:
		{
			int b = pop().Int();
			int a = pop().Int();
			push(Value(a >> b));
			break;
		}
		case op::EQUAL:
		{
			Value b = pop();
			Value a = pop();
			switch (a.Type())
			{
			case ValueType::Int:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Int() == b.Int())); break;
				case ValueType::Double: push(Value((double)a.Int() == b.Double())); break;
				}
				break;
			}
			case ValueType::Double:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Double() == (double)b.Int())); break;
				case ValueType::Double: push(Value(a.Double() == b.Double())); break;
				}
				break;
			}
			case ValueType::Bool: push(Value(a.Bool() == b.Bool())); break;
			case ValueType::Char: push(Value(a.Char() == b.Char())); break;
			case ValueType::String: push(Value(*a.String() == *b.String())); break;
			}
			break;
		}
		case op::UNEQUAL:
		{
			Value b = pop();
			Value a = pop();
			switch (a.Type())
			{
			case ValueType::Int:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Int() != b.Int())); break;
				case ValueType::Double: push(Value((double)a.Int() != b.Double())); break;
				}
				break;
			}
			case ValueType::Double:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Double() != (double)b.Int())); break;
				case ValueType::Double: push(Value(a.Double() != b.Double())); break;
				}
				break;
			}
			case ValueType::Bool: push(Value(a.Bool() != b.Bool())); break;
			case ValueType::Char: push(Value(a.Char() != b.Char())); break;
			case ValueType::String: push(Value(*a.String() != *b.String())); break;
			}
			break;
		}
		case op::GREATER:
		{
			Value b = pop();
			Value a = pop();
			switch (a.Type())
			{
			case ValueType::Int:
			{
				switch (b.Type())
				{
				case ValueType::Int:
				{
					if (forPrep)
					{
						OpCode code = a.Int() <= b.Int() ? op::GREATER : op::LESS;
						ip[-1] = (uint8_t)code;
						if (code == op::GREATER) push(Value(a.Int() > b.Int()));
						else push(Value(a.Int() < b.Int()));
						break;
					}
					push(Value(a.Int() > b.Int()));
					break;
				}
				case ValueType::Double: push(Value((double)a.Int() > b.Double())); break;
				}
				break;
			}
			case ValueType::Double:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Double() > (double)b.Int())); break;
				case ValueType::Double: push(Value(a.Double() > b.Double())); break;
				}
				break;
			}
			}
			break;
		}
		case op::GREATEREQUAL:
		{
			Value b = pop();
			Value a = pop();
			switch (a.Type())
			{
			case ValueType::Int:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Int() >= b.Int())); break;
				case ValueType::Double: push(Value((double)a.Int() >= b.Double())); break;
				}
				break;
			}
			case ValueType::Double:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Double() >= (double)b.Int())); break;
				case ValueType::Double: push(Value(a.Double() >= b.Double())); break;
				}
				break;
			}
			}
			break;
		}
		case op::LESS:
		{
			Value b = pop();
			Value a = pop();
			switch (a.Type())
			{
			case ValueType::Int:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Int() < b.Int())); break;
				case ValueType::Double: push(Value((double)a.Int() < b.Double())); break;
				}
				break;
			}
			case ValueType::Double:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Double() < (double)b.Int())); break;
				case ValueType::Double: push(Value(a.Double() < b.Double())); break;
				}
				break;
			}
			}
			break;
		}
		case op::LESSEQUAL:
		{
			Value b = pop();
			Value a = pop();
			switch (a.Type())
			{
			case ValueType::Int:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Int() <= b.Int())); break;
				case ValueType::Double: push(Value((double)a.Int() <= b.Double())); break;
				}
				break;
			}
			case ValueType::Double:
			{
				switch (b.Type())
				{
				case ValueType::Int: push(Value(a.Double() <= (double)b.Int())); break;
				case ValueType::Double: push(Value(a.Double() <= b.Double())); break;
				}
				break;
			}
			}
			break;
		}
		case op::DEFINE_GLOBAL:
		{
			std::string varName = *readConstant().String();
			Value val = pop();
			ValueType varType = globals->at(varName).Type();
			if ((int)varType >= (int)ValueType::IntArr && (int)varType <= (int)ValueType::StringArr && val.Type() == ValueType::Int)
			{
				switch (varType)
				{
				case ValueType::IntArr: val = Value(std::vector<int>(val.Int(), 0)); break;
				case ValueType::DoubleArr: val = Value(std::vector<double>(val.Int(), 0.0)); break;
				case ValueType::BoolArr: val = Value(std::vector<bool>(val.Int(), false)); break;
				case ValueType::CharArr: val = Value(std::vector<char>(val.Int(), (char)0)); break;
				case ValueType::StringArr: val = Value(std::vector<std::string>(val.Int(), "")); break;
				}
			}

			(*globals)[varName] = std::move(val);
			break;
		}
		case op::DEFINE_LOCAL:
		{
			std::string varName = *readConstant().String();
			int unit = readConstant().Int();
			Value val = pop();
			ValueType varType = locals.at(unit).at(varName).Type();
			if ((int)varType >= (int)ValueType::IntArr && (int)varType <= (int)ValueType::StringArr && val.Type() == ValueType::Int)
			{
				switch (varType)
				{
				case ValueType::IntArr: val = Value(std::vector<int>(val.Int(), 0)); break;
				case ValueType::DoubleArr: val = Value(std::vector<double>(val.Int(), 0.0)); break;
				case ValueType::BoolArr: val = Value(std::vector<bool>(val.Int(), false)); break;
				case ValueType::CharArr: val = Value(std::vector<char>(val.Int(), (char)0)); break;
				case ValueType::StringArr: val = Value(std::vector<std::string>(val.Int(), "")); break;
				}
			}

			locals.at(unit)[varName] = std::move(val);
			break;
		}
		case op::GET_GLOBAL:
		{
			std::string varName = *readConstant().String();
			push(globals->at(varName));
			break;
		}
		case op::GET_LOCAL:
		{
			std::string varName = *readConstant().String();
			int unit = readConstant().Int();
			push(locals.at(unit).at(varName));
			break;
		}
		case op::GET_ARRAY_ELEMENT:
		{
			std::string arrName = *readConstant().String();
			int index = pop().Int();
			switch (globals->at(arrName).Type())
			{
			case ValueType::IntArr: validateArray(globals->at(arrName).IntArr(), index); push(Value(globals->at(arrName).IntArr()->at(index))); break;
			case ValueType::DoubleArr: validateArray(globals->at(arrName).DoubleArr(), index); push(Value(globals->at(arrName).DoubleArr()->at(index))); break;
			case ValueType::BoolArr: validateArray(globals->at(arrName).BoolArr(), index); push(Value(globals->at(arrName).BoolArr()->at(index))); break;
			case ValueType::CharArr: validateArray(globals->at(arrName).CharArr(), index); push(Value(globals->at(arrName).CharArr()->at(index))); break;
			case ValueType::StringArr: validateArray(globals->at(arrName).StringArr(), index); push(Value(globals->at(arrName).StringArr()->at(index))); break;
			default: throw runtime_error("Tried to index non-Array!");
			}
			break;
		}
		case op::GET_ARRAY_ELEMENT_LOCAL:
		{
			std::string arrName = *readConstant().String();
			int unit = readConstant().Int();
			int index = pop().Int();
			switch (locals.at(unit).at(arrName).Type())
			{
			case ValueType::IntArr: validateArray(locals.at(unit).at(arrName).IntArr(), index); push(Value(locals.at(unit).at(arrName).IntArr()->at(index))); break;
			case ValueType::DoubleArr: validateArray(locals.at(unit).at(arrName).DoubleArr(), index); push(Value(locals.at(unit).at(arrName).DoubleArr()->at(index))); break;
			case ValueType::BoolArr: validateArray(locals.at(unit).at(arrName).BoolArr(), index); push(Value(locals.at(unit).at(arrName).BoolArr()->at(index))); break;
			case ValueType::CharArr: validateArray(locals.at(unit).at(arrName).CharArr(), index); push(Value(locals.at(unit).at(arrName).CharArr()->at(index))); break;
			case ValueType::StringArr: validateArray(locals.at(unit).at(arrName).StringArr(), index); push(Value(locals.at(unit).at(arrName).StringArr()->at(index))); break;
			default: throw runtime_error("Tried to index non-Array!");
			}
			break;
		}
		case op::SET_GLOBAL:
		{
			std::string varName = *readConstant().String();
			(*globals)[varName] = std::move(peek(0));
			break;
		}
		case op::SET_LOCAL:
		{
			std::string varName = *readConstant().String();
			int unit = readConstant().Int();
			(locals[unit])[varName] = std::move(peek(0));
			break;
		}
		case op::SET_ARRAY_ELEMENT:
		{
			std::string arrName = *readConstant().String();
			Value val = std::move(pop());
			int index = peek(0).Int();
			switch (globals->at(arrName).Type())
			{
			case ValueType::IntArr: validateArray(globals->at(arrName).IntArr(), index); (*(globals->at(arrName).IntArr()))[index] = val.Int(); break;
			case ValueType::DoubleArr: validateArray(globals->at(arrName).DoubleArr(), index); (*(globals->at(arrName).DoubleArr()))[index] = val.Double(); break;
			case ValueType::BoolArr: validateArray(globals->at(arrName).BoolArr(), index); (*(globals->at(arrName).BoolArr()))[index] = val.Bool(); break;
			case ValueType::CharArr: validateArray(globals->at(arrName).CharArr(), index); (*(globals->at(arrName).CharArr()))[index] = val.Char(); break;
			case ValueType::StringArr: validateArray(globals->at(arrName).StringArr(), index); (*(globals->at(arrName).StringArr()))[index] = *val.String(); break;
			default: throw runtime_error("Tried to index non-Array!");
			}
			break;
		}
		case op::SET_ARRAY_ELEMENT_LOCAL:
		{
			std::string arrName = *readConstant().String();
			int unit = readConstant().Int();
			Value val = std::move(pop());
			int index = peek(0).Int();
			switch (locals.at(unit).at(arrName).Type())
			{
			case ValueType::IntArr: validateArray(locals.at(unit).at(arrName).IntArr(), index); (*(locals.at(unit).at(arrName).IntArr()))[index] = val.Int(); break;
			case ValueType::DoubleArr: validateArray(locals.at(unit).at(arrName).DoubleArr(), index); (*(locals.at(unit).at(arrName).DoubleArr()))[index] = val.Double(); break;
			case ValueType::BoolArr: validateArray(locals.at(unit).at(arrName).BoolArr(), index); (*(locals.at(unit).at(arrName).BoolArr()))[index] = val.Bool(); break;
			case ValueType::CharArr: validateArray(locals.at(unit).at(arrName).CharArr(), index); (*(locals.at(unit).at(arrName).CharArr()))[index] = val.Char(); break;
			case ValueType::StringArr: validateArray(locals.at(unit).at(arrName).StringArr(), index); (*(locals.at(unit).at(arrName).StringArr()))[index] = *val.String(); break;
			default: throw runtime_error("Tried to index non-Array!");
			}
			break;
		}
		case op::JUMP_IF_FALSE:
		{
			uint16_t offset = readShort();
			if (!(peek(0).Bool())) ip += offset;
			break;
		}
		case op::JUMP:
		{
			uint16_t offset = readShort();
			ip += offset;
			break;
		}
		case op::LOOP:
		{
			uint16_t offset = readShort();
			ip -= offset;
			break;
		}
		case op::RETURN:
		{
			//Temp
			if (returnType != ValueType::None) return pop();
			return Value();
		}
		case op::CALL:
		{
			Function* func = &functions->at(*readConstant().String());
			for (int i = func->args.size() - 1; i >= 0; i--)
			{
				func->locals.at(func->argUnit)[func->args.at(i).first] = pop();
			}
			push(func->run(globals, functions));
			break;
		}
		case op::POP: pop(); break;
		case op::FORPREP: forPrep = true; break;
		case op::FORDONE: forPrep = false; break;
#ifndef NDEBUG
		case op::PRINT:
		{
			Value val = pop();
			printValue(val);
			break;
		}
#endif
		default: throw runtime_error(u8"Unknown byte instruction!");
			break;
		}
	}

	if (returnType != ValueType::None) return pop();
	return Value();
}

void Function::push(Value value)
{
	*stackTop = std::move(value);
	stackTop++;
	if (stackTop == stack.end())
		throw runtime_error("Stapel Überfluss!");
}

Value Function::pop()
{
	stackTop--;
	return *stackTop;
}

Value Function::peek(int distance)
{
	return stackTop[-1 - static_cast<ptrdiff_t>(distance)];
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

void Function::printValue(Value& val)
{
	switch (val.Type())
	{
	case ValueType::Int: std::cout << val.Int() << "\n"; break;
	case ValueType::Double: std::cout << val.Double() << "\n"; break;
	case ValueType::Bool: std::cout << (val.Bool() ? u8"wahr" : u8"falsch") << "\n"; break;
	case ValueType::Char: std::cout << val.Char() << "\n"; break;
	case ValueType::String: std::cout << *val.String() << "\n"; break;
	case ValueType::IntArr:
	{
		std::cout << u8"[";
		std::vector<int>*& vec = val.IntArr();
		for (size_t i = 0; i < vec->size() - 1; i++)
		{
			std::cout << vec->at(i) << u8"; ";
		}
		std::cout << vec->at(vec->size() - 1) << u8"]";
		break;
	}
	case ValueType::DoubleArr:
	{
		std::cout << u8"[";
		std::vector<double>*& vec = val.DoubleArr();
		for (size_t i = 0; i < vec->size() - 1; i++)
		{
			std::cout << vec->at(i) << u8"; ";
		}
		std::cout << vec->at(vec->size() - 1) << u8"]";
		break;
	}
	case ValueType::BoolArr:
	{
		std::cout << u8"[";
		std::vector<bool>*& vec = val.BoolArr();
		for (size_t i = 0; i < vec->size() - 1; i++)
		{
			std::cout << (vec->at(i) ? u8"wahr" : u8"falsch") << u8"; ";
		}
		std::cout << (vec->at(vec->size() - 1) ? u8"wahr" : u8"falsch") << u8"]";
		break;
	}
	case ValueType::CharArr:
	{
		std::cout << u8"[";
		std::vector<char>*& vec = val.CharArr();
		for (size_t i = 0; i < vec->size() - 1; i++)
		{
			std::cout << vec->at(i) << u8"; ";
		}
		std::cout << vec->at(vec->size() - 1) << u8"]";
		break;
	}
	case ValueType::StringArr:
	{
		std::cout << u8"[\"";
		std::vector<std::string>*& vec = val.StringArr();
		for (size_t i = 0; i < vec->size() - 1; i++)
		{
			std::cout << vec->at(i) << u8"\"; \"";
		}
		std::cout << vec->at(vec->size() - 1) << u8"\"]";
		break;
	}
	default: std::cout << "Invalid type!\n"; break;
	}
}

void Function::addition()
{
	Value b = pop();
	ValueType bType = b.Type();
	Value a = pop();
	ValueType aType = a.Type();

	switch (aType)
	{
	case ValueType::Int:
		switch (bType)
		{
		case ValueType::Int:
			push(Value(a.Int() + b.Int()));
			return;
		case ValueType::Double:
			push(Value((double)(a.Int() + b.Double())));
			return;
		case ValueType::Char:
			push(Value((a.Int() + b.Char())));
			return;
		case ValueType::String:
			push(Value(std::string(std::to_string(a.Int()) + *b.String())));
			return;
		}
	case ValueType::Double:
		switch (bType)
		{
		case ValueType::Int:
			push(Value((double)(a.Double() + b.Int())));
			return;
		case ValueType::Double:
			push(Value(a.Double() + b.Double()));
			return;
		case ValueType::Char:
			push(Value(((int)a.Double() + (int)b.Char())));
			return;
		case ValueType::String:
			std::string astr(std::to_string(a.Double()));
			astr.replace(astr.begin(), astr.end(), '.', ',');
			push(Value(astr + *b.String()));
			return;
		}
	case ValueType::Char:
		switch (bType)
		{
		case ValueType::Int:
			push(Value((a.Char() + b.Int())));
			return;
		case ValueType::Double:
			push(Value((a.Char() + (int)b.Double())));
			return;
		case ValueType::Char:
			push(Value(std::string(1, a.Char()) + std::string(1, b.Char())));
			return;
		case ValueType::String:
			push(Value(std::string(1, a.Char()) + *b.String()));
			return;
		}
	case ValueType::String:
		switch (bType)
		{
		case ValueType::Int:
			push(Value(*a.String() + std::to_string(b.Int())));
			return;
		case ValueType::Double:
		{
			std::string str(std::to_string(b.Double()));
			str.replace(str.begin(), str.end(), '.', ',');
			push(Value(*a.String() + str));
			return;
		}
		case ValueType::Char:
			push(Value(*a.String() + std::string(1, b.Char())));
			return;
		case ValueType::String:
			push(Value(*a.String() + *b.String()));
			return;
		default:
			return;
		}
	}
}