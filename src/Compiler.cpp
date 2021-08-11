#include "Compiler.h"
#include <iostream>

Compiler::Compiler(const std::string& filePath,
	std::unordered_map<std::string, Value>* globals,
	std::unordered_map<std::string, Function>* functions)
	:
	filePath(filePath),
	globals(globals),
	functions(functions),
	hadError(false),
	panicMode(false),
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
		error(u8"Zu viele Konstanten in diesem Chunk!");
		return -1;
	}

	return constant;
}

void Compiler::error(std::string msg, std::vector<Token>::iterator where)
{
	if (panicMode) return;
	panicMode = true;
	std::cerr << u8"[Zeile " << where->line << u8"] Fehler ";

	if (where->type == TokenType::END) std::cerr << u8"am Ende";
	else if (where->type == TokenType::ERROR);
	else std::cerr << u8"bei '" << where->literal << u8"'";

	std::cerr << u8": " << msg << "\n";
	hadError = true;
}

void Compiler::error(std::string msg)
{
	error(msg, preIt);
}

void Compiler::advance()
{
	preIt = currIt;
	while (true)
	{
		currIt++;
		if (currIt == tokens.end()) currIt--;
		if (currIt->type != TokenType::ERROR) break;

		error(currIt->literal, currIt);
	}
}

void Compiler::consume(TokenType type, std::string errorMsg)
{
	if (currIt->type == type)
	{
		advance();
		return;
	}

	error(errorMsg, currIt);
}

bool Compiler::match(TokenType type)
{
	if (currIt->type != type) return false;
	advance();
	return true;
}

bool Compiler::check(TokenType type)
{
	return currIt->type == type;
}

bool Compiler::checkNext(TokenType type)
{
	return ((currIt + 1) != tokens.end()) && ((currIt + 1)->type == type);
}

ValueType Compiler::expression()
{
	return parsePrecedence(Precedence::Assignement);
}

ValueType Compiler::parsePrecedence(Precedence precedence)
{
	return ValueType();
}

ValueType Compiler::dnumber(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::inumber(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::string(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::character(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::arrLiteral(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::Literal(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::grouping(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::unary(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::binary(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::bitwise(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::and_(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::or_(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::variable(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}

ValueType Compiler::index(bool canAssign)
{
	error("Not implemented yet!");
	return ValueType::None;
}
