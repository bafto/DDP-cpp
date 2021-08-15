#include "VirtualMachine.h"
#include <iostream>
#include <Windows.h>

int runFile(std::string file, std::vector<std::string> sysArgs)
{
	VirtualMachine vm(file, sysArgs);
	InterpretResult result = vm.run();
	switch (result)
	{
	case InterpretResult::OK: return 0;
	case InterpretResult::CompileTimeError:
		std::cerr << u8"Während dem compilieren des Programms ist ein Fehler aufgetreten!\n";
		return -1;
	case InterpretResult::RuntimeError:
		std::cerr << u8"Während dem ausführen des Programms ist ein Fehler aufgetreten!\n";
		return -2;
	case InterpretResult::Exception:
		std::cerr << u8"Während dem ausführen des Programms ist eine Ausnahme aufgetreten!\nDas sollte eigentlich nicht vorkommen, bitte melden sie es zusammen mit dem Error-Log einem DDP Developer!\n";
		return -3;
	default: return -69;
	}
}



int main(int argc, char* argv[])
{
	SetConsoleOutputCP(CP_UTF8); //set the code page of the windows console to utf8 (gotta use u8"" for strings from now on)
	//enable buffering if there are problems with the utf8 printing.
	//setvbuf(stdout, nullptr, _IOFBF, 1000);  //!!!If enabled you gotta flush the stream from time to time (with std::endl or std::flush for example)



	switch (argc)
	{
	case 1: std::cout << u8"Usage: ddp++ <filename.ddp>\n"; break;
	case 2: return runFile(argv[1], {});
	default: return runFile(argv[1], std::vector<std::string>(argv + 2, argv + argc));
	}

	return 0;
}