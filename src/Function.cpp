#include "Function.h"
#include <iostream>

Function::Function(ValueType returnType, const std::vector<ValueType>& args)
	:
	returnType(returnType),
	args(args),
	functions(nullptr),
	globals(nullptr)
{}

Value Function::run(std::unordered_map<std::string, Value>* globals, std::unordered_map<std::string, Function>* functions)
{
	using op = OpCode;

	this->globals = globals;
	this->functions = functions;

	stackTop = stack.begin();
	ip = chunk.bytes.begin();

	while (true)
	{
		switch ((OpCode)readByte())
		{
		case op::CONSTANT: push(readConstant()); break;
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
				case ValueType::Int: push(Value(a.Int() > b.Int())); break;
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
		case op::RETURN:
		{
			//Temp
			if (returnType != ValueType::None) return pop();
			return Value();
		}
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
			std::cout << vec->at(i) << u8", ";
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
			std::cout << vec->at(i) << u8", ";
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
			std::cout << (vec->at(i) ? u8"wahr" : u8"falsch") << u8", ";
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
			std::cout << vec->at(i) << u8", ";
		}
		std::cout << vec->at(vec->size() - 1) << u8"]";
		break;
	}
	case ValueType::StringArr:
	{
		std::cout << u8"[";
		std::vector<std::string>*& vec = val.StringArr();
		for (size_t i = 0; i < vec->size() - 1; i++)
		{
			std::cout << vec->at(i) << u8", ";
		}
		std::cout << vec->at(vec->size() - 1) << u8"]";
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