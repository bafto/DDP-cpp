#include "Scanner.h"
#include <fstream>
#include <streambuf>

Scanner::Scanner(const std::string& file)
	:
	file(file),
	line(1),
	currentDepth(0)
{
	std::ifstream ifs;
	ifs.open(file);
	if (!ifs.is_open())
		throw file_exception(GENERATE_EXCEPTION(file_exception, "Could not open '" + file + "'"));
	source = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	start = source.begin();
	current = source.begin();

#pragma region keywords

	keywords["plus"] = TokenType::PLUS;
	keywords["minus"] = TokenType::MINUS;
	keywords["mal"] = TokenType::MAL;
	keywords["durch"] = TokenType::DURCH;
	keywords["modulo"] = TokenType::MODULO;
	keywords["hoch"] = TokenType::HOCH;
	keywords["wurzel"] = TokenType::WURZEL;
	keywords["ln"] = TokenType::LN;
	keywords["Betrag"] = TokenType::PLUS;
	keywords["pi"] = TokenType::PI;
	keywords["e"] = TokenType::E;
	keywords["tau"] = TokenType::TAU;
	keywords["phi"] = TokenType::PHI;
	keywords["und"] = TokenType::UND;
	keywords["oder"] = TokenType::ODER;
	keywords["nicht"] = TokenType::NICHT;
	keywords["kleiner"] = TokenType::KLEINER;
	keywords["größer"] = TokenType::GROESSER;
	keywords["als"] = TokenType::ALS;
	keywords["ungleich"] = TokenType::UNGLEICH;
	keywords["gleich"] = TokenType::GLEICH;
	keywords["um"] = TokenType::UM;
	keywords["bit"] = TokenType::BIT;
	keywords["nach"] = TokenType::NACH;
	keywords["rechts"] = TokenType::RECHTS;
	keywords["links"] = TokenType::LINKS;
	keywords["verschoben"] = TokenType::VERSCHOBEN;
	keywords["logisch"] = TokenType::LOGISCH;
	keywords["kontra"] = TokenType::KONTRA;
	keywords["wenn"] = TokenType::WENN;
	keywords["aber"] = TokenType::ABER;
	keywords["dann"] = TokenType::DANN;
	keywords["sonst"] = TokenType::SONST;
	keywords["für"] = TokenType::FUER;
	keywords["jede"] = TokenType::JEDE;
	keywords["von"] = TokenType::VON;
	keywords["bis"] = TokenType::BIS;
	keywords["schrittgröße"] = TokenType::SCHRITTGROESSE;
	keywords["solange"] = TokenType::SOLANGE;
	keywords["mache"] = TokenType::MACHE;
	keywords["Funktion"] = TokenType::FUNKTION;
	keywords["macht"] = TokenType::MACHT;
	keywords["vom"] = TokenType::VOM;
	keywords["Typ"] = TokenType::TYP;
	keywords["ist"] = TokenType::IST;
	keywords["sind"] = TokenType::SIND;
	keywords["der"] = TokenType::DER;
	keywords["die"] = TokenType::DIE;
	keywords["das"] = TokenType::DAS;
	keywords["Zahl"] = TokenType::ZAHL;
	keywords["Kommazahl"] = TokenType::KOMMAZAHL;
	keywords["Boolean"] = TokenType::BOOLEAN;
	keywords["Zeichenkette"] = TokenType::ZEICHENKETTE;
	keywords["Zeichen"] = TokenType::ZEICHEN;
	keywords["Zahlen"] = TokenType::ZAHLEN;
	keywords["Kommazahlen"] = TokenType::KOMMAZAHLEN;
	keywords["Zeichenketten"] = TokenType::ZEICHENKETTEN;
	keywords["an"] = TokenType::AN;
	keywords["Stelle"] = TokenType::STELLE;
	keywords["wahr"] = TokenType::WAHR;
	keywords["falsch"] = TokenType::FALSCH;
	keywords["gib"] = TokenType::GIB;
	keywords["zurück"] = TokenType::ZURUECK;

#pragma endregion
}

