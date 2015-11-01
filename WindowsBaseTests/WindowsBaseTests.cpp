// WindowsBaseTests.cpp : Defines the entry point for the console application.
//

#include "gtest/gtest.h"
#include "WindowsBase/File.h"
#include <string>

class StringConversion : public ::testing::Test
{
    virtual void SetUp()
    {
    }

public:
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

int main(int argc, wchar_t ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

