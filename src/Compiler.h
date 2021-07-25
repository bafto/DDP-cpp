#pragma once

#include "Chunk.h"

class Compiler
{
public:
	Compiler(const std::string& file, Chunk* chunk);

	void compile(); //compile the source code in the file into chunk
private:
	Chunk* chunk; //the chunk into which the source code is compiled
	const std::string file; //path to the file with the source code
};

