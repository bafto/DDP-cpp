#include "VirtualMachine.h"
#include "Compiler.h"
#include <iostream>

VirtualMachine::VirtualMachine(const std::string& filePath, const std::vector<std::string>& sysArgs)
	:
	filePath(filePath)
{
	globals.insert(make_pair("System_Argumente", sysArgs));
}

InterpretResult VirtualMachine::run()
{
	try
	{
		{
			Compiler compiler(filePath, &globals, &functions);
			if (!compiler.compile()) return InterpretResult::CompileTimeError;
		}
		functions.at("").run(&globals, &functions);
	}
	catch (std::exception& e)
	{
		std::cerr << "Standard Exception: " << e.what() << "\n";
		return InterpretResult::Exception;
	}
	catch (...)
	{
		std::cerr << "Something went badly wrong!\n";
		return InterpretResult::Exception;
	}
	return InterpretResult::OK;
}
