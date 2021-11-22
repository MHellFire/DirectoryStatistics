#include "pch.h"

#include "../Main/functions.h"

namespace functions {

    TEST(functions, checkCountingLettersWords) {
        auto tmp = std::make_tuple(0, 0, 0);
        tmp = countLettersWords("Ala ma kota!");
        
        // letters
        EXPECT_EQ(std::get<0>(tmp), 9);
        // words
        EXPECT_EQ(std::get<1>(tmp), 3);
        // characters
        EXPECT_EQ(std::get<2>(tmp), 12);
    }

    int main(int argc, char* argv[])
    {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }
}