#pragma once

#include <string>
#include <vector>
#include <ostream>

namespace EXT2FS
{
	struct EXT2Path
	{
		std::string path;
		std::vector<std::string> parts;
		std::string root;
		std::string target;

		friend std::ostream& operator<<(std::ostream& os, const EXT2Path& path);

		operator bool() { return root == "/"; }
	};

	static std::ostream& operator<<(std::ostream& os, const EXT2Path& path)
	{
		os << "Path: " << path.path << "\nParts: ";
		for (const auto& part : path.parts) os << part << " ";
		os << "\nRoot: " << path.root << "\nTarget: " << path.target << std::endl;
		return os;
	}

	EXT2Path EXT2ParsePath(const std::string& path);
}
