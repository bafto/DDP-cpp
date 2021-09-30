#include "Compiler.h"
#include <iostream>
#include <algorithm>

#pragma warning (disable : 26812)

Compiler::Compiler(const std::string& filePath,
	std::unordered_map<std::string, Value>* globals,
	std::unordered_map<std::string, Function>* functions, 
	std::unordered_map<std::string, Value::Struct>* structs)
	:
	filePath(filePath),
	runtimeGlobals(globals),
	functions(functions),
	runtimeStructs(structs),
	hadError(false),
	panicMode(false),
	currentScopeUnit(nullptr),
	lastEmittedType(ValueType(Type::None))
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
		globals.insert(std::make_pair(pair.first, ValueType(pair.second.type())));
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
		val = GetDefaultValue(it->second);

		runtimeGlobals->insert(std::make_pair(it->first, std::move(val)));
	}
	for (auto it = structs.begin(), end = structs.end(); it != end; it++)
	{
		std::unordered_map<std::string, Value> fields;
		for (auto itt = it->second.begin(), endd = it->second.end(); itt != endd; itt++)
		{
			fields.insert(std::make_pair(itt->first, GetDefaultValue(itt->second)));
		}
		runtimeStructs->insert(std::make_pair(it->first, Value::Struct{ std::move(fields), it->first }));
	}
}

Value Compiler::GetDefaultValue(ValueType type)
{
	switch (type.type)
	{
	case Type::Int: return Value(0);
	case Type::Double: return Value(0.0);
	case Type::Bool: return Value(false);
	case Type::Char: return Value((short)0);
	case Type::String: return Value("");
	case Type::Struct: return Value(Value::Struct{ std::unordered_map<std::string, Value>(), type.structIdentifier });
	case Type::IntArr: return Value(std::vector<int>());
	case Type::DoubleArr: return Value(std::vector<double>());
	case Type::BoolArr: return Value(std::vector<bool>());
	case Type::CharArr: return Value(std::vector<short>());
	case Type::StringArr: return Value(std::vector<std::string>());
	case Type::StructArr: return Value(std::vector<Value::Struct>());
	}
}

