#include "Compiler.h"
#include <string>
#include <algorithm>
#include <iostream>

Compiler::Compiler(const std::string& file, Chunk* chunk)
	:
	file(file),
	chunk(chunk),
	hadError(false),
	panicMode(false),
	lastEmittedType(ValueType::NONE),
	globals(),
	currentScopeUnit(nullptr)
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

	ScopeUnit globalUnit(currentScopeUnit);

	while (!match(TokenType::END))
	{
		declaration();
	}

	endCompiler();

	return !hadError;
}

#pragma region helper

void Compiler::advance()
{
	previous = current;
	
	while (true)
	{
		current++;
		if (current == tokens.end()) current--;
		if (current->type != TokenType::ERROR) break;

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
		error(u8"Zu viele Konstanten in diesem Chunk!");
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
	std::cerr << u8"[Zeile " << token.line << u8"] Fehler";

	if (token.type == TokenType::END) std::cerr << u8" am Ende";
	else if (token.type == TokenType::ERROR);
	else std::cerr << u8" bei '" << token.literal << u8"'";

	std::cerr << u8": " << msg;
	hadError = true;
}

#pragma endregion

void Compiler::endCompiler()
{
	emitReturn();
}

void Compiler::synchronize()
{
	panicMode = false;

	while (current->type != TokenType::END)
	{
		if (previous->type == TokenType::DOT) return;
		switch (current->type)
		{
		case TokenType::FUNKTION:
		case TokenType::DER:
		case TokenType::DIE:
		case TokenType::DAS:
		case TokenType::FUER:
		case TokenType::WENN:
		case TokenType::SOLANGE:
#ifdef _MDEBUG_
		case TokenType::PRINT:
#endif
		case TokenType::GIB:
			return;
		default:;
		}

		advance();
	}
}

void Compiler::declaration()
{
	if (match(TokenType::DER) || match(TokenType::DIE) || match(TokenType::DAS))
	{
		/*if (previous->type == TokenType::DIE && match(TokenType::FUNKTION))
			funDeclaration();
		else*/
		varDeclaration();
	}
	else
		statement();

	if (panicMode) synchronize();
}

void Compiler::statement()
{
#ifdef _MDEBUG_
	if (match(TokenType::PRINT))
		printStatement();
	else
#endif
	if (match(TokenType::COLON))
	{
		beginScope();
		block();
		endScope();
	}
	else
		expressionStatement();
}

ValueType Compiler::boolAssignement()
{
	if (match(TokenType::WAHR))
	{
		if (match(TokenType::WENN))
		{
			return expression();
		}
		else
		{
			emitConstant(Value(true));
			return ValueType::BOOL;
		}
	}
	else if (match(TokenType::FALSCH))
	{
		if (match(TokenType::WENN))
		{
			ValueType expr = expression();
			emitByte(op::NOT);
			return expr;
		}
		else
		{
			emitConstant(Value(false));
			return ValueType::BOOL;
		}
	}
	else
		errorAtCurrent("Beim definieren eines Booleans muss wahr oder falsch stehen!");
	return ValueType::BOOL;
}

uint8_t Compiler::identifierConstant(std::string identifier, ValueType type)
{
	if (globals.find(identifier) != globals.end())
	{
		errorAtCurrent(u8"Eine Variable mit dem Namen " + identifier + " existiert bereits!");
		return -1;
	}
	globals.insert({ identifier, type });
	return makeConstant(Value(identifier));
}

uint8_t Compiler::parseVariable(ValueType type, std::string msg)
{
	consume(TokenType::IDENTIFIER, msg);

	declareVariable(type);
	if (currentScopeUnit->scopeDepth > 0) return 0;

	return identifierConstant(previous->literal, type);
}

void Compiler::declareVariable(ValueType type)
{
	if (currentScopeUnit->scopeDepth == 0) return;

	std::string varName = previous->literal;
	for (int i = currentScopeUnit->localCount - 1; i >= 0; i--)
	{
		Local* local = &currentScopeUnit->locals[i];
		if (local->token.depth != -1 && local->token.depth < currentScopeUnit->scopeDepth)
			break;

		if (varName == local->token.literal)
			errorAtCurrent(u8"Die Variable '" + varName + "' existiert bereits!");
	}

	addLocal(*previous, type);
}

void Compiler::addLocal(Token token, ValueType type)
{
	if (currentScopeUnit->localCount == UINT8_MAX + 1) {
		error("Zu viele lokale Variablen in diesem Bereich!");
		return;
	}

	Local* local = &currentScopeUnit->locals[currentScopeUnit->localCount++];
	local->token = token;
	local->token.depth = -1;
	local->type = type;
}

void Compiler::defineVariable(uint8_t global)
{
	if (currentScopeUnit->scopeDepth > 0)
	{
		markInitialized();
		return;
	}
	emitBytes(op::DEFINE_GLOBAL, global);
}

void Compiler::defineVariable(uint8_t global, ValueType arrType)
{
	if (currentScopeUnit->scopeDepth > 0)
	{
		markInitialized();
		switch (arrType)
		{
		case ValueType::INTARR: emitBytes(op::DEFINE_EMPTY_INTARR_LOCAL, currentScopeUnit->localCount - 1); break;
		case ValueType::DOUBLEARR: emitBytes(op::DEFINE_EMPTY_DOUBLEARR_LOCAL, currentScopeUnit->localCount - 1); break;
		case ValueType::BOOLARR: emitBytes(op::DEFINE_EMPTY_BOOLARR_LOCAL, currentScopeUnit->localCount - 1); break;
		case ValueType::CHARARR: emitBytes(op::DEFINE_EMPTY_CHARARR_LOCAL, currentScopeUnit->localCount - 1); break;
		case ValueType::STRINGARR: emitBytes(op::DEFINE_EMPTY_STRINGARR_LOCAL, currentScopeUnit->localCount - 1); break;
		}
		return;
	}

	switch (arrType)
	{
	case ValueType::INTARR: emitBytes(op::DEFINE_EMPTY_INTARR, global); break;
	case ValueType::DOUBLEARR: emitBytes(op::DEFINE_EMPTY_DOUBLEARR, global); break;
	case ValueType::BOOLARR: emitBytes(op::DEFINE_EMPTY_BOOLARR, global); break;
	case ValueType::CHARARR: emitBytes(op::DEFINE_EMPTY_CHARARR, global); break;
	case ValueType::STRINGARR: emitBytes(op::DEFINE_EMPTY_STRINGARR, global); break;
	}
}

#pragma region statements

void Compiler::block()
{
	while (current->type != TokenType::END && current->depth >= currentScopeUnit->scopeDepth)
		declaration();
}

void Compiler::varDeclaration()
{
	ValueType varType = ValueType::NONE;
	switch (previous->type)
	{
	case TokenType::DER:
		if (!match(TokenType::BOOLEAN))
		{
			errorAtCurrent(u8"Der Artikel 'Der' passt nur zum Typ Boolean!");
			return;
		}
		varType = ValueType::BOOL;
		break;
	case TokenType::DAS:
		if (!match(TokenType::ZEICHEN))
		{
			errorAtCurrent(u8"Der Artikel 'Das' passt nur zum Typ Zeichen!");
			return;
		}
		varType = ValueType::CHAR;
		break;
	case TokenType::DIE:
		if (!match(TokenType::ZAHL) && !match(TokenType::ZEICHENKETTE) && !match(TokenType::KOMMAZAHL) &&
			!match(TokenType::ZAHLEN) && !match(TokenType::KOMMAZAHLEN) && !match(TokenType::ZEICHEN) && !match(TokenType::ZEICHENKETTEN) && !match(TokenType::BOOLEANS))
		{
			errorAtCurrent(u8"Der Artikel 'Die' passt nur zu den Typen Zahl, Kommazahl und Zeichenkette!");
			return;
		}
		varType = previous->type == TokenType::ZAHL ? ValueType::INT : ValueType::NONE;
		varType = previous->type == TokenType::KOMMAZAHL ? ValueType::DOUBLE : varType;
		varType = previous->type == TokenType::ZEICHENKETTE ? ValueType::STRING : varType;
		varType = previous->type == TokenType::ZAHLEN ? ValueType::INTARR : varType;
		varType = previous->type == TokenType::KOMMAZAHLEN ? ValueType::DOUBLEARR : varType;
		varType = previous->type == TokenType::BOOLEANS ? ValueType::BOOLARR : varType;
		varType = previous->type == TokenType::ZEICHEN ? ValueType::CHARARR : varType;
		varType = previous->type == TokenType::ZEICHENKETTEN ? ValueType::STRINGARR : varType;
		break;
	}

	uint8_t global = parseVariable(varType, u8"Es wurde Ein Variablen-Name erwartet!");

	if (match(TokenType::IST)) 
	{
		switch (varType)
		{
		case ValueType::INTARR:
		case ValueType::DOUBLEARR:
		case ValueType::BOOLARR:
		case ValueType::CHARARR:
		case ValueType::STRINGARR: errorAt(*previous, u8"Beim definieren einer Variablen Gruppe wird 'sind' statt 'ist' erwartet!"); break;
		default: break;
		}

		ValueType rhs = ValueType::NONE;
		if (varType == ValueType::BOOL)
		{
			rhs = boolAssignement();
		}
		else
			rhs = expression();

		if (rhs != varType) 
		{
			errorAtCurrent(u8"Einer Variable kann nur ein Wert vom gleichen Typ zugewiesen werden!");
			return;
		}
	}
	else if (match(TokenType::SIND))
	{
		switch (varType)
		{
		case ValueType::INT:
		case ValueType::DOUBLE:
		case ValueType::BOOL:
		case ValueType::CHAR:
		case ValueType::STRING: errorAt(*previous, u8"Beim definieren einer einzelnen Variable wurde 'ist' statt 'sind' erwartet!"); break;
		default: break;
		}

		ValueType rhs = expression();
		if (rhs == ValueType::INT)
		{
			consume(TokenType::STUECK, u8"Beim definieren einer leeren Variablen Gruppe wurde 'Stück' erwartet!");
			consume(TokenType::DOT, u8"Satzzeichen beachten! Nach einer Variablen-Definition muss ein '.' folgen");
			defineVariable(global, varType);
			return;
		}
		if (rhs != varType)
		{
			errorAtCurrent(u8"Einer Variable kann nur ein Wert vom gleichen Typ zugewiesen werden!");
			return;
		}
	}
	else 
	{
		errorAtCurrent(u8"Eine Variable muss immer initialisiert werden!");
		return;
	}
	consume(TokenType::DOT, u8"Satzzeichen beachten! Nach einer Variablen-Definition muss ein '.' folgen");

	defineVariable(global);
}

void Compiler::expressionStatement()
{
	expression();
	consume(TokenType::DOT, u8"Es wurde ein '.' nach einem Ausdruck erwartet erwartet!");
	emitByte(op::POP);
}

#ifdef _MDEBUG_
void Compiler::printStatement()
{
	expression();
	consume(TokenType::DOT, u8"Es wurde ein '.' nach '$' erwartet!");
	emitByte(op::PRINT);
}
#endif

#pragma endregion

ValueType Compiler::expression()
{
	return parsePrecedence(Precedence::ASSIGNMENT);
}

ValueType Compiler::parsePrecedence(Precedence precedence)
{
	advance();
	MemFuncPtr prefix = parseRules.at(previous->type).prefix;
	if (prefix == nullptr)
	{
		error(u8"Das Token ist kein prefix Operator!");
		return ValueType::NONE;
	}

	bool canAssign = precedence <= Precedence::ASSIGNMENT;
	ValueType expr = (this->*prefix)(canAssign);

	while (precedence <= parseRules.at(current->type).precedence)
	{
		advance();
		MemFuncPtr infix = parseRules.at(previous->type).infix;
		if (infix == nullptr)
		{
			error(u8"Das Token ist kein prefix Operator!");
			return ValueType::NONE;
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

#pragma region expressions

ValueType Compiler::dnumber(bool canAssign)
{
	std::string str(previous->literal);
	std::replace(str.begin(), str.end(), ',', '.');
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

ValueType Compiler::arrLiteral(bool canAssign)
{
	if (match(TokenType::RIGHT_SQAREBRACKET))
		errorAtCurrent(u8"Eine Variablen Gruppe kann nicht größe 0 haben!");
	ValueType arrType = expression();
	int i = 1;
	while (!match(TokenType::RIGHT_SQAREBRACKET))
	{
		consume(TokenType::SEMICOLON, u8"Unfertiges Array Literal!");
		ValueType rhs = expression();
		if (rhs != arrType)
			errorAt(*previous, u8"Die Typen innerhalb des Array Literals stimmen nicht überein!");
		i++;
	}
	emitBytes(op::ARRAY, makeConstant(Value(i)));
	emitByte((uint8_t)arrType);
	lastEmittedType = ValueType((int)arrType + 5);
	return lastEmittedType;
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
	consume(TokenType::RIGHT_PAREN, u8"Es wurde eine ')' nach einem Ausdruck erwartet!");
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
			errorAtCurrent(u8"Man kann nur Zahlen negieren!");
		emitByte(op::NEGATE);
		return expr;
	case TokenType::NICHT:
		if (expr != ValueType::BOOL)
			errorAtCurrent(u8"Man kann nur Booleans mit 'nicht' negieren!");
		emitByte(op::NOT);
		return expr;
	case TokenType::LN:
		if (expr != ValueType::INT && expr != ValueType::DOUBLE)
			errorAtCurrent(u8"Man kann nur aus Zahlen den ln berechnen!");
		emitByte(op::LN);
		return expr;
	case TokenType::BETRAG:
		if (expr != ValueType::INT && expr != ValueType::DOUBLE)
			errorAtCurrent(u8"Man kann nur den Betrag von Zahlen berechnen!");
		emitByte(op::BETRAG);
		return expr;
	case TokenType::LOGISCHNICHT:
		if(expr != ValueType::INT)
			errorAtCurrent(u8"Man kann nur Zahlen logisch negieren!");
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
			errorAtCurrent(u8"Ein Boolean kann kein Operand in einer addition sein");
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
			errorAtCurrent(u8"Es können nur Zahlen von einander subtrahiert werden!");
		emitByte(op::SUBTRACT);
		if (lhs == ValueType::INT && expr == ValueType::INT) return ValueType::INT;
		else return ValueType::DOUBLE;
	}
	case TokenType::MAL:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent(u8"Es können nur Zahlen miteinander multipliziert werden!");
		emitByte(op::MULTIPLY);
		if (lhs == ValueType::INT && expr == ValueType::INT) return ValueType::INT;
		else return ValueType::DOUBLE;
	}
	case TokenType::DURCH:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent(u8"Es können nur Zahlen durcheinander dividiert werden!");
		emitByte(op::DIVIDE);
		if (lhs == ValueType::INT && expr == ValueType::INT) return ValueType::INT;
		else return ValueType::DOUBLE;
		break;
	}
	case TokenType::MODULO:
	{
		if (lhs != ValueType::INT || expr != ValueType::INT) errorAtCurrent(u8"Es kann nur der Modulo aus zwei Zahlen berechnet werden!");
		emitByte(op::MODULO);
		return ValueType::INT;
	}
	case TokenType::HOCH:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent(u8"Es kann nur der Exponent aus Zahlen berechnet werden!");
		emitByte(op::EXPONENT);
		if (lhs == ValueType::INT && expr == ValueType::INT) return ValueType::INT;
		else return ValueType::DOUBLE;
	}
	case TokenType::WURZEL:
	{
		if (lhs != ValueType::INT || expr != ValueType::INT)
			errorAtCurrent(u8"Es kann nur die Wurzel aus ganzen Zahlen berechnet werden!");
		emitByte(op::ROOT);
		return ValueType::DOUBLE;
	}
	case TokenType::UM:
	{
		if (lhs != ValueType::INT || expr != ValueType::INT) errorAtCurrent(u8"Es können nur die bits von Zahlen verschoben werden!");
		consume(TokenType::BIT, u8"Es fehlt 'bit' nach 'um'!");
		consume(TokenType::NACH, u8"Es fehlt 'nach' nach 'bit'!");
		if (match(TokenType::RECHTS)) emitByte(op::RIGHTBITSHIFT);
		else if (match(TokenType::LINKS)) emitByte(op::LEFTBITSHIFT);
		else errorAtCurrent(u8"Es muss entweder 'links' oder 'rechts' nach beim Verschieben von Bits angegeben sein!");
		consume(TokenType::VERSCHOBEN, u8"Es wurde 'verschoben' erwartet!");
		return ValueType::INT;
	}
	case TokenType::GROESSER:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent(u8"Es können nur Zahlen mit dem Operator 'größer als' verglichen werden!");
		emitByte(op::GREATER);
		consume(TokenType::IST, u8"Nach 'größer als' fehlt 'ist'!");
		return ValueType::BOOL;
	}
	case TokenType::GROESSERODER:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent(u8"Es können nur Zahlen mit dem Operator 'größer als, oder' verglichen werden!");
		emitByte(op::GREATEREQUAL);
		consume(TokenType::IST, u8"Nach 'größer als, oder' fehlt 'ist'!");
		return ValueType::BOOL;
	}
	case TokenType::KLEINER:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent(u8"Es können nur Zahlen mit dem Operator 'kleiner als' verglichen werden!");
		emitByte(op::LESS);
		consume(TokenType::IST, u8"Nach 'kleiner als' fehlt 'ist'!");
		return ValueType::BOOL;
	}
	case TokenType::KLEINERODER:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) || (expr != ValueType::INT && expr != ValueType::DOUBLE))
			errorAtCurrent(u8"Es können nur Zahlen mit dem Operator 'kleiner als, oder' verglichen werden!");
		emitByte(op::LESSEQUAL);
		consume(TokenType::IST, u8"Nach 'kleiner als, oder' fehlt 'ist'!");
		return ValueType::BOOL;
	}
	case TokenType::GLEICH:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) && (expr == ValueType::INT || expr == ValueType::DOUBLE) ||
			(expr != ValueType::INT && expr != ValueType::DOUBLE) && (lhs == ValueType::INT || lhs == ValueType::DOUBLE) ||
			(lhs == ValueType::CHAR && expr != ValueType::CHAR) ||
			(lhs == ValueType::BOOL && expr != ValueType::BOOL) ||
			(lhs == ValueType::STRING && expr != ValueType::STRING))
			errorAtCurrent(u8"Es können nur Zahlen mit dem Operator 'größer als' verglichen werden!");
		emitByte(op::EQUAL);
		consume(TokenType::IST, u8"Nach 'gleich' fehlt 'ist'!");
		return ValueType::BOOL;
	}
	case TokenType::UNGLEICH:
	{
		if ((lhs != ValueType::INT && lhs != ValueType::DOUBLE) && (expr == ValueType::INT || expr == ValueType::DOUBLE) ||
			(expr != ValueType::INT && expr != ValueType::DOUBLE) && (lhs == ValueType::INT || lhs == ValueType::DOUBLE) ||
			(lhs == ValueType::CHAR && expr != ValueType::CHAR) ||
			(lhs == ValueType::BOOL && expr != ValueType::BOOL) ||
			(lhs == ValueType::STRING && expr != ValueType::STRING))
			errorAtCurrent(u8"Es können nur Zahlen mit dem Operator 'größer als' verglichen werden!");
		emitByte(op::UNEQUAL);
		consume(TokenType::IST, u8"Nach 'ungleich' fehlt 'ist'!");
		return ValueType::BOOL;
	}
	default:
		break;
	}

	return expr;
}

