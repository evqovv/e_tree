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
class tree_list {
public:
  void run(int argc, char **argv) {
    parse({argv + 1, argv + argc});

    for (auto &&tree_path : option.tree_paths) {
      ::fast_io::perrln(tree_path);
      walk(tree_path, "", 1);
      ::fast_io::perrln("");
    }

    ::fast_io::perrln(
        ::std::format("{} {}, {} {}.", total_directories,
                      (total_directories == 1 ? "directory" : "directories"),
                      total_files, (total_files == 1 ? "file" : "files")));
  }

private:
  void parse(::std::vector<::std::string_view> const &parts) {
    for (decltype(parts.size()) i = 0; i != parts.size(); ++i) {
      if (parts[i][0] == '-') {
        decltype(i) param_cnt = 0;
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
          case 's':
            option.display_file_size = true;
            break;
          case 'L':
            ++param_cnt;
            if (i + param_cnt >= parts.size()) {
              throw ::std::runtime_error("Missing argument to -L option.");
            }
            option.max_depth =
                ::std::stoull(::std::string(parts[i + param_cnt]));
            break;
          default:
            throw ::std::runtime_error("Parsing failed.");
            break;
          }
        }
        i += param_cnt;
      } else {
        option.tree_paths.emplace_back(parts[i]);
      }
    }

    if (option.tree_paths.empty()) {
      option.tree_paths.emplace_back(".");
    }
  }

  void walk(::std::filesystem::path const &root, ::std::string const &prefix,
            unsigned cur_depth) {
    ::std::vector<::std::filesystem::directory_entry> entries;

    for (auto &&entry : ::std::filesystem::directory_iterator(root)) {
      if (option.display_all_files ||
          entry.path().filename().string()[0] != '.') {
        entries.push_back(entry);
      }
    }

    for (::std::size_t i = 0; i != entries.size(); ++i) {
      ::fast_io::perrln(concatenate_string(prefix, entries[i].path(),
                                           i == entries.size() - 1));

      if (entries[i].is_directory()) {
        ++total_directories;

        if (!::std::filesystem::is_symlink(entries[i])) {
          if (cur_depth < option.max_depth) {
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
      str += std::filesystem::read_symlink(path);
    }

    return str;
  }

  struct {
    bool display_all_files = false;
    bool display_file_size = false;
    bool display_full_path = false;
    bool only_display_directories = false;

    unsigned max_depth = ::std::numeric_limits<unsigned>::max();
    ::std::vector<::std::filesystem::path> tree_paths;
  } option;

  ::std::size_t total_files = 0;
  ::std::size_t total_directories = 0;
};
} // namespace evqovv