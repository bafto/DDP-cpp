#pragma once

#ifdef _DEBUG
#define _MDEBUG_
#endif

#include <string>

class base_exception
{
public:
	base_exception(const std::string& str) : msg(str) {}
	virtual ~base_exception() = default;

	virtual std::string what() { return msg; }
private:
	std::string msg;
};

inline bool utf8;

using namespace std::string_literals;

#define GENERATE_EXCEPTION(type, msg) std::string("["s +  std::string(#type) + "]\nFile: "s +  std::string(__FILE__) + "\nLine: "s + std::to_string(__LINE__) + "\nWhat: "s + msg)