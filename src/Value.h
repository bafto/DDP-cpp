#pragma once

#include <string>
#include <variant>
#include <vector>
#include <iostream>
#include "common.h"

#pragma warning (disable : 26439)

//enum class for all Types a Value can have
//members must me aligned correclty to the _val variant of Value!!!
enum class ValueType
{
	NONE, //aligns with std::monostate (empty variant)
	INT, //aligns with int
	DOUBLE, //aligns with double
	BOOL, //aligns with bool
	CHAR, //aligns with char
	STRING, //aligns with string
	INTARR,
	DOUBLEARR,
	BOOLARR,
	CHARARR,
	STRINGARR
};

//thrown if Value is assigned to another type than it's own
class bad_assignement_exception : public base_exception
{
public:
	bad_assignement_exception(const std::string& msg) : base_exception(msg) {}
};

class Value
{
public:
	Value() : _val(std::monostate()) {} //default constructor
	Value(const int& i) : _val(i) {} //constructor for an int Value
	Value(const double& d) : _val(d) {} //constructor for a double Value
	Value(const bool& b) : _val(b) {} //constructor for a bool Value
	Value(const char& c) : _val(c) {} //constructor for a char Value
	Value(const std::string& s) : _val(new std::string(s)) {} //constructor for a string Value
	Value(const char* s) : _val(new std::string(s)) {} //constructor for a string Value from a const char*
	Value(const std::vector<int>& ints) : _val(new std::vector<int>(ints)) {}
	Value(const std::vector<double>& doubles) : _val(new std::vector<double>(doubles)) {}
	Value(const std::vector<bool>& bools) : _val(new std::vector<bool>(bools)) {}
	Value(const std::vector<char>& chars) : _val(new std::vector<char>(chars)) {}
	Value(const std::vector<std::string>& strs) : _val(new std::vector<std::string>(strs)) {}
	Value(std::vector<int>&& ints) : _val(new std::vector<int>(std::move(ints))) {}
	Value(std::vector<double>&& doubles) : _val(new std::vector<double>(std::move(doubles))) {}
	Value(std::vector<bool>&& bools) : _val(new std::vector<bool>(std::move(bools))) {}
	Value(std::vector<char>&& chars) : _val(new std::vector<char>(std::move(chars))) {}
	Value(std::vector<std::string>&& strs) : _val(new std::vector<std::string>(std::move(strs))) {}
	
