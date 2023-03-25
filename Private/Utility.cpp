#include "Utility.h"

// String Utility

bool IsNumber(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

// Not passing key as reference because we would be overwriting the value
// Likewise not passing value as reference because it might take a const char*
bool CheckStringIgnoreCase(string key, string value)
{
	transform(key.begin(), key.end(), key.begin(), tolower);

	if(key == value)
		return true;

	return false;
}

string QuoteString(string text)
{
	if(text.empty()) {
		text.append("\"\"");
	}

	if (text.find_first_of("\"") != 0)
	{
		text.insert(0, "\"");
	}

	if (text.find_last_of("\"") != text.length()-1)
	{
		text.push_back('\"');
	}

	return text;
}

int IsInvalidNSChar(char c)
{
	return isalnum(c) == 0;
}

int IsInvalidPathChar(char c)
{
	return (isalnum(c) == 0 && c != '/');
}

void ReplaceDuplicates(string& str)
{
	string invalid = "//";
	string with = "/";
	size_t start_pos = str.find(invalid);
	if (start_pos != std::string::npos)
	{
		str.replace(start_pos, invalid.length(), with);
	}
}

// Reflection Utility

string GetAccessString(ReflectionAccess level)
{
	switch (level)
	{
	case ReflectionAccess::Private:
		return "rttr::registration::private_access";
	case ReflectionAccess::Protected:
		return "rttr::registration::protected_access";
	default:
		return "";
	}
}

ReflectionAccess GetAccessLevel(string value)
{
	transform(value.begin(), value.end(), value.begin(), tolower);

	if(value == "public")
	{
		return ReflectionAccess::Public;
	} 
	else if(value == "private")
	{
		return ReflectionAccess::Private;
	} 
	else if(value == "protected")
	{
		return ReflectionAccess::Protected;
	}
	
	return ReflectionAccess::Public;
}

PropertyType GetPropertyType(string value)
{
	auto strpos = value.find_first_of("\"");

	if (strpos != string::npos)
	{
		return STRING;
	}
	else if (IsNumber(value))
	{
		return NUMBER;
	}
	else
		return OBJECT;
}

// Path Utility

string MakeAbsolute(string path)
{
	fs::path absolutePath(path);

	if (absolutePath.is_absolute())
	{
		absolutePath = fs::absolute(absolutePath);
	}

	return absolutePath.generic_string();
}

fs::path& MakeAbsolutePath(fs::path& path)
{
	if (path.is_absolute())
	{
		path = fs::absolute(path);
	}

	return path;
}

bool CreateDirectory(fs::path& path)
{
	if(!path.is_absolute())
		path = fs::absolute(path);

	if (!fs::is_directory(path))
	{
		fs::create_directories(path);

		if (!fs::is_directory(path))
		{
			return false;
		}
	}

	return true;
}
