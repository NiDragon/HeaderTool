#pragma once

#include "Common.h"
#include "CmdParser.h"

class Timestamp
{
public:
    Timestamp(CmdParser& cmd);

    bool NeedsUpdate();

    void Update();

private:
    string GetTimestamp();

    bool UseTimestamps;

    fs::path TimestampDirectory;
    fs::path SourceFile;
};