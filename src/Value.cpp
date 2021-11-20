#include "Value.h"
#include <sstream>

bool isArr(Type t)
{
	return t >= Type::IntArr && t <= Type::StructArr;
}

bool isArr(ValueType t)
{
	return isArr(t.type);
}

Value::Value()
	:
	_val(std::monostate())
{}

Value::Value(const Value& other)
{
	switch (Type())
	{
	case Type::String: delete String(); break;
	case Type::IntArr: delete IntArr(); break;
	case Type::DoubleArr: delete DoubleArr(); break;
	case Type::BoolArr: delete BoolArr(); break;
	case Type::CharArr: delete CharArr(); break;
	case Type::StringArr: delete StringArr(); break;
	case Type::Struct: delete VStruct(); break;
	case Type::StructArr: delete StructArr(); break;
	default: break;
	}

	_val = other._val;

	switch (Type())
	{
	case Type::String: _val = new std::string(*String()); break;
	case Type::IntArr: _val = new std::vector<int>(*IntArr()); break;
	case Type::DoubleArr: _val = new std::vector<double>(*DoubleArr()); break;
	case Type::BoolArr: _val = new std::vector<bool>(*BoolArr()); break;
	case Type::CharArr: _val = new std::vector<short>(*CharArr()); break;
	case Type::StringArr: _val = new std::vector<std::string>(*StringArr()); break;
	case Type::Struct: _val = new Struct(*VStruct()); break;
	case Type::StructArr: _val = new std::vector<Struct>(*StructArr()); break;
	default: break;
	}
}

Value::Value(Value&& other) noexcept
{
	switch (Type())
	{
	case Type::String: delete String(); break;
	case Type::IntArr: delete IntArr(); break;
	case Type::DoubleArr: delete DoubleArr(); break;
	case Type::BoolArr: delete BoolArr(); break;
	case Type::CharArr: delete CharArr(); break;
	case Type::StringArr: delete StringArr(); break;
	case Type::Struct: delete VStruct(); break;
	case Type::StructArr: delete StructArr(); break;
	default: break;
	}

	_val = other._val;

	switch (Type())
	{
	case Type::String: other.String() = nullptr; break;
	case Type::IntArr: other.IntArr() = nullptr; break;
	case Type::DoubleArr: other.DoubleArr() = nullptr; break;
	case Type::BoolArr: other.BoolArr() = nullptr; break;
	case Type::CharArr: other.CharArr() = nullptr; break;
	case Type::StringArr: other.StringArr() = nullptr; break;
	case Type::Struct: other.VStruct() = nullptr; break;
	case Type::StructArr: other.StructArr() = nullptr; break;
	default: break;
	}
}

Value& Value::operator=(const Value& other)
{
	switch (Type())
	{
	case Type::String: delete String(); break;
	case Type::IntArr: delete IntArr(); break;
	case Type::DoubleArr: delete DoubleArr(); break;
	case Type::BoolArr: delete BoolArr(); break;
	case Type::CharArr: delete CharArr(); break;
	case Type::StringArr: delete StringArr(); break;
	case Type::Struct: delete VStruct(); break;
	case Type::StructArr: delete StructArr(); break;
	default: break;
	}

	_val = other._val;

	switch (Type())
	{
	case Type::String: _val = new std::string(*String()); break;
	case Type::IntArr: _val = new std::vector<int>(*IntArr()); break;
	case Type::DoubleArr: _val = new std::vector<double>(*DoubleArr()); break;
	case Type::BoolArr: _val = new std::vector<bool>(*BoolArr()); break;
	case Type::CharArr: _val = new std::vector<short>(*CharArr()); break;
	case Type::StringArr: _val = new std::vector<std::string>(*StringArr()); break;
	case Type::Struct: _val = new Struct(*VStruct()); break;
	case Type::StructArr: _val = new std::vector<Struct>(*StructArr()); break;
	default: break;
	}
	
	return *this;
}

