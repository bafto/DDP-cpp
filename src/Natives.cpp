#include "Natives.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <ctime>
#include <thread>
#include <fstream>
#include <streambuf>
#include "Function.h"
#include <filesystem>
#include <random>

#pragma warning (disable : 26812)

namespace Natives
{
	bool ContainsType(CombineableValueType toCheck, ValueType type)
	{
		if ((toCheck & CombineableValueType::Any))
			return true;

		switch (type.type)
		{
		case Type::None: return false;
		case Type::Int: return (toCheck & CombineableValueType::Int);
		case Type::Double: return (toCheck & CombineableValueType::Double);
		case Type::Bool: return (toCheck & CombineableValueType::Bool);
		case Type::Char: return (toCheck & CombineableValueType::Char);
		case Type::String: return (toCheck & CombineableValueType::String);
		case Type::Struct: return (toCheck & CombineableValueType::Struct);
		case Type::IntArr: return (toCheck & CombineableValueType::IntArr);
		case Type::DoubleArr: return (toCheck & CombineableValueType::DoubleArr);
		case Type::BoolArr: return (toCheck & CombineableValueType::BoolArr);
		case Type::CharArr: return (toCheck & CombineableValueType::CharArr);
		case Type::StringArr: return (toCheck & CombineableValueType::StringArr);
		case Type::StructArr: return (toCheck & CombineableValueType::StructArr);
		case Type::Any: return (toCheck & CombineableValueType::Any);
		case Type::Function: return false;
		}
		return false;
	}

	Value schreibeNative(std::vector<Value> args)
	{
		args.at(0).print(std::cout);
		return Value();
	}

	Value schreibeZeileNative(std::vector<Value> args)
	{
		args.at(0).print(std::cout);
		std::cout << "\n";
		return Value();
	}

	Value leseNative(std::vector<Value> args)
	{
		return Value((short)std::cin.get());
	}

	Value leseZeileNative(std::vector<Value> args)
	{
		std::string line;
		std::getline(std::cin, line);
		return Value(line);
	}

	Value existiertDateiNative(std::vector<Value> args)
	{
		std::string path = *args.at(0).String();
		std::ifstream ifs;
		ifs.open(path);
		Value ret = ifs.is_open();
		ifs.close();
		return ret;
	}

	Value leseDateiNative(std::vector<Value> args)
	{
		std::string path = *args.at(0).String();

		std::ifstream ifs;
		ifs.open(path);
		if (!ifs.is_open()) throw runtime_error("Die Datei '" + path + "' konnte nicht geöffnet werden!");

		std::string file = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

		ifs.close();

		return Value(file);
	}

	Value schreibeDateiNative(std::vector<Value> args)
	{
		std::string path = *args.at(0).String();

		std::ofstream ofs(path);
		if (!ofs.is_open()) throw runtime_error("Die Datei '" + path + "' konnte nicht geöffnet werden!");

		args.at(1).print(ofs);

		ofs.close();

		return Value();
	}

	Value bearbeiteDateiNative(std::vector<Value> args)
	{
		std::string path = *args.at(0).String();

		if (!std::filesystem::exists(path)) throw runtime_error("Die Datie '" + path + "' existiert nicht und kann somit nicht bearbeitet werden!");

		std::ofstream ofs;
		ofs.open(path, std::ofstream::app);
		if (!ofs.is_open()) throw runtime_error("Die Datei '" + path + "' konnte nicht geöffnet werden!");

		args.at(1).print(ofs);

		ofs.close();

		return Value();
	}

	Value leseBytesNative(std::vector<Value> args)
	{
		std::string path = *args.at(0).String();

		std::ifstream ifs;
		ifs.open(path, std::ios::binary);
		if (!ifs.is_open()) throw runtime_error("Die Datei '" + path + "' konnte nicht geöffnet werden!");
		ifs.unsetf(std::ios::skipws);

		ifs.seekg(0, std::ios::end);
		std::streampos fileSize = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		std::vector<uint8_t> file;
		file.reserve(fileSize);

		file.insert(file.begin(), std::istream_iterator<uint8_t>(ifs), std::istream_iterator<uint8_t>());

		ifs.close();

		std::vector<int> ints;
		ints.resize(file.size());
		std::copy(file.begin(), file.end(), ints.begin());

		return Value(ints);
	}