ValueType Compiler::bitwise(bool canAssign)
{
	//advance(); //skip the logisch
	ValueType lhs = parsePrecedence(Precedence::BITWISE); //evaluate the lhs expression
	if (lhs != ValueType::INT)
		errorAtCurrent(u8"Operanden für logische Operatoren müssen Zahlen sein!");

	TokenType operatorType = current->type;
	advance();
	ValueType rhs = parsePrecedence(Precedence::BITWISE);
	if (rhs != ValueType::INT)
		errorAtCurrent(u8"Operanden für logische Operatoren müssen Zahlen sein!");

	switch (operatorType)
	{
	case TokenType::UND: emitByte(op::BITWISEAND); break;
	case TokenType::ODER: emitByte(op::BITWISEOR); break;
	case TokenType::KONTRA: emitByte(op::BITWISEXOR); break;
	default:
		errorAtCurrent(u8"Falscher logischer Operator!");
		break;
	}

	return ValueType::INT;
}

int Compiler::resolveLocal(ScopeUnit* unit, std::string name, ValueType* type)
{
	for (int i = unit->localCount - 1; i >= 0; i--)
	{
		Local* local = &unit->locals[i];
		if (local->token.literal == name)
		{
			if (local->token.depth == -1) {
				error("Du kannst eine Variable nicht mit sich selbst initialisieren!");
			}
			*type = local->type;
			return i;
		}
	}
	return -1;
}

