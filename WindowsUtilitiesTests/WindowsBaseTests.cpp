// WindowsBaseTests.cpp : Defines the entry point for the console application.
//

#include "gtest/gtest.h"
#include "WindowsUtilities/File.h"
#include "WindowsUtilities/Base.h"
#include <string>

class StringConversion : public ::testing::Test
{
    virtual void SetUp()
    {
    }

public:
};

class WindowsBaseTests : public ::testing::Test
{
    virtual void SetUp()
    {
    }
};

TEST_F(StringConversion, ConvertEqualStrings)
{
    std::string aTestStr("Foo");
    std::wstring aTestWString(L"Foo");
    ASSERT_EQ(aTestStr, Windows::Utilities::String::string_cast<std::string>(aTestStr));
    ASSERT_EQ(L"Foo", Windows::Utilities::String::string_cast<std::wstring>(aTestWString));
    ASSERT_EQ(aTestWString, Windows::Utilities::String::string_cast<std::wstring>(aTestWString));
}

TEST_F(StringConversion, ConvertStdToWStd)
{
    std::string aTestStr("Foo");
    std::wstring aWStr(L"Foo");
    ASSERT_EQ(aWStr, Windows::Utilities::String::string_cast<std::wstring>(aTestStr));
    ASSERT_EQ(aWStr, Windows::Utilities::String::string_cast<std::wstring>("Foo"));
}

TEST_F(StringConversion, ConvertWStdToStd)
{
    std::string aTestStr("Foo");
    std::wstring aWStr(L"Foo");
    ASSERT_EQ(aTestStr, Windows::Utilities::String::string_cast<std::string>(aWStr));
    ASSERT_EQ(aTestStr, Windows::Utilities::String::string_cast<std::string>(L"Foo"));
}

TEST_F(StringConversion, TestStringWildcardCompare1)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("", ""));
}

TEST_F(StringConversion, TestStringWildcardCompare2)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("something", "something"));
}

TEST_F(StringConversion, TestStringWildcardCompare3)
{
    ASSERT_FALSE(Windows::Utilities::String::wildcard_compare("something", "else"));
}

TEST_F(StringConversion, TestStringWildcardCompare4)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("s?m?th???", "something"));
}

TEST_F(StringConversion, TestStringWildcardCompare5)
{
    ASSERT_FALSE(Windows::Utilities::String::wildcard_compare("s?m?th???", "somethin"));
}

TEST_F(StringConversion, TestStringWildcardCompare6)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("*", ""));
}

TEST_F(StringConversion, TestStringWildcardCompare7)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("*", "nonsense"));
}

TEST_F(StringConversion, TestStringWildcardCompare8)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("non*", "nonsense"));
}

TEST_F(StringConversion, TestStringWildcardCompare9)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("*nonsense", "nonsense"));
}

TEST_F(StringConversion, TestStringWildcardCompare10)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("*non*nse", "nonsense"));
}

TEST_F(StringConversion, TestStringWildcardCompare11)
{
    ASSERT_FALSE(Windows::Utilities::String::wildcard_compare("*non*nse", "nonsenze"));
}

TEST_F(StringConversion, TestStringWildcardCompare12)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("*non*n?e", "nonsense"));
}

TEST_F(StringConversion, TestStringWildcardCompare13)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("n*on*nse", "nonsense"));
}

TEST_F(StringConversion, TestStringWildcardCompare14)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("n*n*nse", "nonsense"));
}

TEST_F(StringConversion, TestStringWildcardCompare15)
{
    ASSERT_FALSE(Windows::Utilities::String::wildcard_compare("*non*nse", "nonsenze"));
}

TEST_F(StringConversion, TestStringWildcardCompare16)
{
    ASSERT_TRUE(Windows::Utilities::String::wildcard_compare("n*n*n?e", "nonsense"));
}

TEST_F(WindowsBaseTests, FlushFileBuffersTest)
{
    //access denied by default
    ASSERT_THROW(Windows::Common::Base::FlushFileBuffers(L"\\\\.\\PHYSICALDRIVE0"), std::exception);
}

TEST_F(WindowsBaseTests, FileSizeTest)
{
    
}

int main(int argc, wchar_t ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