	Value schreibeBytesNative(std::vector<Value> args)
	{
		std::string path = *args.at(0).String();

		std::ofstream ofs(path, std::ios::binary);
		if (!ofs.is_open()) throw runtime_error("Die Datei '" + path + "' konnte nicht geöffnet werden!");

		std::vector<int> ints = *args.at(1).IntArr();
		for (auto& i : ints)
			ofs << (uint8_t)i;

		ofs.close();

		return Value();
	}

	Value bearbeiteBytesNative(std::vector<Value> args)
	{
		std::string path = *args.at(0).String();

		if (!std::filesystem::exists(path)) throw runtime_error("Die Datie '" + path + "' existiert nicht und kann somit nicht bearbeitet werden!");

		std::ofstream ofs;
		ofs.open(path, std::ofstream::app | std::ios::binary);
		if (!ofs.is_open()) throw runtime_error("Die Datei '" + path + "' konnte nicht geöffnet werden!");

		std::vector<int> ints = *args.at(1).IntArr();
		for (auto& i : ints)
			ofs << (uint8_t)i;

		ofs.close();

		return Value();
	}

	Value clockNative(std::vector<Value> args)
	{
		return Value((double)clock() / (double)CLOCKS_PER_SEC);
	}

	Value warteNative(std::vector<Value> args)
	{
		double seconds = args.at(0).Double();
		std::this_thread::sleep_for(std::chrono::milliseconds((int)(seconds * 1000)));
		return Value();
	}

	Value zuZahlNative(std::vector<Value> args)
	{
		try
		{
			switch (args.at(0).type())
			{
			case Type::Int: return args.at(0);
			case Type::Double: return Value((int)args.at(0).Double());
			case Type::Bool: return Value(args.at(0).Bool() ? 1 : 0);
			case Type::Char: return Value((int)args.at(0).Char());
			case Type::String: return Value(std::stoi(*args.at(0).String()));
			}
		}
		catch (std::exception&)
		{
			throw runtime_error("Diese Zeichenkette kann nicht in eine Zahl umgewandelt werden!");
		}
		return Value();
	}

	Value zuKommazahlNative(std::vector<Value> args)
	{
		try
		{
			switch (args.at(0).type())
			{
			case Type::Int: return Value((double)args.at(0).Int());
			case Type::Double: return args.at(0);
			case Type::Bool: return Value(args.at(0).Bool() ? 1.0 : 0.0);
			case Type::Char: return Value((double)args.at(0).Char());
			case Type::String:
			{
				std::string str = *args.at(0).String();
				std::replace(str.begin(), str.end(), ',', '.');
				return Value(std::stod(str));
			}
			}
		}
		catch (std::exception&)
		{
			throw runtime_error("Diese Zeichenkette kann nicht in eine Kommazahl umgewandelt werden!");
		}
		return Value();
	}

	Value zuBooleanNative(std::vector<Value> args)
	{
		switch (args.at(0).type())
		{
		case Type::Int: return Value((bool)args.at(0).Int());
		case Type::Double: return Value(args.at(0).Double() == 0.0);
		case Type::Bool: return args.at(0);
		case Type::Char:
		{
			if (args.at(0).Char() == (short)'w') return Value(true);
			else return Value(false);
		}
		case Type::String:
		{
			std::string str = *args.at(0).String();
			if (str == "wahr") return Value(true);
			else if (str == "falsch") return Value(false);
			else throw runtime_error("Diese Zeichenkette kann nicht in einen Boolean umgewandelt werden!");
		}
		}
		return Value();
	}

