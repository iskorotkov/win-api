#include "files.h"
#include <Windows.h>
#include "commandline.h"

ReadingMethod GetReadingMethod(int argc, char** argv)
{
	ReadingMethod Method = ReadingMethod::Unknown;
	for (UINT i = 1; i < argc; ++i)
	{
		// TODO: Ignore strings case.
		const char* command = argv[i];
		if (strcmp(command, "-filemapping") == 0)
		{
			Method = ReadingMethod::FileMapping;
		}
		else if (strcmp(command, "-variables") == 0)
		{
			Method = ReadingMethod::Variables;
		}
		else if (strcmp(command, "-streams") == 0)
		{
			Method = ReadingMethod::Streams;
		}
		else if (strcmp(command, "-winapi") == 0)
		{
			Method = ReadingMethod::WinApi;
		}
	}
	return Method;
}
