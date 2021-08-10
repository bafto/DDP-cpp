#pragma once

#include "Scanner.h"
#include "Function.h"

class Compiler
{
public:
	Compiler(const std::string& filePath, std::unordered_map<std::string, Value>* globals, std::unordered_map<std::string, Function>* functions);

	bool compile(); //returns true on success, fills globals with declarations and functions with definitions
private:
	Chunk* currentChunk() { return &currentFunction->chunk; }; //the chunk that is currently filled

	//common emits
	void emitByte(uint8_t byte) { currentChunk()->write(byte); };
	void emitByte(OpCode code) { currentChunk()->write(code); };
	void emitBytes(uint8_t byte1, uint8_t byte2) { emitByte(byte1); emitByte(byte2); };
	void emitBytes(OpCode code, uint8_t byte) { emitByte(code); emitByte(byte); };
	void emitReturn() { emitByte(OpCode::RETURN); };
	void emitConstant(Value value) { emitBytes(OpCode::CONSTANT, makeConstant(std::move(value))); };

	uint8_t makeConstant(Value value);
private:
	const std::string filePath;

	std::unordered_map<std::string, Value>* globals;
	std::unordered_map<std::string, Function>* functions;

	bool hadError;
private:
	std::vector<Token> tokens;
	std::vector<Token>::iterator preIt; //previously scanned token
	std::vector<Token>::iterator curIt; //current token

	Function* currentFunction; //the function that is currently being compiled (most often the nameless main function)
};

