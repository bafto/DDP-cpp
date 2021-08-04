#pragma once

#include <vector>
#include "Chunk.h"

enum class ValueType;

class Function
{
public:
	std::vector<ValueType> args;
	Chunk chunk;
};

