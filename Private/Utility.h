#pragma once

#include "Common.h"
#include "ReflectionEnums.h"

using namespace std;
namespace fs = std::filesystem;

// Stream Utility

template <class _Elem, class _Traits>
basic_ostream<_Elem, _Traits>& tabchar(
	basic_ostream<_Elem, _Traits>& _Ostr) { // insert tab and flush stream
	_Ostr.write("    ", 4);
	_Ostr.flush();
	return _Ostr;
}

// String Utility

bool IsNumber(const std::string& s);

// Not passing key as reference because we would be overwriting the value
// Likewise not passing value as reference because it might take a const char*
bool CheckStringIgnoreCase(string key, string value);

string QuoteString(string text);

int IsInvalidNSChar(char c);

int IsInvalidPathChar(char c);

void ReplaceDuplicates(string& str);

// Reflection Utility

string GetAccessString(ReflectionAccess level);

ReflectionAccess GetAccessLevel(string value);

PropertyType GetPropertyType(string value);

// Path Utility

string MakeAbsolute(string path);

fs::path& MakeAbsolutePath(fs::path& path);

bool CreateDirectory(fs::path& path);
