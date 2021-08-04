#pragma once

#include <string>
#include <variant>
#include <vector>
#include <iostream>
#include "common.h"
#include "Function.h"

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
	STRINGARR,
	FUNCTION,
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
	Value(); //default constructor
	Value(const int& i); //constructor for an int Value
	Value(const double& d); //constructor for a double Value
	Value(const bool& b); //constructor for a bool Value
	Value(const char& c); //constructor for a char Value
	Value(const std::string& s); //constructor for a string Value
	Value(const char* s); //constructor for a string Value from a const char*
	Value(const std::vector<int>& ints);
	Value(const std::vector<double>& doubles);
	Value(const std::vector<bool>& bools);
	Value(const std::vector<char>& chars);
	Value(const std::vector<std::string>& strs);
	Value(std::vector<int>&& ints);
	Value(std::vector<double>&& doubles);
	Value(std::vector<bool>&& bools);
	Value(std::vector<char>&& chars);
	Value(std::vector<std::string>&& strs);
	Value(const Function& func);
	
	//copy constructor, allocates new memory if it's a string, array or function
	Value(const Value& rhs);
	//move constructor, takes ownership of the memory if it's a string, array or function
	Value(Value&& rhs) noexcept;
	//copy-assignement operator, allocates new memory if it's a string, array or function
	Value& operator=(const Value& rhs);
	//move-assignement operator, takes ownership of the memory if it's a string, array or function
	Value& operator=(Value&& rhs) noexcept;


	Value& operator=(const int& rhs);
	Value& operator=(const double& rhs);
	Value& operator=(const bool& rhs);
	Value& operator=(const char& rhs);
	Value& operator=(const std::string& rhs);
	Value& operator=(const char* rhs);
	Value& operator=(const std::vector<int>& rhs);
	Value& operator=(const std::vector<double>& rhs);
	Value& operator=(const std::vector<bool>& rhs);
	Value& operator=(const std::vector<char>& rhs);
	Value& operator=(const std::vector<std::string>& rhs);
	Value& operator=(const Function& rhs);

	//destructor, frees memory if necessery
	~Value();

	//return the type of the Value
	ValueType getType() const;

	/*Returns the value, wraper for std::get*/

	int& asInt();
	double& asDouble();
	bool& asBool();
	char& asChar();
	std::string*& asString();
	std::vector<int>*& asIntArr();
	std::vector<double>*& asDoubleArr();
	std::vector<bool>*& asBoolArr();
	std::vector<char>*& asCharArr();
	std::vector<std::string>*& asStringArr();
	Function*& asFunction();

#ifdef _MDEBUG_
	//print the value, for debug mode
	void printValue();
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
		std::vector<std::string>*,
		Function*
	> _val; //holds the actual value
};