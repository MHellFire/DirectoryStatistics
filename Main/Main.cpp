/*
	Application that analyzes and shows statistics of specified directory that handles recursive directories.

	Usage:
	1. AppName.exe DirectoryPath
	2. AppName.exe
*/

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <filesystem>
#include <array>

#include <condition_variable>
#include <functional>
#include <future>
#include <vector>
#include <thread>
#include <queue>

//typedef unsigned long long int uint64;

// operator+ for std::pair
template <typename T, typename U>
std::pair<T, U> operator+(const std::pair<T, U>& l, const std::pair<T, U>& r)
{
	return { l.first + r.first,l.second + r.second };
}

// function is counting letters, words, characters in given string
std::tuple<uint64_t, uint64_t, uint64_t> countLettersWords(const std::string& str)
{
	// letters
	int numLetters = 0;
	// words
	int numWords = 0;
	// characters
	int numCharacters = 0;

	if (!str.empty()) // the same as size == 0
	{
		/*
			assuming words are white - space separated
			for the "C" locale, white-space characters are any of:
			' '	(0x20)	space(SPC)
			'\t'	(0x09)	horizontal tab(TAB)
			'\n'	(0x0a)	newline(LF)
			'\v'	(0x0b)	vertical tab(VT)
			'\f'	(0x0c)	feed(FF)
			'\r' (0x0d)	carriage return (CR)

			Notice that what is considered a letter may depend on the locale being used;
			In the default "C" locale, a lowercase letter is any of : a b c d e f g h i j k l m n o p q r s t u v w x y z.
			An uppercase letter is any of : A B C D E F G H I J K L M N O P Q R S T U V W X Y.
		*/

		bool inWhiteSpaces = true;
		for (unsigned char const& c : str)
		{
			// characters
			++numCharacters; // without line endings

			// letters
			if (std::isalpha(c))
				++numLetters;

			// words
			// word count as in LibreOffice Writer
			if (std::isspace(c))
			{
				inWhiteSpaces = true;
			}
			else if (inWhiteSpaces)
			{
				++numWords;
				inWhiteSpaces = false;
			}
		}
	}

	return std::make_tuple(numLetters, numWords, numCharacters);
}

// function is counting empty and non-empy lines in given file
// the newline character is '\n'
// returns array { empty lines, non-empty lines, letters, words, characters }
std::array<uint64_t, 5> countLines(const std::filesystem::path& file)
{
	// open file in text mode
	std::ifstream in(file, std::ifstream::in);
	// open file in binary mode
	//std::ifstream in(file, std::ifstream::in | std::ifstream::binary);

	// count lines
	if (in)
	{
		unsigned long long int numEmptyLines = 0;
		unsigned long long int numNonEmptyLines = 0;

		// tuple of letters, words, characters
		std::tuple<uint64_t, uint64_t, uint64_t> numLettersWords;

		// for "getline" the newline character is '\n'
		for (std::string line; std::getline(in, line);)
		{
			if (line.empty()) // the same as size == 0
			{
				// empty lines
				++numEmptyLines;
			}
			else
			{
				// non-empty lines
				++numNonEmptyLines;

				numLettersWords = countLettersWords(line);
			}
		}

		in.close();

		// empty lines, non-empty lines, letters, words, characters
		return { numEmptyLines, numNonEmptyLines, std::get<0>(numLettersWords), std::get<1>(numLettersWords), std::get<2>(numLettersWords) };
	}
	else
	{
		// error while opening file

		in.close();
		//std::cout << "Error while opening file: " << file << std::endl;

		// empty lines, non-empty lines, letters, words, characters
		return { 0, 0, 0, 0, 0 };
	}
}

class ThreadPool
{
public:
	using Task = std::function<void()>;

	explicit ThreadPool(std::size_t numThreads)
	{
		start(numThreads);
	}

	~ThreadPool()
	{
		stop();
	}

	//template<class T>
	template<class T, class... Args>
	//	auto enqueue(T task)->std::future<decltype(task())>
	auto enqueue(T&& task, Args&&... args)->std::future<decltype(task(args...))>
	{
		auto wrapper = std::make_shared <std::packaged_task <decltype(task(args...)) () > >(std::bind(std::forward<T>(task), std::forward<Args>(args)...));

		{
			std::unique_lock<std::mutex> lock{ eventMutex };
			tasksQueue.emplace([=] {
				(*wrapper)();
				});
		}

		eventVar.notify_one();
		return wrapper->get_future();
	}

private:
	std::vector<std::thread> threads;

	std::condition_variable eventVar;

	std::mutex eventMutex;
	bool stopping = false;

	std::queue<Task> tasksQueue;

	void start(std::size_t numThreads)
	{
		for (auto i = 0u; i < numThreads; ++i)
		{
			threads.emplace_back([=] {
				while (true)
				{
					Task task;

					{
						std::unique_lock<std::mutex> lock{ eventMutex };

						eventVar.wait(lock, [=] { return stopping || !tasksQueue.empty(); });

						if (stopping && tasksQueue.empty())
							break;

						task = std::move(tasksQueue.front());
						tasksQueue.pop();
					}

					task();
				}
				});
		}
	}

	void stop() noexcept
	{
		{
			std::unique_lock<std::mutex> lock{ eventMutex };
			stopping = true;
		}

		eventVar.notify_all();

		for (auto& thread : threads)
			thread.join();
	}
};

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

	/*
	procedure check_in_directory (d : directory)

	for each entry e in d             <== recursive exit after last entry in directory
		if e is a file
			check_in_file(f)
		if e is a directory
			check_in_directory(e)     <== recursive call
	*/

	return 0;
}
