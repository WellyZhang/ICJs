#ifndef ICJS_UTIL_H
#define ICJS_UTIL_H

#include <string>
#include <vector>

class Util
{	
public:
	static void split(std::string &s, std::string delim, std::vector<std::string> *ret); 
	static std::string &trim(std::string &s);
};

#endif