void Compiler::makeNatives()
{
	using ty = Natives::CombineableValueType;

	addNative("schreibe", Type::None, { ty::Any }, &Natives::schreibeNative);
	addNative("schreibeZeile", Type::None, { ty::Any }, &Natives::schreibeZeileNative);
	addNative("lese", Type::Char, {}, &Natives::leseNative);
	addNative("leseZeile", Type::String, {}, &Natives::leseZeileNative);

	addNative("existiertDatei", Type::Bool, { ty::String }, &Natives::existiertDateiNative);
	addNative("leseDatei", Type::String, { ty::String }, &Natives::leseDateiNative);
	addNative("schreibeDatei", Type::None, { ty::String, ty::Any }, &Natives::schreibeDateiNative);
	addNative("bearbeiteDatei", Type::None, { ty::String, ty::Any }, &Natives::bearbeiteDateiNative);

	addNative("leseBytes", Type::IntArr, { ty::String }, &Natives::leseBytesNative);
	addNative("schreibeBytes", Type::None, { ty::String, ty::IntArr }, &Natives::schreibeBytesNative);
	addNative("bearbeiteBytes", Type::None, { ty::String, ty::IntArr }, &Natives::bearbeiteBytesNative);

	addNative("clock", Type::Double, {}, &Natives::clockNative);
	addNative("warte", Type::None, { ty::Double }, &Natives::warteNative);

	addNative("zuZahl", Type::Int, { (ty)(ty::Int | ty::Double | ty::Bool | ty::Char | ty::String) }, &Natives::zuZahlNative);
	addNative("zuKommazahl", Type::Double, { (ty)(ty::Int | ty::Double | ty::Bool | ty::Char | ty::String) }, &Natives::zuKommazahlNative);
	addNative("zuBoolean", Type::Bool, { (ty)(ty::Int | ty::Double | ty::Bool | ty::Char | ty::String) }, &Natives::zuBooleanNative);
	addNative("zuBuchstabe", Type::Char, { (ty)(ty::Int | ty::Double | ty::Bool | ty::Char | ty::String) }, &Natives::zuBuchstabeNative);
	addNative("zuText", Type::String, { ty::Any }, &Natives::zuTextNative);

	addNative(u8"Länge", Type::Int, { (ty)(ty::String | ty::IntArr | ty::DoubleArr | ty::BoolArr | ty::CharArr | ty::StringArr) }, &Natives::LaengeNative);

	addNative("Zuschneiden", Type::String, { ty::String, ty::Int, ty::Int }, &Natives::ZuschneidenNative);
	addNative("Spalten", Type::StringArr, { ty::String, (ty)(ty::String | ty::Char) }, &Natives::SpaltenNative);
	addNative("Ersetzen", Type::String, { ty::String, (ty)(ty::String | ty::Char), (ty)(ty::String | ty::Char) }, &Natives::ErsetzenNative);
	addNative("Entfernen", Type::String, { ty::String, ty::Int, ty::Int }, &Natives::EntfernenNative);
	addNative(u8"Einfügen", Type::String, { ty::String, ty::Int, ty::String }, &Natives::EinfügenNative);
	addNative(u8"Enthält", Type::Bool, { ty::String, (ty)(ty::String | ty::Char) }, &Natives::EnthältNative);
	addNative("Beschneiden", Type::String, { ty::String }, &Natives::BeschneidenNative);

	addNative("Max", Type::Double, { (ty)(ty::Double | ty::Int), (ty)(ty::Double | ty::Int) }, &Natives::Max);
	addNative("Min", Type::Double, { (ty)(ty::Double | ty::Int), (ty)(ty::Double | ty::Int) }, &Natives::Min);
	addNative("Clamp", Type::Double, { (ty)(ty::Double | ty::Int), (ty)(ty::Double | ty::Int), (ty)(ty::Double | ty::Int) }, &Natives::Clamp);
	addNative("Trunkiert", Type::Double, { ty::Double }, &Natives::Trunkiert);
	addNative("Rund", Type::Double, { ty::Double }, &Natives::Rund);
	addNative("Decke", Type::Double, { ty::Double }, &Natives::Decke);
	addNative("Boden", Type::Double, { ty::Double }, &Natives::Boden);

	addNative(u8"ZufälligeZahl", Type::Int, { ty::Int, ty::Int }, &Natives::ZufaelligeZahlNative);
	addNative(u8"ZufaelligeZahl", Type::Int, { ty::Int, ty::Int }, &Natives::ZufaelligeZahlNative);
	addNative(u8"ZufälligeKommazahl", Type::Double, { ty::Double, ty::Double }, &Natives::ZufaelligeKommazahlNative);
	addNative(u8"ZufaelligeKommazahl", Type::Double, { ty::Double, ty::Double }, &Natives::ZufaelligeKommazahlNative);
}

void Compiler::addNative(std::string name, Type returnType, std::vector<Natives::CombineableValueType> args, Function::NativePtr native)
{
	Function func;
	func.returnType = ValueType(returnType);
	func.native = native;
	for (int i = 0; i < args.size(); i++)
		func.args.push_back(std::make_pair("", ValueType(Type::None)));
	func.nativeArgs = std::move(args);

	functions->insert(std::make_pair(name, std::move(func)));
}

uint8_t Compiler::makeConstant(Value value)
{
	lastEmittedType = ValueType(value.type());
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
		return ValueType(Type::None);
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
			return ValueType(Type::None);
		}
		if (infix == parseRules.at(TokenType::LEFT_PAREN).infix && expr.type != Type::Function)
		{
			error(u8"Du kannst nur Funktionen aufrufen!");
			return ValueType(Type::None);
		}
		else if (infix == parseRules.at(TokenType::LEFT_CURLY).infix && expr.type != Type::Struct)
		{
			error(u8"Du kannst nur den Konstruktor von Strukturen aufrufen!");
			return ValueType(Type::None);
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
	return ValueType(Type::Double);
}

ValueType Compiler::inumber(bool canAssign)
{
	emitConstant(Value(std::stoi(preIt->literal)));
	return ValueType(Type::Int);
}

ValueType Compiler::string(bool canAssign)
{
	emitConstant(Value(std::string(preIt->literal.begin() + 1, preIt->literal.end() - 1))); //remove the leading and trailing "
	return ValueType(Type::String);
}