	Value zuBuchstabeNative(std::vector<Value> args)
	{
		switch (args.at(0).type())
		{
		case Type::Int: return Value((short)args.at(0).Int());
		case Type::Double: return Value((short)args.at(0).Double());
		case Type::Bool: return Value(args.at(0).Bool() ? (short)'w' : (short)'f');
		case Type::Char: return args.at(0).Char();
		case Type::String:
		{
			char a = args.at(0).String()->at(0);
			char b = args.at(0).String()->length() > 1 ? args.at(0).String()->at(1) : 0;
			if (a >= 32 && a <= 126 || a == '\n' || a == '\t' || a == '\r') {
				return Value((short)a);
			}
			return Value((short)((((short)a) << 8) | (0x00ff & b)));
		}
		}
		return Value();
	}

	Value zuTextNative(std::vector<Value> args)
	{
		std::stringstream ss;
		args.at(0).print(ss);
		return Value(ss.str());
	}

	Value LaengeNative(std::vector<Value> args)
	{
		switch (args.at(0).type())
		{
		case Type::String: return Value((int)(*args.at(0).String()).length());
		case Type::IntArr: return Value((int)args.at(0).IntArr()->size());
		case Type::DoubleArr: return Value((int)args.at(0).DoubleArr()->size());
		case Type::BoolArr: return Value((int)args.at(0).BoolArr()->size());
		case Type::CharArr: return Value((int)args.at(0).CharArr()->size());
		case Type::StringArr: return Value((int)args.at(0).StringArr()->size());
		}
		return Value(-1);
	}

	Value ZuschneidenNative(std::vector<Value> args)
	{
		std::string str = *args.at(0).String();
		int start = args.at(1).Int();
		int length = args.at(2).Int();

		return Value(str.substr(start, length));
	}

	Value SpaltenNative(std::vector<Value> args)
	{
		std::string str = *args.at(0).String();
		std::string delimiter;
		switch (args.at(1).type())
		{
		case Type::Char: delimiter = Value::U8CharToString(args.at(1).Char()); break;
		case Type::String: delimiter = *args.at(1).String(); break;
		}
		size_t pos = 0;
		std::vector<std::string> tokens;
		while ((pos = str.find(delimiter)) != std::string::npos)
		{
			tokens.push_back(str.substr(0, pos));
			str.erase(0, pos + delimiter.length());
		}
		tokens.push_back(str);
		return Value(std::move(tokens));
	}

	Value ErsetzenNative(std::vector<Value> args)
	{
		std::string str = *args.at(0).String();
		std::string from;
		std::string to;
		switch (args.at(1).type())
		{
		case Type::Char: from = Value::U8CharToString(args.at(1).Char()); break;
		case Type::String: from = *args.at(1).String(); break;
		}
		switch (args.at(2).type())
		{
		case Type::Char: to = Value::U8CharToString(args.at(2).Char()); break;
		case Type::String: to = *args.at(2).String(); break;
		}

		if (from.empty())
			return Value(str);
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
		
		return Value(str);
	}

	Value EntfernenNative(std::vector<Value> args)
	{
		std::string str = *args.at(0).String();
		int start = args.at(1).Int();
		int length = args.at(2).Int();

		if (start + length > str.length())
			str.erase(str.begin() + start, str.end());
		else
			str.erase(str.begin() + start, str.begin() + start + length);
		return Value(str);
	}

	Value EinfügenNative(std::vector<Value> args)
	{
		std::string str = *args.at(0).String();
		int pos = args.at(1).Int();
		std::string in = *args.at(2).String();

		if (pos > (int)str.length())
			str.insert(str.length(), in);
		else if (pos < 0)
			str.insert(0, in);
		else
			str.insert(pos, in);
		return Value(str);
	}

	Value EnthältNative(std::vector<Value> args)
	{
		std::string str = *args.at(0).String();
		std::string x;
		switch (args.at(1).type())
		{
		case Type::Char: x = Value::U8CharToString(args.at(1).Char()); break;
		case Type::String: x = *args.at(1).String(); break;
		}
		return Value(str.find(x) != std::string::npos);
	}

