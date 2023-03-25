#include "ClangParser.h"

#include "CmdParser.h"
#include "Utility.h"

const char* macros[] = { "CLASS", "PROPERTY", "ENUM", "FUNCTION" };

#define STR_IS_CHAR(string, c) (memcmp(string, c, 1) == 0)

// Collect the number of arguments in this macro
unsigned int FindArgumentsCount(CXCursor cursor, CXToken* tokens, unsigned int numTokens)
{
	unsigned int argCount = 0;

	bool hasArg = false;
	bool isArray = false;

	for (unsigned int i = 1; i < numTokens; i++)
	{
		CXString tokenString = clang_getTokenSpelling(clang_Cursor_getTranslationUnit(cursor), tokens[i]);

		const char* cstring = clang_getCString(tokenString);

		// Is sequence begining of an array
		if(STR_IS_CHAR(cstring, "[")) 
		{
			isArray = true;
		}
		if (STR_IS_CHAR(cstring, "]"))
		{
			isArray = false;
		}

		// if this value is not ( or ) or 
		if (!STR_IS_CHAR(cstring, "(") &&
			!STR_IS_CHAR(cstring, ")") &&
			!hasArg) 
		{
			argCount++;
			hasArg = true;
		}

		if(STR_IS_CHAR(cstring, ",") && hasArg)
		{
			if(!isArray) {
				hasArg = false;
			} 
		}

		clang_disposeString(tokenString);
	}

	return argCount;
}

// Check if this token is equal to what we want
bool TokenEquals(CXTranslationUnit tu, CXToken token, const char* cstring)
{
	bool ret = false;

	CXString token_text = clang_getTokenSpelling(tu, token);

	string str = clang_getCString(token_text);
	
	if (str == cstring)
		ret = true;

	clang_disposeString(token_text);

	return ret;
}

// Check if this text is a preprocessor macro we accept
ReflectionType IsValidReflectionType(CXString token_text)
{
	const char* cstr = clang_getCString(token_text);

	ReflectionType ret = REFLECT_INVALID;

	for (int i = 0; i < std::extent<decltype(macros)>::value; i++)
	{
		if (memcmp(cstr, macros[i], strlen(macros[i])) == 0)
		{
			ret = (ReflectionType)i;
			break;
		}
	}

	return ret;
}

// Print the tokens to the console output
void _DbgPrintTokens(CXTranslationUnit tu, CXToken* tokens, unsigned int numTokens)
{
	// Debug things
	for (unsigned int i = 0; i < numTokens; i++)
	{
		CXString tokenSpelling = clang_getTokenSpelling(tu, tokens[i]);
		const char* checkstr = clang_getCString(tokenSpelling);

		cout << checkstr << " ";

		clang_disposeString(tokenSpelling);
	}

	cout << endl;
}

#if !defined(VERBOSE_LOGGING)
#define DbgPrint(...)
#define DbgPrintTokens(...)
#else
#define DbgPrint printf
#define DbgPrintTokens _DbgPrintTokens
#endif

