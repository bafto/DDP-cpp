#pragma once

#include "Chunk.h"
#include <unordered_map>
#include <array>

class Function
{
public:
	Function(ValueType returnType = ValueType::None, const std::vector<ValueType>& args = {}); //most often default constructed, but this is still provided

private:
	friend class VirtualMachine;

	//run the functions byte-code and return it's result. If the return type is ValueType::None the return Value should be discarded
	Value run(std::unordered_map<std::string, Value>* globals, std::unordered_map<std::string, Function>* functions); //only the VirtualMachine should call this function

	//these functions are only needed during runtime
	void push(Value value); //push a value onto the stack
	Value pop(); //pop a Value of the stack
	Value peek(int distance); //peek <distance> into the stack

	uint8_t readByte(); //read the next byte in chunk.bytes
	uint16_t readShort(); //read the next 2 bytes in chunk.bytes as short
	Value readConstant(); //read the next byte in chunk.bytes and lookup the constant it indicates
public:
	std::vector<ValueType> args; //the types and count of the arguments the function takes (none for the main function)
	Chunk chunk; //holds the byte code of the function
	ValueType returnType; //the return type of the function
	std::unordered_map<std::string, Value> locals; //local variables of the function. At compile-time the values are empty.

private:
	//Stuff needed during runtime, be carefull here, this should only be touched through it's getter functions
	static constexpr size_t StackMax = UINT8_MAX + 1; //the maximum count of the stack
	std::array<Value, StackMax> stack; //the value stack
	std::array<Value, StackMax>::iterator stackTop; //iterator to the current top of the stack

	std::vector<uint8_t>::iterator ip; //instruction pointer to the current byte in the chunk

	std::unordered_map<std::string, Value>* globals; //pointer to the global variables map
	std::unordered_map<std::string, Function>* functions; //pointer to the map of functions
};

