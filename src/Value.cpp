#include "Value.h"

Value::Value()
	:
	_val(std::monostate())
{}

Value::Value(const Value& other)
{
	switch (Type())
	{
	case ValueType::String: delete String(); break;
	case ValueType::IntArr: delete IntArr(); break;
	case ValueType::DoubleArr: delete DoubleArr(); break;
	case ValueType::BoolArr: delete BoolArr(); break;
	case ValueType::CharArr: delete CharArr(); break;
	case ValueType::StringArr: delete StringArr(); break;
	default: break;
	}

	_val = other._val;

	switch (Type())
	{
	case ValueType::String: _val = new std::string(*String()); break;
	case ValueType::IntArr: _val = new std::vector<int>(*IntArr()); break;
	case ValueType::DoubleArr: _val = new std::vector<double>(*DoubleArr()); break;
	case ValueType::BoolArr: _val = new std::vector<bool>(*BoolArr()); break;
	case ValueType::CharArr: _val = new std::vector<char>(*CharArr()); break;
	case ValueType::StringArr: _val = new std::vector<std::string>(*StringArr()); break;
	default: break;
	}
}

Value::Value(Value&& other) noexcept
{
	switch (Type())
	{
	case ValueType::String: delete String(); break;
	case ValueType::IntArr: delete IntArr(); break;
	case ValueType::DoubleArr: delete DoubleArr(); break;
	case ValueType::BoolArr: delete BoolArr(); break;
	case ValueType::CharArr: delete CharArr(); break;
	case ValueType::StringArr: delete StringArr(); break;
	default: break;
	}

	_val = other._val;

	switch (Type())
	{
	case ValueType::String: other.String() = nullptr; break;
	case ValueType::IntArr: other.IntArr() = nullptr; break;
	case ValueType::DoubleArr: other.DoubleArr() = nullptr; break;
	case ValueType::BoolArr: other.BoolArr() = nullptr; break;
	case ValueType::CharArr: other.CharArr() = nullptr; break;
	case ValueType::StringArr: other.StringArr() = nullptr; break;
	default: break;
	}
}

Value& Value::operator=(const Value& other)
{
	switch (Type())
	{
	case ValueType::String: delete String(); break;
	case ValueType::IntArr: delete IntArr(); break;
	case ValueType::DoubleArr: delete DoubleArr(); break;
	case ValueType::BoolArr: delete BoolArr(); break;
	case ValueType::CharArr: delete CharArr(); break;
	case ValueType::StringArr: delete StringArr(); break;
	default: break;
	}

	_val = other._val;

	switch (Type())
	{
	case ValueType::String: _val = new std::string(*String()); break;
	case ValueType::IntArr: _val = new std::vector<int>(*IntArr()); break;
	case ValueType::DoubleArr: _val = new std::vector<double>(*DoubleArr()); break;
	case ValueType::BoolArr: _val = new std::vector<bool>(*BoolArr()); break;
	case ValueType::CharArr: _val = new std::vector<char>(*CharArr()); break;
	case ValueType::StringArr: _val = new std::vector<std::string>(*StringArr()); break;
	default: break;
	}
	
	return *this;
}

Value& Value::operator=(Value&& other) noexcept
{
	switch (Type())
	{
	case ValueType::String: delete String(); break;
	case ValueType::IntArr: delete IntArr(); break;
	case ValueType::DoubleArr: delete DoubleArr(); break;
	case ValueType::BoolArr: delete BoolArr(); break;
	case ValueType::CharArr: delete CharArr(); break;
	case ValueType::StringArr: delete StringArr(); break;
	default: break;
	}

	_val = other._val;

	switch (Type())
	{
	case ValueType::String: other.String() = nullptr; break;
	case ValueType::IntArr: other.IntArr() = nullptr; break;
	case ValueType::DoubleArr: other.DoubleArr() = nullptr; break;
	case ValueType::BoolArr: other.BoolArr() = nullptr; break;
	case ValueType::CharArr: other.CharArr() = nullptr; break;
	case ValueType::StringArr: other.StringArr() = nullptr; break;
	default: break;
	}

	return *this;
}

Value::~Value()
{
	switch (Type())
	{
	case ValueType::String: delete String(); break;
	case ValueType::IntArr: delete IntArr(); break;
	case ValueType::DoubleArr: delete DoubleArr(); break;
	case ValueType::BoolArr: delete BoolArr(); break;
	case ValueType::CharArr: delete CharArr(); break;
	case ValueType::StringArr: delete StringArr(); break;
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

Value::Value(char v)
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

Value::Value(std::vector<char> v)
	:
	_val(new std::vector<char>(std::move(v)))
{}

Value::Value(std::vector<std::string> v)
	:
	_val(new std::vector<std::string>(std::move(v)))
{}

ValueType Value::Type() const
{
	return (ValueType)_val.index();
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

char& Value::Char()
{
	return std::get<char>(_val);
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

std::vector<char>*& Value::CharArr()
{
	return std::get<std::vector<char>*>(_val);
}

std::vector<std::string>*& Value::StringArr()
{
	return std::get<std::vector<std::string>*>(_val);
}
