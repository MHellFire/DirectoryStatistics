/*
	Application that analyzes and shows statistics of specified directory that handles recursive directories.

	Usage:
	1. AppName.exe DirectoryPath
	2. AppName.exe

	TODO:
	1. make multithread recursive directories analyze
	2. more unit tests
*/

#include <iostream>

#include "threadpool.h"
#include "functions.h"


//typedef unsigned long long int uint64;

// operator+ for std::pair
//template <typename T, typename U>
//std::pair<T, U> operator+(const std::pair<T, U>& l, const std::pair<T, U>& r)
//{
//	return { l.first + r.first,l.second + r.second };
//}

int main(int argc, char* argv[])
{
	// obtain directory path
	const std::filesystem::path dirPath = [&]() -> std::filesystem::path {
		if (argc >= 2)
		{
			return argv[1];
		}
		else
		{
			std::string dirPathStr;
			std::cout << "Folder path: ";
			std::getline(std::cin, dirPathStr);

			// check string
			if (dirPathStr.empty())
			{
				std::cout << "Empty string!" << std::endl;
			}

			return dirPathStr;
		}
	}();

	// check path
	if (!std::filesystem::exists(dirPath))
	{
		std::cout << "Path \"" << dirPath << "\"does not exists!" << std::endl;
		return 1;
	}

	// change the current working directory
	std::filesystem::current_path(dirPath);

	// set number of threads
	// std::thread::hardware_concurrency()  - if this value is not computable or well defined, the function returns 0.
	size_t numThreads = std::thread::hardware_concurrency();
	if (numThreads == 0)
		numThreads = 1;
	std::cout << "\nNumber of hardware thread context: " << numThreads << std::endl;
	//std::cout << "Enter threads number: ";
	//std::cin >> numThreads;

	// create threads pool
	ThreadPool pool(numThreads);

	int numFolders = 0, numFiles = 0, filesystemErrors = 0;
	std::vector<std::future<std::array<uint64_t, 5>>> futures;

	// analyze specified directory
	for (auto& p : std::filesystem::recursive_directory_iterator(std::filesystem::current_path(), std::filesystem::directory_options::skip_permission_denied))
	{
		try
		{
			if (std::filesystem::exists(p.path()))
			{
				if (!std::filesystem::is_directory(p.path()) && std::filesystem::is_regular_file(p.path()))
				{
					// file

					numFiles++;

					std::filesystem::path file = p.path();
					futures.push_back(pool.enqueue(countLines, file));
				}
				else
				{
					// directory

					++numFolders;
				}
			}
		}
		catch (std::filesystem::filesystem_error const& error)
		{
			++filesystemErrors;
			std::cout << error.what() << std::endl;
		}
	}

	std::array<uint64_t, 5> res = { 0, 0, 0, 0, 0 }, r = { 0, 0, 0, 0, 0 };

	for (auto& f : futures)
	{
		r = f.get();
		res[0] += r[0];
		res[1] += r[1];
		res[2] += r[2];
		res[3] += r[3];
		res[4] += r[4];
	}

	// show statistics
	std::cout << std::endl;
	std::cout << "Folders: " << numFolders << std::endl;
	std::cout << "Files: " << numFiles << std::endl;
	std::cout << "Empty lines: " << res[0] << std::endl;
	std::cout << "Non-empty lines: " << res[1] << std::endl;
	std::cout << "Total lines: " << res[0] + res[1] << std::endl;
	std::cout << "Letters: " << res[2] << std::endl;
	std::cout << "Words: " << res[3] << std::endl;
	std::cout << "Characters: " << res[4] << std::endl << std::endl;
	std::cout << "File system errors: " << filesystemErrors << std::endl;

	return 0;
}
