#include "VirtualMachine.h"
#include "Compiler.h"
#include <iostream>

VirtualMachine::VirtualMachine(const std::string& filePath, const std::vector<std::string>& sysArgs)
	:
	filePath(filePath)
{
	globals.insert(std::make_pair("System_Argumente", Value(sysArgs)));
}

InterpretResult VirtualMachine::run()
{
	try
	{
		{
			Compiler compiler(filePath, &globals, &functions, &structs);
			if (!compiler.compile()) return InterpretResult::CompileTimeError;
		}
		functions.at("").run(&globals, &functions, &structs);
	}
	catch (runtime_error& err)
	{
		std::cerr << u8"[runtime error] " << err.what() << "\n";
		return InterpretResult::RuntimeError;
	}
	catch (std::exception& e)
	{
		std::cerr << "[standard exception] " << e.what() << "\n";
		return InterpretResult::Exception;
	}
	catch (...)
	{
		std::cerr << "Something went badly wrong!\n";
		return InterpretResult::Exception;
	}
	return InterpretResult::OK;
}
