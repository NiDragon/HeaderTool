#pragma once

enum PropertyType
{
	STRING,
	NUMBER,
	OBJECT
};

enum ReflectionAccess
{
	Public,
	Private,
	Protected
};

enum ReflectionType
{
	REFLECT_INVALID = -1,
	CLASS = 0,
	PROPERTY = 1,
	ENUM = 2,
	FUNCTION = 3
};