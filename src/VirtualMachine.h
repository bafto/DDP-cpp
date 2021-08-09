#pragma once

#include "Function.h"

class VirtualMachine
{
public:
	VirtualMachine(const std::string& filePath, const std::vector<std::string>& sysArgs);

	void run();
private:
	const std::string filePath;

	std::unordered_map<std::string, Value> globals;
	std::unordered_map<std::string, Function> functions;
};

