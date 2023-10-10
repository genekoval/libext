#pragma once

#include <filesystem>
#include <grp.h>
#include <optional>
#include <pwd.h>
#include <span>
#include <string>
#include <sys/wait.h>
#include <variant>
#include <vector>

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

    auto chown(const std::filesystem::path& path, uid_t uid, gid_t gid) -> void;

    auto chown(const std::filesystem::path& path, const user& owner) -> void;

    auto chown(const std::filesystem::path& path, const group& group) -> void;

    auto chown(
        const std::filesystem::path& path,
        const user& owner,
        const group& group
    ) -> void;

    auto chown(
        const std::filesystem::path& path,
        const std::optional<user>& owner,
        const std::optional<group>& group
    ) -> void;

    auto exec(std::string_view program, std::span<const std::string_view> args)
        -> void;

    struct exit_status {
        int code;
        int status;
    };

    class process {
        const pid_t _pid;

        process(pid_t pid);
    public:
        static auto fork() -> std::optional<process>;

        auto pid() const -> pid_t;

        auto wait() const -> exit_status;
    };

    auto exec_bg(
        std::string_view program,
        std::span<const std::string_view> args
    ) -> process;

    auto wait_exec(
        std::string_view program,
        std::span<const std::string_view> args
    ) -> exit_status;

    template <typename... Args>
    auto $(std::string_view program, Args&&... args) -> exit_status {
        const auto arg_list = std::vector<std::string_view> {args...};
        return wait_exec(program, arg_list);
    }
}