void ClangParser::ProcessMacro(CXCursor cursor)
{
	if (clang_getCursorKind(cursor) == CXCursor_MacroInstantiation)
	{
		ReflectionDefinition definition;

		CXString name = clang_getCursorSpelling(cursor);

		if ((definition.Type = IsValidReflectionType(name)) != REFLECT_INVALID)
		{
			CXSourceRange range = clang_getCursorExtent(cursor);
			CXToken* tokens;
			unsigned int numTokens;

			clang_tokenize(this->tu, range, &tokens, &numTokens);

			if (numTokens != 0) 
			{
				DbgPrintTokens(this->tu, tokens, numTokens);

				CXFile file;
				unsigned int line, column, offset;

				clang_getFileLocation(clang_getTokenLocation(this->tu, tokens[0]), &file, &line, &column, &offset);

				definition.Line = line;

				unsigned int argCount = FindArgumentsCount(cursor, tokens, numTokens);

				DbgPrint("Macro expansion has %d arguments\n", argCount);

				CXToken* currentToken = &tokens[2];

				// Parse our args
				for (unsigned int i = 0; i < argCount; i++)
				{
					// First Part
					CXString currentString = clang_getTokenSpelling(this->tu, *currentToken);
					string key = clang_getCString(currentString);

					// Is this actually a meta tag?
					//if (IsValidMeta(currentString) != META_INVALID)
					if (!TokenEquals(this->tu, *currentToken, ")"))
					{
						currentToken++;

						// Support tokens with no assignment
						// Handle tokens until we reach , or )
						string value;

						// Check if we have a matching equals
						if (TokenEquals(this->tu, *currentToken, "="))
						{
							currentToken++;

							if(TokenEquals(this->tu, *currentToken, "["))
							{
								currentToken++;
								
								// We are parsing an array
								while(!TokenEquals(this->tu, *currentToken, "]"))
								{
									CXString secondString = clang_getTokenSpelling(this->tu, *currentToken);

									value += clang_getCString(secondString);

									clang_disposeString(secondString);

									currentToken++;
								}

								// Skip over ] to keep from messing up the next part
								currentToken++;
							} // End Array this needs its own function

							while (!TokenEquals(this->tu, *currentToken, ",") && !TokenEquals(this->tu, *currentToken, ")"))
							{
								CXString secondString = clang_getTokenSpelling(this->tu, *currentToken);
								value += clang_getCString(secondString);
								clang_disposeString(secondString);

								currentToken++;
							}

							// skip the end token
							currentToken++;
						} else {
							currentToken++;
						}

						// Handle special cases tags here
						if(CheckStringIgnoreCase(key, "access"))
						{
							definition.AccessLevel = GetAccessLevel(value);
						}
						else if (CheckStringIgnoreCase(key, "defaultargs"))
						{
							definition.DefaultArgs = value;
						}
						else if (CheckStringIgnoreCase(key, "policy"))
						{
							definition.ReturnPolicy = value;
						}
						else
						{
							definition.Metadata.insert(make_pair(key, value));
						}
					}

					clang_disposeString(currentString);
				}

				clang_disposeTokens(this->tu, tokens, numTokens);

				DbgPrint("Found Valid Macro: %s\n", clang_getCString(name));

				this->Definitions.push_back(definition);
			}
		}
		clang_disposeString(name);
	}
}

void ClangParser::ProcessClass(CXCursor cursor)
{
	if (clang_getCursorKind(cursor) == CXCursor_ClassDecl)
	{
		CXSourceRange range = clang_getCursorExtent(cursor);
		CXToken* tokens;
		unsigned int numTokens;

		clang_tokenize(this->tu, range, &tokens, &numTokens);

		if (numTokens != 0)
		{
			DbgPrintTokens(this->tu, tokens, numTokens);

			CXFile file;
			unsigned int line, column, offset;

			clang_getFileLocation(clang_getTokenLocation(this->tu, tokens[0]), &file, &line, &column, &offset);

			for (auto& def : this->Definitions)
			{
				if (def.Type == CLASS && def.Line + 1 == line)
				{
					CXString className = clang_getCursorDisplayName(cursor);

					def.EntryName = clang_getCString(className);

					CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
					CXString parentName = clang_getCursorDisplayName(parentCursor);

					if (string(clang_getCString(parentName)).find_first_of(".") == string::npos)
					{
						def.ParentClass = clang_getCString(parentName);
					}

					clang_disposeString(className);
					clang_disposeString(parentName);
				}
			}

			clang_disposeTokens(this->tu, tokens, numTokens);
		}
	}
}

void ClangParser::ProcessEnumConstantDecl(CXCursor cursor)
{
	if (clang_getCursorKind(cursor) == CXCursor_EnumConstantDecl) {
		CXString enumValue = clang_getCursorSpelling(cursor);

		CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
		CXString parentName = clang_getCursorDisplayName(parentCursor);

		for (auto& def : this->Definitions)
		{
			if (def.EntryName == clang_getCString(parentName))
			{
				string first = clang_getCString(enumValue);
				string second = first;

				def.Values.insert(make_pair(first, second));
			}
		}

		clang_disposeString(enumValue);
		clang_disposeString(parentName);
	}
}

void ClangParser::ProcessEnumDecl(CXCursor cursor)
{
	if (clang_getCursorKind(cursor) == CXCursor_EnumDecl)
	{
		CXSourceRange range = clang_getCursorExtent(cursor);
		CXToken* tokens;
		unsigned int numTokens;

		clang_tokenize(this->tu, range, &tokens, &numTokens);

		if (numTokens != 0)
		{
			DbgPrintTokens(this->tu, tokens, numTokens);

			CXFile file;
			unsigned int line, column, offset;

			clang_getFileLocation(clang_getTokenLocation(this->tu, tokens[0]), &file, &line, &column, &offset);

			for (auto& def : this->Definitions)
			{
				if (def.Type == ENUM && def.Line + 1 == line)
				{
					CXString enumName = clang_getCursorDisplayName(cursor);

					def.EntryName = clang_getCString(enumName);

					CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
					CXString parentName = clang_getCursorDisplayName(parentCursor);

					if (string(clang_getCString(parentName)).find_first_of(".") == string::npos)
					{
						def.ParentClass = clang_getCString(parentName);
					}

					clang_disposeString(enumName);
					clang_disposeString(parentName);
				}
			}

			clang_disposeTokens(this->tu, tokens, numTokens);
		}
	}
}

