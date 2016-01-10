#ifndef ICJS_TYPES_H
#define ICJS_TYPES_H

#include <string>
#include <vector>
#include <map>

typedef void * any_t;

class Global
{
	public:
		static const int _undefined = 0;
		static const int _null = 1;
		static const int _string = 2;
		static const int _number = 3;
		static const int _boolean = 4;
		static const int _array = 5;
		static const int _object = 6;
		static const int _function = 7;
		static const int _ok = 0;
		static const int _fault = -1;
};

/*
 * type == 0 / 1 时 data指向空指针
 * type == 2 时 data指向string
 * type == 3 时 data指向double
 * type == 4 时 data指向boolean
 * type == 5 时 data指向一个vector<Element>
 * type == 6 时 data指向...
 * type == 7 时 data指向一个Function实例
 */

class Element
{
	public:
		std::string key;
		int type;
		any_t data;
		Element(){ key = ""; type = Global::_undefined; data = NULL; };
};

class Function
{
	public:
		std::string key;
		std::vector<std::string> param_names;
		std::vector<std::string> body;		
};

#endif ICJS_TYPES_H