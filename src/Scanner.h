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
	DER, DIE,// DAS,
	//Typen
	ZAHL, KOMMAZAHL, BOOLEAN, BUCHSTABE, TEXT,
	//Arrays
	ZAHLEN, KOMMAZAHLEN, BOOLEANS, BUCHSTABEN, TEXTE, AN, STELLE, STUECK, LEFT_SQAREBRACKET, RIGHT_SQAREBRACKET, SEMICOLON,
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
		{u8"plus", TokenType::PLUS},
		{u8"minus", TokenType::MINUS},
		{u8"mal", TokenType::MAL},
		{u8"durch", TokenType::DURCH},
		{u8"modulo", TokenType::MODULO},
		{u8"hoch", TokenType::HOCH},
		{u8"Wurzel", TokenType::WURZEL},
		{u8"ln", TokenType::LN},
		{u8"Betrag", TokenType::BETRAG},
		{u8"pi", TokenType::PI},
		{u8"e", TokenType::E},
		{u8"tau", TokenType::TAU},
		{u8"phi", TokenType::PHI},
		{u8"und", TokenType::UND},
		{u8"oder", TokenType::ODER},
		{u8"nicht", TokenType::NICHT},
		{u8"kleiner", TokenType::KLEINER},
		{u8"größer", TokenType::GROESSER},
		{u8"als", TokenType::ALS},
		{u8"ungleich", TokenType::UNGLEICH},
		{u8"gleich", TokenType::GLEICH},
		{u8"um", TokenType::UM},
		{u8"bit", TokenType::BIT},
		{u8"nach", TokenType::NACH},
		{u8"rechts", TokenType::RECHTS},
		{u8"links", TokenType::LINKS},
		{u8"verschoben", TokenType::VERSCHOBEN},
		{u8"logisch", TokenType::LOGISCH},
		{u8"kontra", TokenType::KONTRA},
		{u8"wenn", TokenType::WENN},
		{u8"aber", TokenType::ABER},
		{u8"dann", TokenType::DANN},
		{u8"sonst", TokenType::SONST},
		{u8"für", TokenType::FUER},
		{u8"jede", TokenType::JEDE},
		{u8"von", TokenType::VON},
		{u8"bis", TokenType::BIS},
		{u8"mit", TokenType::MIT},
		{u8"schrittgröße", TokenType::SCHRITTGROESSE},
		{u8"solange", TokenType::SOLANGE},
		{u8"mache", TokenType::MACHE},
		{u8"Funktion", TokenType::FUNKTION},
		{u8"macht", TokenType::MACHT},
		{u8"vom", TokenType::VOM},
		{u8"Typ", TokenType::TYP},
		{u8"ist", TokenType::IST},
		{u8"sind", TokenType::SIND},
		{u8"der", TokenType::DER},
		{u8"die", TokenType::DIE},
		//{u8"das", TokenType::DAS},
		{u8"Zahl", TokenType::ZAHL},
		{u8"Kommazahl", TokenType::KOMMAZAHL},
		{u8"Boolean", TokenType::BOOLEAN},
		{u8"Buchstabe", TokenType::BUCHSTABE},
		{u8"Text", TokenType::TEXT},
		{u8"Zahlen", TokenType::ZAHLEN},
		{u8"Kommazahlen", TokenType::KOMMAZAHLEN},
		{u8"Booleans", TokenType::BOOLEANS},
		{u8"Buchstaben", TokenType::BUCHSTABEN},
		{u8"Texte", TokenType::TEXTE},
		{u8"an", TokenType::AN},
		{u8"Stelle", TokenType::STELLE},
		{u8"Stück", TokenType::STUECK},
		{u8"wahr", TokenType::WAHR},
		{u8"falsch", TokenType::FALSCH},
		{u8"gib", TokenType::GIB},
		{u8"zurück", TokenType::ZURUECK},
		{u8"binde", TokenType::BINDE},
		{u8"ein", TokenType::EIN}
	};

	std::string::iterator start; //start of the current token
	std::string::iterator current; //one after the current character
	int line; //current line in the source code
	int depth; //current scope depth of the token
	bool hadError; //indicates if the scanner errored
};

