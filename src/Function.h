#pragma once

#include "Chunk.h"
#include "Natives.h"
#include <unordered_map>
#include <array>

class Function
{
public:
	Function(); //most often default constructed, but this is still provided

private:
	friend class VirtualMachine;

	//run the functions byte-code and return it's result. If the return type is ValueType::None the return Value should be discarded
	Value run(std::unordered_map<std::string, Value>* globals,
		std::unordered_map<std::string, Function>* functions,
		std::unordered_map<std::string, Value::Struct>* structs); //only the VirtualMachine should call this function
	Value runNative(std::unordered_map<std::string, Value>* globals,
		std::unordered_map<std::string, Function>* functions,
		std::unordered_map<std::string, Value::Struct>* structs,
		std::vector<Value> args);

	//these functions are only needed during runtime
	void push(Value value); //push a value onto the stack
	Value pop(); //pop a Value of the stack
	Value peek(int distance); //peek <distance> into the stack

	uint8_t readByte(); //read the next byte in chunk.bytes
	uint16_t readShort(); //read the next 2 bytes in chunk.bytes as short
	Value readConstant(); //read the next byte in chunk.bytes and lookup the constant it indicates
	
	void addition(); //seperate function for  the OpCode::Add case in run

	template<typename T>
	void validateArray(std::vector<T> const* vec, int index)
	{
		if (index >= vec->size())
			throw runtime_error("Es wurde versucht auf ein Array Element auﬂerhalb der Reichweite zuzugreifen!");
	}
public:
	std::vector<std::pair<std::string, ValueType>> args; //the types and count of the arguments the function takes (none for the main function)
	int argUnit;
	bool returned;
	Chunk chunk; //holds the byte code of the function
	ValueType returnType; //the return type of the function
	std::unordered_map<int, std::unordered_map<std::string, Value>> locals; //local variables of the function mapped to the numner their scope unit appeared at. At compile-time the values are empty.
public:
	using NativePtr = Value(*)(std::vector<Value>);
	NativePtr native; //the native function, nullptr if the function is not a native
	std::vector<Natives::CombineableValueType> nativeArgs; //the types of the arguments the function takes if it is a native. only used at compile time
private:
	//Stuff needed during runtime, be carefull here, this should only be touched through it's getter functions
	static constexpr size_t StackMax = 1024; //the maximum count of the stack
	std::array<Value, StackMax> stack; //the value stack
	std::array<Value, StackMax>::iterator stackTop; //iterator to the current top of the stack

	std::vector<uint8_t>::iterator ip; //instruction pointer to the current byte in the chunk

	std::unordered_map<std::string, Value>* globals; //pointer to the global variables map
	std::unordered_map<std::string, Function>* functions; //pointer to the map of functions
	std::unordered_map<std::string, Value::Struct>* structs; //pointer to the map of structs
};

