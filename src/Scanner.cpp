#include "Scanner.h"
#include <fstream>
#include <streambuf>
#include <iostream>

using namespace std::string_literals;

Scanner::Scanner(const std::string& file)
	:
	filePath(file),
	line(1),
	depth(0),
	hadError(false)
{
	std::ifstream ifs;
	ifs.open(file);
	if (!ifs.is_open())
	{
		std::cerr << u8"Could not open the source file '" << file << "'!\n";
		hadError = true;
	}
	source = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	source.push_back('\0');
	start = source.begin();
	current = source.begin();
}

void Scanner::error(const std::string& msg, int line)
{
	std::cerr << u8"[Line " << line << u8"] " << msg << "\n";
	hadError = true;
}

void Scanner::consume(TokenType type, const std::string& msg, std::vector<Token>::iterator& it, std::vector<Token>& vec)
{
	if (it >= vec.end() - 1)
	{
		error(msg, (it - 1)->line);
		return;
	}
	if ((it + 1)->type != type)
	{
		error(msg, it->line);
		return;
	}
	it++;
}

void Scanner::consumeErase(TokenType type, const std::string& msg, std::vector<Token>::iterator& it, std::vector<Token>& vec)
{
	consume(type, msg, it, vec);
	it = vec.erase(it) - 1; //set it to the element before the one that was erased
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

std::pair<std::vector<Token>, bool> Scanner::scanTokens()
{
	std::vector<Token> tokens;
	for (Token t = scanToken(); t.type != TokenType::END; t = scanToken())
		tokens.push_back(t);
	tokens.emplace_back(Token{ TokenType::END, "", depth, line });

	for (auto it = tokens.begin(); it != tokens.end(); it++)
	{
		switch (it->type)
		{
		case TokenType::BINDE:
		{
			consume(TokenType::STRING, u8"Es wurde ein Text Literal nach 'binde' erwartet!", it, tokens);
			std::string path(it->literal.begin() + 1, it->literal.end() - 1);
			std::vector<Token> otherFile;
			{
				Scanner otherFileScanner(path + ".ddp");
				auto other = otherFileScanner.scanTokens();
				if (!other.second)
					error(u8"Could not open the source file '" + otherFileScanner.filePath + "'!", it->line);
				otherFile = std::move(other.first);
			}
			consume(TokenType::EIN, u8"Es wurde ein 'ein' beim einbinden einer weiteren Datei erwartet!", it, tokens);
			consume(TokenType::DOT, u8"Es wurde ein '.' nach dem einbinden einer weiteren Datei erwartet!", it, tokens);
			auto after = tokens.erase(it - 3, it + 1);
			auto before = tokens.insert(after, otherFile.begin(), otherFile.end() - 1);
			it = before + otherFile.size() - 2;
			break;
		}
		case TokenType::BETRAG:
		{
			consumeErase(TokenType::VON, u8"Nach 'Betrag' muss 'von' stehen!", it, tokens);
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
						it->type = TokenType::GROESSERODER;
					}
					else
						error(u8"Nach 'größer als,' muss ein 'oder' stehen!", it->line);
				}
				else
					consumeErase(TokenType::ALS, "", it, tokens);
			}
			else
				error(u8"Nach 'größer' fehlt 'als'!", it->line);
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
						it->type = TokenType::KLEINERODER;
					}
					else
						error(u8"Nach 'kleiner als,' muss ein 'oder' stehen!", it->line);
				}
				else
					consumeErase(TokenType::ALS, "", it, tokens);
			}
			else
				error(u8"Nach 'kleiner' fehlt 'als'!", it->line);
			break;
		}
		case TokenType::INUMBER:
		{
			if (check(TokenType::DOT, it, tokens))
			{
				if (check(TokenType::WURZEL, it, tokens, 2))
				{
					consumeErase(TokenType::DOT, "", it, tokens);
					it++;
					consumeErase(TokenType::VON, u8"Es wurde ein 'von' nach 'wurzel' erwartet!", it, tokens);
				}
			}
			break;
		}
		case TokenType::AN:
		{
			consumeErase(TokenType::DER, u8"Nach 'an' wurde 'der' erwartet!", it, tokens);
			consumeErase(TokenType::STELLE, u8"Nach, 'der' wurde 'Stelle' erwartet!", it, tokens);
			break;
		}
		default:
			break;
		}
	}
	tokens.shrink_to_fit();
	return std::make_pair(tokens, !hadError);
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
	case '[': return makeToken(TokenType::LEFT_SQAREBRACKET);
	case ']': return makeToken(TokenType::RIGHT_SQAREBRACKET);
	case ';': return makeToken(TokenType::SEMICOLON);
#ifndef NDEBUG
	case '$': return makeToken(TokenType::PRINT);
#endif
	}

	return errorToken(u8"Unerwartetes Zeichen");
}

bool Scanner::isAtEnd() const
{
	return *current == '\0';
}

Token Scanner::makeToken(TokenType type) const
{
	Token token;
	token.depth = depth;
	token.type = type;
	token.line = line;
	token.literal = std::string(start, current);
	return token;
}

Token Scanner::errorToken(std::string msg) const
{
	Token token;
	token.type = TokenType::ERROR;
	token.depth = depth;
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
				depth++;
				consecutiveSpaceCount = 0;
			}
			__fallthrough;
		case '\r':
			advance();
			break;
		case '\t':
			depth++;
			advance();
			break;
		case '\n':
			line++;
			depth = 0;
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
		if (peek() == '\\')
		{
			switch (peekNext())
			{
			case '\\':
				source.replace(current, current + 2, "\\");
				break;
			case '\"':
				source.replace(current, current + 2, "\"");
				break;
			case 'n':
				source.replace(current, current + 2, "\n");
				break;
			case 't':
				source.replace(current, current + 2, "\t");
				break;
			case 'r':
				source.replace(current, current + 2, "\r");
				break;
			default:
				return errorToken(u8"Unfertige Escape-Sequenz!");
				break;
			}
		}
		if (peek() == '\n') line++;
		advance();
	}
	if (isAtEnd()) return errorToken(u8"Unfertiger Text!");

	advance();
	return makeToken(TokenType::STRING);
}

Token Scanner::character()
{
	while (peek() != '\'' && !isAtEnd()) {
		if (peek() == '\\')
		{
			switch (peekNext())
			{
			case '\\':
				source.replace(current, current + 2, "\\");
				break;
			case '\'':
				source.replace(current, current + 2, "\'");
				break;
			case 'n':
				source.replace(current, current + 2, "\n");
				break;
			case 't':
				source.replace(current, current + 2, "\t");
				break;
			case 'r':
				source.replace(current, current + 2, "\r");
				break;
			default:
				return errorToken(u8"Unfertige Escape-Sequenz!");
				break;
			}
		}
		if (peek() == '\n') line++;
		advance();
	}
	advance();
	Token tok = makeToken(TokenType::CHARACTER);
	if (tok.literal.length() > 4 || (tok.literal.length() == 4 && tok.literal[1] != (char)0xc3)) return errorToken("Zu langes Buchstaben Literal!");
	return tok;
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
