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
}

Token Scanner::scanToken()
{
	skipWhitespaces();
	return Token();
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
	token.literal = std::string(current, start);
	return Token();
}

Token Scanner::errorToken(std::string msg) const
{
	return Token();
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
