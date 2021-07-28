#include "Compiler.h"
#include <string>
#include <algorithm>

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

#pragma region helper

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

#pragma endregion

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

uint8_t Compiler::makeConstant(Value value)
{
	int constant = currentChunk()->addConstant(std::move(value));
	if (constant > UINT8_MAX) {
		error("Zu viele Konstanten in diesem Chunk!");
		return 0;
	}

	return constant;
}

#pragma endregion

void Compiler::endCompiler()
{
	emitReturn();
}

ValueType Compiler::expression()
{
	return parsePrecedence(Precedence::ASSIGNMENT);
}

ValueType Compiler::parsePrecedence(Precedence precedence)
{
	return ValueType();
}

#pragma region expressions

ValueType Compiler::dnumber(bool canAssign)
{
	std::string str(previous->literal);
	str.replace(str.begin(), str.end(), ',', '.');
	double value = std::stod(str);
	emitConstant(Value(value));
	return ValueType::DOUBLE;
}

ValueType Compiler::inumber(bool canAssign)
{
	emitConstant(Value(std::stoi(previous->literal)));
	return ValueType::INT;
}

ValueType Compiler::string(bool canAssign)
{
	emitConstant(Value(std::string(previous->literal.begin() + 1, previous->literal.end() - 1))); //remove the leading and trailing "
	return ValueType::STRING;
}

ValueType Compiler::character(bool canAssign)
{
	emitConstant(Value(previous->literal[1])); //remove the leading and trailing '
	return ValueType::CHAR;
}

ValueType Compiler::grouping(bool canAssign)
{
	ValueType expr = expression();
	consume(TokenType::RIGHT_PAREN, "Es wurde eine ')' nach einem Ausdruck erwartet!");
	return expr;
}

ValueType Compiler::unary(bool canAssign)
{
	TokenType operatorType = previous->type;

	ValueType expr = parsePrecedence(Precedence::UNARY);

	switch (operatorType)
	{
	case TokenType::NEGATEMINUS:
		if (expr != ValueType::INT && expr != ValueType::DOUBLE)
			errorAtCurrent("Man kann nur Zahlen negieren!");
		emitByte(op::NEGATE);
		return expr;
	case TokenType::NICHT:
		if (expr != ValueType::BOOL)
			errorAtCurrent("Man kann nur Booleans mit 'nicht' negieren!");
		emitByte(op::NOT);
		return expr;
	case TokenType::LN:
		if (expr != ValueType::INT && expr != ValueType::DOUBLE)
			errorAtCurrent("Man kann nur aus Zahlen den ln berechnen!");
		emitByte(op::LN);
		return expr;
	case TokenType::BETRAG:
		if (expr != ValueType::INT && expr != ValueType::DOUBLE)
			errorAtCurrent("Man kann nur den Betrag von Zahlen berechnen!");
		emitByte(op::BETRAG);
		return expr;
	case TokenType::LOGISCHNICHT:
		if(expr != ValueType::INT)
			errorAtCurrent("Man kann nur Zahlen logisch negieren!");
		emitByte(op::BITWISENOT);
		return expr;
	default:
		break;
	}
}

ValueType Compiler::binary(bool canAssign)
{
	return ValueType();
}

ValueType Compiler::bitwise(bool canAssign)
{
	advance(); //skip the logisch
	ValueType lhs = parsePrecedence(Precedence::BITWISE); //evaluate the lhs expression
	if (lhs != ValueType::INT)
		errorAtCurrent("Operanden f�r logische Operatoren m�ssen Zahlen sein!");

	TokenType operatorType = previous->type;
	ValueType rhs = parsePrecedence(Precedence::BITWISE);
	if (rhs != ValueType::INT)
		errorAtCurrent("Operanden f�r logische Operatoren m�ssen Zahlen sein!");

	switch (operatorType)
	{
	case TokenType::UND: emitByte(op::BITWISEAND); break;
	case TokenType::ODER: emitByte(op::BITWISEOR); break;
	case TokenType::KONTRA: emitByte(op::BITWISEXOR); break;
	default:
		errorAtCurrent("Falscher logischer Operator!");
		break;
	}

	return ValueType::INT;
}

#pragma endregion