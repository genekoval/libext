#include <ext/math.h>

#include <gtest/gtest.h>

TEST(Digits, Zero) { ASSERT_EQ(1, ext::digits(0)); }

TEST(Digits, One) { ASSERT_EQ(1, ext::digits(1)); }

TEST(Digits, Ten) { ASSERT_EQ(2, ext::digits(10)); }

TEST(Digits, OneHundred) { ASSERT_EQ(3, ext::digits(100)); }

TEST(Digits, NegativeOne) { ASSERT_EQ(1, ext::digits(-1)); }

TEST(Digits, NegativeOneThousand) { ASSERT_EQ(3, ext::digits(-1'000)); }
