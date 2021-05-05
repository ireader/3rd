#ifndef _jsonhelper_h_
#define _jsonhelper_h_

#include <string>
#include <assert.h>

inline void jsonformat(const char* s, std::string& json)
{
	assert(s);
	while(*s)
	{
		switch(*s)
		{
		case '\\':
			json += "\\\\";
			break;

		case '\"':
			json += "\\\"";
			break;

		case '/':
			json += "\\/";
			break;

		case '\b':
			json += "\\b";
			break;

		case '\f':
			json += "\\f";
			break;

		case '\n':
			json += "\\n";
			break;

		case '\r':
			json += "\\r";
			break;

		case '\t':
			json += "\\t";
			break;

		default:
			json += *s;
		}
		++s;
	}
}

class jsonarray;
class jsonobject
{
public:
	jsonobject()
	{
		m_count = 0;
	}

	void clear()
	{
		m_count = 0;
		m_json.clear();
	}

	int size() const
	{
		return m_count;
	}

	std::string json() const
	{
		return std::string("{") + m_json + '}';
	}

public:
	template <typename T>
	jsonobject& add(const char* name, T value)
	{
		addname(name);
		m_json += std::to_string(value);
		return *this;
	}

	jsonobject& add(const char* name, bool value)
	{
		addname(name);
		m_json += value ? "true" : "false";
		return *this;
	}

	jsonobject& add(const char* name, const char* value)
	{
		addname(name);
		m_json += '"';
		jsonformat(value, m_json);
		m_json += '"';
		return *this;
	}

	jsonobject& add(const char* name, const std::string& value)
	{
		return add(name, value.c_str());
	}

	jsonobject& add(const char* name, const jsonarray& array);

	jsonobject& add(const char* name, const jsonobject& object)
	{
		addname(name);
		m_json += object.json();
		return *this;
	}

private:
	void addname(const char* name)
	{
		if(m_json.length() > 0)
			m_json += ',';

		m_json += '"';
		m_json += name;
		m_json += "\":";
		++m_count;
	}

private:
	int m_count;
	std::string m_json;
};

class jsonarray
{
public:
	jsonarray()
	{
		m_count = 0;
	}

	void clear()
	{
		m_count = 0;
		m_json.clear();
	}

	int size() const
	{
		return m_count;
	}

	std::string json() const
	{
		return std::string("[") + m_json + ']';
	}

public:
	template <typename T>
	jsonobject& add(const char* name, T value)
	{
		++m_count;
		if (m_json.length() > 0)
			m_json += ',';
		m_json += std::to_string(value);
		return *this;
	}

	jsonarray& add(const char* value)
	{
		++m_count;
		if(m_json.length() > 0)
			m_json += ',';
		m_json += '"';
		jsonformat(value, m_json);
		m_json += '"';
		return *this;
	}

	jsonarray& add(const std::string& value)
	{
		return add(value.c_str());
	}

	jsonarray& add(const jsonarray& array)
	{
		++m_count;
		if(m_json.length() > 0)
			m_json += ',';
		m_json += array.json();
		return *this;
	}

	jsonarray& add(const jsonobject& object)
	{
		++m_count;
		if(m_json.length() > 0)
			m_json += ',';
		m_json += object.json();
		return *this;
	}

private:
	int m_count;
	std::string m_json;
};

inline jsonobject& jsonobject::add(const char* name, const jsonarray& array)
{
	addname(name);
	m_json += array.json();
	return *this;
}

#endif /* !_jsonhelper_h_ */
