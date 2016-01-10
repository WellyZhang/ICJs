#include "ICJs_util.h"
#include <string>
#include <vector>

void Util::split(std::string& s, std::string delim, std::vector<std::string> *ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		ret->push_back(trim(s.substr(last, index - last)));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		ret->push_back(trim(s.substr(last, index - last)));
	}
}

std::string &Util::trim(std::string &s)
{
	if (s.empty())
	{
		return s;
	}
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}
