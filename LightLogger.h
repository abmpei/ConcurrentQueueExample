#pragma once
#include <iostream>
#include <sstream>

// ===== Thread safe logging to std::cout
std::stringstream& logz_one(std::stringstream& os)
{
	return os;
}

template <class A0, class ...Args>
std::stringstream& logz_one(std::stringstream& os, const A0& a0, const Args& ...args)
{
	os << a0;
	return logz_one(os, args...);
}

template <class ...Args>
std::stringstream& logz(std::stringstream& os, const Args& ...args)
{
	return logz_one(os, args...);
}

template <class ...Args>
void logz(const Args& ...args)
{
	static std::mutex m;
	std::lock_guard<std::mutex> lock(m);
	std::stringstream ss;
	std::cout << logz(ss, "\n", time(0), ": ", args...).str();
}


