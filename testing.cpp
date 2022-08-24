#include <gtest/gtest.h>

int myfunc(int n)
{
    return n + 1;
}

TEST(BasicTest, HandlesPositiveInput)
{
    ASSERT_EQ(myfunc(1), 2);
    EXPECT_EQ(myfunc(2), 3);
}

TEST(TsssTest, Dies)
{
    ASSERT_STRCASEEQ("MEre", "mErE");
    ASSERT_PRED1(myfunc, -2);
}