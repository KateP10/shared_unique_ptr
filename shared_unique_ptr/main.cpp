#include "tests.h"
#include <iostream>

int main() { 
    
    test_unq_subtyping();
    test_shrd_subtyping();
    test_no_leak();
    test_container();
    bench_table();
    print_memory_table();
    return 0;
}