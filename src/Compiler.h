#pragma once

#include "Chunk.h"
#include "Scanner.h"

class Compiler
{
private:
	using op = OpCode;
public:
	Compiler(const std::string& file, Chunk* chunk);

	bool compile(); //compile the source code in the file into chunk
private:
	enum class Precedence
	{
		NONE,
		ASSIGNMENT,  // =
		OR,          // or
		AND,         // and
		BITWISE,	 // ^ ~ & |
		EQUALITY,    // == !=
		COMPARISON,  // < > <= >=
		BITSHIFT,    // << >>
		TERM,        // + -
		FACTOR,      // * / %
		EXPONENT,
		UNARY,       // ! -
		CALL,        // ()
		PRIMARY
	};
private:
	/**Functions only used in compile**/

	void advance();
	void consume(TokenType type, std::string msg);
	bool match(TokenType type);
	bool check(TokenType type);
	bool checkNext(TokenType type);

	void errorAtCurrent(std::string msg);
	void error(std::string msg);
	void errorAt(const Token& token, std::string msg);

	Chunk* currentChunk() { return chunk; }; //will be changed later

	uint8_t makeConstant(Value value);

	void emitByte(uint8_t byte) { currentChunk()->write(byte, previous->line); };
	void emitByte(OpCode code) { currentChunk()->write(code, previous->line); };
	void emitBytes(uint8_t byte1, uint8_t byte2) { emitByte(byte1); emitByte(byte2); };
	void emitBytes(OpCode code, uint8_t byte2) { emitByte(code); emitByte(byte2); };
	void emitReturn() { emitByte(OpCode::RETURN); };
	void emitConstant(Value value) { emitBytes((uint8_t)OpCode::CONSTANT, makeConstant(std::move(value))); };

	void endCompiler();

	void synchronize();

	void declaration();
	void statement();

	ValueType boolAssignement(); //to handle the special x ist wahr wenn syntax of booleans
	uint8_t identifierConstant(std::string identifier, ValueType type);
	uint8_t parseVariable(ValueType type, std::string msg);
	void defineVariable(uint8_t global);
	void defineVariable(uint8_t global, ValueType arrType); //to define empty arrays

	void varDeclaration();
	void expressionStatement();
#ifdef _MDEBUG_
	void printStatement();
#endif


	ValueType expression();
	ValueType parsePrecedence(Precedence precedence);

	ValueType dnumber(bool canAssign);
	ValueType inumber(bool canAssign);
	ValueType string(bool canAssign);
	ValueType character(bool canAssign);
	ValueType arrLiteral(bool canAssign);
	ValueType Literal(bool canAssign);
	ValueType grouping(bool canAssign);
	ValueType unary(bool canAssign);
	ValueType binary(bool canAssign);
	ValueType bitwise(bool canAssign);
	ValueType namedVariable(std::string name, bool canAssign);
	ValueType variable(bool canAssign);
	ValueType index(bool canAssign);
private:
	using MemFuncPtr = ValueType(Compiler::*)(bool);
	struct ParseRule
	{
		MemFuncPtr prefix;
		MemFuncPtr infix;
		Precedence precedence;
	};

