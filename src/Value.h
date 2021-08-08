#pragma once

#include <string>
#include <vector>
#include <variant>

//aligns with _val's template parameters in Value
enum class ValueType
{
	None,
	Int,
	Double,
	Bool,
	Char,
	String,
	IntArr,
	DoubleArr,
	BoolArr,
	CharArr,
	StringArr
};

class Value
{
public:
	Value(); //constructed with std::monostate
	Value(const Value& other); //copy the resources and allocate a new pointer if necessary
	Value(Value&& other) noexcept; //take the resources and take ownership of the pointer if necessary

	Value& operator=(const Value& other);
	Value& operator=(Value&& other) noexcept;

	~Value(); //delete the pointer if necessary

	//constructors for the various types
	Value(int v);
	Value(double v);
	Value(bool v);
	Value(char v);
	Value(std::string v);
	Value(const std::vector<int>& v);
	Value(const std::vector<double>& v);
	Value(const std::vector<bool>& v);
	Value(const std::vector<char>& v);
	Value(const std::vector<std::string>& v);

	ValueType Type(); //return the current type of the variant

	//get a reference to the current value
	int& Int();
	double& Double();
	bool& Bool();
	char& Char();
	std::string*& String();
	std::vector<int>*& IntArr();
	std::vector<double>*& DoubleArr();
	std::vector<bool>*& BoolArr();
	std::vector<char>*& CharArr();
	std::vector<std::string>*& StringArr();
private:
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
	> _val;
};

