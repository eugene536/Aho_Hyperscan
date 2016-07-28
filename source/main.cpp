#include <iostream>

#include <LinearSearch.h>
#include <TrieSearch.h>
#include <Aho.h>
#include <Hyperscan.h>

#ifdef BENCHMARK
#  include <benchmarks.h>
#endif

#ifdef _GTEST
#  include <gtest/gtest.h>
#endif

using namespace std;
using namespace StringAlgos;

int main(int argc, char **argv) {
#ifdef BENCHMARK
    cerr << endl << "LinearSearch" << endl;
    startBM<LinearSearch>();
    cerr << endl << "Hyperscan" << endl;
    startBM<Hyperscan>();
    cerr << endl << "Aho" << endl;
    startBM<Aho>();
    cerr << endl << "TrieSearch" << endl;
    startBM<TrieSearch>();
#endif

#ifdef _GTEST
    std::cout << "Run tests..." << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#endif

    return 0;
}
