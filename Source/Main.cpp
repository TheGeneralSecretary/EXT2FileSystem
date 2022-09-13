#include "buildpch.h"
#include "Core/Logger.h"

#include <iostream>

int main()
{
    EXT2FS::Logger::Init();
	LOGGER_INFO("TESTER");
}
