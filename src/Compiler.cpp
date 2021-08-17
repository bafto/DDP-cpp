#include "Compiler.h"
#include <iostream>
#include <algorithm>

#pragma warning (disable : 26812)

Compiler::Compiler(const std::string& filePath,
	std::unordered_map<std::string, Value>* globals,
	std::unordered_map<std::string, Function>* functions)
	:
	filePath(filePath),
	runtimeGlobals(globals),
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

	for (auto& pair : *runtimeGlobals)
	{
		globals.insert(std::make_pair(pair.first, pair.second.Type()));
	}
	makeNatives();

	Function mainFunction;

	ScopeUnit mainUnit(nullptr, &mainFunction);
	currentScopeUnit = &mainUnit;

	while (!match(TokenType::END))
		declaration();
	emitReturn();

	mainUnit.endUnit(currentScopeUnit);

	functions->insert(std::make_pair("", std::move(mainFunction)));

	finishCompilation();

	return !hadError;
}

void Compiler::finishCompilation()
{
	for (auto it = globals.begin(), end = globals.end(); it != end; it++)
	{
		Value val;
		switch (it->second)
		{
		case ValueType::Int: val = Value(1); break;
		case ValueType::Double: val = Value(1.0); break;
		case ValueType::Bool: val = Value(false); break;
		case ValueType::Char: val = Value((char)0); break;
		case ValueType::String: val = Value(""); break;
		case ValueType::IntArr: val = Value(std::vector<int>()); break;
		case ValueType::DoubleArr: val = Value(std::vector<double>()); break;
		case ValueType::BoolArr: val = Value(std::vector<bool>()); break;
		case ValueType::CharArr: val = Value(std::vector<char>()); break;
		case ValueType::StringArr: val = Value(std::vector<std::string>()); break;
		}

		runtimeGlobals->insert(std::make_pair(it->first, val));
	}
}

void Compiler::makeNatives()
{
	using ty = Natives::CombineableValueType;

	addNative("schreibe", ValueType::None, { ty::Any }, &Natives::schreibeNative);
	addNative("schreibeZeile", ValueType::None, { ty::Any }, &Natives::schreibeZeileNative);
	addNative("lese", ValueType::Char, {}, &Natives::leseNative);
	addNative("leseZeile", ValueType::String, {}, &Natives::leseZeileNative);

	addNative("clock", ValueType::Double, {}, &Natives::clockNative);

	addNative("zuZahl", ValueType::Int, { ty::Any }, &Natives::zuZahlNative);
	addNative("zuKommazahl", ValueType::Double, { ty::Any }, &Natives::zuKommazahlNative);
	addNative("zuBoolean", ValueType::Bool, { (ty)(ty::Int | ty::Double | ty::Bool | ty::Char | ty::String) }, &Natives::zuBooleanNative);
	addNative("zuZeichen", ValueType::Char, { (ty)(ty::Int | ty::Double | ty::Bool | ty::Char | ty::String) }, &Natives::zuZeichenNative);
	addNative("zuZeichenkette", ValueType::String, { ty::Any }, &Natives::zuZeichenketteNative);

	addNative(u8"Länge", ValueType::Int, { (ty)(ty::String | ty::IntArr | ty::DoubleArr | ty::BoolArr | ty::CharArr | ty::StringArr) }, &Natives::LaengeNative);

	addNative("Zuschneiden", ValueType::String, { ty::String, ty::Int, ty::Int }, &Natives::ZuschneidenNative);
	addNative("Spalten", ValueType::StringArr, { ty::String, (ty)(ty::String | ty::Char) }, &Natives::SpaltenNative);
	addNative("Ersetzen", ValueType::String, { ty::String, (ty)(ty::String | ty::Char), (ty)(ty::String | ty::Char) }, &Natives::ErsetzenNative);
	addNative("Entfernen", ValueType::String, { ty::String, ty::Int, ty::Int }, &Natives::EntfernenNative);
	addNative(u8"Einfügen", ValueType::String, { ty::String, ty::Int, ty::String }, &Natives::EinfügenNative);
	addNative(u8"Enthält", ValueType::Bool, { ty::String, (ty)(ty::String | ty::Char) }, &Natives::EnthältNative);
	addNative("Beschneiden", ValueType::String, { ty::String }, &Natives::BeschneidenNative);
}

