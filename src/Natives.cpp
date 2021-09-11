#include "Natives.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <ctime>
#include <thread>
#include <fstream>
#include <streambuf>
#include <filesystem>

#pragma warning (disable : 26812)

namespace Natives
{
	bool ContainsType(CombineableValueType toCheck, ValueType type)
	{
		if ((toCheck & CombineableValueType::Any))
			return true;

		switch (type)
		{
		case ValueType::None: return false;
		case ValueType::Int: return (toCheck & CombineableValueType::Int);
		case ValueType::Double: return (toCheck & CombineableValueType::Double);
		case ValueType::Bool: return (toCheck & CombineableValueType::Bool);
		case ValueType::Char: return (toCheck & CombineableValueType::Char);
		case ValueType::String: return (toCheck & CombineableValueType::String);
		case ValueType::IntArr: return (toCheck & CombineableValueType::IntArr);
		case ValueType::DoubleArr: return (toCheck & CombineableValueType::DoubleArr);
		case ValueType::BoolArr: return (toCheck & CombineableValueType::BoolArr);
		case ValueType::CharArr: return (toCheck & CombineableValueType::CharArr);
		case ValueType::StringArr: return (toCheck & CombineableValueType::StringArr);
		case ValueType::Any: return (toCheck & CombineableValueType::Any);
		case ValueType::Function: return false;
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
			switch (args.at(0).Type())
			{
			case ValueType::Int: return args.at(0);
			case ValueType::Double: return Value((int)args.at(0).Double());
			case ValueType::Bool: return Value(args.at(0).Bool() ? 1 : 0);
			case ValueType::Char: return Value((int)args.at(0).Char());
			case ValueType::String: return Value(std::stoi(*args.at(0).String()));
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
			switch (args.at(0).Type())
			{
			case ValueType::Int: return Value((double)args.at(0).Int());
			case ValueType::Double: return args.at(0);
			case ValueType::Bool: return Value(args.at(0).Bool() ? 1.0 : 0.0);
			case ValueType::Char: return Value((double)args.at(0).Char());
			case ValueType::String:
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
		switch (args.at(0).Type())
		{
		case ValueType::Int: return Value((bool)args.at(0).Int());
		case ValueType::Double: return Value(args.at(0).Double() == 0.0);
		case ValueType::Bool: return args.at(0);
		case ValueType::Char:
		{
			if (args.at(0).Char() == (short)'w') return Value(true);
			else return Value(false);
		}
		case ValueType::String:
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
		switch (args.at(0).Type())
		{
		case ValueType::Int: return Value((short)args.at(0).Int());
		case ValueType::Double: return Value((short)args.at(0).Double());
		case ValueType::Bool: return Value(args.at(0).Bool() ? (short)'w' : (short)'f');
		case ValueType::Char: return args.at(0).Char();
		case ValueType::String:
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
		switch (args.at(0).Type())
		{
		case ValueType::String: return Value((int)(*args.at(0).String()).length());
		case ValueType::IntArr: return Value((int)args.at(0).IntArr()->size());
		case ValueType::DoubleArr: return Value((int)args.at(0).DoubleArr()->size());
		case ValueType::BoolArr: return Value((int)args.at(0).BoolArr()->size());
		case ValueType::CharArr: return Value((int)args.at(0).CharArr()->size());
		case ValueType::StringArr: return Value((int)args.at(0).StringArr()->size());
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
		switch (args.at(1).Type())
		{
		case ValueType::Char: delimiter = Value::U8CharToString(args.at(1).Char()); break;
		case ValueType::String: delimiter = *args.at(1).String(); break;
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
		switch (args.at(1).Type())
		{
		case ValueType::Char: from = Value::U8CharToString(args.at(1).Char()); break;
		case ValueType::String: from = *args.at(1).String(); break;
		}
		switch (args.at(2).Type())
		{
		case ValueType::Char: to = Value::U8CharToString(args.at(2).Char()); break;
		case ValueType::String: to = *args.at(2).String(); break;
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
		switch (args.at(1).Type())
		{
		case ValueType::Char: x = Value::U8CharToString(args.at(1).Char()); break;
		case ValueType::String: x = *args.at(1).String(); break;
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
		switch (args.at(0).Type())
		{
		case ValueType::Int:
		{
			switch (args.at(1).Type())
			{
			case ValueType::Int: return Value(std::max((double)args.at(0).Int(), (double)args.at(1).Int()));
			case ValueType::Double: return Value(std::max((double)args.at(0).Int(), args.at(1).Double()));
			}
		}
		case ValueType::Double:
		{
			switch (args.at(1).Type())
			{
			case ValueType::Int: return Value(std::max(args.at(0).Double(), (double)args.at(1).Int()));
			case ValueType::Double: return Value(std::max(args.at(0).Double(), args.at(1).Double()));
			}
		}
		}
		return Value();
	}

	Value Min(std::vector<Value> args)
	{
		switch (args.at(0).Type())
		{
		case ValueType::Int:
		{
			switch (args.at(1).Type())
			{
			case ValueType::Int: return Value(std::min((double)args.at(0).Int(), (double)args.at(1).Int()));
			case ValueType::Double: return Value(std::min((double)args.at(0).Int(), args.at(1).Double()));
			}
		}
		case ValueType::Double:
		{
			switch (args.at(1).Type())
			{
			case ValueType::Int: return Value(std::min(args.at(0).Double(), (double)args.at(1).Int()));
			case ValueType::Double: return Value(std::min(args.at(0).Double(), args.at(1).Double()));
			}
		}
		}
		return Value();
	}

	Value Clamp(std::vector<Value> args)
	{
		switch (args.at(0).Type())
		{
		case ValueType::Int:
		{
			switch (args.at(1).Type())
			{
			case ValueType::Int:
			{
				switch (args.at(2).Type())
				{
				case ValueType::Int: return Value((double)(std::clamp(args.at(0).Int(), args.at(1).Int(), args.at(2).Int())));
				case ValueType::Double: return Value(std::clamp((double)args.at(0).Int(), (double)args.at(1).Int(), args.at(2).Double()));
				}
			}
			case ValueType::Double:
			{
				switch (args.at(2).Type())
				{
				case ValueType::Int: return Value(std::clamp((double)args.at(0).Int(), args.at(1).Double(), (double)args.at(2).Int()));
				case ValueType::Double: return Value(std::clamp((double)args.at(0).Int(), args.at(1).Double(), args.at(2).Double()));
				}
			}
			}
		}
		case ValueType::Double:
		{
			switch (args.at(1).Type())
			{
			case ValueType::Int:
			{
				switch (args.at(2).Type())
				{
				case ValueType::Int: return Value(std::clamp(args.at(0).Double(), (double)args.at(1).Int(), (double)args.at(2).Int()));
				case ValueType::Double: return Value(std::clamp(args.at(0).Double(), (double)args.at(1).Int(), args.at(2).Double()));
				}
			}
			case ValueType::Double:
			{
				switch (args.at(2).Type())
				{
				case ValueType::Int: return Value(std::clamp(args.at(0).Double(), args.at(1).Double(), (double)args.at(2).Int()));
				case ValueType::Double: return Value(std::clamp(args.at(0).Double(), args.at(1).Double(), args.at(2).Double()));
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

}