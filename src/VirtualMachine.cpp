#include "VirtualMachine.h"
#include "Compiler.h"
#include <iostream>

VirtualMachine::VirtualMachine(const std::string& filePath, const std::vector<std::string>& sysArgs)
	:
	filePath(filePath)
{
	globals.insert(std::make_pair("System_Argumente", Value(sysArgs)));
}

InterpretResult VirtualMachine::run(bool graphics)
{
	try
	{
		{
			Compiler compiler(filePath, &globals, &functions);
			if (!compiler.compile(graphics)) return InterpretResult::CompileTimeError;
		}
		if (graphics)
		{
			if (functions.count("Update") == 0) throw runtime_error("Ein graphisches DDP Programm muss eine 'Update' Funktion definieren!");
			if (functions.count("Male") == 0) throw runtime_error("Ein graphisches DDP Programm muss eine 'Male' Funktion definieren!");
		}
		functions.at("").run(&globals, &functions);
		if (graphics)
		{
			Function::wnd = new sf::RenderWindow;
			Function::wnd->create(sf::VideoMode(800, 600), "Die Deutsche Programmiersprache");
			Function::wnd->setFramerateLimit(60);

			Function::pixels.resize(800 * 600);
			Function::wndSize = sf::Vector2i(800, 600);

			for (unsigned int x = 0; x < Function::wndSize.x; x++)
			{
				for (unsigned int y = 0; y < Function::wndSize.y; y++)
				{
					Function::pixels[Function::wndSize.x * y + x].position = sf::Vector2f(x, y);
				}
			}

			while (Function::wnd->isOpen())
			{
				sf::Event e;
				while (Function::wnd->pollEvent(e))
				{
					switch (e.type)
					{
					case sf::Event::Closed: Function::wnd->close(); break;
					}
				}

				for (unsigned int i = 0; i < Function::pixels.getVertexCount(); i++)
				{
					Function::pixels[i].color = sf::Color::White;
				}

				functions.at("Update").run(&globals, &functions);
				functions.at("Male").run(&globals, &functions);

				Function::wnd->clear();

				Function::wnd->draw(Function::pixels);

				Function::wnd->display();
			}

			delete Function::wnd;
		}
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
