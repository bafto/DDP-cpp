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
		None		= 0b000000000001,
		Int			= 0b000000000010,
		Double		= 0b000000000100,
		Bool		= 0b000000001000,
		Char		= 0b000000010000,
		String		= 0b000000100000,
		IntArr		= 0b000001000000,
		DoubleArr	= 0b000010000000,
		BoolArr		= 0b000100000000,
		CharArr		= 0b001000000000,
		StringArr	= 0b010000000000,
		Any			= 0b100000000000
	};

	bool ContainsType(CombineableValueType toCheck, ValueType type);

	Value schreibeNative(std::vector<Value> args);
	Value schreibeZeileNative(std::vector<Value> args);
	Value leseNative(std::vector<Value> args);
	Value leseZeileNative(std::vector<Value> args);

	Value clockNative(std::vector<Value> args);

	//casts
	Value zuZahlNative(std::vector<Value> args);
	Value zuKommazahlNative(std::vector<Value> args);
	Value zuBooleanNative(std::vector<Value> args);
	Value zuZeichenNative(std::vector<Value> args);
	Value zuZeichenketteNative(std::vector<Value> args);

	Value LaengeNative(std::vector<Value> args);

	//string manipulation (Laenge could be counted too)
	Value ZuschneidenNative(std::vector<Value> args);
	Value SpaltenNative(std::vector<Value> args);
	Value ErsetzenNative(std::vector<Value> args);
	Value EntfernenNative(std::vector<Value> args);
	Value EinfügenNative(std::vector<Value> args);
	Value EnthältNative(std::vector<Value> args);
	Value BeschneidenNative(std::vector<Value> args);
}