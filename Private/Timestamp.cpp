#include "Timestamp.h"

Timestamp::Timestamp(CmdParser& cmd) :
	UseTimestamps(false)
{
    UseTimestamps = cmd.UseTimeStamps;
    TimestampDirectory = cmd.TimestampDirectory;
    SourceFile = cmd.SourceFile;
}

bool Timestamp::NeedsUpdate()
{
    if(!UseTimestamps)
        return true;

	fs::path infile = SourceFile;
	fs::path path = TimestampDirectory;

    path.append(infile.filename().generic_string());

	path.replace_extension(".generated" + infile.extension().generic_string() + ".timestamp");

	if (!fs::exists(path))
		return true;

	fstream strm;

	strm.open(path, fstream::in);

	if (!strm.is_open())
		return true;

	string oldStamp;

	strm >> oldStamp;

	string newStamp = GetTimestamp();

	if (oldStamp != newStamp)
		return true;

	return false;
}

void Timestamp::Update()
{
    fs::path infile = SourceFile;
	fs::path path = TimestampDirectory;

    path.append(infile.filename().generic_string());

	path.replace_extension(".generated" + infile.extension().generic_string() + ".timestamp");

	MakeAbsolutePath(path);

	fstream strm;

	strm.open(path, fstream::out);

	if (!strm.is_open())
		return;

	string timestamp = GetTimestamp();

	strm << timestamp;

	strm.flush();
	strm.close();
}

string Timestamp::GetTimestamp()
{
    auto ftime = fs::last_write_time(SourceFile).time_since_epoch();

	return to_string(ftime.count());
}