	//copy constructor, allocates new memory if it's a string, array or function
	Value(const Value& rhs)
		:
		_val(rhs._val)
	{
		if (rhs.getType() >= ValueType::STRING)
		{
			switch (rhs.getType())
			{
			case ValueType::STRING: _val = new std::string(*asString()); break;
			case ValueType::INTARR: _val = new std::vector<int>(*asIntArr()); break;
			case ValueType::DOUBLEARR: _val = new std::vector<double>(*asDoubleArr()); break;
			case ValueType::BOOLARR: _val = new std::vector<bool>(*asBoolArr()); break;
			case ValueType::CHARARR: _val = new std::vector<char>(*asCharArr()); break;
			case ValueType::STRINGARR: _val = new std::vector<std::string>(*asStringArr()); break;
			}
		}
	}
	//move constructor, takes ownership of the memory if it's a string, array or function
	Value(Value&& rhs) noexcept
		:
		_val(rhs._val)
	{
		if (rhs.getType() >= ValueType::STRING)
		{
			switch (rhs.getType())
			{
			case ValueType::STRING: rhs.asString() = nullptr; break;
			case ValueType::INTARR: rhs.asIntArr() = nullptr; break;
			case ValueType::DOUBLEARR: rhs.asDoubleArr() = nullptr; break;
			case ValueType::BOOLARR: rhs.asBoolArr() = nullptr; break;
			case ValueType::CHARARR: rhs.asCharArr() = nullptr; break;
			case ValueType::STRINGARR: rhs.asStringArr() = nullptr; break;
			}
		}
	}
	//copy-assignement operator, allocates new memory if it's a string, array or function
	Value& operator=(const Value& rhs)
	{
		if (getType() != rhs.getType())
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to Value of other type"));

		_val = rhs._val;

		if (rhs.getType() >= ValueType::STRING)
		{
			switch (rhs.getType())
			{
			case ValueType::STRING: _val = new std::string(*asString()); break;
			case ValueType::INTARR: _val = new std::vector<int>(*asIntArr()); break;
			case ValueType::DOUBLEARR: _val = new std::vector<double>(*asDoubleArr()); break;
			case ValueType::BOOLARR: _val = new std::vector<bool>(*asBoolArr()); break;
			case ValueType::CHARARR: _val = new std::vector<char>(*asCharArr()); break;
			case ValueType::STRINGARR: _val = new std::vector<std::string>(*asStringArr()); break;
			}
		}

		return *this;
	}
	//move-assignement operator, takes ownership of the memory if it's a string, array or function
	Value& operator=(Value&& rhs) noexcept
	{
		_val = rhs._val;

		if (rhs.getType() >= ValueType::STRING)
		{
			switch (rhs.getType())
			{
			case ValueType::STRING: rhs.asString() = nullptr; break;
			case ValueType::INTARR: rhs.asIntArr() = nullptr; break;
			case ValueType::DOUBLEARR: rhs.asDoubleArr() = nullptr; break;
			case ValueType::BOOLARR: rhs.asBoolArr() = nullptr; break;
			case ValueType::CHARARR: rhs.asCharArr() = nullptr; break;
			case ValueType::STRINGARR: rhs.asStringArr() = nullptr; break;
			}
		}

		return *this;
	}


	Value& operator=(const int& rhs)
	{ 
		if (getType() != ValueType::INT)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to int"));
		_val = rhs;
		return *this;
	}
	Value& operator=(const double& rhs)
	{
		if (getType() != ValueType::DOUBLE)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to double"));
		_val = rhs;
		return *this;
	}
	Value& operator=(const bool& rhs)
	{
		if (getType() != ValueType::BOOL)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to bool"));
		_val = rhs;
		return *this;
	}
	Value& operator=(const char& rhs)
	{
		if (getType() != ValueType::CHAR)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to char"));
		_val = rhs;
		return *this;
	}
	Value& operator=(const std::string& rhs)
	{
		if (getType() != ValueType::STRING)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to std::string"));
		*asString() = rhs;
		return *this;
	}
	Value& operator=(const char* rhs)
	{
		if (getType() != ValueType::STRING)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to const char*"));
		*asString() = rhs;
		return *this;
	}
	Value& operator=(const std::vector<int>& rhs)
	{
		if (getType() != ValueType::INTARR)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to std::vector<int>"));
		*asIntArr() = rhs;
		return *this;
	}
	Value& operator=(const std::vector<double>& rhs)
	{
		if (getType() != ValueType::DOUBLEARR)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to std::vector<double>"));
		*asDoubleArr() = rhs;
		return *this;
	}
	Value& operator=(const std::vector<bool>& rhs)
	{
		if (getType() != ValueType::BOOLARR)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to std::vector<bool>"));
		*asBoolArr() = rhs;
		return *this;
	}
	Value& operator=(const std::vector<char>& rhs)
	{
		if (getType() != ValueType::CHARARR)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to std::vector<char>"));
		*asCharArr() = rhs;
		return *this;
	}
	Value& operator=(const std::vector<std::string>& rhs)
	{
		if (getType() != ValueType::STRINGARR)
			throw bad_assignement_exception(GENERATE_EXCEPTION(bad_assignement_exception, "cannot assign to std::vector<std::string>"));
		*asStringArr() = rhs;
		return *this;
	}