void Compiler::markInitialized()
{
	currentScopeUnit->locals[currentScopeUnit->localCount - 1].token.depth = currentScopeUnit->scopeDepth;
}

ValueType Compiler::namedVariable(std::string name, bool canAssign)
{
	OpCode getOp, setOp;
	ValueType type;
	int arg = resolveLocal(currentScopeUnit, name, &type);
	int arrArg = arg;
	if (arg != -1) //it is a local variable
	{
		getOp = op::GET_LOCAL;
		setOp = op::SET_LOCAL;
	}
	else
	{
		if (globals.find(name) == globals.end())
		{
			errorAtCurrent(u8"Die Variable " + name + u8" wurde noch nicht definiert!");
			return ValueType::NONE;
		}
		getOp = op::GET_GLOBAL;
		setOp = op::SET_GLOBAL;
		arg = (int)makeConstant(Value(name));
		type = globals.at(name);
		//if(type == ValueType::FUNCTION) calledFunc = functions.at(name);
	}

	if (canAssign && match(TokenType::IST))
	{
		ValueType expr = ValueType::NONE;
		if (type >= ValueType::INTARR && type <= ValueType::STRINGARR)
			errorAt(*previous, "Bei einer Array Zuweisung muss 'sind' anstatt 'ist' stehen!");
		if (type == ValueType::BOOL)
			expr = boolAssignement();
		else
			expr = expression();
		if (expr != type)
			errorAtCurrent(u8"Falscher Zuweisungs Typ");
		emitBytes(setOp, arg);
	}
	else if (canAssign && match(TokenType::SIND))
	{
		if (!(type >= ValueType::INTARR && type <= ValueType::STRINGARR))
			errorAt(*previous, "'sind' darf nur zum Zuweisen von Arrays verwendet werden!");
		ValueType expr = expression();
		if (expr != type)
			errorAtCurrent(u8"Falscher Zuweisungs Typ");
		emitBytes(setOp, arg);
	}
	else if (check(TokenType::AN))
	{
		if (arrArg == -1) emitBytes(op::CONSTANT, arg);
		localArrSlot = arrArg;
	}
	else
		emitBytes(getOp, arg);

	lastEmittedType = type;
	return type;
}