	Value BeschneidenNative(std::vector<Value> args)
	{
		std::string s = *args.at(0).String();
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
			return !std::isspace(ch);
			}));
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
			}).base(), s.end());
		return Value(s);
	}

	Value Max(std::vector<Value> args)
	{
		switch (args.at(0).type())
		{
		case Type::Int:
		{
			switch (args.at(1).type())
			{
			case Type::Int: return Value(std::max((double)args.at(0).Int(), (double)args.at(1).Int()));
			case Type::Double: return Value(std::max((double)args.at(0).Int(), args.at(1).Double()));
			}
		}
		case Type::Double:
		{
			switch (args.at(1).type())
			{
			case Type::Int: return Value(std::max(args.at(0).Double(), (double)args.at(1).Int()));
			case Type::Double: return Value(std::max(args.at(0).Double(), args.at(1).Double()));
			}
		}
		}
		return Value();
	}

	Value Min(std::vector<Value> args)
	{
		switch (args.at(0).type())
		{
		case Type::Int:
		{
			switch (args.at(1).type())
			{
			case Type::Int: return Value(std::min((double)args.at(0).Int(), (double)args.at(1).Int()));
			case Type::Double: return Value(std::min((double)args.at(0).Int(), args.at(1).Double()));
			}
		}
		case Type::Double:
		{
			switch (args.at(1).type())
			{
			case Type::Int: return Value(std::min(args.at(0).Double(), (double)args.at(1).Int()));
			case Type::Double: return Value(std::min(args.at(0).Double(), args.at(1).Double()));
			}
		}
		}
		return Value();
	}

	Value Clamp(std::vector<Value> args)
	{
		switch (args.at(0).type())
		{
		case Type::Int:
		{
			switch (args.at(1).type())
			{
			case Type::Int:
			{
				switch (args.at(2).type())
				{
				case Type::Int: return Value((double)(std::clamp(args.at(0).Int(), args.at(1).Int(), args.at(2).Int())));
				case Type::Double: return Value(std::clamp((double)args.at(0).Int(), (double)args.at(1).Int(), args.at(2).Double()));
				}
			}
			case Type::Double:
			{
				switch (args.at(2).type())
				{
				case Type::Int: return Value(std::clamp((double)args.at(0).Int(), args.at(1).Double(), (double)args.at(2).Int()));
				case Type::Double: return Value(std::clamp((double)args.at(0).Int(), args.at(1).Double(), args.at(2).Double()));
				}
			}
			}
		}
		case Type::Double:
		{
			switch (args.at(1).type())
			{
			case Type::Int:
			{
				switch (args.at(2).type())
				{
				case Type::Int: return Value(std::clamp(args.at(0).Double(), (double)args.at(1).Int(), (double)args.at(2).Int()));
				case Type::Double: return Value(std::clamp(args.at(0).Double(), (double)args.at(1).Int(), args.at(2).Double()));
				}
			}
			case Type::Double:
			{
				switch (args.at(2).type())
				{
				case Type::Int: return Value(std::clamp(args.at(0).Double(), args.at(1).Double(), (double)args.at(2).Int()));
				case Type::Double: return Value(std::clamp(args.at(0).Double(), args.at(1).Double(), args.at(2).Double()));
				}
			}
			}
		}
		}
		return Value();
	}

	Value Trunkiert(std::vector<Value> args)
	{
		return Value(std::trunc(args.at(0).Double()));
	}

	Value Rund(std::vector<Value> args)
	{
		return Value(std::round(args.at(0).Double()));
	}

	Value Decke(std::vector<Value> args)
	{
		return Value(std::ceil(args.at(0).Double()));
	}

	Value Boden(std::vector<Value> args)
	{
		return Value(std::floor(args.at(0).Double()));
	}

	Value ZufaelligeZahlNative(std::vector<Value> args)
	{
		int v1 = args.at(0).Int();
		int v2 = args.at(1).Int();

		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution uni(v1, v2);
		return Value(uni(rng));
	}

	Value ZufaelligeKommazahlNative(std::vector<Value> args)
	{
		double v1 = args.at(0).Double();
		double v2 = args.at(1).Double();

		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<double> uni(v1, v2);
		return Value(uni(rng));
	}


	//graphics functions

	Value ErstelleFenster(std::vector<Value> args)
	{
		std::vector<int> size = *args.at(0).IntArr();
		if (size.size() < 2) throw runtime_error(u8"Beim erstellen eines Fensters müssen 2 Werte für die Größe vorhanden sein!");
		if (size.at(0) < 0 || size.at(1) < 0) throw runtime_error(u8"Ein Fenster kann keine negative Größe haben!");

		std::string wndTitle = *args.at(1).String();

		gfx::wndSize = sf::Vector2i(size.at(0), size.at(1));

		delete gfx::wnd;
		gfx::wnd = new sf::RenderWindow(sf::VideoMode(gfx::wndSize.x, gfx::wndSize.y), wndTitle, sf::Style::Titlebar | sf::Style::Close);
		gfx::wnd->setFramerateLimit(140);
		gfx::pixels.create(gfx::wndSize.x, gfx::wndSize.y);
		gfx::tex.create(gfx::wndSize.x, gfx::wndSize.y);
		gfx::sprite.setTexture(gfx::tex);

		return Value();
	}

	Value SchliesseFenster(std::vector<Value> args)
	{
		if (gfx::wnd != nullptr)
			gfx::wnd->close();
		return Value();
	}

	Value FensterOffen(std::vector<Value> args)
	{
		if (gfx::wnd == nullptr)
			return Value(false);
		return Value(gfx::wnd->isOpen());
	}

	Value MaleRechteck(std::vector<Value> args)
	{
		std::vector<int> pos = *args.at(0).IntArr();
		std::vector<int> size = *args.at(1).IntArr();
		std::vector<int> color = *args.at(2).IntArr();

		if (pos.size() < 2) throw runtime_error(u8"Die Position eines Rechtecks muss mindestens 2 Werte enthalten!");
		if (size.size() < 2) throw runtime_error(u8"Die Größe eines Rechtecks muss mindestens 2 Werte enthalten!");
		if (color.size() < 3) throw runtime_error(u8"Eine Farbe muss mindestens 3 Werte enthalten!");

		for (int x = pos.at(0); x < pos.at(0) + size.at(0); x++)
		{
			for (int y = pos.at(1); y < pos.at(1) + size.at(1); y++)
			{
				if (x >= 0 && x <= gfx::wndSize.x - 1 &&
					y >= 0 && y <= gfx::wndSize.y - 1)
					gfx::pixels.setPixel(x, y, sf::Color(color.at(0), color.at(1), color.at(2)));
			}
		}

		return Value();
	}

	Value TasteGedrueckt(std::vector<Value> args)
	{
		std::string key = *args.at(0).String();

		if (gfx::keyMap.count(key) == 0)
			return Value(false);

		return Value(sf::Keyboard::isKeyPressed(gfx::keyMap.at(key)));
	}

	Value AktualisiereFenster(std::vector<Value> args)
	{
		if (gfx::wnd == nullptr)
			return Value();

		sf::Event e;
		while (gfx::wnd->pollEvent(e))
		{
			switch (e.type)
			{
			case sf::Event::Closed: gfx::wnd->close();
			}
		}

		gfx::wnd->clear();

		gfx::tex.loadFromImage(gfx::pixels);
		gfx::wnd->draw(gfx::sprite);

		gfx::wnd->display();

		for (int x = 0; x < gfx::wndSize.x; x++)
		{
			for (int y = 0; y < gfx::wndSize.y; y++)
			{
				gfx::pixels.setPixel(x, y, sf::Color::White);
			}
		}

		return Value();
	}
}