ValueType Compiler::character(bool canAssign)
{
	if (preIt->literal.length() == 3)
		emitConstant(Value((short)preIt->literal[1])); //remove the leading and trailing '
	else if (preIt->literal.length() == 4)
	{
		char a = preIt->literal[1];
		char b = preIt->literal[2];
		emitConstant(Value((short)((((short)a) << 8) | (0x00ff & b))));
	}
	else
		error("Zu langes Buchstaben literal");
	return ValueType(Type::Char);
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
		{
			error(u8"Die Typen innerhalb des Array Literals stimmen nicht überein!");
			break;
		}
		i++;
	}
	emitBytes(op::ARRAY, makeConstant(Value(i)));
	lastEmittedType = arrType;
	lastEmittedType.type = ((Type)((int)arrType.type + 6));
	emitByte((uint8_t)lastEmittedType.type);
	return lastEmittedType;
}

ValueType Compiler::structLiteral(bool canAssign)
{
	if (match(TokenType::RIGHT_CURLY))
	{
		emitBytes(op::STRUCT, makeConstant(calledFuncName));
		emitByte(makeConstant(0));
		return ValueType(calledFuncName, Type::Struct);
	}

	int i = 1;
	while(true)
	{
		consume(TokenType::IDENTIFIER, "Es wurde ein Feld-Name erwartet!");
		std::string fieldName = preIt->literal;
		if (fieldName == calledFuncName)
			error(u8"Eine Struktur kann sich nicht selbst als Feld haben!");
		emitConstant(Value(fieldName));
		consume(TokenType::COLON, "Es wurde ein ':' erwartet!");
		ValueType rhs = expression();
		if (structs.at(calledFuncName).count(fieldName) == 0)
			error(u8"Die Struktur '" + calledFuncName + u8"' enthält kein Feld mit dem Namen '" + fieldName + u8"'!");
		if (rhs != structs.at(calledFuncName).at(fieldName))
			error(u8"Der Typ stimmt nicht mit dem Typ des Feldes überein!");
		if (match(TokenType::RIGHT_CURLY))
			break;
		consume(TokenType::SEMICOLON, "Unfertiges Struktur Literal!");
		i++;
	}

	emitBytes(op::STRUCT, makeConstant(calledFuncName));
	emitByte(makeConstant(i));
	return ValueType(calledFuncName, Type::Struct);
}

