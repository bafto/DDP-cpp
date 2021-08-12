#pragma once

#include "Scanner.h"
#include "Function.h"

class Compiler
{
private:
	using op = OpCode;
public:
	Compiler(const std::string& filePath, std::unordered_map<std::string, Value>* globals, std::unordered_map<std::string, Function>* functions);

	bool compile(); //returns true on success, fills globals with declarations and functions with definitions
private:
	Chunk* currentChunk() { return &currentFunction->chunk; }; //the chunk that is currently filled

	//common emits
	void emitByte(uint8_t byte) { currentChunk()->write(byte); };
	void emitByte(OpCode code) { currentChunk()->write(code); };
	void emitBytes(uint8_t byte1, uint8_t byte2) { emitByte(byte1); emitByte(byte2); };
	void emitBytes(OpCode code, uint8_t byte) { emitByte(code); emitByte(byte); };
	void emitReturn() { emitByte(OpCode::RETURN); };
	void emitConstant(Value value) { emitBytes(OpCode::CONSTANT, makeConstant(std::move(value))); };

	uint8_t makeConstant(Value value); //add a constant to the current chunk and return its index

	void error(std::string msg, std::vector<Token>::iterator where);
	void error(std::string msg);

	//Functions to work with the tokens
	void advance();
	void consume(TokenType type, std::string errorMsg); //if currIt matches the type we advance, else we report an error with errorMsg
	bool match(TokenType type); //if currIt matches the type we advance and return true, else we return false and don't advance
	bool check(TokenType type); //same as match but without advancing
	bool checkNext(TokenType type); //check but the Token after currIt
private:
	void declaration(); //starting point of the compiler
	void statement(); //statements like loops or function calls
	//if we are in panicMode after a declaration
	//we advance until we reach the end of a statement 
	//and then continue compiling
	// so that we can report all following errors
	void synchronize(); 

	//scoped statements
	void beginScope();
	void block();
	void endScope();

	void expressionStatement(); //if you simply write an expression without anything else it should not be an error

	//declarations
	void varDeclaration();
	void funDeclaration();

	//statements where we need to jump over code
	void patchJump(int offset);
	void ifStatement();
	void whileStatement();
	void forStatement();

#ifndef NDEBUG
	void printStatement();
#endif
private:
	enum class Precedence
	{
		None,
		Assignement, // =
		Or, // ||
		And, // &&
		Bitwise, // ^ ~ & |
		Equality, // == !=
		Comparison, // < > <= >=
		Bitshift, // << >>
		Term, // + -
		Factor, // * / %
		Exponent, // pow sqrt
		Unary, // ! -
		Indexing, // []
		Call, // ()
		Primary
	};
private:
	[[nodiscard]] ValueType expression(); //parse the next expression and return the type it evaluates to
	[[nodiscard]] ValueType parsePrecedence(Precedence precedence); //parse the next operator/expression with the given precedence

	//parse functions for all operators