void Compiler::addNative(std::string name, ValueType returnType, std::vector<Natives::CombineableValueType> args, Function::NativePtr native)
{
	Function func;
	func.returnType = returnType;
	func.native = native;
	for (int i = 0; i < args.size(); i++)
		func.args.push_back(std::make_pair("", ValueType::None));
	func.nativeArgs = std::move(args);

	functions->insert(std::make_pair(name, std::move(func)));
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
		if (infix == parseRules.at(TokenType::LEFT_PAREN).infix && expr != ValueType::Function)
		{
			error(u8"Du kannst nur Funktionen aufrufen!");
			return ValueType::None;
		}
		expr = (this->*infix)(false);
		lastEmittedType = expr;
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
		if (lhs >= ValueType::IntArr && lhs <= ValueType::StringArr)
		{
			ValueType elementType = (ValueType)((int)lhs - 5);
			if (expr != elementType && expr != lhs)
				error(u8"Du kannst Arrays nur elemente oder andere Arrays desselben Typs hinzufügen!");

			emitByte(op::ADD);
			return lhs;
		}

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
	int endJump = emitJump(op::JUMP_IF_FALSE);
	emitByte(op::POP);

	ValueType expr = parsePrecedence(Precedence::And);
	if (expr != ValueType::Bool) error(u8"Die Operanden eines 'und' Ausdrucks müssen Booleans sein!");

	patchJump(endJump);
	return expr;
}

ValueType Compiler::or_(bool canAssign)
{
	int elseJump = emitJump(op::JUMP_IF_FALSE);
	int endJump = emitJump(op::JUMP);

	patchJump(elseJump);
	emitByte(op::POP);

	ValueType expr = parsePrecedence(Precedence::Or);
	if (expr != ValueType::Bool) error(u8"Die Operanden eines 'oder' Ausdrucks müssen Booleans sein!");
	patchJump(endJump);

	return expr;
}

std::pair<int, ValueType> Compiler::getLocal(std::string name)
{
	for (ScopeUnit* unit = currentScopeUnit; unit != nullptr; unit = unit->enclosingUnit)
	{
		if (unit->locals.count(name) != 0)
			return std::make_pair(unit->identifier, unit->locals.at(name));
	}
	return std::make_pair(-1, ValueType::None);
}

ValueType Compiler::variable(bool canAssign)
{
	std::string varName = preIt->literal;
	if (functions->count(varName) == 1)
	{
		calledFuncName = varName;
		return ValueType::Function;
	}
	auto pair = getLocal(varName); //pair of unit and type
	int local = pair.first;
	ValueType type = pair.second;
	OpCode getOp, setOp;
	if (local != -1)
	{
		getOp = op::GET_LOCAL;
		setOp = op::SET_LOCAL;
	}
	else
	{
		if (globals.count(varName) == 0)
		{
			error(u8"Die globale Variable '" + varName + u8"' wurde noch nicht definiert!");
			return ValueType::None;
		}
		getOp = op::GET_GLOBAL;
		setOp = op::SET_GLOBAL;
		type = globals.at(varName);
	}

	if (canAssign && match(TokenType::IST))
	{
		ValueType expr = ValueType::None;
		if (type >= ValueType::IntArr && type <= ValueType::StringArr)
			error(u8"Bei einer Array Zuweisung muss 'sind' anstatt 'ist' stehen!");
		if (type == ValueType::Bool)
			expr = boolAssignement();
		else
			expr = expression();
		if (expr != type)
			error(u8"Falscher Zuweisungs Typ!");
		emitBytes(setOp, makeConstant(varName));
		if (setOp == op::SET_LOCAL)
			emitByte(makeConstant(local));
	}
	else if (canAssign && match(TokenType::SIND))
	{
		if (!(type >= ValueType::IntArr && type <= ValueType::StringArr))
			error(u8"Bei einer Variablen zuweisung muss 'ist' anstatt 'sind' stehen!");
		ValueType expr = expression();
		if (expr != type)
			error(u8"Falscher Zuweisungs Typ!");
		emitBytes(setOp, makeConstant(varName));
		if (setOp == op::SET_LOCAL)
			emitByte(makeConstant(local));
	}
	else if (match(TokenType::AN))
	{
		if (!(type >= ValueType::IntArr && type <= ValueType::StringArr))
			error(u8"Es können nur Arrays indexiert werden!");
		index(canAssign, varName, type, local);
	}
	else
	{
		emitBytes(getOp, makeConstant(varName));
		if (setOp == op::SET_LOCAL)
			emitByte(makeConstant(local));
	}
	lastEmittedType = type;
	return type;
}

