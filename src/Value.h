#pragma once

#include <string>
#include <variant>
#include <iostream>
#include "common.h"

//enum class for all Types a Value can have
//members must me aligned correclty to the _val variant of Value!!!
enum class ValueType
{
	NONE, //aligns with std::monostate (empty variant)
	INT, //aligns with int
	DOUBLE, //aligns with double
	BOOL, //aligns with bool
	CHAR, //aligns with char
	STRING //aligns with string
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
	
	Value(const Value& rhs)
		:
		_val(rhs._val)
	{
		if (rhs.getType() == ValueType::STRING) _val = new std::string(*asString());
	}

	Value& operator=(const Value& rhs)
	{
		_val = rhs._val;
		if (getType() == ValueType::STRING) _val = new std::string(*asString());
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

	~Value() //destructor
	{
		if (getType() == ValueType::STRING) delete asString();
	}

	ValueType getType() const { return (ValueType)_val.index(); }

	int& asInt() { return std::get<int>(_val); }
	double& asDouble() { return std::get<double>(_val); }
	bool& asBool() { return std::get<bool>(_val); }
	char& asChar() { return std::get<char>(_val); }
	std::string* asString() { return std::get<std::string*>(_val); }

#ifdef _MDEBUG_
	void printValue()
	{
		std::cout << "[Value] ";
		switch (getType())
		{
		case ValueType::INT: std::cout << asInt(); break;
		case ValueType::DOUBLE: std::cout << asDouble(); break;
		case ValueType::BOOL: std::cout << asBool(); break;
		case ValueType::CHAR: std::cout << asChar(); break;
		case ValueType::STRING: std::cout << *asString(); break;
		default: std::cout << "Invalid Type";
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
		std::string*
	> _val; //holds the actual value
};