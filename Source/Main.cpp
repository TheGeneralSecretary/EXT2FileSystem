#include "buildpch.h"
#include "Core/Logger.h"
#include "FileSystem/EXT2FileSystem.h"

#include "Util/linenoise.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace EXT2FS;

std::vector<std::string> StringSplit(const std::string& str, char delim)
{
	std::vector<std::string> data;
	std::stringstream ss(str);
	std::string line;
	while (std::getline(ss, line, delim)) data.push_back(line);
	return data;
}

int main()
{
    Logger::Init();
	
	EXT2FileSystem* ext2fs = new EXT2FileSystem();
	if (!ext2fs->Init())
	{
		LOGGER_ERROR("Failed to Init EXT2FS");
		return 1;
	}

	ext2fs->OpenDevice("ext2devdisk.img");
	//ext2fs->DumpSuperblock();
	//ext2fs->DumpBlockGroupDescriptors();
	ext2fs->CD("mydir");
	ext2fs->LS("");

	/*bool quit = false;
	while (!quit)
	{
		std::string line;
		quit = linenoise::Readline("EXT2FS> ", line);
		if (quit) break;
		if (!line.size()) continue;

		std::vector<std::string> args = StringSplit(line, ' ');
		
		if (args[0] == "help")
		{
			std::cout << "Commands:\n1. help\n2. exit\n3. ls\n";
		}
		else if (args[0] == "exit")
		{
			quit = true;
		}
		else if (args[0] == "ls")
		{

		}
		else
		{
			LOGGER_ERROR("Invalid Command: {}", line);
			LOGGER_INFO("use 'help' for Commands");
			continue;
		}

		linenoise::AddHistory(line.c_str());
	}*/

	ext2fs->Destroy();
	delete ext2fs;
	return 0;
}
