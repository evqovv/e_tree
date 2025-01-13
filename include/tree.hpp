#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <format>
#include <stdexcept>
#include <fast_io.h>

namespace evqovv {
class tree_list {
public:
    void run(int argc, char **argv) {
        auto tree_path = parse({ argv + 1, argv + argc });

        ::fast_io::perrln(tree_path);

        walk(tree_path, "", 1);

        auto to_print = ::std::format("{} {}, {} {}", total_directories, (total_directories == 1 ? "directory" : "directories"), total_files, (total_files == 1 ? "file" : "files"));
        ::fast_io::perrln(to_print);
    }

private:
    ::std::string parse(::std::vector<::std::string> const &parts) {
        bool not_set = true;
        ::std::string tree_path;

        for (decltype(parts.size()) i = 0; i != parts.size(); ++i) {
            if (parts[i] == "-a") {
                display_all_files = true;
            } else if (parts[i] == "-d") {
                if (i + 1 == parts.size()) {
                    throw ::std::runtime_error("Parsing failed.");
                } else {
                    depth = ::std::stoull(parts[i + i]);
                    if (depth == 0) {
                        throw ::std::runtime_error("The depth of recursion can't be 0.");
                    }
                }
            } else {
                if (not_set) {
                    tree_path = parts[i];
                    if (!::std::filesystem::exists(tree_path)) {
                        throw ::std::runtime_error("The path doesn't exist.");
                    }
                    if (!::std::filesystem::is_directory(tree_path)) {
                        throw ::std::runtime_error("The specified path is not a directory.");
                    }

                    not_set = false;
                } else {
                    throw ::std::runtime_error("Parsing failed.");
                }
            }
        }

        return tree_path;
    }

    void walk(::std::filesystem::path const &root, ::std::string const &prefix, unsigned cur_depth) {
        ::std::vector<::std::filesystem::directory_entry> entries;

        for (auto &&entry : ::std::filesystem::directory_iterator(root)) {
            entries.push_back(entry);
        }

        for (::std::size_t i = 0; i != entries.size(); ++i) {
            auto to_print = ::std::format("{}{} {}{}",
                prefix,
                (i == entries.size() - 1 ? "└──" : "├──"),
                entries[i].path().filename().string(), (std::filesystem::is_symlink(entries[i]) ?
                    " -> " + std::filesystem::read_symlink(entries[i].path()).string() :
                    ""));

            ::fast_io::perrln(to_print);

            if (entries[i].is_directory()) {
                ++total_directories;

                if (!::std::filesystem::is_symlink(entries[i])) {
                    if (cur_depth < depth) {
                        walk(root / entries[i].path().filename(), prefix + (i == entries.size() - 1 ? "    " : "│   "), cur_depth + 1);
                    }
                }
            } else {
                ++total_files;
            }
        }
    }

    bool display_all_files = false;
    unsigned depth = ::std::numeric_limits<unsigned>::max();

    ::std::size_t total_files = 0;
    ::std::size_t total_directories = 0;
};
}