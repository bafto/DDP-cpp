#include "VirtualMachine.h"

VirtualMachine::VirtualMachine(const std::string& filePath, const std::vector<std::string>& sysArgs)
	:
	filePath(filePath)
{
	globals.insert(make_pair("System_Argumente", sysArgs));
}

void VirtualMachine::run()
{
	/*{
		Compiler compiler(filePath);
		if(!compiler.compile(&globals, &functions))
		{
			error
		}
	}*/
	functions.at("").run(&globals, &functions);
}
