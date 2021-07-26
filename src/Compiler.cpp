#include "Compiler.h"

Compiler::Compiler(const std::string& file, Chunk* chunk)
	:
	file(file),
	chunk(chunk),
	hadError(false),
	panicMode(false)
{
}

void Compiler::compile()
{
	{
		Scanner scanner(file);
		tokens = scanner.scanTokens();
	}
	previous = tokens.begin();
	current = tokens.begin();
}

void Compiler::advance()
{
	previous = current;
	
	while (true)
	{
		current++;
		if (current == tokens.end()) current--;
		else if (current->type != TokenType::ERROR) break;

		errorAtCurrent(current->literal);
	}
}

void Compiler::consume(TokenType type, std::string msg)
{
	if (current->type == type) {
		advance();
		return;
	}

	errorAtCurrent(msg);
}

bool Compiler::match(TokenType type)
{
	if (current->type != type) return false;
	advance();
	return true;
}

bool Compiler::check(TokenType type)
{
	return current->type == type;
}

bool Compiler::checkNext(TokenType type)
{
	return ((current + 1) != tokens.end()) && ((current + 1)->type == type);
}

#pragma region error

void Compiler::errorAtCurrent(std::string msg)
{
	errorAt(*current, std::move(msg));
}

void Compiler::error(std::string msg)
{
	errorAt(*previous, std::move(msg));
}

void Compiler::errorAt(const Token& token, std::string msg)
{
	if (panicMode) return;
	panicMode = true;
	std::cerr << "[Zeile " << token.line << "] Fehler";

	if (token.type == TokenType::END) std::cerr << " am Ende";
	else if (token.type == TokenType::ERROR);
	else std::cerr << " bei '" << token.literal << "'";

	std::cerr << ": " << msg;
	hadError = true;
}

#pragma endregion