void ClangParser::ProcessFieldDecl(CXCursor cursor)
{
	if (clang_getCursorKind(cursor) == CXCursor_FieldDecl)
	{
		CXSourceRange range = clang_getCursorExtent(cursor);
		CXToken* tokens;
		unsigned int numTokens;

		clang_tokenize(this->tu, range, &tokens, &numTokens);

		if (numTokens != 0)
		{
			DbgPrintTokens(this->tu, tokens, numTokens);

			CXFile file;
			unsigned int line, column, offset;

			clang_getFileLocation(clang_getTokenLocation(this->tu, tokens[0]), &file, &line, &column, &offset);

			for (auto& def : this->Definitions)
			{
				if (def.Type == PROPERTY && def.Line + 1 == line)
				{
					CXString propName = clang_getCursorSpelling(cursor);

					def.EntryName = clang_getCString(propName);

					// Find the class that contains this cursor
					CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
					CXString parentName = clang_getCursorDisplayName(parentCursor);

					def.ParentClass = clang_getCString(parentName);

					clang_disposeString(propName);
					clang_disposeString(parentName);
				}
			}

			clang_disposeTokens(this->tu, tokens, numTokens);
		}
	}
}

void ClangParser::ProcessVarDecl(CXCursor cursor)
{
	if (clang_getCursorKind(cursor) == CXCursor_VarDecl)
	{
		CXSourceRange range = clang_getCursorExtent(cursor);
		CXToken* tokens;
		unsigned int numTokens;

		clang_tokenize(this->tu, range, &tokens, &numTokens);

		if (numTokens != 0)
		{
			DbgPrintTokens(this->tu, tokens, numTokens);

			CXFile file;
			unsigned int line, column, offset;

			clang_getFileLocation(clang_getTokenLocation(this->tu, tokens[0]), &file, &line, &column, &offset);

			for (auto& def : this->Definitions)
			{
				if (def.Type == PROPERTY && def.Line + 1 == line)
				{
					CXString propName = clang_getCursorSpelling(cursor);

					def.EntryName = clang_getCString(propName);

					clang_disposeString(propName);
				}
			}

			clang_disposeTokens(this->tu, tokens, numTokens);
		}
	}
}

void ClangParser::ProcessCXXMethod(CXCursor cursor)
{
	if (clang_getCursorKind(cursor) == CXCursor_CXXMethod)
	{
		CXSourceRange range = clang_getCursorExtent(cursor);
		CXToken* tokens;
		unsigned int numTokens;

		clang_tokenize(this->tu, range, &tokens, &numTokens);

		if (numTokens != 0)
		{
			DbgPrintTokens(this->tu, tokens, numTokens);

			CXFile file;
			unsigned int line, column, offset;

			clang_getFileLocation(clang_getTokenLocation(this->tu, tokens[0]), &file, &line, &column, &offset);

			for (auto& def : this->Definitions)
			{
				if ((def.Type == FUNCTION || def.Type == PROPERTY) && def.Line + 1 == line)
				{
					if(def.Type == PROPERTY) def.IsAccessor = true;

					CXString funcName = clang_getCursorSpelling(cursor);

					// Build Arg List Here
					int numArgs = clang_Cursor_getNumArguments(cursor);

					for (int i = 0; i < numArgs; i++)
					{
						CXCursor argCursor = clang_Cursor_getArgument(cursor, i);
						CXString argString = clang_getCursorSpelling(argCursor);

						def.ParamNames.push_back(clang_getCString(argString));

						clang_disposeString(argString);
					}

					def.EntryName = clang_getCString(funcName);

					// Find the class that contains this cursor
					CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
					CXString parentName = clang_getCursorDisplayName(parentCursor);

					def.ParentClass = clang_getCString(parentName);

					clang_disposeString(funcName);
					clang_disposeString(parentName);
				}
			}

			clang_disposeTokens(this->tu, tokens, numTokens);
		}
	}
}

