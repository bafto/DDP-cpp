#pragma once

#include "Value.h"

class runtime_error : public std::exception
{
public:
	runtime_error(std::string msg) : std::exception(msg.c_str()) {};
};

namespace Natives
{
	enum CombineableValueType
	{
		None		= 0b00000000000001,
		Int			= 0b00000000000010,
		Double		= 0b00000000000100,
		Bool		= 0b00000000001000,
		Char		= 0b00000000010000,
		String		= 0b00000000100000,
		Struct		= 0b00000001000000,
		IntArr		= 0b00000010000000,
		DoubleArr	= 0b00000100000000,
		BoolArr		= 0b00001000000000,
		CharArr		= 0b00010000000000,
		StringArr	= 0b00100000000000,
		StructArr	= 0b01000000000000,
		Any			= 0b10000000000000
	};

	bool ContainsType(CombineableValueType toCheck, ValueType type);

	Value schreibeNative(std::vector<Value> args);
	Value schreibeZeileNative(std::vector<Value> args);
	Value leseNative(std::vector<Value> args);
	Value leseZeileNative(std::vector<Value> args);

	Value existiertDateiNative(std::vector<Value> args);
	Value leseDateiNative(std::vector<Value> args);
	Value schreibeDateiNative(std::vector<Value> args);
	Value bearbeiteDateiNative(std::vector<Value> args);
	Value leseBytesNative(std::vector<Value> args);
	Value schreibeBytesNative(std::vector<Value> args);
	Value bearbeiteBytesNative(std::vector<Value> args);

	Value clockNative(std::vector<Value> args);
	Value warteNative(std::vector<Value> args);

	//casts
	Value zuZahlNative(std::vector<Value> args);
	Value zuKommazahlNative(std::vector<Value> args);
	Value zuBooleanNative(std::vector<Value> args);
	Value zuBuchstabeNative(std::vector<Value> args);
	Value zuTextNative(std::vector<Value> args);

	Value LaengeNative(std::vector<Value> args);

	//string manipulation (Laenge could be counted too)
	Value ZuschneidenNative(std::vector<Value> args);
	Value SpaltenNative(std::vector<Value> args);
	Value ErsetzenNative(std::vector<Value> args);
	Value EntfernenNative(std::vector<Value> args);
	Value EinfügenNative(std::vector<Value> args);
	Value EnthältNative(std::vector<Value> args);
	Value BeschneidenNative(std::vector<Value> args);

	//math stuff
	Value Max(std::vector<Value> args);
	Value Min(std::vector<Value> args);
	Value Clamp(std::vector<Value> args);
	Value Trunkiert(std::vector<Value> args);
	Value Rund(std::vector<Value> args);
	Value Decke(std::vector<Value> args);
	Value Boden(std::vector<Value> args);

}