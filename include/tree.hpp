#pragma once

#include <algorithm>
#include <filesystem>
#include <format>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <fast_io.h>

namespace evqovv {
class tree_list {
public:
  void run(int argc, char **argv) {
    options.init();

    parse({argv + 1, argv + argc});

    for (auto &&tree_path : options.tree_paths) {
      ::fast_io::perrln(tree_path);
      walk(tree_path, "", 1);
      ::fast_io::perrln("");
    }

    ::std::string to_print =
        ::std::format("{} {}, {} {}.", total_directories,
                      (total_directories == 1 ? "directory" : "directories"),
                      total_files, (total_files == 1 ? "file" : "files"));
    ::fast_io::perrln(to_print);
  }

private:
  void
  sort_by_filename(::std::vector<::std::filesystem::directory_entry> &entries) {
    auto sort_by_filename =
        [](::std::filesystem::directory_entry const &a,
           ::std::filesystem::directory_entry const &b) -> bool {
      return a.path().filename() < b.path().filename();
    };

    ::std::sort(entries.begin(), entries.end(), sort_by_filename);
  }

  void parse(::std::vector<::std::string_view> const &parts) {
    for (decltype(parts.size()) i = 0; i != parts.size(); ++i) {
      if (parts[i][0] == '-') {
        decltype(i) param_cnt = 0;
        for (auto &&ch : parts[i].substr(1)) {
          switch (ch) {
          case 'a':
            options.display_all_files = true;
            break;
          case 'd':
            options.only_display_directories = true;
            break;
          case 'r':
            options.reverse_order = true;
            break;
          case 'z':
            options.display_file_size = true;
            break;
          case 'L':
            ++param_cnt;
            if (i + param_cnt >= parts.size()) {
              throw ::std::runtime_error("Missing argument to -L option.");
            }
            options.max_depth =
                ::std::stoull(::std::string(parts[i + param_cnt]));
            break;
          default:
            throw ::std::runtime_error("Parsing failed.");
            break;
          }
        }
        i += param_cnt;
      } else {
        options.tree_paths.emplace_back(parts[i]);
      }
    }

    if (options.tree_paths.empty()) {
      options.tree_paths.emplace_back(".");
    }
  }

  void walk(::std::filesystem::path const &root, ::std::string const &prefix,
            unsigned cur_depth) {
    ::std::vector<::std::filesystem::directory_entry> entries;

    for (auto &&entry : ::std::filesystem::directory_iterator(root)) {
      entries.push_back(entry);
    }

    for (::std::size_t i = 0; i != entries.size(); ++i) {
      ::std::string to_print =
          prefix + (i == entries.size() - 1 ? "└── " : "├── ") +
          entries[i].path().filename().string() +
          (std::filesystem::is_symlink(entries[i])
               ? " -> " +
                     std::filesystem::read_symlink(entries[i].path()).string()
               : "");
      ::fast_io::perrln(to_print);

      if (entries[i].is_directory()) {
        ++total_directories;

        if (!::std::filesystem::is_symlink(entries[i])) {
          if (cur_depth < options.max_depth) {
            walk(root / entries[i].path().filename(),
                 prefix + (i == entries.size() - 1 ? "    " : "│   "),
                 cur_depth + 1);
          }
        }
      } else {
        ++total_files;
      }
    }
  }

  ::std::string concatenate_string(::std::string const &prefix,
                                   ::std::filesystem::path const &path,
                                   bool last) const {
    ::std::string str = prefix + (last ? "└── " : "├── ");

    if (options.display_file_size) {
      str += ::std::format("[{}] ", ::std::filesystem::file_size(path));
    }

    str += path;

    if (::std::filesystem::is_symlink(path)) {
      str += " -> ";
      str += std::filesystem::read_symlink(path);
    }

    return str;
  }

  struct {
    bool display_all_files;
    bool display_file_size;
    bool only_display_directories;
    bool reverse_order;

    unsigned max_depth;
    ::std::vector<::std::filesystem::path> tree_paths;

    void init() noexcept {
      display_all_files = false;
      display_file_size = false;
      only_display_directories = false;
      reverse_order = false;

      max_depth = ::std::numeric_limits<unsigned>::max();
      tree_paths.clear();
    }
  } options;

  ::std::size_t total_files = 0;
  ::std::size_t total_directories = 0;
};
} // namespace evqovv