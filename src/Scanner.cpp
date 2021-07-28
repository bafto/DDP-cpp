#include "Scanner.h"
#include <fstream>
#include <streambuf>
#include <iostream>

Scanner::Scanner(const std::string& file)
	:
	file(file),
	line(1),
	currentDepth(0),
	hadError(false)
{
	std::ifstream ifs;
	ifs.open(file);
	if (!ifs.is_open())
		throw file_exception(GENERATE_EXCEPTION(file_exception, "Could not open '" + file + "'"));
	source = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	source.push_back('\0');
	start = source.begin();
	current = source.begin();
}

void Scanner::error(const std::string& msg, int line)
{
	std::cerr << "[Line " << line << "] " << msg << "\n";
	hadError = true;
}

void Scanner::consume(TokenType type, const std::string& msg, std::vector<Token>::iterator& it, std::vector<Token>& vec)
{
	if (it >= vec.end() - 1) error(msg, (it - 1)->line);
	if ((it + 1)->type != type)
		error(msg, it->line);
	it++;
}

void Scanner::consumeErase(TokenType type, const std::string& msg, std::vector<Token>::iterator& it, std::vector<Token>& vec)
{
	consume(type, msg, it, vec);
	it = vec.erase(it);
}

bool Scanner::check(TokenType type, std::vector<Token>::iterator& it, std::vector<Token>& vec)
{
	if (it >= vec.end() - 1) return false;
	return (it + 1)->type == type;
}

bool Scanner::check(TokenType type, std::vector<Token>::iterator& it, std::vector<Token>& vec, int offset)
{
	if (it >= vec.end() - offset) return false;
	return (it + offset)->type == type;
}

std::vector<Token> Scanner::scanTokens()
{
	std::vector<Token> tokens;
	for (Token t = scanToken(); t.type != TokenType::END; t = scanToken())
		tokens.push_back(t);
	tokens.emplace_back(Token{TokenType::END, "", line, currentDepth});

	for (auto it = tokens.begin(); it != tokens.end(); it++)
	{
		switch (it->type)
		{
		case TokenType::BINDE:
		{
			consume(TokenType::STRING, "Es wurde ein Zeichenketten Literal nach 'binde' erwartet!", it, tokens);
			std::string path(it->literal.begin() + 1, it->literal.end() - 1);
			std::vector<Token> otherFile;
			{
				Scanner otherFileScanner(path + ".ddp");
				otherFile = otherFileScanner.scanTokens();
				hadError = otherFileScanner.errored();
			}
			consume(TokenType::EIN, "Es wurde ein 'ein' beim einbinden einer weiteren Datei erwartet!", it, tokens);
			consume(TokenType::DOT, "Es wurde ein '.' nach dem einbinden einer weiteren Datei erwartet!", it, tokens);
			auto after = tokens.erase(it - 3, it + 1);
			auto before = tokens.insert(after, otherFile.begin(), otherFile.end() - 1);
			it = before + otherFile.size();
			break;
		}
		case TokenType::BETRAG:
		{
			consumeErase(TokenType::VON, "Nach 'Betrag' muss 'von' stehen!", it, tokens);
			break;
		}
		case TokenType::LOGISCH:
		{
			if (check(TokenType::NICHT, it, tokens))
			{
				it->type = TokenType::LOGISCHNICHT;
				consumeErase(TokenType::NICHT, "", it, tokens);
			}
			break;
		}
		case TokenType::GROESSER:
		{
			if (check(TokenType::ALS, it, tokens))
			{
				if (check(TokenType::COMMA, it, tokens, 2))
				{
					if (check(TokenType::ODER, it, tokens, 3))
					{
						consumeErase(TokenType::ALS, "", it, tokens);
						consumeErase(TokenType::COMMA, "", it, tokens);
						consumeErase(TokenType::ODER, "", it, tokens);
						(it - 1)->type = TokenType::GROESSERODER;
					}
					else
						error("Nach 'größer als,' muss ein 'oder' stehen!", it->line);
				}
				else
					consumeErase(TokenType::ALS, "", it, tokens);
			}
			else
				error("Nach 'größer' fehlt 'als'!", it->line);
			break;
		}
		case TokenType::KLEINER:
		{
			if (check(TokenType::ALS, it, tokens))
			{
				if (check(TokenType::COMMA, it, tokens, 2))
				{
					if (check(TokenType::ODER, it, tokens, 3))
					{
						consumeErase(TokenType::ALS, "", it, tokens);
						consumeErase(TokenType::COMMA, "", it, tokens);
						consumeErase(TokenType::ODER, "", it, tokens);
						(it - 1)->type = TokenType::KLEINERODER;
					}
					else
						error("Nach 'kleiner als,' muss ein 'oder' stehen!", it->line);
				}
				else
					consumeErase(TokenType::ALS, "", it, tokens);
			}
			else
				error("Nach 'kleiner' fehlt 'als'!", it->line);
			break;
		}
		default:
			break;
		}
	}
	tokens.shrink_to_fit();
	return tokens;
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
				advance();
				advance();
				while (peek() != '*' && peekNext() != '/' && !isAtEnd()) if (advance() == '\n') line++;;
				advance();
				advance();
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

	if (keywords.find(identifier) != keywords.end()) return makeToken(keywords.at(identifier));

	return makeToken(TokenType::IDENTIFIER);
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

	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
		c == 'ß' ||
		c == 'ü' || c == 'ä' || c == 'ö' ||
		c == 'Ü' || c == 'Ä' || c == 'Ö') return true;
	else return false;
}
