#pragma once

#include "common.h"

#pragma warning (disable : 26495)

class file_exception : public base_exception
{
public:
	file_exception(const std::string& msg) : base_exception(msg) {}
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
	PI, E, TAU,
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
	FUNKTION, NIMMT, MIT, MACHT, VOM, TYP,
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

	Token scanToken();

private:
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
private:
	const std::string file; //path to the source file
	std::string source; //the source code

	std::string::iterator start; //start of the current token
	std::string::iterator current; //one after the current character
	int line; //current line in the source code
	int currentDepth; //depth on the current line
};

