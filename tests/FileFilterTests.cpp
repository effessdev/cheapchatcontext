#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

// Replace with the correct header.
#include "../src/FileFilter.h"

namespace fs = std::filesystem;

class FilterFilesTest : public ::testing::Test
{
protected:
    std::vector<fs::path> files{
        "README.md",
        "LICENSE",

        "src/main.cpp",
        "src/util.cpp",
        "src/util.hpp",

        "src/foo/bar.cpp",
        "src/foo/bar.hpp",
        "src/foo/test.cpp",

        "src/foo/deep/file.cpp",

        "include/lib.hpp",
        "include/detail/helper.hpp",

        "test/main_test.cpp",
        "test/data/input.txt",
    };
};

TEST_F(FilterFilesTest, EmptyPatternsReturnsNothing)
{
    auto result = filterFiles(files, {}, {});

    EXPECT_TRUE(result.empty());
}

TEST_F(FilterFilesTest, IncludeEverything)
{
    auto result = filterFiles(
        files,
        {"**"},
        {});

    EXPECT_EQ(result.size(), files.size());
}

TEST_F(FilterFilesTest, IncludeSingleFile)
{
    auto result = filterFiles(
        files,
        {"README.md"},
        {});

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], fs::path("README.md"));
}

TEST_F(FilterFilesTest, IncludeSingleDirectoryWildcard)
{
    auto result = filterFiles(
        files,
        {"src/*.cpp"},
        {});

    std::vector<fs::path> expected{
        "src/main.cpp",
        "src/util.cpp",
    };

    EXPECT_EQ(result, expected);
}

TEST_F(FilterFilesTest, RecursiveInclude)
{
    auto result = filterFiles(
        files,
        {"src/**/*.cpp"},
        {});

    std::vector<fs::path> expected{
        "src/foo/bar.cpp",
        "src/foo/test.cpp",
        "src/foo/deep/file.cpp",
    };

    EXPECT_EQ(result, expected);
}

TEST_F(FilterFilesTest, MultipleIncludePatterns)
{
    auto result = filterFiles(
        files,
        {"README.md",
         "include/**"},
        {});

    std::vector<fs::path> expected{
        "README.md",
        "include/lib.hpp",
        "include/detail/helper.hpp",
    };

    EXPECT_EQ(result, expected);
}

TEST_F(FilterFilesTest, ExcludeOverridesInclude)
{
    auto result = filterFiles(
        files,
        {"**"},
        {"**/*.hpp"});

    for (const auto &file : result)
    {
        EXPECT_NE(file.extension(), ".hpp");
    }
}

TEST_F(FilterFilesTest, ExcludeSpecificFile)
{
    auto result = filterFiles(
        files,
        {"**"},
        {"src/foo/test.cpp"});

    EXPECT_EQ(
        std::find(result.begin(), result.end(), fs::path("src/foo/test.cpp")),
        result.end());
}

TEST_F(FilterFilesTest, MultipleExcludePatterns)
{
    auto result = filterFiles(
        files,
        {"**"},
        {"**/*.hpp",
         "**/test.cpp"});

    for (const auto &file : result)
    {
        EXPECT_NE(file.extension(), ".hpp");
        EXPECT_NE(file.filename(), "test.cpp");
    }
}

TEST_F(FilterFilesTest, IncludeThenExclude)
{
    auto result = filterFiles(
        files,
        {"src/**",
         "include/**"},
        {"src/foo/**"});

    std::vector<fs::path> expected{
        "src/main.cpp",
        "src/util.cpp",
        "src/util.hpp",
        "include/lib.hpp",
        "include/detail/helper.hpp",
    };

    EXPECT_EQ(result, expected);
}

TEST_F(FilterFilesTest, NoMatches)
{
    auto result = filterFiles(
        files,
        {"does/not/exist/**"},
        {});

    EXPECT_TRUE(result.empty());
}

TEST_F(FilterFilesTest, ExcludingNonIncludedFilesHasNoEffect)
{
    auto result = filterFiles(
        files,
        {"README.md"},
        {"src/**"});

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], fs::path("README.md"));
}

TEST_F(FilterFilesTest, MultipleIncludesDoNotDuplicateFiles)
{
    auto result = filterFiles(
        files,
        {"**",
         "src/**",
         "src/main.cpp"},
        {});

    EXPECT_EQ(result.size(), files.size());
}

TEST_F(FilterFilesTest, RecursiveExclude)
{
    auto result = filterFiles(
        files,
        {"**"},
        {"src/foo/**"});

    EXPECT_EQ(
        std::find(result.begin(), result.end(), fs::path("src/foo/bar.cpp")),
        result.end());

    EXPECT_EQ(
        std::find(result.begin(), result.end(), fs::path("src/foo/deep/file.cpp")),
        result.end());
}

TEST_F(FilterFilesTest, PreserveOriginalOrder)
{
    auto result = filterFiles(
        files,
        {"**"},
        {"**/*.hpp"});

    for (std::size_t i = 1; i < result.size(); ++i)
    {
        auto lhs = std::find(files.begin(), files.end(), result[i - 1]);
        auto rhs = std::find(files.begin(), files.end(), result[i]);

        EXPECT_LT(lhs, rhs);
    }
}
