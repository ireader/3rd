#ifndef _jsonparser_h_
#define _jsonparser_h_

#include "json.h"
#include <assert.h>
#include <stack>
#include <string>
#include <stdlib.h>

class jsonparser
{
public:
	jsonparser():m_root(NULL){}
	jsonparser(const char* json):m_root(NULL)
	{
		parse(json);
	}

	~jsonparser()
	{
		if(m_root)
			json_value_free(m_root);
	}

	bool parse(const char* json)
	{
		assert(!m_root && json);
		m_root = json_parse(json, strlen(json));
		if(m_root)
			m_stacks.push(m_root);
		return !!m_root;
	}

	bool valid() const
	{
		return m_stacks.size()>0;
	}

	const json_value* root() const
	{
		return m_root;
	}

	const json_value* top() const
	{
		assert(m_stacks.size() > 0);
		return m_stacks.top();
	}

public:
	bool push(const json_value* json)
	{
		assert(json_object==json->type || json_array==json->type);
		if(json->type!=json_object && json->type!=json_array)
			return false;

		m_stacks.push(json);
		return true;
	}

	void pop()
	{
		assert(m_stacks.size() > 0);
		m_stacks.pop();
	}

	class autopush
	{
		jsonparser& m_parser;
		autopush(autopush& obj):m_parser(obj.m_parser){}
		autopush& operator= (autopush&) { return *this; }

	public:
		autopush(jsonparser& parser, const json_value* json):m_parser(parser){ parser.push(json);}
		~autopush(){ m_parser.pop(); }
	};

public:
	bool get(const char* xpath, int& value) const
	{
		const json_value* json = find(xpath);
		return json ? get(json, value) : false;
	}
	
	bool get(const char* xpath, int64_t& value) const
	{
		const json_value* json = find(xpath);
		return json ? get(json, value) : false;
	}

	bool get(const char* xpath, bool& value) const
	{
		const json_value* json = find(xpath);
		return json ? get(json, value) : false;
	}

	bool get(const char* xpath, double& value) const
	{
		const json_value* json = find(xpath);
		return json ? get(json, value) : false;		
	}

	bool get(const char* xpath, std::string& value) const
	{
		const json_value* json = find(xpath);
		return json ? get(json, value) : false;		
	}

	int get2(const char* xpath, int defaultValue) const
	{
		int v = 0;
		return get(xpath, v) ? v : defaultValue;
	}

	bool get2(const char* xpath, bool defaultValue) const
	{
		bool v = 0;
		return get(xpath, v) ? v : defaultValue;
	}

	double get2(const char* xpath, double defaultValue) const
	{
		double v = 0.0;
		return get(xpath, v) ? v : defaultValue;
	}

	std::string get2(const char* xpath, const char* defaultValue) const
	{
		std::string v;
		return get(xpath, v) ? v : defaultValue;
	}

public:
	bool get(const json_value* json, int& value) const
	{
		if(json->type == json_string)
		{
			value = atoi(json->u.string.ptr);
		}
		else
		{
			assert(json->type == json_integer);
			if(json->type != json_integer)
				return false;
			value = (int)json->u.integer;
		}
		return true;
	}

	bool get(const json_value* json, int64_t& value) const
	{
		if (json->type == json_string)
		{
			value = strtoll(json->u.string.ptr, NULL, 10);
		}
		else
		{
			assert(json->type == json_integer);
			if (json->type != json_integer)
				return false;
			value = json->u.integer;
		}
		return true;
	}

	bool get(const json_value* json, bool& value) const
	{
		if(json->type == json_string)
		{
#if defined(_WIN32) || defined(_WIN64)
			value = 0==_stricmp("true", json->u.string.ptr);
#else
			value = 0==strcasecmp("true", json->u.string.ptr);
#endif
		}
		else
		{
			assert(json->type==json_boolean);
			if(json->type != json_boolean)
				return false;
			value = 0!=json->u.boolean;
		}
		return true;
	}

	bool get(const json_value* json, double& value) const
	{
		if (json->type == json_string)
		{
			value = atof(json->u.string.ptr);
		}
		else if (json->type == json_double)
		{
			value = json->u.dbl;
		}
		else if (json->type == json_integer)
		{
			value = (double)json->u.integer;
		}
		else
		{
			assert(0);
			return false;
		}
		return true;
	}

	bool get(const json_value* json, std::string& value) const
	{
		assert(json->type == json_string);
		if(json->type != json_string)
			return false;
		value.assign(json->u.string.ptr, json->u.string.length);
		return true;
	}

public:
	const json_value* find(const char* xpath) const
	{
		if(!valid() || !xpath || 0==*xpath)
			return NULL;

		const json_value* json = NULL;
		if('/' == *xpath)
		{
			json = m_root;
			++xpath;
		}
		else
		{
			assert(m_stacks.size() > 0);
			json = m_stacks.top();
			assert(json_object==json->type || json_array==json->type);
		}

		const char* s = xpath;
		const char* p = strchr(xpath, '/');
		while(s && *s)
		{
			std::string path;
			if(p)
			{
				path.assign(s, p-s);

				// next path
				s = p+1;
				p = strchr(s, '/');
			}
			else
			{
				path.assign(s);

				s = p;
			}

			if(0 == strcmp(".", path.c_str()))
			{
			}
			else if(0 == strcmp("..", path.c_str()))
			{
				assert(false);
			}
			else
			{
				const json_value& item = (*json)[path.c_str()];
				if(item.type == json_none)
					return NULL;

				json = &item;
			}
		}
		return json;
	}

public:
	size_t length(const json_value* json) const
	{
		assert(json);
		assert(json_object==json->type || json_array==json->type);
		if(json_object!=json->type && json_array!=json->type)
			return 0;
		return json->type==json_object ? json->u.object.length : json->u.array.length;
	}

	const json_value* child(const json_value* json, size_t index) const
	{
		assert(json);
		assert(json_object==json->type || json_array==json->type);

		switch(json->type)
		{
		case json_object:
			if(index >= json->u.object.length)
				return NULL;
			return json->u.object.values[index].value;

		case json_array:
			if(index >= json->u.array.length)
				return NULL;
			return json->u.array.values[index];

		default:
			return NULL;
		}
	}

	const char* childname(const json_value* json, size_t index) const
	{
		assert(json);
		assert(json_object==json->type);
		if(json_object != json->type)
			return NULL;

		if(index >= json->u.object.length)
			return NULL;
		return json->u.object.values[index].name;
	}

private:
	json_value* m_root;
	typedef std::stack<const json_value*> TJsonStack;
	TJsonStack m_stacks;
};

#endif /* !_jsonparser_h_ */
