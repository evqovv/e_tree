#pragma once

#include <fast_io.h>
#include <algorithm>
#include <cstddef>
#include <limits>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>

namespace evqovv {
class tree_list {
public:
    void run(int argc, char **argv) {
        auto tree_path = parse({ argv + 1, argv + argc });
        walk(tree_path, "", 1);
    }

private:
    ::std::filesystem::path parse(::std::vector<::std::string> const &parts) {
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
            ::std::string to_print(prefix);
            to_print += (i == entries.size() - 1 ? "└── " : "├── ");
            to_print += entries[i].path().filename().string();
            if (::std::filesystem::is_symlink(entries[i])) {
                to_print += " -> ";
                to_print += ::std::filesystem::read_symlink(entries[i].path());
            }

            ::fast_io::perrln(to_print);

            if (entries[i].is_directory() && !::std::filesystem::is_symlink(entries[i])) {
                if (cur_depth < depth) {
                    walk(root / entries[i].path().filename(), prefix + (i == entries.size() - 1 ? "    " : "│   "), cur_depth + 1);
                }
            }
        }
    }

    bool display_all_files = false;
    unsigned depth = ::std::numeric_limits<unsigned>::max();
};
}