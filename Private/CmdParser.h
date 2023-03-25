#pragma once

#include "Common.h"
#include "Utility.h"

class CmdParser
{
public:
	CmdParser(int argc, char** argv);

	fs::path SourceFile;
	fs::path OutputFile;
	fs::path OutputDirectory;
	
	string Module;
	bool UseClassPath;
	
	vector<string> IncludeDirectories;

	bool EnableHotswap;
	bool UseTimeStamps;

	fs::path TimestampDirectory;

	void ParseCommandLine();

private:
	int argc;
	char** argv;
};