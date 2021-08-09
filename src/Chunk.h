#pragma once

#include "Value.h"

//the operation codes that will be executed
enum class OpCode
{
	CONSTANT,
	ARRAY, //define a array Literal at runtime
	NEGATE, //negate
	NOT, //bool negate
	ADD, //addition
	SUBTRACT, //subtraction
	MULTIPLY, //multiplication
	DIVIDE, //division
	MODULO, //int modulo
	EXPONENT, //exponent
	ROOT, //square root
	LN, //ln
	BETRAG, //abs
	BITWISENOT, // ~
	BITWISEAND, // &
	BITWISEOR, // |
	BITWISEXOR, // ^
	LEFTBITSHIFT, // <<
	RIGHTBITSHIFT, // >>
	EQUAL, // ==
	UNEQUAL, // !=
	GREATER, // >
	GREATEREQUAL, // >=
	LESS, // <
	LESSEQUAL, // <=
	DEFINE_GLOBAL,
	DEFINE_EMPTY_INTARR,
	DEFINE_EMPTY_DOUBLEARR,
	DEFINE_EMPTY_BOOLARR,
	DEFINE_EMPTY_CHARARR,
	DEFINE_EMPTY_STRINGARR,
	DEFINE_EMPTY_INTARR_LOCAL,
	DEFINE_EMPTY_DOUBLEARR_LOCAL,
	DEFINE_EMPTY_BOOLARR_LOCAL,
	DEFINE_EMPTY_CHARARR_LOCAL,
	DEFINE_EMPTY_STRINGARR_LOCAL,
	SET_ARRAY_ELEMENT,
	GET_ARRAY_ELEMENT,
	SET_ARRAY_ELEMENT_LOCAL,
	GET_ARRAY_ELEMENT_LOCAL,
	GET_GLOBAL,
	SET_GLOBAL,
	GET_LOCAL,
	SET_LOCAL,
	JUMP,
	JUMP_IF_FALSE,
	LOOP,
	POP, // pop the top of the value stack
	FORPREP,
	FORDONE,
	RETURN,
#ifndef NDEBUG
	PRINT,
#endif
};

//holds a chunk of byte-code and the corresponding constant Values
class Chunk
{
public:
	void write(uint8_t byte); //write a byte to the chunk
	void write(OpCode code); //overload so I don't always have to cast to uint8_t
	size_t addConstant(Value value); //add a constant and return it's index in the vector
public:
	std::vector<uint8_t> bytes; //the byte code
	std::vector<Value> constants; //the constant values, referenced in the byte code
};

