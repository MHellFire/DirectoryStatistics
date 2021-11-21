#pragma once

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
