#ifndef ICJS_UTIL_H
#define ICJS_UTIL_H

#include "ICJs_types.h"
#include <string>
#include <vector>

class Util
{	
public:
	static void split(std::string &s, std::string delim, std::vector<std::string> *ret, bool preserveBlank); 
	static std::string &trim(std::string &s);
	static int numOfChar(std::string &s, char c);
	static std::string arrayToString(std::vector<Element> ary);
	//static void rmBlankInParenth(std::string &s);
	//static void numBlankPos(std::string &s, std::vector<int> pos);
};

#endif

