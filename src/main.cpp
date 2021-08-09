#include "VirtualMachine.h"
#include <iostream>
#include <Windows.h>

int main(int argc, char* argv[])
{
	SetConsoleOutputCP(CP_UTF8); //set the code page of the windows console to utf8 (gotta use u8"" for strings from now on)
	//enable buffering if there are problems with the utf8 printing.
	//setvbuf(stdout, nullptr, _IOFBF, 1000);  //!!!If enabled you gotta flush the stream from time to time (with std::endl or std::flush for example)
}