Value& Value::operator=(Value&& other) noexcept
{
	switch (Type())
	{
	case Type::String: delete String(); break;
	case Type::IntArr: delete IntArr(); break;
	case Type::DoubleArr: delete DoubleArr(); break;
	case Type::BoolArr: delete BoolArr(); break;
	case Type::CharArr: delete CharArr(); break;
	case Type::StringArr: delete StringArr(); break;
	case Type::Struct: delete VStruct(); break;
	case Type::StructArr: delete StructArr(); break;
	default: break;
	}

	_val = other._val;

	switch (Type())
	{
	case Type::String: other.String() = nullptr; break;
	case Type::IntArr: other.IntArr() = nullptr; break;
	case Type::DoubleArr: other.DoubleArr() = nullptr; break;
	case Type::BoolArr: other.BoolArr() = nullptr; break;
	case Type::CharArr: other.CharArr() = nullptr; break;
	case Type::StringArr: other.StringArr() = nullptr; break;
	case Type::Struct: other.VStruct() = nullptr; break;
	case Type::StructArr: other.StructArr() = nullptr; break;
	default: break;
	}

	return *this;
}

Value::~Value()
{
	switch (Type())
	{
	case Type::String: delete String(); break;
	case Type::IntArr: delete IntArr(); break;
	case Type::DoubleArr: delete DoubleArr(); break;
	case Type::BoolArr: delete BoolArr(); break;
	case Type::CharArr: delete CharArr(); break;
	case Type::StringArr: delete StringArr(); break;
	case Type::Struct: delete VStruct(); break;
	case Type::StructArr: delete StructArr(); break;
	default: break;
	}
}

Value::Value(int v)
	:
	_val(v)
{}

Value::Value(double v)
	:
	_val(v)
{}

Value::Value(bool v)
	:
	_val(v)
{}

Value::Value(short v)
	:
	_val(v)
{}

Value::Value(std::string v)
	:
	_val(new std::string(std::move(v)))
{}

Value::Value(const char* v)
	:
	_val(new std::string(v))
{
}

Value::Value(std::vector<int> v)
	:
	_val(new std::vector<int>(std::move(v)))
{}

Value::Value(std::vector<double> v)
	:
	_val(new std::vector<double>(std::move(v)))
{}

Value::Value(std::vector<bool> v)
	:
	_val(new std::vector<bool>(std::move(v)))
{}

Value::Value(std::vector<short> v)
	:
	_val(new std::vector<short>(std::move(v)))
{}

Value::Value(std::vector<std::string> v)
	:
	_val(new std::vector<std::string>(std::move(v)))
{}

Value::Value(Struct v)
	:
	_val(new Struct(std::move(v)))
{}

Value::Value(std::vector<Struct> v)
	:
	_val(new std::vector<Struct>(std::move(v)))
{}

Type Value::type() const
{
	return (Type)_val.index();
}

std::string Value::U8CharToString(short ch)
{
	char a = ch;
	char b = (ch >> 8);
	if (a >= 32 && a <= 126 || a == '\n' || a == '\t' || a == '\r') {
		return std::string(1, a);
	}
	std::string s = "  ";
	s[0] = b;
	s[1] = a;
	return s;
}

int& Value::Int()
{
	return std::get<int>(_val);
}

double& Value::Double()
{
	return std::get<double>(_val);
}

bool& Value::Bool()
{
	return std::get<bool>(_val);
}

short& Value::Char()
{
	return std::get<short>(_val);
}

std::string*& Value::String()
{
	return std::get<std::string*>(_val);
}

std::vector<int>*& Value::IntArr()
{
	return std::get<std::vector<int>*>(_val);
}

std::vector<double>*& Value::DoubleArr()
{
	return std::get<std::vector<double>*>(_val);
}

std::vector<bool>*& Value::BoolArr()
{
	return std::get<std::vector<bool>*>(_val);
}

std::vector<short>*& Value::CharArr()
{
	return std::get<std::vector<short>*>(_val);
}

std::vector<std::string>*& Value::StringArr()
{
	return std::get<std::vector<std::string>*>(_val);
}

Value::Struct*& Value::VStruct()
{
	return std::get<Struct*>(_val);
}

std::vector<Value::Struct>*& Value::StructArr()
{
	return std::get<std::vector<Struct>*>(_val);
}

std::string Value::to_string_precision(const double& d, const size_t& precision)
{
	std::ostringstream out;
	out.precision(precision);
	out << std::fixed << d;
	return out.str();
}