void Compiler::index(bool canAssign, std::string arrName, ValueType type, int local)
{
	OpCode getOp = local == -1 ? op::GET_ARRAY_ELEMENT : op::GET_ARRAY_ELEMENT_LOCAL;
	OpCode setOp = local == -1 ? op::SET_ARRAY_ELEMENT : op::SET_ARRAY_ELEMENT_LOCAL;

	ValueType elementType = (ValueType)((int)type - 5);

	ValueType rhs = parsePrecedence(Precedence::Indexing);
	if (rhs != ValueType::Int)
		error(u8"Du kannst nur ganze Zahlen zum indexieren benutzen!");

	if (match(TokenType::IST))
	{
		ValueType expr = ValueType::None;
		if (elementType == ValueType::Bool)
			expr = boolAssignement();
		else
			expr = expression();
		if (expr != elementType)
			error(u8"Falscher Zuweisungs Typ!");
		emitBytes(setOp, makeConstant(arrName));
		if (setOp == op::SET_ARRAY_ELEMENT_LOCAL)
			emitByte(makeConstant(local));
	}
	else
	{
		emitBytes(getOp, makeConstant(arrName));
		if (getOp == op::GET_ARRAY_ELEMENT_LOCAL)
			emitByte(makeConstant(local));
	}
	lastEmittedType = elementType;
}

