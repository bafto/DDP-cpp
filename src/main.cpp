#include "VirtualMachine.h"
#include <iostream>
#include <Windows.h>
#include <io.h>
#include <stdlib.h>

bool hasOwnWindow()
{
	return _isatty(_fileno(stdin));
}

void pauseIfWindowOwner() 
{
	if (hasOwnWindow())
		system("pause");
}

int runFile(std::string file, std::vector<std::string> sysArgs)
{
	VirtualMachine vm(file, sysArgs);
	InterpretResult result = vm.run();
	switch (result)
	{
	case InterpretResult::OK: return 0;
	case InterpretResult::CompileTimeError:
		std::cerr << u8"Während dem compilieren des Programms ist ein Fehler aufgetreten!\n";
		pauseIfWindowOwner();
		return 1;
	case InterpretResult::RuntimeError:
		std::cerr << u8"Während dem ausführen des Programms ist ein Fehler aufgetreten!\n";
		pauseIfWindowOwner();
		return 2;
	case InterpretResult::Exception:
		std::cerr << u8"Während dem ausführen des Programms ist eine Ausnahme aufgetreten!\nDas sollte eigentlich nicht vorkommen, bitte melden sie es zusammen mit dem Error-Log einem DDP++ Developer!\n";
		pauseIfWindowOwner();
		return 3;
	default:
		pauseIfWindowOwner();
		return 69;
	}
}

int main(int argc, char* argv[])
{
	if (hasOwnWindow()) {
		SetConsoleOutputCP(CP_UTF8); //set the code page of the windows console to utf8
		SetConsoleCP(CP_UTF8);
	}

	if (!hasOwnWindow())
		std::cout << std::unitbuf;

	switch (argc)
	{
	case 1: std::cout << u8"Usage: ddp++ <filename.ddp>\n"; pauseIfWindowOwner(); break;
	case 2: return runFile(argv[1], {});
	default: return runFile(argv[1], std::vector<std::string>(argv + 2, argv + argc));
	}

	return 0;
}