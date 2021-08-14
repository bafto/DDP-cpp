#pragma once

#include "Chunk.h"
#include <unordered_map>
#include <array>

class runtime_error : public std::exception
{
public:
	runtime_error(std::string msg) : std::exception(msg.c_str()) {};
};

class Function
{
public:
	Function(); //most often default constructed, but this is still provided

private:
	friend class VirtualMachine;

	//run the functions byte-code and return it's result. If the return type is ValueType::None the return Value should be discarded
	Value run(std::unordered_map<std::string, Value>* globals, std::unordered_map<std::string, Function>* functions); //only the VirtualMachine should call this function
	Value runNative(std::unordered_map<std::string, Value>* globals,
		std::unordered_map<std::string, Function>* functions,
		std::vector<Value> args);

	//these functions are only needed during runtime
	void push(Value value); //push a value onto the stack
	Value pop(); //pop a Value of the stack
	Value peek(int distance); //peek <distance> into the stack

	uint8_t readByte(); //read the next byte in chunk.bytes
	uint16_t readShort(); //read the next 2 bytes in chunk.bytes as short
	Value readConstant(); //read the next byte in chunk.bytes and lookup the constant it indicates

	template<class stream>
	void printValue(Value& val, stream& ostr)
	{
		switch (val.Type())
		{
		case ValueType::Int: ostr << val.Int(); break;
		case ValueType::Double: ostr << val.Double(); break;
		case ValueType::Bool: ostr << (val.Bool() ? u8"wahr" : u8"falsch"); break;
		case ValueType::Char: ostr << val.Char(); break;
		case ValueType::String: ostr << *val.String(); break;
		case ValueType::IntArr:
		{
			ostr << u8"[";
			std::vector<int>*& vec = val.IntArr();
			for (size_t i = 0; i < vec->size() - 1; i++)
			{
				ostr << vec->at(i) << u8"; ";
			}
			ostr << vec->at(vec->size() - 1) << u8"]";
			break;
		}
		case ValueType::DoubleArr:
		{
			ostr << u8"[";
			std::vector<double>*& vec = val.DoubleArr();
			for (size_t i = 0; i < vec->size() - 1; i++)
			{
				ostr << vec->at(i) << u8"; ";
			}
			ostr << vec->at(vec->size() - 1) << u8"]";
			break;
		}
		case ValueType::BoolArr:
		{
			ostr << u8"[";
			std::vector<bool>*& vec = val.BoolArr();
			for (size_t i = 0; i < vec->size() - 1; i++)
			{
				ostr << (vec->at(i) ? u8"wahr" : u8"falsch") << u8"; ";
			}
			ostr << (vec->at(vec->size() - 1) ? u8"wahr" : u8"falsch") << u8"]";
			break;
		}
		case ValueType::CharArr:
		{
			ostr << u8"[";
			std::vector<char>*& vec = val.CharArr();
			for (size_t i = 0; i < vec->size() - 1; i++)
			{
				ostr << vec->at(i) << u8"; ";
			}
			ostr << vec->at(vec->size() - 1) << u8"]";
			break;
		}
		case ValueType::StringArr:
		{
			ostr << u8"[\"";
			std::vector<std::string>*& vec = val.StringArr();
			for (size_t i = 0; i < vec->size() - 1; i++)
			{
				ostr << vec->at(i) << u8"\"; \"";
			}
			ostr << vec->at(vec->size() - 1) << u8"\"]";
			break;
		}
		default: ostr << "Invalid type!\n"; break;
		}
	}
	void addition(); //seperate function for  the OpCode::Add case in run

	template<typename T>
	void validateArray(std::vector<T> const* vec, int index)
	{
		if (index >= vec->size())
			throw runtime_error("Es wurde versucht auf ein Array Element auﬂerhalb der Reichweite zuzugreifen!");
	}
public: //Natives
	Value schreibeNative(std::vector<Value> args);
	Value schreibeZeileNative(std::vector<Value> args);
	Value leseNative(std::vector<Value> args);
	Value leseZeileNative(std::vector<Value> args);

	Value clockNative(std::vector<Value> args);

	Value zuZahlNative(std::vector<Value> args);
	Value zuKommazahlNative(std::vector<Value> args);
	Value zuBooleanNative(std::vector<Value> args);
	Value zuZeichenNative(std::vector<Value> args);
	Value zuZeichenketteNative(std::vector<Value> args);
public:
	std::vector<std::pair<std::string, ValueType>> args; //the types and count of the arguments the function takes (none for the main function)
	int argUnit;
	bool returned;
	Chunk chunk; //holds the byte code of the function
	ValueType returnType; //the return type of the function
	std::unordered_map<int, std::unordered_map<std::string, Value>> locals; //local variables of the function mapped to the numner their scope unit appeared at. At compile-time the values are empty.
public:
	using MemFuncPtr = Value(Function::*)(std::vector<Value>);
	MemFuncPtr native;
private:
	//Stuff needed during runtime, be carefull here, this should only be touched through it's getter functions
	static constexpr size_t StackMax = UINT8_MAX + 1; //the maximum count of the stack
	std::array<Value, StackMax> stack; //the value stack
	std::array<Value, StackMax>::iterator stackTop; //iterator to the current top of the stack

	std::vector<uint8_t>::iterator ip; //instruction pointer to the current byte in the chunk

	std::unordered_map<std::string, Value>* globals; //pointer to the global variables map
	std::unordered_map<std::string, Function>* functions; //pointer to the map of functions
};