ValueType Compiler::Literal(bool canAssign)
{
	switch (preIt->type)
	{
	case TokenType::WAHR: emitConstant(Value(true)); return Type::Bool;
	case TokenType::FALSCH: emitConstant(Value(false)); return Type::Bool;
	case TokenType::E: emitConstant(Value(2.71828182845904523536)); return Type::Double;
	case TokenType::PI: emitConstant(Value(3.14159265358979323846)); return Type::Double;
	case TokenType::PHI: emitConstant(Value(1.61803)); return Type::Double;
	case TokenType::TAU: emitConstant(Value(6.283185307179586)); return Type::Double;
	default: break;
	}

	return Type::None;
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
		if (expr.type != Type::Int && expr.type != Type::Double)
			error(u8"Man kann nur Zahlen negieren!");
		emitByte(op::NEGATE);
		return expr;
	case TokenType::NICHT:
		if (expr.type != Type::Bool)
			error(u8"Man kann nur Booleans mit 'nicht' negieren!");
		emitByte(op::NOT);
		return expr;
	case TokenType::LN:
		if (expr.type != Type::Int && expr.type != Type::Double)
			error(u8"Man kann nur aus Zahlen den ln berechnen!");
		emitByte(op::LN);
		return expr;
	case TokenType::BETRAG:
		if (expr.type != Type::Int && expr.type != Type::Double)
			error(u8"Man kann nur den Betrag von Zahlen berechnen!");
		emitByte(op::BETRAG);
		return expr;
	case TokenType::SIN:
		if (expr.type != Type::Double)
			error(u8"Man kann nur den Sinus von Kommazahlen berechnen!");
		emitByte(op::SIN);
		return expr;
	case TokenType::COS:
		if (expr.type != Type::Double)
			error(u8"Man kann nur den Kosinus von Kommazahlen berechnen!");
		emitByte(op::COS);
		return expr;
	case TokenType::TAN:
		if (expr.type != Type::Double)
			error(u8"Man kann nur den Tangens von Kommazahlen berechnen!");
		emitByte(op::TAN);
		return expr;
	case TokenType::ASIN:
		if (expr.type != Type::Double)
			error(u8"Man kann nur den Arkussinus von Kommazahlen berechnen!");
		emitByte(op::ASIN);
		return expr;
	case TokenType::ACOS:
		if (expr.type != Type::Double)
			error(u8"Man kann nur den Arkuskosinus von Kommazahlen berechnen!");
		emitByte(op::ACOS);
		return expr;
	case TokenType::ATAN:
		if (expr.type != Type::Double)
			error(u8"Man kann nur den Arkustangens von Kommazahlen berechnen!");
		emitByte(op::ATAN);
		return expr;
	case TokenType::SINH:
		if (expr.type != Type::Double)
			error(u8"Man kann nur den Hyperbelsinus von Kommazahlen berechnen!");
		emitByte(op::SINH);
		return expr;
	case TokenType::COSH:
		if (expr.type != Type::Double)
			error(u8"Man kann nur den Hyperbelkosinus von Kommazahlen berechnen!");
		emitByte(op::COSH);
		return expr;
	case TokenType::TANH:
		if (expr.type != Type::Double)
			error(u8"Man kann nur den Hyperbeltangens von Kommazahlen berechnen!");
		emitByte(op::TANH);
		return expr;
	case TokenType::LOGISCHNICHT:
		if (expr.type != Type::Int)
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
		if (isArr(lhs))
		{
			ValueType elementType = (Type)((int)lhs.type - 6);
			if (expr != elementType && expr != lhs)
				error(u8"Du kannst Arrays nur elemente oder andere Arrays desselben Typs hinzufügen!");

			emitByte(op::ADD);
			return lhs;
		}

		if (lhs.type == Type::Bool || expr.type == Type::Bool)
			error(u8"Ein Boolean kann kein Operand in einer addition sein");
		emitByte(op::ADD);
		switch (lhs.type)
		{
		case Type::Int:
			switch (expr.type)
			{
			case Type::Int: return Type::Int;
			case Type::Double: return Type::Double;
			case Type::Char: return Type::Int;
			case Type::String: return Type::String;
			}
			break;
		case Type::Double:
			switch (expr.type)
			{
			case Type::Int: return Type::Double;
			case Type::Double: return Type::Double;
			case Type::Char: return Type::Int;
			case Type::String: return Type::String;
			}
			break;
		case Type::Char:
			switch (expr.type)
			{
			case Type::Int: return Type::Int;
			case Type::Double: return Type::Int;
			case Type::Char: return Type::String;
			case Type::String: return Type::String;
			}
			break;
		case Type::String: return Type::String;
		default:
			break;
		}
		break;
	}
	case TokenType::MINUS:
	{
		if ((lhs.type != Type::Int && lhs.type != Type::Double) || (expr.type != Type::Int && expr.type != Type::Double))
			error(u8"Es können nur Zahlen von einander subtrahiert werden!");
		emitByte(op::SUBTRACT);
		if (lhs.type == Type::Int && expr.type == Type::Int) return Type::Int;
		else return Type::Double;
	}
	case TokenType::MAL:
	{
		if ((lhs.type != Type::Int && lhs.type != Type::Double) || (expr.type != Type::Int && expr.type != Type::Double))
			error(u8"Es können nur Zahlen miteinander multipliziert werden!");
		emitByte(op::MULTIPLY);
		if (lhs.type == Type::Int && expr.type == Type::Int) return Type::Int;
		else return Type::Double;
	}
	case TokenType::DURCH:
	{
		if ((lhs.type != Type::Int && lhs.type != Type::Double) || (expr.type != Type::Int && expr.type != Type::Double))
			error(u8"Es können nur Zahlen durcheinander dividiert werden!");
		emitByte(op::DIVIDE);
		if (lhs.type == Type::Int && expr.type == Type::Int) return Type::Int;
		else return Type::Double;
		break;
	}
	case TokenType::MODULO:
	{
		if (lhs.type != Type::Int || expr.type != Type::Int) error(u8"Es kann nur der Modulo aus zwei Zahlen berechnet werden!");
		emitByte(op::MODULO);
		return Type::Int;
	}
	case TokenType::HOCH:
	{
		if ((lhs.type != Type::Int && lhs.type != Type::Double) || (expr.type != Type::Int && expr.type != Type::Double))
			error(u8"Es kann nur der Exponent aus Zahlen berechnet werden!");
		emitByte(op::EXPONENT);
		if (lhs.type == Type::Int && expr.type == Type::Int) return Type::Int;
		else return Type::Double;
	}
	case TokenType::WURZEL:
	{
		if (lhs.type != Type::Int || expr.type != Type::Int)
			error(u8"Es kann nur die Wurzel aus ganzen Zahlen berechnet werden!");
		emitByte(op::ROOT);
		return Type::Double;
	}
	case TokenType::UM:
	{
		if (lhs.type != Type::Int || expr.type != Type::Int) error(u8"Es können nur die bits von Zahlen verschoben werden!");
		consume(TokenType::BIT, u8"Es fehlt 'bit' nach 'um'!");
		consume(TokenType::NACH, u8"Es fehlt 'nach' nach 'bit'!");
		if (match(TokenType::RECHTS)) emitByte(op::RIGHTBITSHIFT);
		else if (match(TokenType::LINKS)) emitByte(op::LEFTBITSHIFT);
		else error(u8"Es muss entweder 'links' oder 'rechts' nach beim Verschieben von Bits angegeben sein!");
		consume(TokenType::VERSCHOBEN, u8"Es wurde 'verschoben' erwartet!");
		return Type::Int;
	}
	case TokenType::GROESSER:
	{
		if ((lhs.type != Type::Int && lhs.type != Type::Double) || (expr.type != Type::Int && expr.type != Type::Double))
			error(u8"Es können nur Zahlen mit dem Operator 'größer als' verglichen werden!");
		emitByte(op::GREATER);
		consume(TokenType::IST, u8"Nach 'größer als' fehlt 'ist'!");
		return Type::Bool;
	}
	case TokenType::GROESSERODER:
	{
		if ((lhs.type != Type::Int && lhs.type != Type::Double) || (expr.type != Type::Int && expr.type != Type::Double))
			error(u8"Es können nur Zahlen mit dem Operator 'größer als, oder' verglichen werden!");
		emitByte(op::GREATEREQUAL);
		consume(TokenType::IST, u8"Nach 'gr��er als, oder' fehlt 'ist'!");
		return Type::Bool;
	}
	case TokenType::KLEINER:
	{
		if ((lhs.type != Type::Int && lhs.type != Type::Double) || (expr.type != Type::Int && expr.type != Type::Double))
			error(u8"Es können nur Zahlen mit dem Operator 'kleiner als' verglichen werden!");
		emitByte(op::LESS);
		consume(TokenType::IST, u8"Nach 'kleiner als' fehlt 'ist'!");
		return Type::Bool;
	}
	case TokenType::KLEINERODER:
	{
		if ((lhs.type != Type::Int && lhs.type != Type::Double) || (expr.type != Type::Int && expr.type != Type::Double))
			error(u8"Es können nur Zahlen mit dem Operator 'kleiner als, oder' verglichen werden!");
		emitByte(op::LESSEQUAL);
		consume(TokenType::IST, u8"Nach 'kleiner als, oder' fehlt 'ist'!");
		return Type::Bool;
	}
	case TokenType::GLEICH:
	{
		if ((lhs.type != Type::Int && lhs.type != Type::Double) && (expr.type == Type::Int || expr.type == Type::Double) ||
			(expr.type != Type::Int && expr.type != Type::Double) && (lhs.type == Type::Int || lhs.type == Type::Double) ||
			(lhs.type == Type::Char && expr.type != Type::Char) ||
			(lhs.type == Type::Bool && expr.type != Type::Bool) ||
			(lhs.type == Type::String && expr.type != Type::String))
			error(u8"Es können nur Zahlen mit dem Operator 'größer als' verglichen werden!");
		emitByte(op::EQUAL);
		consume(TokenType::IST, u8"Nach 'gleich' fehlt 'ist'!");
		return Type::Bool;
	}
	case TokenType::UNGLEICH:
	{
		if ((lhs.type != Type::Int && lhs.type != Type::Double) && (expr.type == Type::Int || expr.type == Type::Double) ||
			(expr.type != Type::Int && expr.type != Type::Double) && (lhs.type == Type::Int || lhs.type == Type::Double) ||
			(lhs.type == Type::Char && expr.type != Type::Char) ||
			(lhs.type == Type::Bool && expr.type != Type::Bool) ||
			(lhs.type == Type::String && expr.type != Type::String))
			error(u8"Es können nur Zahlen mit dem Operator 'größer als' verglichen werden!");
		emitByte(op::UNEQUAL);
		consume(TokenType::IST, u8"Nach 'ungleich' fehlt 'ist'!");
		return Type::Bool;
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
	if (lhs.type != Type::Int)
		error(u8"Operanden für logische Operatoren müssen Zahlen sein!");

	TokenType operatorType = currIt->type;
	advance();
	ValueType rhs = parsePrecedence(Precedence::Bitwise);
	if (rhs.type != Type::Int)
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

	return Type::Int;
}