Token Scanner::scanToken()
{
	skipWhitespaces();
	start = current;

	if (isAtEnd()) return makeToken(TokenType::END);

	char c = advance();
	if (isAlphabetical(c, true)) return identifier();
	if (isDigit(c)) return number();

	switch (c)
	{
	/*case '#':
	{
		while (isAlphabetical(peek(), false) || peek() == '!' || isDigit(peek())) advance();
		std::string directive(start, current);
		if (directive == "!ascii")
		{
			utf8 = true;
			return makeToken(TokenType::DIRECTIVE);
		}
		else if (directive == "ascii")
		{
			utf8 = false;
			return makeToken(TokenType::DIRECTIVE);
		}
		else return errorToken("Unerwartete Direktive, meintest du #ascii oder #!ascii?");
		break;
	}*/
	case ':': return makeToken(TokenType::COLON);
	case '.': return makeToken(TokenType::DOT);
	case ',': return makeToken(TokenType::COMMA);
	case '"': return string();
	case '\'': return character();
	case '(': return makeToken(TokenType::LEFT_PAREN);
	case ')': return makeToken(TokenType::RIGHT_PAREN);
	case '-': return makeToken(TokenType::NEGATEMINUS);
#ifdef _MDEBUG_
	case '$': return makeToken(TokenType::PRINT);
#endif
	}

	return errorToken("Unerwartetes Zeichen");
}

bool Scanner::isAtEnd() const
{
	return *current == '\0';
}

Token Scanner::makeToken(TokenType type) const
{
	Token token;
	token.depth = currentDepth;
	token.type = type;
	token.line = line;
	token.literal = std::string(start, current);
	return token;
}

Token Scanner::errorToken(std::string msg) const
{
	Token token;
	token.type = TokenType::ERROR;
	token.depth = currentDepth;
	token.line = line;
	token.literal = msg;
	return token;
}

void Scanner::skipWhitespaces()
{
	int consecutiveSpaceCount = 0;
	while (true)
	{
		char c = peek();
		consecutiveSpaceCount = c == ' ' ? consecutiveSpaceCount + 1 : 0;
		switch (c)
		{
		case ' ':
			if (consecutiveSpaceCount == 4)
			{
				currentDepth++;
				consecutiveSpaceCount = 0;
			}
			__fallthrough;
		case '\r':
			advance();
			break;
		case '\t':
			currentDepth++;
			advance();
			break;
		case '\n':
			line++;
			currentDepth = 0;
			advance();
			break;
		case '/':
			if (peekNext() == '/')
			{
				while (peek() != '\n' && !isAtEnd()) advance();
			}
			else if (peekNext() == '*')
			{
				while (peek() != '*' && peekNext() != '/' && !isAtEnd()) advance();
			}
			else return;
			break;
		default:
			return;
		}
	}
}

Token Scanner::identifier()
{
	while (isAlphabetical(peek(), false) || isDigit(peek())) advance();
	std::string identifier(start, current);

	if()

	return Token();
}

Token Scanner::string()
{
	while (peek() != '"' && !isAtEnd()) {
		if (peek() == '\n') line++;
		advance();
	}

	if (isAtEnd()) return errorToken("Unfertige Zeichenkette");

	advance();
	return makeToken(TokenType::STRING);
}

Token Scanner::character()
{
	if (!isAtEnd() && peekNext() != '\'') return errorToken("Unfertiger Buchstabe");
	if (peek() == '\n') line++;
	advance();
	advance();
	return makeToken(TokenType::CHARACTER);
}

Token Scanner::number()
{
	TokenType type = TokenType::INUMBER;
	while (isDigit(peek())) advance();

	if (peek() == ',' && isDigit(peekNext()))
	{
		type = TokenType::DNUMBER;
		advance();
		while (isDigit(peek())) advance();
 	}

	return makeToken(type);
}

char Scanner::advance()
{
	current++;
	return current[-1];
}

char Scanner::peek()
{
	return *current;
}

char Scanner::peekNext()
{
	if (isAtEnd()) return '\0';
	return current[1];
}

bool Scanner::match(char expected)
{
	if (isAtEnd()) return false;
	if (peek() != expected) return false;
	current++;
	return true;
}

bool Scanner::isDigit(char c)
{
	return c >= '0' && c <= '9';
}

bool Scanner::isAlphabetical(char c, bool firstLetter)
{
	if (!utf8) return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; //no utf8 encoding

	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') return true;
	else if (c == '\xc3') {
		switch (firstLetter ? peek() : peekNext())
		{
		case '\x9c': //Ü
		case '\x96': //Ö
		case '\x84': //Ä
		case '\xbc': //ü
		case '\xa4': //ä
		case '\x9f': //ß
		case '\xb6': advance(); return true; //ö
		default: return false;
		}
	}
	else return false;
}
