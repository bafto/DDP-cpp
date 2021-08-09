#include "Scanner.h"
#include <iostream>

int main(int argc, char* argv[])
{
	Scanner scanner(argv[1]);
	auto pair = scanner.scanTokens();
	if (pair.second)
	{
		for (auto& t : pair.first)
		{
			std::cout << t.line << " " << t.depth << " " << (int)t.type << " " << t.literal << "\n";
		}
	}
}