ValueType Compiler::variable(bool canAssign)
{
	return namedVariable(previous->literal, canAssign);
}

ValueType Compiler::index(bool canAssign)
{
	if (!(lastEmittedType >= ValueType::INTARR && lastEmittedType <= ValueType::STRINGARR))
	{
		errorAt(*previous, u8"Du kannst nur Arrays indexieren!");
		return ValueType::NONE;
	}
	ValueType arrType = (ValueType)((int)lastEmittedType - 5);
	ValueType rhs = parsePrecedence(Precedence::CALL);

	if (rhs != ValueType::INT)
	{
		errorAtCurrent(u8"Du kannst nur ganze Zahlen zum indexieren benutzen!");
		return arrType;
	}

	if (match(TokenType::IST))
	{
		ValueType expr = ValueType::NONE;
		if (arrType == ValueType::BOOL)
			expr = boolAssignement();
		else
			expr = expression();
		if (expr != arrType)
			errorAtCurrent(u8"Falscher Zuweisungs Typ!");
		if (localArrSlot == -1)
			emitByte(op::SET_ARRAY_ELEMENT);
		else
			emitBytes(op::SET_ARRAY_ELEMENT_LOCAL, localArrSlot);
	}
	else
	{
		if (localArrSlot == -1)
			emitByte(op::GET_ARRAY_ELEMENT);
		else
			emitBytes(op::GET_ARRAY_ELEMENT_LOCAL, localArrSlot);
	}

	return arrType;
}

#pragma endregion