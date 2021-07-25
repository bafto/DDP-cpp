#include <iostream>

#include "VirtualMachine.h"

void runFile(std::string file)
{
	try
	{
		VirtualMachine vm;
		InterpretResult result = vm.interpret(file);
		if (result == InterpretResult::CompilationError) std::cerr << "A compilation error occured\n";
		else if (result == InterpretResult::RuntimeError) std::cerr << "A runtime error occured\n";
	}
	catch (base_exception& e)
	{
		std::cout << e.what();
	}
	catch (std::exception& e)
	{
		std::cout << e.what();
	}
	catch (...)
	{
		std::cout << "You fucked up!";
	}
}

int main(int argc, char* argv[])
{
	utf8 = true;

	if (argc == 1) /*repl()*/;
	else
	{
		int file = -1;
		for (int i = 0; i < argc; i++)
		{
			std::string arg = argv[i];
			if (arg == "-ascii") utf8 = false;
			else file = i;
		}
		if (file != -1) runFile(std::string(argv[file]));
	}

	return 0;
}