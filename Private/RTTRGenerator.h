#pragma once

#include "Common.h"
#include "CmdParser.h"
#include "ClangParser.h"

class RTTRGenerator
{
public:
    RTTRGenerator(CmdParser& cmd, ClangParser& parser);

    void Generate();

private:

    void WriteFunctionParameters(ReflectionDefinition& item);
    void WriteEnumValues(ReflectionDefinition& item);
    void WriteMetaTags(ReflectionDefinition& item);
    void WriteMetaData(ReflectionDefinition& item);
    void WriteSubProps(ReflectionDefinition& item);

    void WriteConstructors(ReflectionDefinition& item);

    void WriteSpecialTags(ReflectionDefinition& item);

    fstream strm;

    // Copied values from CmdParser
    bool UseClassPath;
    bool EnableHotswap;

    string Module;

    // Output file from CmdParser
    fs::path OutputFile;
    
    // Definitions from ClandParser
    vector<ReflectionDefinition>& Definitions;
};