#include "Common.h"

#include "CmdParser.h"
#include "ClangParser.h"
#include "Timestamp.h"
#include "RTTRGenerator.h"

int main(int argc, char** argv)
{
	// Create a command line parser
	CmdParser CmdParse = CmdParser(argc, argv);

	// This function calls exit on error
	CmdParse.ParseCommandLine();

	// If we did not find a source file exit the program
	if (!fs::exists(CmdParse.SourceFile))
	{
		cerr << "No file to parse. (For more info see -help)" << endl;
		return GENERIC_FILE_ERROR;
	}

	// check if the file we were parse ends in .h does not matter if its hpp
	string source_path = CmdParse.SourceFile.generic_string();
	size_t found = source_path.find_last_of(".h");

	if (found == string::npos) 
	{
		cerr << "File \"" << source_path << "\" not a recognized header." << endl;
		return GENERIC_FILE_ERROR;
	}

	// Create a timestamp managment object
	Timestamp timestamp(CmdParse);
	
	// If we do not require an update exit
	if (!timestamp.NeedsUpdate())
		return GENERIC_SUCCESS;

	// Create a clang parser
	ClangParser Parser(CmdParse);

	if (Parser.Parse() != GENERIC_SUCCESS)
		return GENERIC_PARSER_ERROR;

	// Support wide character paths on all io operations in the future
	CmdParse.OutputFile = CmdParse.SourceFile;

	// Set the output file extension .generated.input_extension
	CmdParse.OutputFile.replace_extension(string(".generated") + CmdParse.OutputFile.extension().string());

	// Set the output directory if we have one
	if (!CmdParse.OutputDirectory.empty())
	{
		fs::path outputDir = CmdParse.OutputDirectory;
		CmdParse.OutputFile = outputDir.append(CmdParse.OutputFile.filename().generic_string());
	}

	MakeAbsolutePath(CmdParse.OutputFile);

	// If we have some reflection defintions write them to a file otherwise just exit
	if (!Parser.Definitions.empty())
	{
		RTTRGenerator Generator(CmdParse, Parser);

		Generator.Generate();

		if(CmdParse.UseTimeStamps)
			timestamp.Update();
	}

	return GENERIC_SUCCESS;
}
