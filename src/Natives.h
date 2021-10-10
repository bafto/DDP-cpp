#pragma once

#include "Value.h"

#include "SFML/System.hpp"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

class runtime_error : public std::exception
{
public:
	runtime_error(std::string msg) : std::exception(msg.c_str()) {};
};

//sfml doesn't like static stuff
class Gfx
{
public:
	sf::RenderWindow* wnd = nullptr;
	sf::Vector2i wndSize;

	sf::Image pixels;
	sf::Texture tex;
	sf::Sprite sprite;

	static inline std::unordered_map<std::string, sf::Keyboard::Key> keyMap = std::unordered_map<std::string, sf::Keyboard::Key>{
		{"A", sf::Keyboard::Key::A},
		{"B", sf::Keyboard::Key::B},
		{"C", sf::Keyboard::Key::C},
		{"D", sf::Keyboard::Key::D},
		{"E", sf::Keyboard::Key::E},
		{"F", sf::Keyboard::Key::F},
		{"G", sf::Keyboard::Key::G},
		{"H", sf::Keyboard::Key::H},
		{"I", sf::Keyboard::Key::I},
		{"J", sf::Keyboard::Key::J},
		{"K", sf::Keyboard::Key::K},
		{"L", sf::Keyboard::Key::L},
		{"M", sf::Keyboard::Key::M},
		{"N", sf::Keyboard::Key::N},
		{"O", sf::Keyboard::Key::O},
		{"P", sf::Keyboard::Key::P},
		{"Q", sf::Keyboard::Key::Q},
		{"R", sf::Keyboard::Key::R},
		{"S", sf::Keyboard::Key::S},
		{"T", sf::Keyboard::Key::T},
		{"U", sf::Keyboard::Key::U},
		{"V", sf::Keyboard::Key::V},
		{"W", sf::Keyboard::Key::W},
		{"X", sf::Keyboard::Key::X},
		{"Y", sf::Keyboard::Key::Y},
		{"Z", sf::Keyboard::Key::Z},
		{"Up", sf::Keyboard::Key::Up},
		{"Down", sf::Keyboard::Key::Down},
		{"Left", sf::Keyboard::Key::Left},
		{"Right", sf::Keyboard::Key::Right},
		{"0", sf::Keyboard::Key::Num0},
		{"1", sf::Keyboard::Key::Num1},
		{"2", sf::Keyboard::Key::Num2},
		{"3", sf::Keyboard::Key::Num3},
		{"4", sf::Keyboard::Key::Num4},
		{"5", sf::Keyboard::Key::Num5},
		{"6", sf::Keyboard::Key::Num6},
		{"7", sf::Keyboard::Key::Num7},
		{"8", sf::Keyboard::Key::Num8},
		{"9", sf::Keyboard::Key::Num9},
		{"Shift", sf::Keyboard::LShift},
		{"STRG", sf::Keyboard::LControl},
		{"Enter", sf::Keyboard::Enter},
		{"Backspace", sf::Keyboard::Backspace},
		{"Esc", sf::Keyboard::Escape},
		{"Tab", sf::Keyboard::Tab},
		{"F1", sf::Keyboard::F1},
		{"F2", sf::Keyboard::F2},
		{"F3", sf::Keyboard::F3},
		{"F4", sf::Keyboard::F4},
		{"F5", sf::Keyboard::F5},
		{"F6", sf::Keyboard::F6},
		{"F7", sf::Keyboard::F7},
		{"F8", sf::Keyboard::F8},
		{"F9", sf::Keyboard::F9},
		{"F10", sf::Keyboard::F10},
		{"F11", sf::Keyboard::F11},
		{"F12", sf::Keyboard::F12},
	};

	static inline Gfx* gfx = nullptr;
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

	Value ZufaelligeZahlNative(std::vector<Value> args);
	Value ZufaelligeKommazahlNative(std::vector<Value> args);

	//graphics functions
	Value ErstelleFenster(std::vector<Value> args);
	Value SchliesseFenster(std::vector<Value> args);
	Value FensterOffen(std::vector<Value> args);
	Value MaleRechteck(std::vector<Value> args);
	Value TasteGedrueckt(std::vector<Value> args);
	Value AktualisiereFenster(std::vector<Value> args);
}