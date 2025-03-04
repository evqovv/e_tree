#pragma once

#include <filesystem>
#include <format>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <sys/statvfs.h>
#include <fast_io.h>

namespace evqovv {
class e_tree_list {
    enum class sort_option {
        none,
        name,
        time,
    };

    enum class first_option {
        none,
        files,
        directories,
    };

  public:
    void run(int argc, char **argv) {
        parse({argv + 1, argv + argc});

        for (auto &&tree_path : option.tree_paths) {
            ::fast_io::perrln(tree_path);
            list(tree_path, "", 1);
            ::fast_io::perrln("");
        }

        ::fast_io::perrln(::std::format(
            "{} {}, {} {}.", total_directories,
            (total_directories == 1 ? "directory" : "directories"), total_files,
            (total_files == 1 ? "file" : "files")));
    }

  private:
    void parse(::std::vector<::std::string_view> const &parts) {
        for (decltype(parts.size()) i{}; i != parts.size(); ++i) {
            if (parts[i][0] == '-') {
                if (parts[i][1] == '-') {
                    auto str = parts[i].substr(2);
                    if (str == "help") {
                        ::fast_io::perrln(R"(------- Listing options -------
-a           All files are listed.
-d           List directories only.
-f           Print the full path prefix for each file.
-n           Sort by the filename.
-r           Reverse the order of the sort.
-s           Print the size in bytes of each file.
-t           Sort by the last time the file was modified.
-L           Maximum depth limit for recursive directories.
-U           Leave files unsorted.
--dirsfirst  List directories before files.
--filesfirst List files before directories.
--help       Print usage and this help message and exit.)");
                        ::std::exit(0);
                    } else if (str == "dirsfirst") {
                        option.first = first_option::directories;
                    } else if (str == "filesfirst") {
                        option.first = first_option::files;
                    } else {
                        throw ::std::runtime_error("Parsing failed.");
                    }
                } else {
                    decltype(i) param_cnt{};
                    for (auto &&ch : parts[i].substr(1)) {
                        switch (ch) {
                        case 'a':
                            option.display_all_files = true;
                            break;
                        case 'd':
                            option.only_display_directories = true;
                            break;
                        case 'f':
                            option.display_full_path = true;
                            break;
                        case 'n':
                            option.sort = sort_option::name;
                            break;
                        case 'r':
                            option.reverse_order = true;
                            break;
                        case 's':
                            option.display_file_size = true;
                            break;
                        case 't':
                            option.sort = sort_option::time;
                            break;
                        case 'L':
                            ++param_cnt;
                            if (i + param_cnt >= parts.size()) {
                                throw ::std::runtime_error(
                                    "Missing argument to -L option.");
                            }
                            option.max_depth = ::std::stoul(
                                ::std::string(parts[i + param_cnt]));
                            break;
                        case 'U':
                            option.sort = sort_option::none;
                            option.first = first_option::none;
                            break;
                        default:
                            throw ::std::runtime_error("Parsing failed.");
                            break;
                        }
                    }
                    i += param_cnt;
                }
            } else {
                if (::std::find(option.tree_paths.cbegin(),
                                option.tree_paths.cend(),
                                parts[i]) == option.tree_paths.cend()) {
                    option.tree_paths.emplace_back(parts[i]);
                }
            }
        }

        if (option.tree_paths.empty()) {
            option.tree_paths.emplace_back(".");
        }
    }

    void
    sort(::std::vector<::std::filesystem::directory_entry> &entries) const {
        switch (option.sort) {
        case sort_option::name:
            ::std::sort(entries.begin(), entries.end(),
                        [](auto const &lhs, auto const &rhs) {
                            return lhs.path().filename() <
                                   rhs.path().filename();
                        });
            break;
        case sort_option::time:
            ::std::sort(entries.begin(), entries.end(),
                        [](auto const &lhs, auto const &rhs) {
                            return lhs.last_write_time() <
                                   rhs.last_write_time();
                        });
            break;
        default:
            break;
        }

        if (option.reverse_order) {
            ::std::reverse(entries.begin(), entries.end());
        }

        switch (option.first) {
        case first_option::directories:
            ::std::stable_sort(entries.begin(), entries.end(),
                               [](auto const &lhs, auto const &rhs) {
                                   return ::std::filesystem::is_directory(
                                              lhs) &&
                                          !::std::filesystem::is_directory(rhs);
                               });
            break;
        case first_option::files:
            ::std::stable_sort(entries.begin(), entries.end(),
                               [](auto const &lhs, auto const &rhs) {
                                   return !::std::filesystem::is_directory(
                                              lhs) &&
                                          ::std::filesystem::is_directory(rhs);
                               });
            break;
        default:
            break;
        }
    }

    void list(::std::filesystem::path const &root, ::std::string const &prefix,
              unsigned cur_depth) {
        ::std::vector<::std::filesystem::directory_entry> entries;

        for (auto &&entry : ::std::filesystem::directory_iterator(root)) {
            if (option.display_all_files ||
                entry.path().filename().string()[0] != '.') {
                entries.push_back(entry);
            }
        }

        sort(entries);

        for (decltype(entries.size()) i{}; i != entries.size(); ++i) {
            ::fast_io::perrln(concatenate_string(prefix, entries[i].path(),
                                                 i == entries.size() - 1));

            if (entries[i].is_directory()) {
                ++total_directories;

                if (!::std::filesystem::is_symlink(entries[i])) {
                    if (cur_depth < option.max_depth) {
                        list(root / entries[i].path().filename(),
                             prefix +
                                 (i == entries.size() - 1 ? "    " : "│   "),
                             cur_depth + 1);
                    }
                }
            } else {
                ++total_files;
            }
        }
    }

    static auto directory_size(::std::filesystem::path const &path) {
        struct statvfs fs_info{};

        if (::statvfs(path.c_str(), &fs_info) != 0) {
            throw ::std::runtime_error(::std::strerror(errno));
        }

        return fs_info.f_bsize;
    }

    ::std::string concatenate_string(::std::string const &prefix,
                                     ::std::filesystem::path const &path,
                                     bool last) const {
        ::std::string str = prefix + (last ? "└── " : "├── ");

        if (option.display_file_size) {
            str += "[";
            str += ::std::to_string(::std::filesystem::is_directory(path)
                                        ? directory_size(path)
                                        : ::std::filesystem::file_size(path));
            str += "] ";
        }

        str += option.display_full_path ? path : path.filename();

        if (::std::filesystem::is_symlink(path)) {
            str += " -> ";
            str += ::std::filesystem::read_symlink(path);
        }

        return str;
    }

    struct {
        bool display_all_files = false;
        bool display_file_size = false;
        bool display_full_path = false;
        bool reverse_order = false;
        bool only_display_directories = false;

        sort_option sort = sort_option::none;
        first_option first = first_option::none;

        unsigned max_depth = ::std::numeric_limits<unsigned>::max();
        ::std::vector<::std::filesystem::path> tree_paths;
    } option;

    ::std::size_t total_files{};
    ::std::size_t total_directories{};
};
} // namespace evqovv