void ClangParser::ProcessFunctionDecl(CXCursor cursor)
{
	if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl)
	{
		CXSourceRange range = clang_getCursorExtent(cursor);
		CXToken* tokens;
		unsigned int numTokens;

		clang_tokenize(this->tu, range, &tokens, &numTokens);

		if (numTokens != 0)
		{
			DbgPrintTokens(this->tu, tokens, numTokens);

			CXFile file;
			unsigned int line, column, offset;

			clang_getFileLocation(clang_getTokenLocation(this->tu, tokens[0]), &file, &line, &column, &offset);

			for (auto& def : this->Definitions)
			{
				if ((def.Type == FUNCTION || def.Type == PROPERTY) && def.Line + 1 == line)
				{
					if(def.Type == PROPERTY) def.IsAccessor = true;

					CXString funcName = clang_getCursorSpelling(cursor);

					// Build Arg List Here
					int numArgs = clang_Cursor_getNumArguments(cursor);

					for (int i = 0; i < numArgs; i++)
					{
						CXCursor argCursor = clang_Cursor_getArgument(cursor, i);
						CXString argString = clang_getCursorSpelling(argCursor);

						def.ParamNames.push_back(clang_getCString(argString));

						clang_disposeString(argString);
					}

					def.EntryName = clang_getCString(funcName);

					clang_disposeString(funcName);
				}
			}

			clang_disposeTokens(this->tu, tokens, numTokens);
		}
	}
}

void ClangParser::ProcessConstructor(CXCursor cursor)
{
	if (clang_getCursorKind(cursor) == CXCursor_Constructor)
	{
		unsigned int numArgs = clang_Cursor_getNumArguments(cursor);

		if (numArgs > 0)
		{
			string argsString;

			for (unsigned int i = 0; i < numArgs; i++)
			{
				CXCursor arg = clang_Cursor_getArgument(cursor, i);
				CXType type = clang_getCursorType(arg);

				CXString cxstr = clang_getTypeSpelling(type);

				argsString += clang_getCString(cxstr);

				clang_disposeString(cxstr);

				if (i + 1 < numArgs)
					argsString += ", ";
			}

			CXCursor parentCursor = clang_getCursorSemanticParent(cursor);
			CXString parentString = clang_getCursorSpelling(parentCursor);

			string parentName = clang_getCString(parentString);

			for (auto& def : this->Definitions)
			{
				if (def.EntryName == parentName)
				{
					def.Constructors.push_back(argsString);
				}
			}

			clang_disposeString(parentString);
		}
	}
}

CXChildVisitResult ClangParser::Visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {

	ClangParser* instance = (ClangParser*)client_data;

    switch (cursor.kind)
    {
    case CXCursor_MacroInstantiation:
        instance->ProcessMacro(cursor);
        break;

    case CXCursor_ClassDecl:
        instance->ProcessClass(cursor);
        break;

    case CXCursor_EnumConstantDecl:
        instance->ProcessEnumConstantDecl(cursor);
        break;

    case CXCursor_EnumDecl:
        instance->ProcessEnumDecl(cursor);
        break;

    case CXCursor_FieldDecl:
        instance->ProcessFieldDecl(cursor);
        break;

    case CXCursor_VarDecl:
        instance->ProcessVarDecl(cursor);
        break;

    case CXCursor_CXXMethod:
        instance->ProcessCXXMethod(cursor);
        break;

    case CXCursor_FunctionDecl:
        instance->ProcessFunctionDecl(cursor);
        break;

    case CXCursor_Constructor:
        instance->ProcessConstructor(cursor);
        break;
    }

    return CXChildVisit_Recurse;
}

ClangParser::ClangParser(CmdParser& cmd)
{
	// Tell clang this is C++ even if the extension is .h
	arg_vec.push_back("-x");
	arg_vec.push_back("c++");

	// Add include paths for resolving types
	for (unsigned int i = 0; i < cmd.IncludeDirectories.size(); i++)
	{
		arg_vec.push_back("--include-directory");
		arg_vec.push_back(cmd.IncludeDirectories[i].c_str());
	}

	arg_vec.push_back(NULL);

    this->filename = cmd.SourceFile;
}

int ClangParser::Parse() {

    // Create and empty index for the translation unit
	CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit;

	// create a translation unit for processing the file
	CXErrorCode error = clang_parseTranslationUnit2(
		index,
		filename.generic_string().c_str(),
		&arg_vec[0],
		arg_vec.size()-1 & 0xFFFFFFFF,
		0,
		0,
		CXTranslationUnit_DetailedPreprocessingRecord, &unit);

	if (error != CXError_Success)
	{
		cerr << "Failed to parse the translation unit." << endl;
		return GENERIC_FILE_ERROR;
	}

	// Get the cursor at the desired location
	CXCursor cursor = clang_getTranslationUnitCursor(unit);

	this->tu = unit;

	// Process the source itself
	clang_visitChildren(cursor, ClangParser::Visitor, this);

	// Destroy the translation unit
	clang_disposeTranslationUnit(this->tu);

	return GENERIC_SUCCESS;
}