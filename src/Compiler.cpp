#include "Compiler.h"
#include <string>
#include <algorithm>

Compiler::Compiler(const std::string& file, Chunk* chunk)
	:
	file(file),
	chunk(chunk),
	hadError(false),
	panicMode(false),
	lastEmittedType(ValueType::NONE)
{
}

bool Compiler::compile()
{
	{
		Scanner scanner(file);
		tokens = scanner.scanTokens();
		hadError = scanner.errored();
	}
	previous = tokens.begin();
	current = tokens.begin();

	return hadError;
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

uint8_t Compiler::makeConstant(Value value)
{
	lastEmittedType = value.getType();
	int constant = currentChunk()->addConstant(std::move(value));
	if (constant > UINT8_MAX) {
		error("Zu viele Konstanten in diesem Chunk!");
		return 0;
	}

	return constant;
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

ValueType Compiler::Literal(bool canAssign)
{
	switch (previous->type)
	{
	case TokenType::WAHR: emitConstant(Value(true)); return ValueType::BOOL;
	case TokenType::FALSCH: emitConstant(Value(false)); return ValueType::BOOL;
	case TokenType::E: emitConstant(Value(2.71828182845904523536)); return ValueType::DOUBLE;
	case TokenType::PI: emitConstant(Value(3.14159265358979323846)); return ValueType::DOUBLE;
	case TokenType::PHI: emitConstant(Value(1.61803)); return ValueType::DOUBLE;
	case TokenType::TAU: emitConstant(Value(6.283185307179586)); return ValueType::DOUBLE;
	default: break;
	}

	return ValueType::NONE;
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
	}
	return expr;
}

ValueType Compiler::binary(bool canAssign)
{
	TokenType operatorType = previous->type;
	ValueType lhs = lastEmittedType;
	ParseRule rule = parseRules.at(operatorType);
	ValueType expr = parsePrecedence((Precedence)((int)rule.precedence + 1));

	switch (operatorType)
	{
	case TokenType::PLUS:
	{
		if (lhs == ValueType::BOOL || expr == ValueType::BOOL)
			errorAtCurrent("Ein Boolean kann kein Operand in einer addition sein");
		emitByte(op::ADD);
		switch (lhs)
		{
		case ValueType::INT:
			switch (expr)
			{
			case ValueType::INT: return ValueType::INT;
			case ValueType::DOUBLE: return ValueType::DOUBLE;
			case ValueType::CHAR: return ValueType::INT;
			case ValueType::STRING: return ValueType::STRING;
			}
			break;
		case ValueType::DOUBLE:
			switch (expr)
			{
			case ValueType::INT: return ValueType::DOUBLE;
			case ValueType::DOUBLE: return ValueType::DOUBLE;
			case ValueType::CHAR: return ValueType::INT;
			case ValueType::STRING: return ValueType::STRING;
			}
			break;
		case ValueType::CHAR:
			switch (expr)
			{
			case ValueType::INT: return ValueType::INT;
			case ValueType::DOUBLE: return ValueType::INT;
			case ValueType::CHAR: return ValueType::STRING;
			case ValueType::STRING: return ValueType::STRING;
			}
			break;
		case ValueType::STRING: return ValueType::STRING;
		default:
			break;
		}
		break;
	}
	case TokenType::MINUS:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent("Es k�nnen nur Zahlen von einander subtrahiert werden!");
		emitByte(op::SUBTRACT);
		if (lhs == ValueType::INT && expr == ValueType::INT) return ValueType::INT;
		else return ValueType::DOUBLE;
	}
	case TokenType::MAL:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent("Es k�nnen nur Zahlen miteinander multipliziert werden!");
		emitByte(op::MULTIPLY);
		if (lhs == ValueType::INT && expr == ValueType::INT) return ValueType::INT;
		else return ValueType::DOUBLE;
	}
	case TokenType::DURCH:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent("Es k�nnen nur Zahlen durcheinander dividiert werden!");
		emitByte(op::DIVIDE);
		if (lhs == ValueType::INT && expr == ValueType::INT) return ValueType::INT;
		else return ValueType::DOUBLE;
		break;
	}
	case TokenType::MODULO:
	{
		if (lhs != ValueType::INT || expr != ValueType::INT) errorAtCurrent("Es kann nur der Modulo aus zwei Zahlen berechnet werden!");
		emitByte(op::MODULO);
		return ValueType::INT;
	}
	case TokenType::HOCH:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent("Es kann nur der Exponent aus Zahlen berechnet werden!");
		emitByte(op::EXPONENT);
		if (lhs == ValueType::INT && expr == ValueType::INT) return ValueType::INT;
		else return ValueType::DOUBLE;
	}
	case TokenType::WURZEL:
	{
		if (lhs != ValueType::INT || expr != ValueType::INT)
			errorAtCurrent("Es kann nur die Wurzel aus ganzen Zahlen berechnet werden!");
		emitByte(op::ROOT);
		return ValueType::DOUBLE;
	}
	case TokenType::UM:
	{
		if (lhs != ValueType::INT || expr != ValueType::INT) errorAtCurrent("Es k�nnen nur die bits von Zahlen verschoben werden!");
		consume(TokenType::BIT, "Es fehlt 'bit' nach 'um'!");
		consume(TokenType::NACH, "Es fehlt 'nach' nach 'bit'!");
		if (match(TokenType::RECHTS)) emitByte(op::RIGHTBITSHIFT);
		else if (match(TokenType::LINKS)) emitByte(op::LEFTBITSHIFT);
		else errorAtCurrent("Es muss entweder 'links' oder 'rechts' nach beim Verschieben von Bits angegeben sein!");
		consume(TokenType::VERSCHOBEN, "Es wurde 'verschoben' erwartet!");
		return ValueType::INT;
	}
	case TokenType::GROESSER:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent("Es k�nnen nur Zahlen mit dem Operator 'gr��er als' verglichen werden!");
		emitByte(op::GREATER);
		return ValueType::BOOL;
	}
	case TokenType::GROESSERODER:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent("Es k�nnen nur Zahlen mit dem Operator 'gr��er als, oder' verglichen werden!");
		emitByte(op::GREATEREQUAL);
		return ValueType::BOOL;
	}
	case TokenType::KLEINER:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent("Es k�nnen nur Zahlen mit dem Operator 'kleiner als' verglichen werden!");
		emitByte(op::LESS);
		return ValueType::BOOL;
	}
	case TokenType::KLEINERODER:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent("Es k�nnen nur Zahlen mit dem Operator 'kleiner als, oder' verglichen werden!");
		emitByte(op::LESSEQUAL);
		return ValueType::BOOL;
	}
	case TokenType::GLEICH:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) && (expr == ValueType::INT || expr == ValueType::DOUBLE) ||
			(expr != ValueType::INT && expr != ValueType::DOUBLE) && (lhs == ValueType::INT || lhs == ValueType::DOUBLE) ||
			(lhs == ValueType::CHAR && expr != ValueType::CHAR) ||
			(lhs == ValueType::BOOL && expr != ValueType::BOOL) ||
			(lhs == ValueType::STRING && expr != ValueType::STRING))
			errorAtCurrent("Es k�nnen nur Zahlen mit dem Operator 'gr��er als' verglichen werden!");
		emitByte(op::EQUAL);
		return ValueType::BOOL;
	}
	case TokenType::UNGLEICH:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) && (expr == ValueType::INT || expr == ValueType::DOUBLE) ||
			(expr != ValueType::INT && expr != ValueType::DOUBLE) && (lhs == ValueType::INT || lhs == ValueType::DOUBLE) ||
			(lhs == ValueType::CHAR && expr != ValueType::CHAR) ||
			(lhs == ValueType::BOOL && expr != ValueType::BOOL) ||
			(lhs == ValueType::STRING && expr != ValueType::STRING))
			errorAtCurrent("Es k�nnen nur Zahlen mit dem Operator 'gr��er als' verglichen werden!");
		emitByte(op::UNEQUAL);
		return ValueType::BOOL;
	}
	default:
		break;
	}

	return expr;
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