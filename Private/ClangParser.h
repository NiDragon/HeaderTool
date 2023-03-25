#pragma once

#include <clang-c/Index.h>

#include "Common.h"
#include "ReflectionEnums.h"
#include "CmdParser.h"

struct ReflectionDefinition
{
	ReflectionDefinition()
	{
		Type = REFLECT_INVALID;
		Line = -1;
		IsAccessor = false;
		AccessLevel = Public;
	}

	// The type of reflection object this is
	ReflectionType Type;

	// Use this in the future for overriding specific class paths
	string Module;

	// Name of the pointer object for property and functions
	string EntryName; 

	// This is used to find the object the this definition is attached to
	int Line;

	// Is this a Property Accessor
	bool IsAccessor;

	// Default function arguments
	string DefaultArgs;

	// The policy for return RawPointer Reference WrappedPointer
	// this is valid on CLASS FUNCTION and PROPERTY
	string ReturnPolicy;

	// The level of access or visibility of this definition
	ReflectionAccess AccessLevel;

	// If this is an object defined in another class
	string ParentClass;

	// If this is a class this is for RTTR_ENABLE()
	string BaseClass;

	// Constructors
	vector<string> Constructors;

	// Parameter Names
	vector<string> ParamNames;

	// This is to remap enum values to strings
	map<string, string> Values;

	// metadata tags this follows (string, then anything really)
	map<string, string> Metadata;
};

class ClangParser
{
public:
    ClangParser(CmdParser& cmd);

    // If this returns anything but 0 we failed
    int Parse();

    fs::path filename;
    CXTranslationUnit tu;

    vector<ReflectionDefinition> Definitions;

private:
    void ProcessMacro(CXCursor cursor);
    void ProcessClass(CXCursor cursor);
    void ProcessEnumConstantDecl(CXCursor cursor);
    void ProcessEnumDecl(CXCursor cursor);
    void ProcessFieldDecl(CXCursor cursor);
    void ProcessVarDecl(CXCursor cursor);
    void ProcessCXXMethod(CXCursor cursor);
    void ProcessFunctionDecl(CXCursor cursor);
    void ProcessConstructor(CXCursor cursor);

    static CXChildVisitResult Visitor(CXCursor cursor,
        CXCursor parent,
        CXClientData client_data);

    // Arguments for libclang
    vector<const char*> arg_vec;
};