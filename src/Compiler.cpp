#include "Compiler.h"
#include <iostream>
#include <algorithm>

Compiler::Compiler(const std::string& filePath,
	std::unordered_map<std::string, Value>* globals,
	std::unordered_map<std::string, Function>* functions)
	:
	filePath(filePath),
	globals(globals),
	functions(functions),
	hadError(false),
	panicMode(false),
	currentScopeUnit(nullptr),
	lastEmittedType(ValueType::None)
{}

bool Compiler::compile()
{
	{
		Scanner scanner(filePath);
		auto result = scanner.scanTokens();
		if (!result.second) hadError = true;
		tokens = std::move(result.first);
		currIt = tokens.begin();
	}

	Function mainFunction;

	ScopeUnit mainUnit(nullptr, &mainFunction, 0);
	currentScopeUnit = &mainUnit;

	while (!match(TokenType::END))
		declaration();
	emitReturn();

	functions->insert(std::make_pair("", std::move(mainFunction)));

	return !hadError;
}

uint8_t Compiler::makeConstant(Value value)
{
	lastEmittedType = value.Type();
	size_t constant = currentChunk()->addConstant(std::move(value));
	if (constant > UINT8_MAX) {
		error(u8"Zu viele Konstanten in diesem Chunk!");
		return -1;
	}

	return (uint8_t)constant;
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

#pragma region Token helper

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

#pragma endregion

#pragma region expressions

ValueType Compiler::expression()
{
	return parsePrecedence(Precedence::Assignement);
}

ValueType Compiler::parsePrecedence(Precedence precedence)
{
	advance();
	MemFunctPtr prefix = parseRules.at(preIt->type).prefix;
	if (prefix == nullptr)
	{
		error(u8"Das Token '" + preIt->literal + "' ist kein prefix Operator!");
		return ValueType::None;
	}

	bool canAssign = precedence <= Precedence::Assignement;
	ValueType expr = (this->*prefix)(canAssign);

	while (precedence <= parseRules.at(currIt->type).precedence)
	{
		advance();
		MemFunctPtr infix = parseRules.at(preIt->type).infix;
		if (infix == nullptr)
		{
			error(u8"Das Token '" + preIt->literal + "' ist kein infix Operator!");
			return ValueType::None;
		}
		/*if (infix == parseRules.at(TokenType::LEFT_PAREN).infix && expr != ValueType::FUNCTION)
		{
			errorAtCurrent("Du kannst nur Funktionen aufrufen!");
			return ValueType::NONE;
		}*/
		expr = (this->*infix)(false);
	}

	if (canAssign && match(TokenType::IST))
		error(u8"Ungültiges Zuweisungs Ziel!");

	return expr;
}

ValueType Compiler::dnumber(bool canAssign)
{
	std::string str(preIt->literal);
	std::replace(str.begin(), str.end(), ',', '.');
	double value = std::stod(str);
	emitConstant(Value(value));
	return ValueType::Double;
}

ValueType Compiler::inumber(bool canAssign)
{
	emitConstant(Value(std::stoi(preIt->literal)));
	return ValueType::Int;
}

ValueType Compiler::string(bool canAssign)
{
	emitConstant(Value(std::string(preIt->literal.begin() + 1, preIt->literal.end() - 1))); //remove the leading and trailing "
	return ValueType::String;
}

ValueType Compiler::character(bool canAssign)
{
	emitConstant(Value(preIt->literal[1])); //remove the leading and trailing '
	return ValueType::Char;
}

ValueType Compiler::arrLiteral(bool canAssign)
{
	if (match(TokenType::RIGHT_SQAREBRACKET))
		error(u8"Eine Variablen Gruppe muss mindestens ein Element enthalten!");
	ValueType arrType = expression();
	int i = 1;
	while (!match(TokenType::RIGHT_SQAREBRACKET))
	{
		consume(TokenType::SEMICOLON, u8"Unfertiges Array Literal!");
		ValueType rhs = expression();
		if (rhs != arrType)
			error(u8"Die Typen innerhalb des Array Literals stimmen nicht überein!");
		i++;
	}
	emitBytes(op::ARRAY, makeConstant(Value(i)));
	lastEmittedType = ValueType((int)arrType + 5);
	emitByte((uint8_t)lastEmittedType);
	return lastEmittedType;
}

ValueType Compiler::Literal(bool canAssign)
{
	switch (preIt->type)
	{
	case TokenType::WAHR: emitConstant(Value(true)); return ValueType::Bool;
	case TokenType::FALSCH: emitConstant(Value(false)); return ValueType::Bool;
	case TokenType::E: emitConstant(Value(2.71828182845904523536)); return ValueType::Double;
	case TokenType::PI: emitConstant(Value(3.14159265358979323846)); return ValueType::Double;
	case TokenType::PHI: emitConstant(Value(1.61803)); return ValueType::Double;
	case TokenType::TAU: emitConstant(Value(6.283185307179586)); return ValueType::Double;
	default: break;
	}

	return ValueType::None;
}

ValueType Compiler::grouping(bool canAssign)
{
	ValueType expr = expression();
	consume(TokenType::RIGHT_PAREN, u8"Es wurde eine ')' nach dem Ausdruck erwartet!");
	return expr;
}

ValueType Compiler::unary(bool canAssign)
{
	TokenType operatorType = preIt->type;

	ValueType expr = parsePrecedence(Precedence::Unary);

	switch (operatorType)
	{
	case TokenType::NEGATEMINUS:
		if (expr != ValueType::Int && expr != ValueType::Double)
			error(u8"Man kann nur Zahlen negieren!");
		emitByte(op::NEGATE);
		return expr;
	case TokenType::NICHT:
		if (expr != ValueType::Bool)
			error(u8"Man kann nur Booleans mit 'nicht' negieren!");
		emitByte(op::NOT);
		return expr;
	case TokenType::LN:
		if (expr != ValueType::Int && expr != ValueType::Double)
			error(u8"Man kann nur aus Zahlen den ln berechnen!");
		emitByte(op::LN);
		return expr;
	case TokenType::BETRAG:
		if (expr != ValueType::Int && expr != ValueType::Double)
			error(u8"Man kann nur den Betrag von Zahlen berechnen!");
		emitByte(op::BETRAG);
		return expr;
	case TokenType::LOGISCHNICHT:
		if (expr != ValueType::Int)
			error(u8"Man kann nur Zahlen logisch negieren!");
		emitByte(op::BITWISENOT);
		return expr;
	}
	return expr;
}

ValueType Compiler::binary(bool canAssign)
{
	TokenType operatorType = preIt->type;
	ValueType lhs = lastEmittedType;
	ParseRule rule = parseRules.at(operatorType);
	ValueType expr = parsePrecedence((Precedence)((int)rule.precedence + 1));

	switch (operatorType)
	{
	case TokenType::PLUS:
	{
		if (lhs == ValueType::Bool || expr == ValueType::Bool)
			error(u8"Ein Boolean kann kein Operand in einer addition sein");
		emitByte(op::ADD);
		switch (lhs)
		{
		case ValueType::Int:
			switch (expr)
			{
			case ValueType::Int: return ValueType::Int;
			case ValueType::Double: return ValueType::Double;
			case ValueType::Char: return ValueType::Int;
			case ValueType::String: return ValueType::String;
			}
			break;
		case ValueType::Double:
			switch (expr)
			{
			case ValueType::Int: return ValueType::Double;
			case ValueType::Double: return ValueType::Double;
			case ValueType::Char: return ValueType::Int;
			case ValueType::String: return ValueType::String;
			}
			break;
		case ValueType::Char:
			switch (expr)
			{
			case ValueType::Int: return ValueType::Int;
			case ValueType::Double: return ValueType::Int;
			case ValueType::Char: return ValueType::String;
			case ValueType::String: return ValueType::String;
			}
			break;
		case ValueType::String: return ValueType::String;
		default:
			break;
		}
		break;
	}
	case TokenType::MINUS:
	{
		if ((lhs != ValueType::Int && lhs != ValueType::Double) || (expr != ValueType::Int && expr != ValueType::Double))
			error(u8"Es können nur Zahlen von einander subtrahiert werden!");
		emitByte(op::SUBTRACT);
		if (lhs == ValueType::Int && expr == ValueType::Int) return ValueType::Int;
		else return ValueType::Double;
	}
	case TokenType::MAL:
	{
		if ((lhs != ValueType::Int && lhs != ValueType::Double) || (expr != ValueType::Int && expr != ValueType::Double))
			error(u8"Es können nur Zahlen miteinander multipliziert werden!");
		emitByte(op::MULTIPLY);
		if (lhs == ValueType::Int && expr == ValueType::Int) return ValueType::Int;
		else return ValueType::Double;
	}
	case TokenType::DURCH:
	{
		if ((lhs != ValueType::Int && lhs != ValueType::Double) || (expr != ValueType::Int && expr != ValueType::Double))
			error(u8"Es können nur Zahlen durcheinander dividiert werden!");
		emitByte(op::DIVIDE);
		if (lhs == ValueType::Int && expr == ValueType::Int) return ValueType::Int;
		else return ValueType::Double;
		break;
	}
	case TokenType::MODULO:
	{
		if (lhs != ValueType::Int || expr != ValueType::Int) error(u8"Es kann nur der Modulo aus zwei Zahlen berechnet werden!");
		emitByte(op::MODULO);
		return ValueType::Int;
	}
	case TokenType::HOCH:
	{
		if ((lhs != ValueType::Int && lhs != ValueType::Double) || (expr != ValueType::Int && expr != ValueType::Double))
			error(u8"Es kann nur der Exponent aus Zahlen berechnet werden!");
		emitByte(op::EXPONENT);
		if (lhs == ValueType::Int && expr == ValueType::Int) return ValueType::Int;
		else return ValueType::Double;
	}
	case TokenType::WURZEL:
	{
		if (lhs != ValueType::Int || expr != ValueType::Int)
			error(u8"Es kann nur die Wurzel aus ganzen Zahlen berechnet werden!");
		emitByte(op::ROOT);
		return ValueType::Double;
	}
	case TokenType::UM:
	{
		if (lhs != ValueType::Int || expr != ValueType::Int) error(u8"Es können nur die bits von Zahlen verschoben werden!");
		consume(TokenType::BIT, u8"Es fehlt 'bit' nach 'um'!");
		consume(TokenType::NACH, u8"Es fehlt 'nach' nach 'bit'!");
		if (match(TokenType::RECHTS)) emitByte(op::RIGHTBITSHIFT);
		else if (match(TokenType::LINKS)) emitByte(op::LEFTBITSHIFT);
		else error(u8"Es muss entweder 'links' oder 'rechts' nach beim Verschieben von Bits angegeben sein!");
		consume(TokenType::VERSCHOBEN, u8"Es wurde 'verschoben' erwartet!");
		return ValueType::Int;
	}
	case TokenType::GROESSER:
	{
		if ((lhs != ValueType::Int && lhs != ValueType::Double) || (expr != ValueType::Int && expr != ValueType::Double))
			error(u8"Es können nur Zahlen mit dem Operator 'größer als' verglichen werden!");
		emitByte(op::GREATER);
		consume(TokenType::IST, u8"Nach 'größer als' fehlt 'ist'!");
		return ValueType::Bool;
	}
	case TokenType::GROESSERODER:
	{
		if ((lhs != ValueType::Int && lhs != ValueType::Double) || (expr != ValueType::Int && expr != ValueType::Double))
			error(u8"Es können nur Zahlen mit dem Operator 'größer als, oder' verglichen werden!");
		emitByte(op::GREATEREQUAL);
		consume(TokenType::IST, u8"Nach 'gr��er als, oder' fehlt 'ist'!");
		return ValueType::Bool;
	}
	case TokenType::KLEINER:
	{
		if ((lhs != ValueType::Int && lhs != ValueType::Double) || (expr != ValueType::Int && expr != ValueType::Double))
			error(u8"Es können nur Zahlen mit dem Operator 'kleiner als' verglichen werden!");
		emitByte(op::LESS);
		consume(TokenType::IST, u8"Nach 'kleiner als' fehlt 'ist'!");
		return ValueType::Bool;
	}
	case TokenType::KLEINERODER:
	{
		if ((lhs != ValueType::Int && lhs != ValueType::Double) || (expr != ValueType::Int && expr != ValueType::Double))
			error(u8"Es können nur Zahlen mit dem Operator 'kleiner als, oder' verglichen werden!");
		emitByte(op::LESSEQUAL);
		consume(TokenType::IST, u8"Nach 'kleiner als, oder' fehlt 'ist'!");
		return ValueType::Bool;
	}
	case TokenType::GLEICH:
	{
		if ((lhs != ValueType::Int && lhs != ValueType::Double) && (expr == ValueType::Int || expr == ValueType::Double) ||
			(expr != ValueType::Int && expr != ValueType::Double) && (lhs == ValueType::Int || lhs == ValueType::Double) ||
			(lhs == ValueType::Char && expr != ValueType::Char) ||
			(lhs == ValueType::Bool && expr != ValueType::Bool) ||
			(lhs == ValueType::String && expr != ValueType::String))
			error(u8"Es können nur Zahlen mit dem Operator 'größer als' verglichen werden!");
		emitByte(op::EQUAL);
		consume(TokenType::IST, u8"Nach 'gleich' fehlt 'ist'!");
		return ValueType::Bool;
	}
	case TokenType::UNGLEICH:
	{
		if ((lhs != ValueType::Int && lhs != ValueType::Double) && (expr == ValueType::Int || expr == ValueType::Double) ||
			(expr != ValueType::Int && expr != ValueType::Double) && (lhs == ValueType::Int || lhs == ValueType::Double) ||
			(lhs == ValueType::Char && expr != ValueType::Char) ||
			(lhs == ValueType::Bool && expr != ValueType::Bool) ||
			(lhs == ValueType::String && expr != ValueType::String))
			error(u8"Es können nur Zahlen mit dem Operator 'größer als' verglichen werden!");
		emitByte(op::UNEQUAL);
		consume(TokenType::IST, u8"Nach 'ungleich' fehlt 'ist'!");
		return ValueType::Bool;
	}
	default:
		break;
	}

	return expr;
}

ValueType Compiler::bitwise(bool canAssign)
{
	//advance(); //skip the logisch
	ValueType lhs = parsePrecedence(Precedence::Bitwise); //evaluate the lhs expression
	if (lhs != ValueType::Int)
		error(u8"Operanden für logische Operatoren müssen Zahlen sein!");

	TokenType operatorType = currIt->type;
	advance();
	ValueType rhs = parsePrecedence(Precedence::Bitwise);
	if (rhs != ValueType::Int)
		error(u8"Operanden für logische Operatoren müssen Zahlen sein!");

	switch (operatorType)
	{
	case TokenType::UND: emitByte(op::BITWISEAND); break;
	case TokenType::ODER: emitByte(op::BITWISEOR); break;
	case TokenType::KONTRA: emitByte(op::BITWISEXOR); break;
	default:
		error(u8"Falscher logischer Operator!");
		break;
	}

	return ValueType::Int;
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

#pragma endregion

void Compiler::declaration()
{
	if (match(TokenType::DER) || match(TokenType::DIE) || match(TokenType::DAS))
	{
		if (preIt->type == TokenType::DIE && match(TokenType::FUNKTION))
			funDeclaration();
		else
			varDeclaration();
	}
	else
		statement();

	if (panicMode) synchronize();
}

void Compiler::statement()
{
#ifndef NDEBUG
	if (match(TokenType::PRINT))
	{
		printStatement();
		return;
	}
#endif

	if (match(TokenType::WENN))
		ifStatement();
	else if (match(TokenType::FUER))
		forStatement();
	else if (match(TokenType::SOLANGE))
		whileStatement();
	else if (match(TokenType::COLON))
	{
		beginScope();
		block();
		endScope();
	}
	else
		expressionStatement();
}

void Compiler::synchronize()
{
	panicMode = false;

	while (currIt->type != TokenType::END)
	{
		if (preIt->type == TokenType::DOT) return;
		switch (currIt->type)
		{
#ifndef NDEBUG
		case TokenType::PRINT:
#endif
		case TokenType::FUNKTION:
		case TokenType::DER:
		case TokenType::DIE:
		case TokenType::DAS:
		case TokenType::FUER:
		case TokenType::WENN:
		case TokenType::SOLANGE:
		case TokenType::GIB:
			return;
		default:;
		}

		advance();
	}
}

#pragma region statements

void Compiler::beginScope()
{
	error("Not implemented yet!");
}

void Compiler::block()
{
	error("Not implemented yet!");
}

void Compiler::endScope()
{
	error("Not implemented yet!");
}

void Compiler::expressionStatement()
{
	ValueType expr = expression();
	consume(TokenType::DOT, u8"Es wrude ein '.' nach einem Ausdruck erwartet!");
	emitByte(op::POP);
}

void Compiler::varDeclaration()
{
	error("Not implemented yet!");
}

void Compiler::funDeclaration()
{
	error("Not implemented yet!");
}

void Compiler::patchJump(int offset)
{
	error("Not implemented yet!");
}

void Compiler::ifStatement()
{
	error("Not implemented yet!");
}

void Compiler::whileStatement()
{
	error("Not implemented yet!");
}

void Compiler::forStatement()
{
	error("Not implemented yet!");
}

#ifndef NDEBUG
void Compiler::printStatement()
{
	ValueType type = expression();
	if (type == ValueType::None)
		error(u8"Cannot print expression of type none!");
	consume(TokenType::DOT, u8"Es wurde ein '.' nach '$' erwartet!");
	emitByte(op::PRINT);
}
#endif

#pragma endregion

#pragma region ScopeUnit

Compiler::ScopeUnit::ScopeUnit(ScopeUnit* enclosingUnit, Function* enclosingFunction, int scopeDepth)
	:
	identifier(count++),
	enclosingUnit(enclosingUnit),
	enclosingFunction(enclosingFunction),
	scopeDepth(scopeDepth)
{}

Compiler::ScopeUnit::~ScopeUnit()
{}

#pragma endregion