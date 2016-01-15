#ifndef ICJS_TYPES_H
#define ICJS_TYPES_H

#include <string>
#include <vector>
#include <map>

typedef void * any_t;

/*
 * 全局变量类
 * type == 0 / 1 时 data指向空指针
 * type == 2 时 data指向string
 * type == 3 时 data指向double
 * type == 4 时 data指向boolean
 * type == 5 时 data指向一个vector<Element>
 * type == 6 时 data指向...
 * type == 7 时 data指向一个Function实例
 */

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
 * 函数类
 * 记录函数名，函数参数与函数体
 */

class Function
{
	public:
		std::string key;
		std::vector<std::string> param_names;
		std::vector<std::string> body;
};

/* 
 * 元素类
 * 记录元素名，类型与具体数据
 */

class Element
{
	public:
		std::string key;
		int type;
		any_t data;
		Element(){ key = ""; type = Global::_undefined; data = NULL; };
		Element(std::string _key, int _type, any_t _data=NULL){
			key = _key;
			type = _type;
			switch (type){
			case 0:case 1:data = NULL;
				break;
			case 2:data = (std::string*)_data;
				break;
			case 3:data = (double*)_data;
				break;
			case 4:data = (bool*)_data;
				break;
			case 5:data = (std::vector<Element>*)_data;
				break;
			case 6:break;
			case 7:data = (Function*)_data;
				break;
			default:data = NULL;
			}
		}
};


#endif ICJS_TYPES_H