ValueType Compiler::call(bool canAssign)
{
	std::string funcName = calledFuncName;
	Function* func = &functions->at(funcName);

	int argCount = 0;
	if (currIt->type != TokenType::RIGHT_PAREN)
	{
		do
		{
			ValueType expr = expression();
			try
			{
				if (func->native != nullptr)
				{
					if (!Natives::ContainsType(func->nativeArgs.at(argCount), expr))
						error(u8"Falscher Argument Typ!");
				}
				else
				{
					if (func->args.at(argCount).second != expr)
						error(u8"Falscher Argument Typ!");
				}
			}
			catch (std::exception&)
			{
				error(u8"Zu viele Argumente beim Funktions Aufruf!");
			}
			argCount++;
		} while (match(TokenType::COMMA));
	}
	if (argCount > func->args.size())
		error(u8"Zu viele Argumente beim Funktions Aufruf!");
	else if (argCount < func->args.size())
		error(u8"Zu wenige Argumente beim Funktions Aufruf!");
	consume(TokenType::RIGHT_PAREN, u8"Es wurde eine ')' beim Funktions Aufruf erwartet!");

	emitBytes(op::CALL, makeConstant(Value(funcName)));
	lastEmittedType = func->returnType;
	return func->returnType;
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
		block();
	else if (match(TokenType::GIB))
		returnStatement();
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

void Compiler::block()
{
	ScopeUnit unit(currentScopeUnit, currentFunction());
	currentScopeUnit = &unit;

	while (currIt->type != TokenType::END && currIt->depth >= currentScopeUnit->scopeDepth)
		declaration();

	unit.endUnit(currentScopeUnit);
}

void Compiler::expressionStatement()
{
	ValueType expr = expression();
	consume(TokenType::DOT, u8"Es wrude ein '.' nach einem Ausdruck erwartet!");
	emitByte(op::POP);
}

void Compiler::addGlobal(std::string name, ValueType type)
{
	if (globals.count(name) != 0)
	{
		error(u8"Eine Variable mit dem Namen '" + name + "' existiert bereits im Globalen Bereich!");
		return;
	}

	globals.insert(std::make_pair(name, type));
}

void Compiler::addLocal(std::string name, ValueType type)
{
	if (currentScopeUnit->locals.count(name) != 0)
	{
		error(u8"Eine Variable mit dem Namen '" + name + "' existiert bereits in diesem Bereich!");
		return;
	}

	currentScopeUnit->locals.insert(std::make_pair(name, type));
}

ValueType Compiler::boolAssignement()
{
	if (match(TokenType::WAHR))
	{
		if (match(TokenType::WENN))
			return expression();
		else
			emitConstant(Value(true));
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
			emitConstant(Value(false));
	}
	else
		error("Ein Boolean muss als wahr oder falsch definiert werden!");
	return ValueType::Bool;
}

void Compiler::varDeclaration()
{
	ValueType varType = ValueType::None;
	switch (preIt->type)
	{
	case TokenType::DER:
	{
		if (!match(TokenType::BOOLEAN))
		{
			error(u8"Falscher Artikel!", currIt);
			return;
		}
		varType = ValueType::Bool;
		break;
	}
	case TokenType::DAS:
	{
		if (!match(TokenType::ZEICHEN))
		{
			error(u8"Falscher Artikel!", currIt);
			return;
		}
		varType = ValueType::Char;
		break;
	}
	case TokenType::DIE:
	{
		if (!match(TokenType::ZAHL) && !match(TokenType::ZEICHENKETTE) && !match(TokenType::KOMMAZAHL) &&
			!match(TokenType::ZAHLEN) && !match(TokenType::KOMMAZAHLEN) && !match(TokenType::ZEICHEN) && !match(TokenType::ZEICHENKETTEN) && !match(TokenType::BOOLEANS))
		{
			error(u8"Falscher Artikel!", currIt);
			return;
		}
		varType = preIt->type == TokenType::ZAHL ? ValueType::Int : ValueType::None;
		varType = preIt->type == TokenType::KOMMAZAHL ? ValueType::Double : varType;
		varType = preIt->type == TokenType::ZEICHENKETTE ? ValueType::String : varType;
		varType = preIt->type == TokenType::ZAHLEN ? ValueType::IntArr : varType;
		varType = preIt->type == TokenType::KOMMAZAHLEN ? ValueType::DoubleArr : varType;
		varType = preIt->type == TokenType::BOOLEANS ? ValueType::BoolArr : varType;
		varType = preIt->type == TokenType::ZEICHEN ? ValueType::CharArr : varType;
		varType = preIt->type == TokenType::ZEICHENKETTEN ? ValueType::StringArr : varType;
		break;
	}
	}

	consume(TokenType::IDENTIFIER, "Es wurde ein Variablen-Name erwartet!");

	std::string varName = preIt->literal;
	OpCode defineCode;
	int unit = -1;
	if (currentScopeUnit->scopeDepth == 0) //scopeDepth is zero so we are in global scope
	{
		addGlobal(varName, varType);
		defineCode = op::DEFINE_GLOBAL;
	}
	else
	{
		addLocal(varName, varType);
		defineCode = op::DEFINE_LOCAL;
		unit = currentScopeUnit->identifier;
	}

	if (match(TokenType::IST))
	{
		//if the variable is an Array you must use 'sind' instead of 'ist'
		if (varType >= ValueType::IntArr && varType <= ValueType::StringArr)
			error(u8"Beim definieren einer Variablen Gruppe sollte 'sind' verwendet werden!");

		ValueType rhs = ValueType::None;
		if (varType == ValueType::Bool)
			rhs = boolAssignement();
		else
			rhs = expression();

		if (rhs != varType)
			error(u8"Der Zuweisungs-Typ und der Variablen-Typ stimmen nicht überein!");
	}
	else if (match(TokenType::SIND))
	{
		//if the variable is not an Array you must use 'ist' instead of 'sind'
		if (varType >= ValueType::Int && varType <= ValueType::String)
			error(u8"Beim definieren einer Variable sollte 'ist' verwendet werden!");

		ValueType rhs = expression();
		if (rhs == ValueType::Int)
			consume(TokenType::STUECK, u8"Beim definieren einer leeren Variablen Gruppe wurde 'Stück' erwartet!");
		if (rhs != varType && rhs != ValueType::Int)
			error(u8"Der Zuweisungs-Typ und der Variablen-Typ stimmen nicht überein!");
	}
	else
		error(u8"Eine Variable muss immer definiert werden!");
	consume(TokenType::DOT, u8"Es fehlt ein Punkt nach einer Variablen Definition!");

	emitBytes(defineCode, makeConstant(Value(varName)));
	if (defineCode == op::DEFINE_LOCAL)
		emitByte(makeConstant(Value(unit)));
}

ValueType Compiler::tokenToValueType(TokenType type)
{
	switch (type)
	{
	case TokenType::ZAHL: return ValueType::Int;
	case TokenType::KOMMAZAHL: return ValueType::Double;
	case TokenType::BOOLEAN: return ValueType::Bool;
	case TokenType::ZEICHEN: return ValueType::Char;
	case TokenType::ZEICHENKETTE: return ValueType::String;
	case TokenType::ZAHLEN: return ValueType::IntArr;
	case TokenType::KOMMAZAHLEN: return ValueType::DoubleArr;
	case TokenType::BOOLEANS: return ValueType::BoolArr;
	case TokenType::ZEICHENKETTEN: return ValueType::StringArr;
	}
	return ValueType::None;
}

void Compiler::funDeclaration()
{
	if (currentScopeUnit->scopeDepth > 0) error(u8"Du kannst nur globale Funktionen definieren!");
	consume(TokenType::IDENTIFIER, u8"Es wurde ein Funktions-Name erwartet!");
	std::string funcName = preIt->literal;
	Function function;
	ScopeUnit unit(currentScopeUnit, &function);
	currentScopeUnit = &unit;
	function.argUnit = unit.identifier;

	consume(TokenType::LEFT_PAREN, u8"Es wurde eine '(' erwartet!");

	while (currIt->type != TokenType::RIGHT_PAREN)
	{
		do
		{
			if (currentScopeUnit->enclosingFunction->args.size() > 254) error(u8"Eine Funktion darf maximal 255 Parameter enthalten!");
			advance();
			TokenType parameterType = preIt->type;
			if (!(parameterType >= TokenType::ZAHL && parameterType <= TokenType::ZEICHENKETTEN))
				error(u8"Es wurde ein Typ spezifizierer erwartet!");
			ValueType argType = tokenToValueType(parameterType);
			consume(TokenType::IDENTIFIER, u8"Es wurde ein Parameter-Name erwartet!");
			addLocal(preIt->literal, argType);
			currentFunction()->args.push_back(std::make_pair(preIt->literal, argType));
		} while (match(TokenType::COMMA));
	}

	consume(TokenType::RIGHT_PAREN, u8"Es wurde eine ')' erwartet!");
	if (match(TokenType::VOM))
	{
		consume(TokenType::TYP, u8"Es wurde 'Typ' nach 'vom' erwartet!");
		currentFunction()->returnType = tokenToValueType(currIt->type);
		advance();
	}
	consume(TokenType::MACHT, u8"Es wurde 'macht' erwartet!");
	consume(TokenType::COLON, "Es wurde ein ':' erwartet!");

	functions->insert(std::make_pair(funcName, function));

	while (currIt->type != TokenType::END && currIt->depth >= currentScopeUnit->scopeDepth)
		declaration();

	if (!function.returned)
	{
		emitReturn();
		if (function.returnType != ValueType::None)
			error(u8"Es fehlt eine Rückgabe Anweisung!");
	}

	unit.endUnit(currentScopeUnit);

	(*functions)[funcName] = std::move(function);
}

void Compiler::returnStatement()
{
	if (currentScopeUnit->scopeDepth == 0)
		error("In der Hauptfunktion ist keine Rückgabe Anweisung erlaubt!");
	if (match(TokenType::ZURUECK))
	{
		emitReturn();
		if(currentScopeUnit->scopeDepth == 1) currentFunction()->returned = true;
		if (currentFunction()->returnType != ValueType::None) 
			error(u8"Diese funktion sollte nichts zurückgeben!");
	}
	else
	{
		ValueType expr = expression();
		if (expr != currentFunction()->returnType)
			error(u8"Der Rückgabe Typ stimmt nicht mit dem Rückgabe Typ der Funktion überein!");
		consume(TokenType::ZURUECK, u8"Es wurde 'zurück' erwartet!");
		emitReturn();
		if (currentScopeUnit->scopeDepth == 1) currentFunction()->returned = true;
	}
	consume(TokenType::DOT, u8"Es wurde '.' erwartet!");
}

void Compiler::patchJump(int offset)
{
	int jump = static_cast<int>(currentChunk()->bytes.size() - offset - 2);

	if (jump > UINT16_MAX)
		error(u8"Zu viele bytes zu überspringen!");

	currentChunk()->bytes[offset] = (jump >> 8) & 0xff;
	currentChunk()->bytes[offset + 1] = jump & 0xff;
}

void Compiler::ifStatement()
{
	ValueType expr = expression();
	if (expr != ValueType::Bool)
		error(u8"Die Bedingung einer 'wenn' Anweisung muss einen Boolschen Wert ergeben!");
	consume(TokenType::COMMA, u8"Nach der Bedingung einer 'wenn' Anweisung wurde ein ',' erwartet!");
	consume(TokenType::DANN, u8"Nach dem ',' einer 'wenn' Anweisung wurde ein 'dann' erwartet!");

	int thenJump = emitJump(op::JUMP_IF_FALSE);
	emitByte(op::POP);
	statement();

	int elseJump = emitJump(op::JUMP);
	emitByte(op::POP);

	patchJump(thenJump);

	if (match(TokenType::SONST))
		statement();
	else if (check(TokenType::WENN) && checkNext(TokenType::ABER))
	{
		advance();
		advance();
		ifStatement();
	}

	patchJump(elseJump);
}

void Compiler::whileStatement()
{
	int loopStart = static_cast<int>(currentChunk()->bytes.size());
	ValueType expr = expression();
	if (expr != ValueType::Bool) error(u8"Die Bedingung einer 'solange' Anweisung muss ein Boolean sein!");

	consume(TokenType::COMMA, u8"Nach der Bedingung einer 'solange' Anweisung wurde ein ',' erwartet!");
	consume(TokenType::MACHE, u8"Nach dem ',' einer 'solange' Anweisung wurde ein 'mache' erwartet!");

	int exitJump = emitJump(op::JUMP_IF_FALSE);
	emitByte(op::POP);
	statement();
	emitLoop(loopStart);

	patchJump(exitJump);
	emitByte(op::POP);
}

void Compiler::forStatement()
{
	ScopeUnit unit(currentScopeUnit, currentFunction());
	currentScopeUnit = &unit;

	consume(TokenType::JEDE, u8"Nach einem 'für' sollte ein 'jede' stehen!");
	consume(TokenType::ZAHL, u8"Eine fuer Anweisung kann nur durch Zahlen iterieren!");

	consume(TokenType::IDENTIFIER, u8"Es wurde ein Variablen-Name erwartet!");
	std::string localName = preIt->literal;
	int localNameConstant = makeConstant(localName);
	uint8_t unitConstant = makeConstant(currentScopeUnit->identifier);
	addLocal(localName, ValueType::Int);
	consume(TokenType::VON, u8"Es wurde ein 'von' erwartet!");

	ValueType expr = expression();
	if (expr != ValueType::Int) error(u8"Eine für Anweisung kann nur durch Zahlen iterieren!");

	emitBytes(op::SET_LOCAL, localNameConstant);
	emitByte(unitConstant);
	emitByte(op::POP);

	consume(TokenType::BIS, u8"Es wurde ein 'bis' erwartet!");

	emitByte(op::FORPREP);
	int conditionLoop = static_cast<int>(currentChunk()->bytes.size());
	
	emitBytes(op::GET_LOCAL, localNameConstant);
	emitByte(unitConstant);

	expr = expression();
	if (expr != ValueType::Int) error(u8"Eine für Anweisung kann nur durch Zahlen iterieren!");

	emitBytes(op::GREATER, (uint8_t)op::NOT);
	emitByte(op::FORDONE);

	int exitJump = emitJump(op::JUMP_IF_FALSE);
	emitByte(op::POP);

	int loopJump = emitJump(op::JUMP);

	int loopStart = static_cast<int>(currentChunk()->bytes.size());

	if (match(TokenType::MIT))
	{
		consume(TokenType::SCHRITTGROESSE, u8"Nach 'mit' in einer für Anweisung wird 'schrittgröße' erwartet!");
		expr = expression();
		if (expr != ValueType::Int) error(u8"Eine für Anweisung kann nur durch Zahlen iterieren!");
		emitBytes(op::GET_LOCAL, localNameConstant);
		emitByte(unitConstant);
		emitByte(op::ADD);

		emitBytes(op::SET_LOCAL, localNameConstant);
		emitByte(unitConstant);
		emitByte(op::POP);

		emitLoop(conditionLoop);
	}
	else
	{
		emitConstant(Value(1));
		emitBytes(op::GET_LOCAL, localNameConstant);
		emitByte(unitConstant);
		emitByte(op::ADD);
		emitBytes(op::SET_LOCAL, localNameConstant);
		emitByte(unitConstant);
		emitByte(op::POP);

		emitLoop(conditionLoop);
	}

	consume(TokenType::COMMA, u8"Es wurde ein ',' erwartet!");
	consume(TokenType::MACHE, u8"Es wurde ein 'mache' erwartet!");
	consume(TokenType::COLON, u8"Nach einer für Anweisung sollte ein neuer Bereich beginnen!");

	patchJump(loopJump);

	while (currIt->type != TokenType::END && currIt->depth >= currentScopeUnit->scopeDepth)
		declaration();

	emitLoop(loopStart);

	patchJump(exitJump);
	emitByte(op::POP);

	unit.endUnit(currentScopeUnit);
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

Compiler::ScopeUnit::ScopeUnit(ScopeUnit* enclosingUnit, Function* enclosingFunction)
	:
	identifier(count++),
	enclosingUnit(enclosingUnit),
	enclosingFunction(enclosingFunction),
	scopeDepth(enclosingUnit == nullptr ? 0 : enclosingUnit->scopeDepth + 1)
{}

Compiler::ScopeUnit::~ScopeUnit()
{}

void Compiler::ScopeUnit::endUnit(ScopeUnit*& currentScopeUnit)
{
	std::unordered_map<std::string, Value> temp;
	for (auto it = locals.begin(), end = locals.end(); it != end; it++)
	{
		Value val;
		switch (it->second)
		{
		case ValueType::Int: val = Value(1); break;
		case ValueType::Double: val = Value(1.0); break;
		case ValueType::Bool: val = Value(false); break;
		case ValueType::Char: val = Value((char)0); break;
		case ValueType::String: val = Value(""); break;
		case ValueType::IntArr: val = Value(std::vector<int>()); break;
		case ValueType::DoubleArr: val = Value(std::vector<double>()); break;
		case ValueType::BoolArr: val = Value(std::vector<bool>()); break;
		case ValueType::CharArr: val = Value(std::vector<char>()); break;
		case ValueType::StringArr: val = Value(std::vector<std::string>()); break;
		}

		temp.insert(std::make_pair(it->first, val));
	}
	enclosingFunction->locals.insert(std::make_pair(identifier, std::move(temp)));

	currentScopeUnit = enclosingUnit;
}

#pragma endregion