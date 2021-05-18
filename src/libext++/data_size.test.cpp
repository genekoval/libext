#include <gtest/gtest.h>

#include <ext/data_size.h>

using namespace ext::literals;

TEST(DataSize, ZeroBytes) {
    const auto size = ext::data_size::format(0);

    ASSERT_EQ(0, size.bytes);
    ASSERT_EQ(0, size.value);
    ASSERT_EQ(ext::byte_multiple::B, size.multiple);
    ASSERT_EQ("0 B", size.str(3));
}

TEST(DataSize, OneByte) {
    const auto size = ext::data_size::format(1);

    ASSERT_EQ(1, size.bytes);
    ASSERT_EQ(1, size.value);
    ASSERT_EQ(ext::byte_multiple::B, size.multiple);
    ASSERT_EQ("1 B", size.str(3));
}

TEST(DataSize, CreateFromKibibyte) {
    constexpr auto value = 1;
    constexpr auto multiple = ext::byte_multiple::KiB;
    const auto size = ext::data_size(value, multiple);

    ASSERT_EQ(1'024, size.bytes);
    ASSERT_EQ(value, size.value);
    ASSERT_EQ(multiple, size.multiple);
}

TEST(DataSize, FormatBytes) {
    constexpr auto bytes = 64;
    const auto size = ext::data_size::format(bytes);

    ASSERT_EQ(bytes, size.bytes);
    ASSERT_EQ(bytes, size.value);
    ASSERT_EQ(ext::byte_multiple::B, size.multiple);
    ASSERT_EQ("64 B", size.str(2));
}

TEST(DataSize, FormatKibibyte) {
    constexpr auto bytes = 1'024;
    const auto size = ext::data_size::format(bytes);

    ASSERT_EQ(bytes, size.bytes);
    ASSERT_EQ(1, size.value);
    ASSERT_EQ(ext::byte_multiple::KiB, size.multiple);
    ASSERT_EQ("1 KiB", size.str(2));
}

TEST(DataSize, FormatMebibyteWithDecimal) {
    constexpr auto bytes = 3'407'872;
    const auto size = ext::data_size::format(bytes);

    ASSERT_EQ(bytes, size.bytes);
    ASSERT_EQ(3.25, size.value);
    ASSERT_EQ(ext::byte_multiple::MiB, size.multiple);
    ASSERT_EQ("3.25 MiB", size.str(3));
}

TEST(DataSize, FormatGibibtyesWithRound) {
    constexpr auto bytes = 2'530'809'479; // approximately 2.357 gibibytes
    const auto size = ext::data_size::format(bytes);

    ASSERT_EQ(bytes, size.bytes);
    ASSERT_TRUE(size.value < 2.357 && size.value > 2.356);
    ASSERT_EQ(ext::byte_multiple::GiB, size.multiple);
    ASSERT_EQ("2.36 GiB", size.str(2));
}

TEST(DataSize, LiteralKibibyte) {
    ASSERT_EQ(1'024, 1_KiB);
}

TEST(DataSize, LiteralMebibyte) {
    ASSERT_EQ(1'048'576, 1_MiB);
}

TEST(DataSize, LiteralGibibyte) {
    ASSERT_EQ(1'073'741'824, 1_GiB);
}

TEST(DataSize, LiteralTebibyte) {
    ASSERT_EQ(1'099'511'627'776, 1_TiB);
}

TEST(DataSize, LiteralKibibyteMultiple) {
    ASSERT_EQ(8'192, 8_KiB);
}

TEST(DataSize, LiteralGibibyteMultiple) {
    ASSERT_EQ(2'530'809'479, 2.357_GiB);
}
