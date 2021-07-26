#pragma once

#include "Chunk.h"
#include "Scanner.h"

class Compiler
{
public:
	Compiler(const std::string& file, Chunk* chunk);

	void compile(); //compile the source code in the file into chunk
private:
	/**Functions only used in compile**/

	void advance();
	void consume(TokenType type, std::string msg);
	bool match(TokenType type);
	bool check(TokenType type);
	bool checkNext(TokenType type);

	void errorAtCurrent(std::string msg);
	void error(std::string msg);
	void errorAt(const Token& token, std::string msg);
private:
	Chunk* chunk; //the chunk into which the source code is compiled
	const std::string file; //path to the file with the source code

	bool hadError;
	bool panicMode;

	std::vector<Token> tokens;
	std::vector<Token>::iterator current;
	std::vector<Token>::iterator previous;
};