	//destructor, frees memory if necessery
	~Value() 
	{
		if (getType() >= ValueType::STRING)
		{
			switch (getType())
			{
			case ValueType::STRING: delete asString(); break;
			case ValueType::INTARR: delete asIntArr(); break;
			case ValueType::DOUBLEARR: delete asDoubleArr(); break;
			case ValueType::BOOLARR: delete asBoolArr(); break;
			case ValueType::CHARARR: delete asCharArr(); break;
			case ValueType::STRINGARR: delete asStringArr(); break;
			}
		}
	}

	//return the type of the Value
	ValueType getType() const { return (ValueType)_val.index(); }

	/*Returns the value, wraper for std::get*/

	int& asInt() { return std::get<int>(_val); }
	double& asDouble() { return std::get<double>(_val); }
	bool& asBool() { return std::get<bool>(_val); }
	char& asChar() { return std::get<char>(_val); }
	std::string*& asString() { return std::get<std::string*>(_val); }
	std::vector<int>*& asIntArr() { return std::get<std::vector<int>*>(_val); }
	std::vector<double>*& asDoubleArr() { return std::get<std::vector<double>*>(_val); }
	std::vector<bool>*& asBoolArr() { return std::get<std::vector<bool>*>(_val); }
	std::vector<char>*& asCharArr() { return std::get<std::vector<char>*>(_val); }
	std::vector<std::string>*& asStringArr() { return std::get<std::vector<std::string>*>(_val); }

#ifdef _MDEBUG_
	//print the value, for debug mode
	void printValue()
	{
		std::cout << u8"[Value] ";
		switch (getType())
		{
		case ValueType::NONE: std::cout << "None"; break;
		case ValueType::INT: std::cout << asInt(); break;
		case ValueType::DOUBLE: std::cout << asDouble(); break;
		case ValueType::BOOL: std::cout << asBool(); break;
		case ValueType::CHAR: std::cout << asChar(); break;
		case ValueType::STRING: std::cout << *asString(); break;
		case ValueType::INTARR:
		{
			std::cout << u8"[";
			std::vector<int>*& vec = asIntArr();
			for (unsigned int i = 0; i < vec->size() - 1; i++)
			{
				std::cout << (*vec)[i] << u8", ";
			}
			std::cout << (*vec)[vec->size() - 1] << u8"]";
			break;
		}
		case ValueType::DOUBLEARR:
		{
			std::cout << u8"[";
			std::vector<double>*& vec = asDoubleArr();
			for (unsigned int i = 0; i < vec->size() - 1; i++)
			{
				std::cout << (*vec)[i] << u8", ";
			}
			std::cout << (*vec)[vec->size() - 1] << u8"]";
			break;
		}
		case ValueType::BOOLARR:
		{
			std::cout << u8"[";
			std::vector<bool>*& vec = asBoolArr();
			for (unsigned int i = 0; i < vec->size() - 1; i++)
			{
				std::cout << ((*vec)[i] ? u8"wahr" : u8"falsch") << u8", ";
			}
			std::cout << ((*vec)[vec->size() - 1] ? u8"wahr" : u8"falsch") << u8"]";
			break;
		}
		case ValueType::CHARARR:
		{
			std::cout << u8"[";
			std::vector<char>*& vec = asCharArr();
			for (unsigned int i = 0; i < vec->size() - 1; i++)
			{
				std::cout << (*vec)[i] << u8", ";
			}
			std::cout << (*vec)[vec->size() - 1] << u8"]";
			break;
		}
		case ValueType::STRINGARR:
		{
			std::cout << u8"[";
			std::vector<std::string>*& vec = asStringArr();
			for (unsigned int i = 0; i < vec->size() - 1; i++)
			{
				std::cout << (*vec)[i] << u8", ";
			}
			std::cout << (*vec)[vec->size() - 1] << u8"]";
			break;
		}
		default: std::cout << u8"Invalid Type"; break;
		}
	}
#endif
public:
	std::variant<
		std::monostate,
		int,
		double,
		bool,
		char,
		std::string*,
		std::vector<int>*,
		std::vector<double>*,
		std::vector<bool>*,
		std::vector<char>*,
		std::vector<std::string>*
	> _val; //holds the actual value
};