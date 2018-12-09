#include "stdafx.h"

#include "utils.h"

#include <chrono>
#include <mutex>
#include <regex>

#include <boost/filesystem.hpp>


//in nano seconds
long long meassuretime()
{
	using namespace std::chrono;

	static steady_clock::time_point t1 = steady_clock::now();

	steady_clock::time_point t2 = steady_clock::now();
	// duration<type of count, type of period>
	// A duration object expresses a time span by means of a count and a period.
    // This count is expresed in terms of periods. The length of a period is integrated in 
	//   the type (on compile time) by its second template parameter (Period), 
	//   which is a ratio type that expresses the number (or fraction) of seconds that elapse in each period.
	duration<long long, std::nano> time_span = duration_cast<duration<long long, std::nano>>(t2 - t1);
	t1 = std::move(t2);

	return time_span.count();
}



static std::mutex mutex_output;

std::mutex &__getoutputmutex()
{
	return mutex_output;
}



//output backspace
const wchar_t *backspace(int n)
{
	const int buffersize = 128;
	static wchar_t str[buffersize];
	static int lastn = 0;
	static bool init = false;

	if (!init) {
		init = true;

		for (int i = 0; i < buffersize; i++) {
			str[i] = '\b';
		}
	}

	if (n < buffersize) {
		str[lastn] = '\b';
		str[n] = '\0';
		lastn = n;
	}

	return str;
}



// if path doesn't exist, return false
// if path is a zero-length file, return false
// if path is a directory, return false
bool fileexists(const boost::filesystem::path &path)
{
	using namespace boost::filesystem;

	//symlink_status: follow symbol links
	//status: just return the type of the file, won't follow symbol links.
	file_type type = symlink_status(path).type();

	if (type == file_not_found || type == directory_file || type == status_error) return false;

	if (type == regular_file && file_size(path) == 0) return false;

	return true;

	
	
}


// check the syntax of a regular expression
bool checkregex(const wchar_t *regex, std::wstring &errordescription) noexcept
{
	try {
		std::wregex re(regex);
	}
	catch (std::regex_error ex) {
		std::regex_constants::error_type code = ex.code();
		switch (code)
		{
		case std::regex_constants::error_collate:
			errordescription.assign(L"The expression contained an invalid collating element name.");
			break;
		case std::regex_constants::error_ctype:
			errordescription.assign(L"The expression contained an invalid character class name.");
			break;
		case std::regex_constants::error_escape:
			errordescription.assign(L"The expression contained an invalid escaped character, or a trailing escape.");
			break;
		case std::regex_constants::error_backref:
			errordescription.assign(L"The expression contained an invalid back reference.");
			break;
		case std::regex_constants::error_brack:
			errordescription.assign(L"The expression contained mismatched brackets [ and ].");
			break;
		case std::regex_constants::error_paren:
			errordescription.assign(L"The expression contained mismatched parentheses ( and ).");
			break;
		case std::regex_constants::error_brace:
			errordescription.assign(L"The expression contained mismatched braces { and }.");
			break;
		case std::regex_constants::error_badbrace:
			errordescription.assign(L"The expression contained an invalid range between braces { and }.");
			break;
		case std::regex_constants::error_range:
			errordescription.assign(L"The expression contained an invalid character range.");
			break;
		case std::regex_constants::error_space:
			errordescription.assign(L"There was insufficient memory to convert the expression into a finite state machine.");
			break;
		case std::regex_constants::error_badrepeat:
			errordescription.assign(L"The expression contained a repeat specifier (one of *?+{) that was not preceded by a valid regular expression.");
			break;
		case std::regex_constants::error_complexity:
			errordescription.assign(L"The complexity of an attempted match against a regular expression exceeded a pre-set level.");
			break;
		case std::regex_constants::error_stack:
			errordescription.assign(L"There was insufficient memory to determine whether the regular expression could match the specified character sequence.");
			break;
		case std::regex_constants::error_parse:
			errordescription.assign(L"Parse error.");
			break;
		case std::regex_constants::error_syntax:
			errordescription.assign(L"Syntax error.");
			break;
		default:
			errordescription.assign(L"Unknown error in regex.");
			break;
		}

		return false;
	}
	catch (...) {
		errordescription.assign(L"Unknown error in checkregex.");
		return false;
	}

	return true;
}