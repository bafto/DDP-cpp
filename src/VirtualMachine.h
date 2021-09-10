#pragma once

#include "Function.h"

enum class InterpretResult
{
	OK,
	CompileTimeError,
	RuntimeError,
	Exception
};

class VirtualMachine
{
public:
	VirtualMachine(const std::string& filePath, const std::vector<std::string>& sysArgs);

	InterpretResult run(bool graphics);
private:
	const std::string filePath;

	std::unordered_map<std::string, Value> globals;
	std::unordered_map<std::string, Function> functions;
};

