#pragma once

#include <string>
#include <vector>
#include <variant>
#include <algorithm>
#include <unordered_map>

//#pragma warning (disable : 4244)

//aligns with _val's template parameters in Value
enum class Type
{
	None,
	Int,
	Double,
	Bool,
	Char,
	String,
	Struct,
	IntArr,
	DoubleArr,
	BoolArr,
	CharArr,
	StringArr,
	StructArr,
	Any, //not inside Value, only used for native functions that take any or multiple types as Arguments
	Function, //not inside Value, but used in the compiler to indicate wether a variable is a function
};

//encapsulates ValueType and the name of the struct if type is Struct or StructArr
struct ValueType
{
	ValueType(Type t)
		:
		structIdentifier(""),
		type(t)
	{}

	ValueType(std::string i, Type t)
		:
		structIdentifier(i),
		type(t)
	{}

	bool operator==(ValueType& rhs)
	{
		return type == rhs.type && structIdentifier == rhs.structIdentifier;
	}

	bool operator!=(ValueType& rhs)
	{
		return !(*this == rhs);
	}

	std::string structIdentifier;
	Type type;
};

bool isArr(Type t);
bool isArr(ValueType t);

class Value
{
public:
	struct Struct
	{
		std::unordered_map<std::string, Value> fields;
		std::string identifier;
	};
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
	Value(std::vector<double> v);
	Value(std::vector<bool> v);
	Value(std::vector<short> v);
	Value(std::vector<std::string> v);
	Value(Struct v);
	Value(std::vector<Struct> v);

	Type type() const; //return the current type of the variant

	static std::string U8CharToString(short ch);

	template<class stream>
	void print(stream& ostr)
	{
		switch (this->type())
		{
		case Type::Int: ostr << this->Int(); break;
		case Type::Double:
		{
			std::string str = std::to_string(this->Double());
			std::replace(str.begin(), str.end(), '.', ',');
			ostr << str;
			break;
		}
		case Type::Bool: ostr << (this->Bool() ? u8"wahr" : u8"falsch"); break;
		case Type::Char: ostr << U8CharToString(this->Char()); break;
		case Type::String: ostr << *this->String(); break;
		case Type::IntArr:
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
		case Type::DoubleArr:
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
		case Type::BoolArr:
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
		case Type::CharArr:
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
		case Type::StringArr:
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
		case Type::Struct:
		{
			Struct*& s = this->VStruct();
			if (s->fields.empty())
			{
				ostr << u8"{}";
				return;
			}
			ostr << u8"{";
			size_t i = s->fields.size();
			auto itt = s->fields.begin();
			for (auto it = s->fields.begin(); i > 1; it++, i--)
			{
				ostr << it->first << u8": ";
				it->second.print(ostr);
				ostr << u8"; ";
				if (i == 2)
				{
					itt = ++it;
				}
			}
			ostr << itt->first << u8": ";
			itt->second.print(ostr);
			ostr << u8"}";
			break;
		}
		case Type::StructArr:
		{
			std::vector<Struct>*& sarr = this->StructArr();
			if (sarr->empty())
			{
				ostr << u8"[]";
				return;
			}
			ostr << u8"[";
			for (int i = 0; i < (int)sarr->size() - 1; i++)
			{
				Value(sarr->at(i)).print(ostr);
				ostr << u8"; ";
			}
			Value(sarr->at(sarr->size() - 1)).print(ostr);
			ostr << u8"]";
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
	Struct*& VStruct();
	std::vector<int>*& IntArr();
	std::vector<double>*& DoubleArr();
	std::vector<bool>*& BoolArr();
	std::vector<short>*& CharArr();
	std::vector<std::string>*& StringArr();
	std::vector<Struct>*& StructArr();
private:
	std::variant<
		std::monostate,
		int,
		double,
		bool,
		short,
		std::string*,
		Struct*,
		std::vector<int>*,
		std::vector<double>*,
		std::vector<bool>*,
		std::vector<short>*,
		std::vector<std::string>*,
		std::vector<Struct>*
	> _val;
};

