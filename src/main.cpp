#include "e_tree_list.hpp"

int main(int argc, char **argv) {
    evqovv::e_tree_list e_tree_list;

    try {
        e_tree_list.run(argc, argv);
    } catch (::std::exception const &e) {
        ::fast_io::perrln(::fast_io::mnp::os_c_str(e.what()));
    }
}
