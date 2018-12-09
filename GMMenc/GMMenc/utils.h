#pragma once

#include <mutex>

#include <boost/filesystem.hpp>

//in nano seconds
long long meassuretime();




std::mutex &__getoutputmutex();

//printf with mutex
template<typename... Args>
int mprintf(const char *format, Args&&... args)
{
	std::unique_lock<std::mutex> lock(__getoutputmutex());
	return printf(format, std::forward<Args>(args)...);
}


//output backspace
const wchar_t *backspace(int n);


// if path doesn't exist, return false
// if path is a zero-length file, return false
// if path is a directory, return false
bool fileexists(const boost::filesystem::path &path);


// check the syntax of a regular expression
bool checkregex(const wchar_t *regex, std::wstring &errordescription) noexcept;