ValueType Compiler::and_(bool canAssign)
{
	int endJump = emitJump(op::JUMP_IF_FALSE);
	emitByte(op::POP);

	ValueType expr = parsePrecedence(Precedence::And);
	if (expr.type != Type::Bool) error(u8"Die Operanden eines 'und' Ausdrucks müssen Booleans sein!");

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
	if (expr.type != Type::Bool) error(u8"Die Operanden eines 'oder' Ausdrucks müssen Booleans sein!");
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
	return std::make_pair(-1, Type::None);
}

ValueType Compiler::variable(bool canAssign)
{
	std::string varName = preIt->literal;
	if (functions->count(varName) == 1)
	{
		calledFuncName = varName;
		return Type::Function;
	}
	else if (structs.count(varName) == 1)
	{
		calledFuncName = varName;
		return Type::Struct;
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
			return Type::None;
		}
		getOp = op::GET_GLOBAL;
		setOp = op::SET_GLOBAL;
		type = globals.at(varName);
	}

	if (canAssign && match(TokenType::IST))
	{
		ValueType expr = Type::None;
		if (isArr(type))
			error(u8"Bei einer Array Zuweisung muss 'sind' anstatt 'ist' stehen!");
		if (type.type == Type::Bool)
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
		if (!isArr(type))
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
		if (!isArr(type))
			error(u8"Es können nur Arrays indexiert werden!");
		index(canAssign, varName, type, local);
		type = lastEmittedType;
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

	ValueType elementType = (Type)((int)type.type - 6);

	ValueType rhs = parsePrecedence(Precedence::Indexing);
	if (rhs.type != Type::Int)
		error(u8"Du kannst nur ganze Zahlen zum indexieren benutzen!");

	if (match(TokenType::IST))
	{
		ValueType expr = Type::None;
		if (elementType.type == Type::Bool)
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
	if (match(TokenType::DER) || match(TokenType::DIE)/* || match(TokenType::DAS) */)
	{
		if (preIt->type == TokenType::DIE && match(TokenType::FUNKTION))
			funDeclaration();
		else if (preIt->type == TokenType::DIE && match(TokenType::STRUKTUR))
			structDeclaration();
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
		//case TokenType::DAS:
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
	return Type::Bool;
}

void Compiler::varDeclaration()
{
	ValueType varType = Type::None;
	switch (preIt->type)
	{
	case TokenType::DER:
	{
		if (!match(TokenType::BOOLEAN) && !match(TokenType::BUCHSTABE) && !match(TokenType::TEXT))
		{
			error(u8"Falscher Artikel!", currIt);
			return;
		}
		varType = tokenToValueType(preIt->type);
		break;
	}
	case TokenType::DIE:
	{
		if (match(TokenType::IDENTIFIER))
			varType.structIdentifier = preIt->literal;

		if (!match(TokenType::ZAHL) && !match(TokenType::KOMMAZAHL) &&
			!match(TokenType::ZAHLEN) && !match(TokenType::KOMMAZAHLEN) &&
			!match(TokenType::BUCHSTABEN) && !match(TokenType::TEXTE) &&
			!match(TokenType::BOOLEANS) && !match(TokenType::STRUKTUR) &&
			!match(TokenType::STRUKTUREN))
		{
			error(u8"Falscher Artikel!", currIt);
			return;
		}
		varType.type = tokenToValueType(preIt->type).type;
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
		if (isArr(varType))
			error(u8"Beim definieren einer Variablen Gruppe sollte 'sind' verwendet werden!");

		ValueType rhs = Type::None;
		if (varType.type == Type::Bool)
			rhs = boolAssignement();
		else
			rhs = expression();

		if (rhs != varType)
			error(u8"Der Zuweisungs-Typ und der Variablen-Typ stimmen nicht überein!");
	}
	else if (match(TokenType::SIND))
	{
		//if the variable is not an Array you must use 'ist' instead of 'sind'
		if (!isArr(varType))
			error(u8"Beim definieren einer Variable sollte 'ist' verwendet werden!");

		ValueType rhs = expression();
		if (rhs.type == Type::Int)
			consume(TokenType::STUECK, u8"Beim definieren einer leeren Variablen Gruppe wurde 'Stück' erwartet!");
		if (rhs != varType && rhs.type != Type::Int)
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
	case TokenType::ZAHL: return Type::Int;
	case TokenType::KOMMAZAHL: return Type::Double;
	case TokenType::BOOLEAN: return Type::Bool;
	case TokenType::BUCHSTABE: return Type::Char;
	case TokenType::TEXT: return Type::String;
	case TokenType::ZAHLEN: return Type::IntArr;
	case TokenType::KOMMAZAHLEN: return Type::DoubleArr;
	case TokenType::BOOLEANS: return Type::BoolArr;
	case TokenType::BUCHSTABEN: return Type::CharArr;
	case TokenType::TEXTE: return Type::StringArr;
	case TokenType::STRUKTUR: return Type::Struct;
	case TokenType::STRUKTUREN: return Type::StructArr;
	}
	return Type::None;
}

void Compiler::funDeclaration()
{
	if (currentScopeUnit->scopeDepth > 0) error(u8"Du kannst nur globale Funktionen definieren!");
	consume(TokenType::IDENTIFIER, u8"Es wurde ein Funktions-Name erwartet!");
	std::string funcName = preIt->literal;
	if (functions->count(funcName) == 1)
		error(u8"Eine Funktion mit diesem Namen existiert bereits!");
	else if (structs.count(funcName) == 1)
		error(u8"Diese Funktion würde den Konstruktor einer Struktur überschreiben!");
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
			ValueType argType = Type::None;
			if (preIt->type == TokenType::IDENTIFIER)
			{
				argType.structIdentifier = preIt->literal;
				if (!match(TokenType::STRUKTUR) && !match(TokenType::STRUKTUREN))
					error(u8"Es wurde ein Typ spezifizierer erwartet!");
				parameterType = preIt->type;
			}
			if (!(parameterType >= TokenType::ZAHL && parameterType <= TokenType::STRUKTUREN))
				error(u8"Es wurde ein Typ spezifizierer erwartet!");
			argType.type = tokenToValueType(parameterType).type;
			consume(TokenType::IDENTIFIER, u8"Es wurde ein Parameter-Name erwartet!");
			addLocal(preIt->literal, argType);
			currentFunction()->args.push_back(std::make_pair(preIt->literal, argType));
		} while (match(TokenType::COMMA));
	}

	consume(TokenType::RIGHT_PAREN, u8"Es wurde eine ')' erwartet!");
	if (match(TokenType::VOM))
	{
		consume(TokenType::TYP, u8"Es wurde 'Typ' nach 'vom' erwartet!");
		if (match(TokenType::IDENTIFIER))
		{
			currentFunction()->returnType.structIdentifier = preIt->literal;
		}
		currentFunction()->returnType.type = tokenToValueType(currIt->type).type;
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
		if (function.returnType.type != Type::None)
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
		if (currentFunction()->returnType.type != Type::None) 
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

void Compiler::structDeclaration()
{
	if (currentScopeUnit->scopeDepth > 0) error(u8"Du kannst Strukturen nur im globalen Bereich definieren!");
	consume(TokenType::IDENTIFIER, u8"Es wurde ein Struktur-Name erwartet!");
	std::string structName = preIt->literal;
	if (structs.count(structName) != 0)
		error("Diese Struktur existiert bereits!");
	else if (functions->count(structName) != 0)
		error("Es gibt bereits eine Funktion mit dem Namen der Struktur!");
	std::unordered_map<std::string, ValueType> stru;

	consume(TokenType::BESCHREIBT, u8"Es wurde 'beschreibt' erwartet!");
	consume(TokenType::COLON, u8"Es wurde ':' nach 'beschreibt' erwartet!");

	int i = 0;
	loopStart:
	do
	{
		TokenType fieldToken = currIt->type;
		std::string structType = "";
		if (fieldToken == TokenType::IDENTIFIER)
		{
			consume(TokenType::IDENTIFIER, "");
			structType = preIt->literal;
			if (structType == structName)
			{
				error(u8"Eine Struktur kann sich nicht selbst enthalten!");
				advance();
				goto loopStart; //nothing to see here, just an italian cooking
			}
			fieldToken = currIt->type;
		}
		if (!(fieldToken >= TokenType::ZAHL && fieldToken <= TokenType::STRUKTUREN))
		{
			error(u8"Es wurde ein Typ spezifizierer erwartet!", currIt);
			advance();
			goto loopStart; //nothing to see here, just an italian cooking
		}
		ValueType fieldType = tokenToValueType(fieldToken);
		fieldType.structIdentifier = structType;
		advance();
		consume(TokenType::IDENTIFIER, u8"Es wurde ein Parameter-Name erwartet!");
		if (stru.count(preIt->literal) != 0)
			error("Die Struktur '" + structName + "' hat bereits ein Feld mit diesem Namen!");
		stru.insert(std::make_pair(preIt->literal, fieldType));
		emitConstant(Value(preIt->literal));
		if (!isArr(fieldType.type)) consume(TokenType::IST, "Es wurde 'ist' erwartet!");
		else if (isArr(fieldType.type)) consume(TokenType::SIND, "Es wurde 'sind' erwartet!");
		ValueType expr = expression();
		if (expr != fieldType)
			error(u8"Der Standard Wert stimmt nicht mit dem Typ des Feldes überein!");
		i++;
	} while (match(TokenType::COMMA));

	if (stru.empty())
		error(u8"Du darfst keine leeren Strukturen definieren!");

	emitBytes(op::DEFINE_STRUCT, makeConstant(structName));
	emitByte(makeConstant(i));

	structs.insert(std::make_pair(structName, stru));
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
	if (expr.type != Type::Bool)
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
	if (expr.type != Type::Bool) error(u8"Die Bedingung einer 'solange' Anweisung muss ein Boolean sein!");

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
	addLocal(localName, Type::Int);
	consume(TokenType::VON, u8"Es wurde ein 'von' erwartet!");

	ValueType expr = expression();
	if (expr.type != Type::Int) error(u8"Eine für Anweisung kann nur durch Zahlen iterieren!");

	emitBytes(op::SET_LOCAL, localNameConstant);
	emitByte(unitConstant);
	emitByte(op::POP);

	consume(TokenType::BIS, u8"Es wurde ein 'bis' erwartet!");

	emitByte(op::FORPREP);
	int conditionLoop = static_cast<int>(currentChunk()->bytes.size());
	
	emitBytes(op::GET_LOCAL, localNameConstant);
	emitByte(unitConstant);

	expr = expression();
	if (expr.type != Type::Int) error(u8"Eine für Anweisung kann nur durch Zahlen iterieren!");

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
		if (expr.type != Type::Int) error(u8"Eine für Anweisung kann nur durch Zahlen iterieren!");
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
	if (type.type == Type::None)
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
		Value val = GetDefaultValue(it->second);

		temp.insert(std::make_pair(it->first, val));
	}
	enclosingFunction->locals.insert(std::make_pair(identifier, std::move(temp)));

	currentScopeUnit = enclosingUnit;
}

#pragma endregion