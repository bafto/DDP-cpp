#include <iostream>
#include <Windows.h>

#include "VirtualMachine.h"

void runFile(std::string file)
{
	try
	{
		VirtualMachine vm;
		InterpretResult result = vm.interpret(file);
		if (result == InterpretResult::CompilationError) std::cerr << u8"\nA compilation error occured\n";
		else if (result == InterpretResult::RuntimeError) std::cerr << u8"\nA runtime error occured\n";
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
		std::cout << u8"You fucked up!";
	}
}

int main(int argc, char* argv[])
{
	SetConsoleOutputCP(CP_UTF8); //set the code page of the windows console to utf8 (gotta use u8"" for strings from now on)
	//enable buffering if there are problems with the utf8 printing.
	//setvbuf(stdout, nullptr, _IOFBF, 1000);  //!!!If enabled you gotta flush the stream from time to time (with std::endl or std::flush for example)

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