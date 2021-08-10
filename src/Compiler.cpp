#include "Compiler.h"

Compiler::Compiler(const std::string& filePath,
	std::unordered_map<std::string, Value>* globals,
	std::unordered_map<std::string, Function>* functions)
	:
	filePath(filePath),
	globals(globals),
	functions(functions),
	hadError(false),
	currentFunction(nullptr)
{}

bool Compiler::compile()
{
	{
		Scanner scanner(filePath);
		auto result = scanner.scanTokens();
		if (!result.second) hadError = true;
		tokens = std::move(result.first);
	}

	Function mainFunction;
	currentFunction = &mainFunction;

	//compile code into mainFunction

	functions->insert(std::make_pair("", std::move(mainFunction)));

	return !hadError;
}

uint8_t Compiler::makeConstant(Value value)
{
	//lastEmittedType = value.getType();
	int constant = currentChunk()->addConstant(std::move(value));
	if (constant > UINT8_MAX) {
		//error(u8"Zu viele Konstanten in diesem Chunk!");
		return 0;
	}

	return constant;
}