	static inline const std::unordered_map<TokenType, ParseRule> parseRules = {
		{ TokenType::LEFT_PAREN,	ParseRule{&Compiler::grouping, nullptr,			Precedence::CALL}},
		{ TokenType::RIGHT_PAREN,	ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::COLON,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::COMMA,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::DOT,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::MINUS,			ParseRule{nullptr,			&Compiler::binary,  Precedence::TERM}},
		{ TokenType::NEGATEMINUS,   ParseRule{&Compiler::unary,	nullptr,			Precedence::TERM}},
		{ TokenType::PLUS,			ParseRule{nullptr,			&Compiler::binary,  Precedence::TERM}},
		{ TokenType::DURCH,			ParseRule{nullptr,			&Compiler::binary,  Precedence::FACTOR}},
		{ TokenType::MAL,			ParseRule{nullptr,			&Compiler::binary,  Precedence::FACTOR}},
		{ TokenType::NICHT,			ParseRule{&Compiler::unary,	nullptr,			Precedence::UNARY}},
		{ TokenType::MODULO,		ParseRule{nullptr,			&Compiler::binary,	Precedence::TERM}},
		{ TokenType::HOCH,			ParseRule{nullptr,			&Compiler::binary,	Precedence::EXPONENT}},
		{ TokenType::WURZEL,		ParseRule{nullptr,			&Compiler::binary,  Precedence::EXPONENT}},
		{ TokenType::UM,			ParseRule{nullptr,			&Compiler::binary,	Precedence::BITSHIFT}},
		{ TokenType::LOGISCH,		ParseRule{&Compiler::bitwise,nullptr,			Precedence::BITWISE}},
		{ TokenType::LOGISCHNICHT,  ParseRule{&Compiler::unary, nullptr,			Precedence::UNARY}},
		{ TokenType::LN,			ParseRule{&Compiler::unary, nullptr,			Precedence::UNARY}},
		{ TokenType::BETRAG,		ParseRule{&Compiler::unary, nullptr,			Precedence::UNARY}},
		{ TokenType::UNGLEICH,		ParseRule{nullptr,			&Compiler::binary,	Precedence::EQUALITY}},
		{ TokenType::GLEICH,		ParseRule{nullptr,			&Compiler::binary,	Precedence::EQUALITY}},
		{ TokenType::GROESSER,		ParseRule{nullptr,			&Compiler::binary,	Precedence::COMPARISON}},
		{ TokenType::KLEINER,		ParseRule{nullptr,			&Compiler::binary,	Precedence::COMPARISON}},
		{ TokenType::GROESSERODER,	ParseRule{nullptr,			&Compiler::binary,	Precedence::COMPARISON}},
		{ TokenType::KLEINERODER,	ParseRule{nullptr,			&Compiler::binary,	Precedence::COMPARISON}},
		{ TokenType::IDENTIFIER,	ParseRule{&Compiler::variable,nullptr,			Precedence::NONE}},
		{ TokenType::STRING,		ParseRule{&Compiler::string,nullptr,			Precedence::NONE}},
		{ TokenType::INUMBER,		ParseRule{&Compiler::inumber,nullptr,			Precedence::NONE}},
		{ TokenType::DNUMBER,		ParseRule{&Compiler::dnumber,nullptr,			Precedence::NONE}},
		{ TokenType::CHARACTER,		ParseRule{&Compiler::character,nullptr,		Precedence::NONE}},
		{ TokenType::UND,			ParseRule{nullptr,			nullptr,			Precedence::AND}},
		{ TokenType::SONST,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::FALSCH,		ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::PI,			ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::TAU,			ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::E,				ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::PHI,			ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::FUER,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::FUNKTION,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::WENN,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::ODER,			ParseRule{nullptr,			nullptr,			Precedence::OR}},
		{ TokenType::IST,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::GIB,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::ZURUECK,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::WAHR,			ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::ZAHL,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::KOMMAZAHL,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::BOOLEAN,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::ZEICHEN,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::ZEICHENKETTE,	ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::ZAHLEN,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::BOOLEANS,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::KOMMAZAHLEN,	ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::ZEICHENKETTEN,	ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::AN,			ParseRule{nullptr,			&Compiler::index,	Precedence::CALL}},
		{ TokenType::STELLE,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::STUECK,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::LEFT_SQAREBRACKET,ParseRule{&Compiler::arrLiteral,	nullptr,	Precedence::NONE}},
		{ TokenType::RIGHT_SQAREBRACKET,ParseRule{nullptr,		nullptr,			Precedence::NONE}},
		{ TokenType::SEMICOLON,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::SOLANGE,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::ALS,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::BIT,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::NACH,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::RECHTS,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::LINKS,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::VERSCHOBEN,	ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::KONTRA,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::ABER,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::DANN,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::JEDE,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::VON,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::BIS,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::SCHRITTGROESSE,ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::MACHE,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::VOM,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::TYP,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::SIND,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::DER,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::DIE,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::DAS,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::ERROR,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::END,			ParseRule{nullptr,			nullptr,			Precedence::NONE}}
#ifdef _MDEBUG_
		,{TokenType::PRINT,			ParseRule{nullptr,			nullptr,			Precedence::NONE}}
#endif
	};
private:
	Chunk* chunk; //the chunk into which the source code is compiled
	const std::string file; //path to the file with the source code

	bool hadError;
	bool panicMode;

	std::vector<Token> tokens;
	std::vector<Token>::iterator current;
	std::vector<Token>::iterator previous;

	ValueType lastEmittedType;

	std::unordered_map<std::string, ValueType> globals;
};

