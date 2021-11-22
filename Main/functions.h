#pragma once

#include <fstream>
#include <string>
#include <filesystem>
#include <array>

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

/*
* void DisplayDirTree(const fs::path& pathToShow, int level)
{
    if (fs::exists(pathToShow) && fs::is_directory(pathToShow))
    {
        auto lead = std::string(level * 3, ' ');
        for (const auto& entry : fs::directory_iterator(pathToShow))
        {
            auto filename = entry.path().filename();
            if (fs::is_directory(entry.status()))
            {
                cout << lead << "[+] " << filename << "\n";
                DisplayDirTree(entry, level + 1);
                cout << "\n";
            }
            else if (fs::is_regular_file(entry.status()))
                DisplayFileInfo(entry, lead, filename);
            else
                cout << lead << " [?]" << filename << "\n";
        }
    }
}
void checkInDirectory (d : directory)

	for each entry e in d             <== recursive exit after last entry in directory
		if e is a file
			check_in_file(f)
		if e is a directory
			check_in_directory(e)     <== recursive call
*/