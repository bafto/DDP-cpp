#pragma once

#include "Value.h"
#include "Chunk.h"
#include "Scanner.h"

class Compiler
{
private:
	using op = OpCode;
public:
	Compiler(const std::string& file);

	Function compile(); //compile the source code in the file into chunk
	bool errored() { return hadError; };
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
		INDEXING,
		UNARY,       // ! -
		CALL,        // ()
		PRIMARY
	};
	enum class FunctionType
	{
		FUNCTION,
		SCRIPT,
	};
	struct Local
	{
		Token token;
		ValueType type;
	};
	struct ScopeUnit
	{
		Function function;
		FunctionType type;

		Local locals[UINT8_MAX + 1];
		int localCount;
		int scopeDepth;

		ScopeUnit(ScopeUnit*& currentUnit, FunctionType type)
		{
			localCount = 0;
			scopeDepth = 0;
			type = type;
			currentUnit = this;

			Local* local = &currentUnit->locals[currentUnit->localCount++];
			local->token.depth = 0;
			local->token.literal = "";
		}
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

	Chunk* currentChunk() { return &currentScopeUnit->function.chunk; }; //will be changed later

	uint8_t makeConstant(Value value);

	void emitByte(uint8_t byte) { currentChunk()->write(byte, previous->line); };
	void emitByte(OpCode code) { currentChunk()->write(code, previous->line); };
	void emitBytes(uint8_t byte1, uint8_t byte2) { emitByte(byte1); emitByte(byte2); };
	void emitBytes(OpCode code, uint8_t byte2) { emitByte(code); emitByte(byte2); };
	void emitReturn() { emitByte(OpCode::RETURN); };
	int emitJump(OpCode code) { emitByte(code); emitBytes(0xff, 0xff); return currentChunk()->code.size() - 2; };
	void emitLoop(int loopStart)
	{
		emitByte(op::LOOP);
		int offset = currentChunk()->code.size() - loopStart + 2;
		if (offset > UINT16_MAX) error("Zuviele Anweisungen in einer 'solange' Anweisung!");

		emitByte((offset >> 8) & 0xff);
		emitByte(offset & 0xff);
	}
	void emitConstant(Value value) { emitBytes((uint8_t)OpCode::CONSTANT, makeConstant(std::move(value))); };

	void initScopeUnit(ScopeUnit& unit) { unit.localCount = 0; unit.scopeDepth = 0; currentScopeUnit = &unit; };

	Function endCompiler();

	void synchronize();

	void declaration();
	void statement();

	ValueType boolAssignement(); //to handle the special x ist wahr wenn syntax of booleans
	uint8_t identifierConstant(std::string identifier, ValueType type);
	uint8_t parseVariable(ValueType type, std::string msg);
	void declareVariable(ValueType type);
	void addLocal(Token token, ValueType type);
	void defineVariable(uint8_t global);
	void defineVariable(uint8_t global, ValueType arrType); //to define empty arrays

	void beginScope() { currentScopeUnit->scopeDepth++; };
	void endScope() 
	{ 
		currentScopeUnit->scopeDepth--; 
		while (currentScopeUnit->localCount > 0 && currentScopeUnit->locals[currentScopeUnit->localCount - 1].token.depth > currentScopeUnit->scopeDepth)
		{
			emitByte(op::POP);
			currentScopeUnit->localCount--;
		}
	};
	void block();
	void varDeclaration();
	void expressionStatement();
	void patchJump(int offset);
	void ifStatement();
	void whileStatement();
	void forStatement();
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
	ValueType and_(bool canAssign);
	ValueType or_(bool canAssign);
	int resolveLocal(ScopeUnit* unit, std::string name, ValueType* type);
	void markInitialized();
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
		{ TokenType::CHARACTER,		ParseRule{&Compiler::character,nullptr,			Precedence::NONE}},
		{ TokenType::UND,			ParseRule{nullptr,			&Compiler::and_,	Precedence::AND}},
		{ TokenType::SONST,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::FALSCH,		ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::PI,			ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::TAU,			ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::E,				ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::PHI,			ParseRule{&Compiler::Literal,nullptr,			Precedence::NONE}},
		{ TokenType::FUER,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::FUNKTION,		ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::WENN,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
		{ TokenType::ODER,			ParseRule{nullptr,			&Compiler::or_,		Precedence::OR}},
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
		{ TokenType::MIT,			ParseRule{nullptr,			nullptr,			Precedence::NONE}},
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
	const std::string file; //path to the file with the source code

	bool hadError;
	bool panicMode;

	std::vector<Token> tokens;
	std::vector<Token>::iterator current;
	std::vector<Token>::iterator previous;

	ValueType lastEmittedType;

	std::unordered_map<std::string, ValueType> globals;

	ScopeUnit* currentScopeUnit;

	int localArrSlot; //for arrays
};

