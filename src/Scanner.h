#pragma once

#include "common.h"
#include <unordered_map>

#pragma warning (disable : 26495)

class file_exception : public base_exception
{
public:
	file_exception(const std::string& msg) : base_exception(msg) {}
};

class scan_exception : public base_exception
{
public:
	scan_exception(const std::string& msg) : base_exception(msg) {}
};

enum class TokenType
{
	//syntax
	COLON, DOT, COMMA, LEFT_PAREN, RIGHT_PAREN,
	//literals
	IDENTIFIER, STRING, CHARACTER, INUMBER, DNUMBER,
	//maths
	NEGATEMINUS,
	PLUS, MINUS, MAL, DURCH, MODULO, HOCH, WURZEL, LN, BETRAG,
	PI, E, TAU, PHI,
	//boolsche operatoren
	UND, ODER, NICHT,
	KLEINER, GROESSER, ALS, KLEINERODER, GROESSERODER, UNGLEICH, GLEICH,
	//bitshifting
	UM, BIT, NACH, RECHTS, LINKS, VERSCHOBEN,
	//bitwise
	LOGISCH, KONTRA, LOGISCHNICHT,
	//if
	WENN, ABER, DANN, SONST,
	//for
	FUER, JEDE, VON, BIS, SCHRITTGROESSE,
	//while
	SOLANGE,
	//loops
	MACHE,
	//functions
	FUNKTION, MACHT, VOM, TYP,
	//assignement
	IST, SIND,
	//Artikel
	DER, DIE, DAS,
	//Typen
	ZAHL, KOMMAZAHL, BOOLEAN, ZEICHENKETTE, ZEICHEN,
	//Arrays
	ZAHLEN, KOMMAZAHLEN, ZEICHENKETTEN, AN, STELLE,
	//bool literals
	WAHR, FALSCH,
	//return
	GIB, ZURUECK,
#ifdef _MDEBUG_
	PRINT,
#endif
	BINDE, EIN,
	DIRECTIVE,
	ERROR, END
};

struct Token
{
	TokenType type;
	std::string literal;
	int line;
	int depth;
};

class Scanner
{
public:
	Scanner(const std::string& file);

	const std::vector<Token> scanTokens();
private:
	Token scanToken();
	/**Functions used in scanToken**/

	bool isAtEnd() const;
	Token makeToken(TokenType type) const;
	Token errorToken(std::string msg) const;
	void skipWhitespaces();

	Token identifier();
	Token string();
	Token character();
	Token number();

	/**Helper functions for scanning**/

	char advance();
	char peek();
	char peekNext();
	bool match(char expected);
	bool isDigit(char c);
	bool isAlphabetical(char c, bool firstLetter);

	//helper for scanTokens
	void consume(TokenType type, const std::string& msg, std::vector<Token>::iterator& it);
private:
	const std::string file; //path to the source file
	std::string source; //the source code

	static inline const std::unordered_map<std::string, TokenType> keywords = {
		{"plus", TokenType::PLUS},
		{"minus", TokenType::MINUS},
		{"mal", TokenType::MAL},
		{"durch", TokenType::DURCH},
		{"modulo", TokenType::MODULO},
		{"hoch", TokenType::HOCH},
		{"wurzel", TokenType::WURZEL},
		{"ln", TokenType::LN},
		{"Betrag", TokenType::PLUS},
		{"pi", TokenType::PI},
		{"e", TokenType::E},
		{"tau", TokenType::TAU},
		{"phi", TokenType::PHI},
		{"und", TokenType::UND},
		{"oder", TokenType::ODER},
		{"nicht", TokenType::NICHT},
		{"kleiner", TokenType::KLEINER},
		{"größer", TokenType::GROESSER},
		{"als", TokenType::ALS},
		{"ungleich", TokenType::UNGLEICH},
		{"gleich", TokenType::GLEICH},
		{"um", TokenType::UM},
		{"bit", TokenType::BIT},
		{"nach", TokenType::NACH},
		{"rechts", TokenType::RECHTS},
		{"links", TokenType::LINKS},
		{"verschoben", TokenType::VERSCHOBEN},
		{"logisch", TokenType::LOGISCH},
		{"kontra", TokenType::KONTRA},
		{"wenn", TokenType::WENN},
		{"aber", TokenType::ABER},
		{"dann", TokenType::DANN},
		{"sonst", TokenType::SONST},
		{"für", TokenType::FUER},
		{"jede", TokenType::JEDE},
		{"von", TokenType::VON},
		{"bis", TokenType::BIS},
		{"schrittgröße", TokenType::SCHRITTGROESSE},
		{"solange", TokenType::SOLANGE},
		{"mache", TokenType::MACHE},
		{"Funktion", TokenType::FUNKTION},
		{"macht", TokenType::MACHT},
		{"vom", TokenType::VOM},
		{"Typ", TokenType::TYP},
		{"ist", TokenType::IST},
		{"sind", TokenType::SIND},
		{"der", TokenType::DER},
		{"die", TokenType::DIE},
		{"das", TokenType::DAS},
		{"Zahl", TokenType::ZAHL},
		{"Kommazahl", TokenType::KOMMAZAHL},
		{"Boolean", TokenType::BOOLEAN},
		{"Zeichenkette", TokenType::ZEICHENKETTE},
		{"Zeichen", TokenType::ZEICHEN},
		{"Zahlen", TokenType::ZAHLEN},
		{"Kommazahlen", TokenType::KOMMAZAHLEN},
		{"Zeichenketten", TokenType::ZEICHENKETTEN},
		{"an", TokenType::AN},
		{"Stelle", TokenType::STELLE},
		{"wahr", TokenType::WAHR},
		{"falsch", TokenType::FALSCH},
		{"gib", TokenType::GIB},
		{"zurück", TokenType::ZURUECK},
		{"binde", TokenType::BINDE},
		{"ein", TokenType::EIN}
	};

	std::string::iterator start; //start of the current token
	std::string::iterator current; //one after the current character
	int line; //current line in the source code
	int currentDepth; //depth on the current line
};

