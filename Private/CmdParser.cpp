#include "CmdParser.h"

CmdParser::CmdParser(int argc, char** argv) :
    UseClassPath(false),
    EnableHotswap(false),
    UseTimeStamps(false)
{
    this->argc = argc;
    this->argv = argv;
}

void CmdParser::ParseCommandLine() {
    if (argc == 1)
    {
        PrintHelp();
        exit(GENERIC_FILE_ERROR);
    }

    for (int i = 0; i < argc; i++)
    {
        string arg = argv[i];

        if (arg == "-f")
        {
            if(!SourceFile.empty())
            {
                cerr << "Multiple files found only one may be specified!" << endl;
                exit(GENERIC_FILE_ERROR);
            }
            
            // If it is even possible to have a file after this point
            if (i + 1 < argc)
            {
                fs::path filepath = argv[i + 1];
                
                // Check if file was just an empty path
                if(filepath.filename() != "") 
                {
                    // Check if this file actually exists
                    if(fs::exists(filepath))
                    {
                        SourceFile = MakeAbsolute(filepath);
                        MakeAbsolutePath(SourceFile);
                    }
                    else // Provided file was not found
                    {
                        cerr << "Error: \"" << argv[i + 1] << "\" No such file found." << endl;
                        exit(GENERIC_FILE_ERROR);
                    }
                }
            }
            
            if(SourceFile.empty())
            {
                cerr << "No file to parse. (For more info see -help)" << endl;
                exit(GENERIC_FILE_ERROR);
            }
        }

        if (arg == "-i")
        {
            if (i + 1 < argc)
            {
                IncludeDirectories.push_back(MakeAbsolute(argv[i + 1]));
            }
        }

        if (arg == "-m")
        {
            if (i + 1 < argc)
            {
                Module = argv[i + 1];
                UseClassPath = true;
            }
        }

        if (arg == "-o")
        {
            if (i + 1 < argc)
            {
                OutputDirectory = argv[i + 1];

                fs::path path = OutputDirectory;

                if (!CreateDirectory(path))
                {
                    exit(FAILURE_OUTPUT_DIRECTORY);
                }

                OutputDirectory = path.generic_string();

                MakeAbsolutePath(OutputDirectory);
            }
        }

        if (arg == "-t")
        {
            if (i + 1 < argc)
            {
                UseTimeStamps = true;

                TimestampDirectory = argv[i + 1];

                if (!CreateDirectory(TimestampDirectory))
                {
                    UseTimeStamps = false;
                }

                MakeAbsolutePath(TimestampDirectory);
            }
        }

        if (arg == "-hotswap")
            EnableHotswap = true;

        if (arg == "-help")
        {
            PrintHelp();
            exit(GENERIC_SUCCESS);
        }
    }
}

void CmdParser::PrintHelp() {
    cout << "RTTR HeaderTool (C) 2023 Illusionist Softworks" << endl << endl;
    cout << "Supported Arguments: " << endl;
    cout << tabchar << "-f <filename>       - Source filename to be parsed." << endl;
    cout << tabchar << "-i <path>           - Include directory to resolve references." << endl;
    cout << tabchar << "-m <module name>    - Module name for class path generation." << endl;
    cout << tabchar << "-o <output path>    - Directory to write generated output to." << endl;
    cout << tabchar << "-t <timestamp path> - Directory to write timestamps to." << endl;
    cout << tabchar << "-hotswap            - Parse reflection info as plugin for hot reload." << endl;
    cout << tabchar << "-help               - Shows this information." << endl;
}