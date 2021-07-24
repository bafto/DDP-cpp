#include <iostream>

#include "VirtualMachine.h"

int main(int argc, char* argv[])
{
	try
	{
		VirtualMachine vm;
		vm.interpret("");
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

	return 0;
}