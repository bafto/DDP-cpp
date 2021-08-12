#pragma once

#include <string>
#include <vector>
#include <unordered_map>

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
	FUER, JEDE, VON, BIS, MIT, SCHRITTGROESSE,
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
	ZAHLEN, KOMMAZAHLEN, BOOLEANS, ZEICHENKETTEN, AN, STELLE, STUECK, LEFT_SQAREBRACKET, RIGHT_SQAREBRACKET, SEMICOLON,
	//bool literals
	WAHR, FALSCH,
	//return
	GIB, ZURUECK,
#ifndef NDEBUG
	PRINT,
#endif
	BINDE, EIN,
	DIRECTIVE,
	ERROR, END,
	COUNT
};

struct Token
{
	TokenType type = TokenType::ERROR;
	std::string literal = u8"";
	int depth = 0;
	int line = 1;
};

class Scanner
{
public:
	Scanner(const std::string& file);

	std::pair<std::vector<Token>, bool> scanTokens(); //the bool is false if an error occured, but even if that happened the vector may still be used
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
	void error(const std::string& msg, int line);

	void consume(TokenType type, const std::string& msg, std::vector<Token>::iterator& it, std::vector<Token>& vec);
	void consumeErase(TokenType type, const std::string& msg, std::vector<Token>::iterator& it, std::vector<Token>& vec);
	bool check(TokenType type, std::vector<Token>::iterator& it, std::vector<Token>& vec);
	bool check(TokenType type, std::vector<Token>::iterator& it, std::vector<Token>& vec, int offset);
private:
	const std::string filePath;
	std::string source; //the source code read from filePath

	static inline const std::unordered_map<std::string, TokenType> keywords = {
		{"plus", TokenType::PLUS},
		{"minus", TokenType::MINUS},
		{"mal", TokenType::MAL},
		{"durch", TokenType::DURCH},
		{"modulo", TokenType::MODULO},
		{"hoch", TokenType::HOCH},
		{"Wurzel", TokenType::WURZEL},
		{"ln", TokenType::LN},
		{"Betrag", TokenType::BETRAG},
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
		{"mit", TokenType::MIT},
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
		{"Booleans", TokenType::BOOLEANS},
		{"Zeichenketten", TokenType::ZEICHENKETTEN},
		{"an", TokenType::AN},
		{"Stelle", TokenType::STELLE},
		{"Stück", TokenType::STUECK},
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
	int depth; //current scope depth of the token
	bool hadError; //indicates if the scanner errored
};

