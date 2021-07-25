#include <ext/except.h>
#include <ext/unix.h>

#include <fmt/format.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

namespace {
    constexpr auto default_buffer_length = 1024;
}

namespace ext {
    group::group(std::variant<gid_t, std::string> value) {
        auto buflen = sysconf(_SC_GETGR_R_SIZE_MAX);
        if (buflen == -1) buflen = default_buffer_length;

        buffer = std::unique_ptr<char[]>(new char[buflen]);

        auto ret = 0;
        ::group* result = nullptr;

        if (auto* gid = std::get_if<gid_t>(&value)) {
            ret = getgrgid_r(
                *gid,
                &data,
                buffer.get(),
                buflen,
                &result
            );
        }
        else if (auto* group_name = std::get_if<std::string>(&value)) {
            ret = getgrnam_r(
                group_name->c_str(),
                &data,
                buffer.get(),
                buflen,
                &result
            );
        }

        if (ret != 0) {
            std::visit([ret](auto&& arg) -> void {
                throw std::system_error(
                    ret,
                    std::generic_category(),
                    fmt::format("failed to look up group ({})", arg)
                );
            }, value);
        }

        if (!result) {
            std::visit([](auto&& arg) -> void {
                throw std::runtime_error(fmt::format(
                    "group ({}) does not exist", arg
                ));
            }, value);
        }
    }

    auto group::gid() const -> gid_t {
        return data.gr_gid;
    }

    auto group::name() const -> std::string_view {
        return data.gr_name;
    }

    auto group::password() const -> std::string_view {
        return data.gr_passwd;
    }

    user::user(std::variant<uid_t, std::string> value) {
        auto buflen = sysconf(_SC_GETPW_R_SIZE_MAX);
        if (buflen == -1) buflen = default_buffer_length;

        buffer = std::unique_ptr<char[]>(new char[buflen]);

        auto ret = 0;
        passwd* result = nullptr;

        if (auto* uid = std::get_if<uid_t>(&value)) {
            ret = getpwuid_r(
                *uid,
                &data,
                buffer.get(),
                buflen,
                &result
            );
        }
        else if (auto* username = std::get_if<std::string>(&value)) {
            ret = getpwnam_r(
                username->c_str(),
                &data,
                buffer.get(),
                buflen,
                &result
            );
        }

        if (ret != 0) {
            std::visit([ret](auto&& arg) -> void {
                throw std::system_error(
                    ret,
                    std::generic_category(),
                    fmt::format("failed to look up user ({})", arg)
                );
            }, value);
        }

        if (!result) {
            std::visit([](auto&& arg) -> void {
                throw std::runtime_error(fmt::format(
                    "user ({}) does not exist", arg
                ));
            }, value);
        }
    }

    auto user::home() const -> std::string_view {
        return data.pw_dir;
    }

    auto user::gid() const -> gid_t {
        return data.pw_gid;
    }

    auto user::info() const -> std::string_view {
        return data.pw_gecos;
    }

    auto user::name() const -> std::string_view {
        return data.pw_name;
    }

    auto user::password() const -> std::string_view {
        return data.pw_passwd;
    }

    auto user::shell() const -> std::string_view {
        return data.pw_shell;
    }

    auto user::uid() const -> uid_t {
        return data.pw_uid;
    }

    auto chown(
        const std::filesystem::path& path,
        const std::optional<user>& owner,
        const std::optional<group>& group
    ) -> void {
        const auto str = path.string();
        const auto uid = owner ? owner.value().uid() : getuid();
        const auto gid = group ? group.value().gid() : getgid();

        if (::chown(str.c_str(), uid, gid) == -1) {
            throw system_error(fmt::format(
                "changing ownership of '{}'",
                str
            ));
        }
    }

    auto exec(
        std::string_view program,
        std::span<const std::string_view> args
    ) -> void {
        auto argv = std::unique_ptr<char*[]>(new char*[args.size() + 2]);

        argv[0] = (char*) (program.data());

        for (auto i = 0ul; i < args.size(); ++i) {
            argv[i + 1] = (char*) args[i].data();
        }

        argv[args.size() + 1] = nullptr;

        if (execvp(program.data(), argv.get()) == -1) {
            throw ext::system_error(fmt::format(
                "Failed to execute '{}' command",
                program
            ));
        }
    }

    process::process(pid_t pid) : _pid(pid) {}

    auto process::fork() -> std::optional<process> {
        auto pid = ::fork();

        if (pid < 0) {
            throw ext::system_error("Failed to create child process");
        }

        if (pid > 0) return process(pid);
        return {};
    }

    auto process::pid() const -> pid_t {
        return _pid;
    }

    auto process::wait() const -> exit_status {
        auto info = siginfo_t();

        if (waitid(P_PID, _pid, &info, WEXITED) == -1) {
            throw ext::system_error("Failed to wait for child process");
        }

        return {
            .code = info.si_code,
            .status = info.si_status
        };
    }

    auto exec_bg(
        std::string_view program,
        std::span<const std::string_view> args
    ) -> process {
        const auto parent = process::fork();

        if (!parent) {
            exec(program, args);
        }

        return *parent;
    }

    auto wait_exec(
        std::string_view program,
        std::span<const std::string_view> args
    ) -> exit_status {
        return exec_bg(program, args).wait();
    }
}
