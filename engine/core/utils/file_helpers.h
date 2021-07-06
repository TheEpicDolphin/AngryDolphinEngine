#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace file_helpers {

	std::vector<fs::path> AllFilePathsInDirectoryWithExtension(std::string directory_path, std::string extension) {
		std::vector<fs::path> paths;
		for (auto& iter : fs::recursive_directory_iterator(directory_path))
		{
			if (iter.path().extension() == extension) {
				paths.push_back(iter.path());
				std::ifstream ifs(iter.path());
			}
		}
		return paths;
	}

	std::vector<char> ReadFileWithPath(fs::path path) 
	{
		std::string file_path = path.name;
		std::ifstream file_stream(path, std::ios::in);
		if (file_stream.is_open()) {
			std::stringstream sstr;
			sstr << file_stream.rdbuf();
			std::string file_string = sstr.str();
			std::vector<char> file_contents(file_string.begin(), file_string.end());
			file_stream.close();
			return file_contents;
		}
		else {
			std::cout << "Failed to open file with path: " << file_path << std::endl;
			return std::vector<char>();
		}
	}
}