	[[nodiscard]] ValueType dnumber(bool canAssign);
	[[nodiscard]] ValueType inumber(bool canAssign);
	[[nodiscard]] ValueType string(bool canAssign);
	[[nodiscard]] ValueType character(bool canAssign);
	[[nodiscard]] ValueType arrLiteral(bool canAssign);
	[[nodiscard]] ValueType Literal(bool canAssign);
	[[nodiscard]] ValueType grouping(bool canAssign);
	[[nodiscard]] ValueType unary(bool canAssign);
	[[nodiscard]] ValueType binary(bool canAssign);
	[[nodiscard]] ValueType bitwise(bool canAssign);
	[[nodiscard]] ValueType and_(bool canAssign);
	[[nodiscard]] ValueType or_(bool canAssign);
	[[nodiscard]] ValueType variable(bool canAssign);
	[[nodiscard]] ValueType index(bool canAssign);
private:
	using MemFunctPtr = ValueType(Compiler::*)(bool); //pointer to a member function of Compiler that takes a bool and returns a ValueType
	struct ParseRule
	{
		MemFunctPtr prefix; //function for the prefix use of the operator
		MemFunctPtr infix; //function for the infix use of the operator
		Precedence precedence; //the precedende of the operator
	};
	static inline const std::unordered_map<TokenType, ParseRule> parseRules = {
		{ TokenType::LEFT_PAREN,	ParseRule{&Compiler::grouping, nullptr,			Precedence::Call}},
		{ TokenType::RIGHT_PAREN,	ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::COLON,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::COMMA,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::DOT,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::MINUS,			ParseRule{nullptr,			&Compiler::binary,  Precedence::Term}},
		{ TokenType::NEGATEMINUS,   ParseRule{&Compiler::unary,	nullptr,			Precedence::Term}},
		{ TokenType::PLUS,			ParseRule{nullptr,			&Compiler::binary,  Precedence::Term}},
		{ TokenType::DURCH,			ParseRule{nullptr,			&Compiler::binary,  Precedence::Factor}},
		{ TokenType::MAL,			ParseRule{nullptr,			&Compiler::binary,  Precedence::Factor}},
		{ TokenType::NICHT,			ParseRule{&Compiler::unary,	nullptr,			Precedence::Unary}},
		{ TokenType::MODULO,		ParseRule{nullptr,			&Compiler::binary,	Precedence::Term}},
		{ TokenType::HOCH,			ParseRule{nullptr,			&Compiler::binary,	Precedence::Exponent}},
		{ TokenType::WURZEL,		ParseRule{nullptr,			&Compiler::binary,  Precedence::Exponent}},
		{ TokenType::UM,			ParseRule{nullptr,			&Compiler::binary,	Precedence::Bitshift}},
		{ TokenType::LOGISCH,		ParseRule{&Compiler::bitwise,nullptr,			Precedence::Bitwise}},
		{ TokenType::LOGISCHNICHT,  ParseRule{&Compiler::unary, nullptr,			Precedence::Unary}},
		{ TokenType::LN,			ParseRule{&Compiler::unary, nullptr,			Precedence::Unary}},
		{ TokenType::BETRAG,		ParseRule{&Compiler::unary, nullptr,			Precedence::Unary}},
		{ TokenType::UNGLEICH,		ParseRule{nullptr,			&Compiler::binary,	Precedence::Equality}},
		{ TokenType::GLEICH,		ParseRule{nullptr,			&Compiler::binary,	Precedence::Equality}},
		{ TokenType::GROESSER,		ParseRule{nullptr,			&Compiler::binary,	Precedence::Comparison}},
		{ TokenType::KLEINER,		ParseRule{nullptr,			&Compiler::binary,	Precedence::Comparison}},
		{ TokenType::GROESSERODER,	ParseRule{nullptr,			&Compiler::binary,	Precedence::Comparison}},
		{ TokenType::KLEINERODER,	ParseRule{nullptr,			&Compiler::binary,	Precedence::Comparison}},
		{ TokenType::IDENTIFIER,	ParseRule{&Compiler::variable,nullptr,			Precedence::None}},
		{ TokenType::STRING,		ParseRule{&Compiler::string,nullptr,			Precedence::None}},
		{ TokenType::INUMBER,		ParseRule{&Compiler::inumber,nullptr,			Precedence::None}},
		{ TokenType::DNUMBER,		ParseRule{&Compiler::dnumber,nullptr,			Precedence::None}},
		{ TokenType::CHARACTER,		ParseRule{&Compiler::character,nullptr,			Precedence::None}},
		{ TokenType::UND,			ParseRule{nullptr,			&Compiler::and_,	Precedence::And}},
		{ TokenType::SONST,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::FALSCH,		ParseRule{&Compiler::Literal,nullptr,			Precedence::None}},
		{ TokenType::PI,			ParseRule{&Compiler::Literal,nullptr,			Precedence::None}},
		{ TokenType::TAU,			ParseRule{&Compiler::Literal,nullptr,			Precedence::None}},
		{ TokenType::E,				ParseRule{&Compiler::Literal,nullptr,			Precedence::None}},
		{ TokenType::PHI,			ParseRule{&Compiler::Literal,nullptr,			Precedence::None}},
		{ TokenType::FUER,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::FUNKTION,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::WENN,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::ODER,			ParseRule{nullptr,			&Compiler::or_,		Precedence::Or}},
		{ TokenType::IST,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::GIB,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::ZURUECK,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::WAHR,			ParseRule{&Compiler::Literal,nullptr,			Precedence::None}},
		{ TokenType::ZAHL,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::KOMMAZAHL,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::BOOLEAN,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::ZEICHEN,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::ZEICHENKETTE,	ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::ZAHLEN,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::BOOLEANS,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::KOMMAZAHLEN,	ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::ZEICHENKETTEN,	ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::AN,			ParseRule{nullptr,			&Compiler::index,	Precedence::Call}},
		{ TokenType::STELLE,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::STUECK,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::LEFT_SQAREBRACKET,ParseRule{&Compiler::arrLiteral,	nullptr,	Precedence::None}},
		{ TokenType::RIGHT_SQAREBRACKET,ParseRule{nullptr,		nullptr,			Precedence::None}},
		{ TokenType::SEMICOLON,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::SOLANGE,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::ALS,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::BIT,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::NACH,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::RECHTS,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::LINKS,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::VERSCHOBEN,	ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::KONTRA,		ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::ABER,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::DANN,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::JEDE,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::VON,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::BIS,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::MIT,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::SCHRITTGROESSE,ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::MACHE,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::VOM,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::TYP,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::SIND,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::DER,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::DIE,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::DAS,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::ERROR,			ParseRule{nullptr,			nullptr,			Precedence::None}},
		{ TokenType::END,			ParseRule{nullptr,			nullptr,			Precedence::None}}
#ifdef _MDEBUG_
		,{TokenType::PRINT,			ParseRule{nullptr,			nullptr,			Precedence::None}}
#endif
	};
private:
	const std::string filePath;

	std::unordered_map<std::string, Value>* globals;
	std::unordered_map<std::string, Function>* functions;

	bool hadError; //did an error occure?
	bool panicMode; //are we currently handling an error?
private:
	std::vector<Token> tokens; //output from the scanner
	std::vector<Token>::iterator preIt; //previously scanned token
	std::vector<Token>::iterator currIt; //current token

	Function* currentFunction; //the function that is currently being compiled (most often the nameless main function)

	ValueType lastEmittedType;
};

