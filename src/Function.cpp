#include "Function.h"
#include <iostream>
#include <ctime>
#include <algorithm>
#include <sstream>

#pragma warning (disable : 4267)

Function::Function()
	:
	returnType(ValueType(Type::None)),
	args(),
	functions(nullptr),
	globals(nullptr),
	structs(nullptr),
	argUnit(0),
	returned(false),
	native(nullptr)
{}

Value Function::run(std::unordered_map<std::string, Value>* globals,
	std::unordered_map<std::string, Function>* functions,
	std::unordered_map<std::string, Value::Struct>* structs)
{
	using op = OpCode;

	this->globals = globals;
	this->functions = functions;
	this->structs = structs;

	stack.resize(StackMax);

	stackTop = stack.begin();
	ip = chunk.bytes.begin();

	bool forPrep = false;

	while (true)
	{
		switch ((OpCode)readByte())
		{
		case op::CONSTANT: push(readConstant()); break;
		case op::DEFINE_STRUCT:
		{
			std::string structType = *readConstant().String();
			int n = readConstant().Int();

			for (int i = 0; i < n; i++)
			{
				Value field = pop();
				std::string fieldName = *pop().String();
				(*structs)[structType].fields[fieldName] = field;
			}

			break;
		}
		case op::STRUCT:
		{
			std::string structType = *readConstant().String();
			int n = readConstant().Int();

			Value::Struct s = structs->at(structType);

			for (int i = 0; i < n; i++)
			{
				Value field = pop();
				std::string fieldName = *pop().String();
				s.fields[fieldName] = std::move(field);
			}
			push(s);
			break;
		}
		case op::ARRAY:
		{
			int size = readConstant().Int();
			Type type = (Type)readByte();
			switch (type)
			{
			case Type::IntArr:
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
			case Type::DoubleArr:
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
			case Type::BoolArr:
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
			case Type::CharArr:
			{
				std::vector<short> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(pop().Char());
				}
				std::reverse(vec.begin(), vec.end());
				push(Value(std::move(vec)));
				break;
			}
			case Type::StringArr:
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
			case Type::StructArr:
			{
				std::vector<Value::Struct> vec;
				vec.reserve(size);
				for (int i = 0; i < size; i++)
				{
					vec.push_back(*pop().VStruct());
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
			switch (val.type())
			{
			case Type::Int: push(-val.Int()); break;
			case Type::Double: push(-val.Double()); break;
			}
			break;
		}
		case op::ADD: addition(); break;
		case op::MULTIPLY:
		{
			Value b = pop();
			Value a = pop();
			switch (a.type())
			{
			case Type::Int:
				switch (b.type())
				{
				case Type::Int: push(Value(a.Int() * b.Int())); break;
				case Type::Double: push(Value((double)(a.Int() * b.Double()))); break;
				}
				break;
			case Type::Double:
				switch (b.type())
				{
				case Type::Int: push(Value((double)(a.Double() * b.Int()))); break;
				case Type::Double: push(Value(a.Double() * b.Double())); break;
				}
				break;
			}
			break;
		}
		case op::DIVIDE:
		{
			Value b = pop();
			Value a = pop();
			switch (a.type())
			{
			case Type::Int:
				switch (b.type())
				{
				case Type::Int: push(Value(a.Int() / b.Int())); break;
				case Type::Double: push(Value((double)(a.Int() / b.Double()))); break;
				}
				break;
			case Type::Double:
				switch (b.type())
				{
				case Type::Int: push(Value((double)(a.Double() / b.Int()))); break;
				case Type::Double: push(Value(a.Double() / b.Double())); break;
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
			switch (a.type())
			{
			case Type::Int:
				switch (b.type())
				{
				case Type::Int: push(Value(a.Int() - b.Int())); break;
				case Type::Double: push(Value((double)(a.Int() - b.Double()))); break;
				}
				break;
			case Type::Double:
				switch (b.type())
				{
				case Type::Int: push(Value((double)(a.Double() - b.Int()))); break;
				case Type::Double: push(Value(a.Double() - b.Double())); break;
				}
				break;
			}
			break;
		}
		case op::EXPONENT:
		{
			Value b = pop();
			Value a = pop();
			switch (a.type())
			{
			case Type::Int:
				switch (b.type())
				{
				case Type::Int: push(Value((int)pow(a.Int(), b.Int()))); break;
				case Type::Double: push(Value((double)pow(a.Int(), b.Double()))); break;
				}
				break;
			case Type::Double:
				switch (b.type())
				{
				case Type::Int: push(Value((double)pow(a.Double(), b.Int()))); break;
				case Type::Double: push(Value(pow(a.Double(), b.Double()))); break;
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
			switch (val.type())
			{
			case Type::Int: push(Value((double)log(val.Int()))); break;
			case Type::Double: push(Value(log(val.Double()))); break;
			}
			break;
		}
		case op::BETRAG:
		{
			Value val = pop();
			switch (val.type())
			{
			case Type::Int: push(Value(abs(val.Int()))); break;
			case Type::Double: push(Value(abs(val.Double()))); break;
			}
			break;
		}
		case op::SIN: push(Value(std::sin(pop().Double()))); break;
		case op::COS: push(Value(std::cos(pop().Double()))); break;
		case op::TAN: push(Value(std::tan(pop().Double()))); break;
		case op::ASIN: push(Value(std::asin(pop().Double()))); break;
		case op::ACOS: push(Value(std::acos(pop().Double()))); break;
		case op::ATAN: push(Value(std::atan(pop().Double()))); break;
		case op::SINH: push(Value(std::sinh(pop().Double()))); break;
		case op::COSH: push(Value(std::cosh(pop().Double()))); break;
		case op::TANH: push(Value(std::tanh(pop().Double()))); break;
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
			switch (a.type())
			{
			case Type::Int:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Int() == b.Int())); break;
				case Type::Double: push(Value((double)a.Int() == b.Double())); break;
				}
				break;
			}
			case Type::Double:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Double() == (double)b.Int())); break;
				case Type::Double: push(Value(a.Double() == b.Double())); break;
				}
				break;
			}
			case Type::Bool: push(Value(a.Bool() == b.Bool())); break;
			case Type::Char: push(Value(a.Char() == b.Char())); break;
			case Type::String: push(Value(*a.String() == *b.String())); break;
			}
			break;
		}
		case op::UNEQUAL:
		{
			Value b = pop();
			Value a = pop();
			switch (a.type())
			{
			case Type::Int:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Int() != b.Int())); break;
				case Type::Double: push(Value((double)a.Int() != b.Double())); break;
				}
				break;
			}
			case Type::Double:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Double() != (double)b.Int())); break;
				case Type::Double: push(Value(a.Double() != b.Double())); break;
				}
				break;
			}
			case Type::Bool: push(Value(a.Bool() != b.Bool())); break;
			case Type::Char: push(Value(a.Char() != b.Char())); break;
			case Type::String: push(Value(*a.String() != *b.String())); break;
			}
			break;
		}
		case op::GREATER:
		{
			Value b = pop();
			Value a = pop();
			switch (a.type())
			{
			case Type::Int:
			{
				switch (b.type())
				{
				case Type::Int:
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
				case Type::Double: push(Value((double)a.Int() > b.Double())); break;
				}
				break;
			}
			case Type::Double:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Double() > (double)b.Int())); break;
				case Type::Double: push(Value(a.Double() > b.Double())); break;
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
			switch (a.type())
			{
			case Type::Int:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Int() >= b.Int())); break;
				case Type::Double: push(Value((double)a.Int() >= b.Double())); break;
				}
				break;
			}
			case Type::Double:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Double() >= (double)b.Int())); break;
				case Type::Double: push(Value(a.Double() >= b.Double())); break;
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
			switch (a.type())
			{
			case Type::Int:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Int() < b.Int())); break;
				case Type::Double: push(Value((double)a.Int() < b.Double())); break;
				}
				break;
			}
			case Type::Double:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Double() < (double)b.Int())); break;
				case Type::Double: push(Value(a.Double() < b.Double())); break;
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
			switch (a.type())
			{
			case Type::Int:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Int() <= b.Int())); break;
				case Type::Double: push(Value((double)a.Int() <= b.Double())); break;
				}
				break;
			}
			case Type::Double:
			{
				switch (b.type())
				{
				case Type::Int: push(Value(a.Double() <= (double)b.Int())); break;
				case Type::Double: push(Value(a.Double() <= b.Double())); break;
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
			Type varType = globals->at(varName).type();
			if ((int)varType >= (int)Type::IntArr && (int)varType <= (int)Type::StructArr && val.type() == Type::Int)
			{
				if (val.Int() <= 0)
					throw runtime_error(u8"Ein Array muss mindestens 1 Element enthalten!");
				switch (varType)
				{
				case Type::IntArr: val = Value(std::vector<int>(val.Int(), 0)); break;
				case Type::DoubleArr: val = Value(std::vector<double>(val.Int(), 0.0)); break;
				case Type::BoolArr: val = Value(std::vector<bool>(val.Int(), false)); break;
				case Type::CharArr: val = Value(std::vector<short>(val.Int(), (short)0)); break;
				case Type::StringArr: val = Value(std::vector<std::string>(val.Int(), "")); break;
				case Type::StructArr:
				{
					std::string structIdentifier = *readConstant().String();
					val = Value(std::vector<Value::Struct>(val.Int(), structs->at(structIdentifier))); break;
				}
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
			Type varType = locals.at(unit).at(varName).type();
			if ((int)varType >= (int)Type::IntArr && (int)varType <= (int)Type::StringArr && val.type() == Type::Int)
			{
				switch (varType)
				{
				case Type::IntArr: val = Value(std::vector<int>(val.Int(), 0)); break;
				case Type::DoubleArr: val = Value(std::vector<double>(val.Int(), 0.0)); break;
				case Type::BoolArr: val = Value(std::vector<bool>(val.Int(), false)); break;
				case Type::CharArr: val = Value(std::vector<short>(val.Int(), (short)0)); break;
				case Type::StringArr: val = Value(std::vector<std::string>(val.Int(), "")); break;
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
		case op::GET_MEMBER_GLOBAL:
		{
			std::string varName = *readConstant().String();
			std::string memberName = *readConstant().String();
			uint8_t n = readByte();
			Value struc = globals->at(varName);
			for (int i = 0; i < n; i++)
			{
				struc = struc.VStruct()->fields.at(*readConstant().String());
			}
			push(struc.VStruct()->fields.at(memberName));
			break;
		}
		case op::GET_MEMBER_ARRAY_GLOBAL:
		{
			std::string varName = *readConstant().String();
			std::string memberName = *readConstant().String();
			int index = pop().Int();
			uint8_t n = readByte();
			Value struc = globals->at(varName).StructArr()->at(index);
			for (int i = 0; i < n; i++)
			{
				struc = struc.VStruct()->fields.at(*readConstant().String());
			}
			push(struc.VStruct()->fields.at(memberName));
			break;
		}
		case op::GET_LOCAL:
		{
			std::string varName = *readConstant().String();
			int unit = readConstant().Int();
			push(locals.at(unit).at(varName));
			break;
		}
		case op::GET_MEMBER_LOCAL:
		{
			std::string varName = *readConstant().String();
			int unit = readConstant().Int();
			std::string memberName = *readConstant().String();
			uint8_t n = readByte();
			Value struc = locals.at(unit).at(varName);
			for (int i = 0; i < n; i++)
			{
				struc = struc.VStruct()->fields.at(*readConstant().String());
			}
			push(struc.VStruct()->fields.at(memberName));
			break;
		}
		case op::GET_MEMBER_ARRAY_LOCAL:
		{
			std::string varName = *readConstant().String();
			int unit = readConstant().Int();
			std::string memberName = *readConstant().String();
			int index = pop().Int();
			uint8_t n = readByte();
			Value struc = locals.at(unit).at(varName).StructArr()->at(index);
			for (int i = 0; i < n; i++)
			{
				struc = struc.VStruct()->fields.at(*readConstant().String());
			}
			push(struc.VStruct()->fields.at(memberName));
			break;
		}
		case op::GET_ARRAY_ELEMENT:
		{
			std::string arrName = *readConstant().String();
			int index = pop().Int();
			switch (globals->at(arrName).type())
			{
			case Type::IntArr: validateArray(globals->at(arrName).IntArr(), index); push(Value(globals->at(arrName).IntArr()->at(index))); break;
			case Type::DoubleArr: validateArray(globals->at(arrName).DoubleArr(), index); push(Value(globals->at(arrName).DoubleArr()->at(index))); break;
			case Type::BoolArr: validateArray(globals->at(arrName).BoolArr(), index); push(Value(globals->at(arrName).BoolArr()->at(index))); break;
			case Type::CharArr: validateArray(globals->at(arrName).CharArr(), index); push(Value(globals->at(arrName).CharArr()->at(index))); break;
			case Type::StringArr: validateArray(globals->at(arrName).StringArr(), index); push(Value(globals->at(arrName).StringArr()->at(index))); break;
			default: throw runtime_error("Tried to index non-Array!");
			}
			break;
		}
		case op::GET_ARRAY_ELEMENT_LOCAL:
		{
			std::string arrName = *readConstant().String();
			int unit = readConstant().Int();
			int index = pop().Int();
			switch (locals.at(unit).at(arrName).type())
			{
			case Type::IntArr: validateArray(locals.at(unit).at(arrName).IntArr(), index); push(Value(locals.at(unit).at(arrName).IntArr()->at(index))); break;
			case Type::DoubleArr: validateArray(locals.at(unit).at(arrName).DoubleArr(), index); push(Value(locals.at(unit).at(arrName).DoubleArr()->at(index))); break;
			case Type::BoolArr: validateArray(locals.at(unit).at(arrName).BoolArr(), index); push(Value(locals.at(unit).at(arrName).BoolArr()->at(index))); break;
			case Type::CharArr: validateArray(locals.at(unit).at(arrName).CharArr(), index); push(Value(locals.at(unit).at(arrName).CharArr()->at(index))); break;
			case Type::StringArr: validateArray(locals.at(unit).at(arrName).StringArr(), index); push(Value(locals.at(unit).at(arrName).StringArr()->at(index))); break;
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
		case op::SET_MEMBER_GLOBAL:
		{
			std::string varName = *readConstant().String();
			std::string memberName = *readConstant().String();
			uint8_t n = readByte();
			Value struc = globals->at(varName);
			for (int i = 0; i < n; i++)
			{
				struc = struc.VStruct()->fields.at(*readConstant().String());
			}
			struc.VStruct()->fields[memberName] = peek(0);
			break;
		}
		case op::SET_LOCAL:
		{
			std::string varName = *readConstant().String();
			int unit = readConstant().Int();
			(locals[unit])[varName] = std::move(peek(0));
			break;
		}
		case op::SET_MEMBER_LOCAL:
		{
			std::string varName = *readConstant().String();
			int unit = readConstant().Int();
			std::string memberName = *readConstant().String();
			uint8_t n = readByte();
			Value struc = locals.at(unit).at(varName);
			for (int i = 0; i < n; i++)
			{
				struc = struc.VStruct()->fields.at(*readConstant().String());
			}
			struc.VStruct()->fields[memberName] = peek(0);
			break;
		}
		case op::SET_ARRAY_ELEMENT:
		{
			std::string arrName = *readConstant().String();
			Value val = std::move(pop());
			int index = peek(0).Int();
			switch (globals->at(arrName).type())
			{
			case Type::IntArr: validateArray(globals->at(arrName).IntArr(), index); (*(globals->at(arrName).IntArr()))[index] = val.Int(); break;
			case Type::DoubleArr: validateArray(globals->at(arrName).DoubleArr(), index); (*(globals->at(arrName).DoubleArr()))[index] = val.Double(); break;
			case Type::BoolArr: validateArray(globals->at(arrName).BoolArr(), index); (*(globals->at(arrName).BoolArr()))[index] = val.Bool(); break;
			case Type::CharArr: validateArray(globals->at(arrName).CharArr(), index); (*(globals->at(arrName).CharArr()))[index] = val.Char(); break;
			case Type::StringArr: validateArray(globals->at(arrName).StringArr(), index); (*(globals->at(arrName).StringArr()))[index] = *val.String(); break;
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
			switch (locals.at(unit).at(arrName).type())
			{
			case Type::IntArr: validateArray(locals.at(unit).at(arrName).IntArr(), index); (*(locals.at(unit).at(arrName).IntArr()))[index] = val.Int(); break;
			case Type::DoubleArr: validateArray(locals.at(unit).at(arrName).DoubleArr(), index); (*(locals.at(unit).at(arrName).DoubleArr()))[index] = val.Double(); break;
			case Type::BoolArr: validateArray(locals.at(unit).at(arrName).BoolArr(), index); (*(locals.at(unit).at(arrName).BoolArr()))[index] = val.Bool(); break;
			case Type::CharArr: validateArray(locals.at(unit).at(arrName).CharArr(), index); (*(locals.at(unit).at(arrName).CharArr()))[index] = val.Char(); break;
			case Type::StringArr: validateArray(locals.at(unit).at(arrName).StringArr(), index); (*(locals.at(unit).at(arrName).StringArr()))[index] = *val.String(); break;
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
			if (returnType.type != Type::None) return pop();
			return Value();
		}
		case op::CALL:
		{
			std::string funcName = *readConstant().String();
			Function func;
			try
			{
				func = functions->at(funcName);
			}
			catch (std::exception)
			{
				throw runtime_error("Die Funktion '" + funcName + "' ist nicht definiert!");
			}
			if (func.native != nullptr)
			{
				std::vector<Value> args;
				args.resize(func.args.size());
				for (int i = func.args.size() - 1; i >= 0; i--)
				{
					args[i] = std::move(pop());
				}
				try
				{
					push(func.runNative(globals, functions, structs, std::move(args)));
				}
				catch (runtime_error& e)
				{
					throw e;
				}
				catch (std::exception&)
				{
					throw runtime_error("Falsche Nutzung einer eingebauten Funktion!");
				}
				break;
			}

			for (int i = func.args.size() - 1; i >= 0; i--)
			{
				func.locals.at(func.argUnit)[func.args.at(i).first] = pop();
			}
			push(func.run(globals, functions, structs));
			break;
		}
		case op::POP: pop(); break;
		case op::FORPREP: forPrep = true; break;
		case op::FORDONE: forPrep = false; break;
#ifndef NDEBUG
		case op::PRINT:
		{
			pop().print(std::cout);
			break;
		}
#endif
		default: throw runtime_error(u8"Falsch generierter Byte-code!");
			break;
		}
	}

	if (returnType.type != Type::None) return pop();
	return Value();
}

Value Function::runNative(std::unordered_map<std::string, Value>* globals,
	std::unordered_map<std::string, Function>* functions,
	std::unordered_map<std::string, Value::Struct>* structs,
	std::vector<Value> args)
{
	this->globals = globals;
	this->functions = functions;
	this->structs = structs;

	return (*native)(std::move(args));
}

void Function::push(Value value)
{
	*stackTop = std::move(value);
	++stackTop;
	if (stackTop == stack.end())
		throw runtime_error("Stapel Überfluss!");
}

Value Function::pop()
{
	--stackTop;
	return *stackTop;
}

Value Function::peek(int distance)
{
	return stackTop[-1 - static_cast<ptrdiff_t>(distance)];
}

uint8_t Function::readByte()
{
	uint8_t byte = *ip;
	++ip;
	return byte;
}

uint16_t Function::readShort()
{
	return (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]));
}

Value Function::readConstant()
{
	return chunk.constants[readShort()];
}

void Function::addition()
{
	Value b = pop();
	Type bType = b.type();
	Value a = pop();
	Type aType = a.type();

	switch (aType)
	{
	case Type::Int:
		switch (bType)
		{
		case Type::Int:
			push(Value(a.Int() + b.Int()));
			return;
		case Type::Double:
			push(Value((double)(a.Int() + b.Double())));
			return;
		case Type::Char:
			push(Value((a.Int() + b.Char())));
			return;
		case Type::String:
			push(Value(std::string(std::to_string(a.Int()) + *b.String())));
			return;
		}
	case Type::Double:
		switch (bType)
		{
		case Type::Int:
			push(Value((double)(a.Double() + b.Int())));
			return;
		case Type::Double:
			push(Value(a.Double() + b.Double()));
			return;
		case Type::Char:
			push(Value(((int)a.Double() + (int)b.Char())));
			return;
		case Type::String:
			std::string astr(std::to_string(a.Double()));
			astr.replace(astr.begin(), astr.end(), '.', ',');
			push(Value(astr + *b.String()));
			return;
		}
	case Type::Char:
		switch (bType)
		{
		case Type::Int:
			push(Value((a.Char() + b.Int())));
			return;
		case Type::Double:
			push(Value((a.Char() + (int)b.Double())));
			return;
		case Type::Char:
			push(Value(Value::U8CharToString(a.Char()) + Value::U8CharToString(b.Char())));
			return;
		case Type::String:
			push(Value(Value::U8CharToString(a.Char()) + *b.String()));
			return;
		}
	case Type::String:
		switch (bType)
		{
		case Type::Int:
			push(Value(*a.String() + std::to_string(b.Int())));
			return;
		case Type::Double:
		{
			std::string str(std::to_string(b.Double()));
			std::replace(str.begin(), str.end(), '.', ',');
			push(Value(*a.String() + str));
			return;
		}
		case Type::Char:
			push(Value(*a.String() + Value::U8CharToString(b.Char())));
			return;
		case Type::String:
			push(Value(*a.String() + *b.String()));
			return;
		default:
			return;
		}
	case Type::IntArr:
	{
		std::vector<int> vec = *a.IntArr();
		switch (bType)
		{
		case Type::Int:
			vec.push_back(b.Int());
			push(Value(vec));
			return;
		case Type::IntArr:
		{
			std::vector<int> bvec = *b.IntArr();
			vec.insert(vec.end(), bvec.begin(), bvec.end());
			push(Value(vec));
			return;
		}
		default:
			return;
		}
	}
	case Type::DoubleArr:
	{
		std::vector<double> vec = *a.DoubleArr();
		switch (bType)
		{
		case Type::Double:
			vec.push_back(b.Int());
			push(Value(vec));
			return;
		case Type::DoubleArr:
		{
			std::vector<double> bvec = *b.DoubleArr();
			vec.insert(vec.end(), bvec.begin(), bvec.end());
			push(Value(vec));
			return;
		}
		default:
			return;
		}
	}
	case Type::BoolArr:
	{
		std::vector<bool> vec = *a.BoolArr();
		switch (bType)
		{
		case Type::Bool:
			vec.push_back(b.Bool());
			push(Value(vec));
			return;
		case Type::BoolArr:
		{
			std::vector<bool> bvec = *b.BoolArr();
			vec.insert(vec.end(), bvec.begin(), bvec.end());
			push(Value(vec));
			return;
		}
		default:
			return;
		}
	}
	case Type::CharArr:
	{
		std::vector<short> vec = *a.CharArr();
		switch (bType)
		{
		case Type::Char:
			vec.push_back(b.Char());
			push(Value(vec));
			return;
		case Type::CharArr:
		{
			std::vector<short> bvec = *b.CharArr();
			vec.insert(vec.end(), bvec.begin(), bvec.end());
			push(Value(vec));
			return;
		}
		default:
			return;
		}
	}
	case Type::StringArr:
	{
		std::vector<std::string> vec = *a.StringArr();
		switch (bType)
		{
		case Type::String:
			vec.push_back(*b.String());
			push(Value(vec));
			return;
		case Type::StringArr:
		{
			std::vector<std::string> bvec = *b.StringArr();
			vec.insert(vec.end(), bvec.begin(), bvec.end());
			push(Value(vec));
			return;
		}
		default:
			return;
		}
	}
	}
}