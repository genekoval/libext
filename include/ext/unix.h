#pragma once

#include <filesystem>
#include <grp.h>
#include <optional>
#include <pwd.h>
#include <string>
#include <variant>

namespace ext {
    class group {
        std::unique_ptr<char[]> buffer;
        ::group data;
    public:
        group() = default;

        group(std::variant<gid_t, std::string> value);

        auto gid() const -> gid_t;

        auto name() const -> std::string_view;

        auto password() const -> std::string_view;
    };

    class user {
        std::unique_ptr<char[]> buffer;
        passwd data;
    public:
        user() = default;

        user(std::variant<uid_t, std::string> value);

        auto gid() const -> gid_t;

        auto home() const -> std::string_view;

        auto info() const -> std::string_view;

        auto name() const -> std::string_view;

        auto password() const -> std::string_view;

        auto shell() const -> std::string_view;

        auto uid() const -> uid_t;
    };

    auto chown(
        const std::filesystem::path& path,
        const std::optional<user>& owner = {},
        const std::optional<group>& group = {}
    ) -> void;
}
