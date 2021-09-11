#pragma once

#include <string>
#include <vector>
#include <variant>
#include <algorithm>

//#pragma warning (disable : 4244)

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
	StringArr,
	Any, //not inside Value, only used for native functions that take any or multiple types as Arguments
	Function, //not inside Value, but used in the compiler to indicate wether a variable is a function
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
	Value(short v);
	Value(std::string v);
	Value(const char* v);
	Value(std::vector<int> v);
	Value( std::vector<double> v);
	Value( std::vector<bool> v);
	Value( std::vector<short> v);
	Value( std::vector<std::string> v);

	ValueType Type() const; //return the current type of the variant

	static std::string U8CharToString(short ch);

	template<class stream>
	void print(stream& ostr)
	{
		switch (this->Type())
		{
		case ValueType::Int: ostr << this->Int(); break;
		case ValueType::Double:
		{
			std::string str = std::to_string(this->Double());
			std::replace(str.begin(), str.end(), '.', ',');
			ostr << str;
			break;
		}
		case ValueType::Bool: ostr << (this->Bool() ? u8"wahr" : u8"falsch"); break;
		case ValueType::Char: ostr << U8CharToString(this->Char()); break;
		case ValueType::String: ostr << *this->String(); break;
		case ValueType::IntArr:
		{
			std::vector<int>*& vec = this->IntArr();
			if (vec->empty())
			{
				ostr << u8"[]";
				return;
			}
			ostr << u8"[";
			for (int i = 0; i < (int)vec->size() - 1; i++)
			{
				ostr << vec->at(i) << u8"; ";
			}
			ostr << vec->at(vec->size() - 1) << u8"]";
			break;
		}
		case ValueType::DoubleArr:
		{
			std::vector<double>*& vec = this->DoubleArr();
			if (vec->empty())
			{
				ostr << u8"[]";
				return;
			}
			ostr << u8"[";
			for (int i = 0; i < (int)vec->size() - 1; i++)
			{
				ostr << vec->at(i) << u8"; ";
			}
			ostr << vec->at(vec->size() - 1) << u8"]";
			break;
		}
		case ValueType::BoolArr:
		{
			std::vector<bool>*& vec = this->BoolArr();
			if (vec->empty())
			{
				ostr << u8"[]";
				return;
			}
			ostr << u8"[";
			for (int i = 0; i < (int)vec->size() - 1; i++)
			{
				ostr << (vec->at(i) ? u8"wahr" : u8"falsch") << u8"; ";
			}
			ostr << (vec->at(vec->size() - 1) ? u8"wahr" : u8"falsch") << u8"]";
			break;
		}
		case ValueType::CharArr:
		{
			std::vector<short>*& vec = this->CharArr();
			if (vec->empty())
			{
				ostr << u8"[]";
				return;
			}
			ostr << u8"['";
			for (int i = 0; i < (int)vec->size() - 1; i++)
			{
				ostr << U8CharToString(vec->at(i)) << u8"'; '";
			}
			ostr << U8CharToString(vec->at(vec->size() - 1)) << u8"']";
			break;
		}
		case ValueType::StringArr:
		{
			std::vector<std::string>*& vec = this->StringArr();
			if (vec->empty())
			{
				ostr << u8"[]";
				return;
			}
			ostr << u8"[\"";
			for (int i = 0; i < (int)vec->size() - 1; i++)
			{
				ostr << vec->at(i) << u8"\"; \"";
			}
			ostr << vec->at(vec->size() - 1) << u8"\"]";
			break;
		}
		default: ostr << "Invalid type!\n"; break;
		}
	}

	//get a reference to the current value
	int& Int();
	double& Double();
	bool& Bool();
	short& Char();
	std::string*& String();
	std::vector<int>*& IntArr();
	std::vector<double>*& DoubleArr();
	std::vector<bool>*& BoolArr();
	std::vector<short>*& CharArr();
	std::vector<std::string>*& StringArr();
private:
	std::variant<
		std::monostate,
		int,
		double,
		bool,
		short,
		std::string*,
		std::vector<int>*,
		std::vector<double>*,
		std::vector<bool>*,
		std::vector<short>*,
		std::vector<std::string>*
	> _val;
};

