// BM_FIND        - time of Find with fixed dict in `war and peace`, text was appended to himself many times (~1.1GB)
// BM_RANDOM_FIND - time of Find with random dict with 100 words(100 characters) in random text 1GB

// benchmark results:
//LinearSearch
//  BM_INSERT: 0.000406
//  BM_DELETE: 0.000203
//  BM_BUILD: 1e-06
//  BM_FIND: 9.70157

//Aho
//  BM_INSERT: 0.04713
//  BM_DELETE: 0.012047
//  BM_BUILD: 0.047742
//  BM_FIND: 3.25081
//  BM_RANDOM_FIND: 0.559128

//AhoModified
//  BM_INSERT: 0.049656
//  BM_DELETE: 0.012167
//  BM_BUILD: 0.048488
//  BM_FIND: 2.94572
//  BM_RANDOM_FIND: 0.281513

//TrieSearch
//  BM_INSERT: 0.027562
//  BM_DELETE: 0.011311
//  BM_BUILD: 0
//  BM_FIND: 4.24023
//  BM_RANDOM_FIND: 1.03291

#include <iostream>
#include <fstream>
#include <algorithm>
#include <PatternSearch.h>
#include <iomanip>

namespace {

using namespace std;

class PatternSearchBenchmark {
public:
    PatternSearchBenchmark() {
        std::ifstream file;
        file.open("/home/nemchenko/work/tasks/sorm/sorm-probe/lib/StringAlgos/resources/war_peace");
        if (!file) {
            std::cerr << std::endl << "benchmark.h: CAN'T FIND TEXT" << std::endl << std::endl;
            return;
        }

        text = string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());


        std::vector<std::string> p{
            "CHAPTER",
            "reward",
//            "?...",
            "Pierre",
            "asdfasdf",
            "HelloAAA",
            "AHello",
            "Helloo",
            "123213",
//            "^%$&^",
//            "~~~~~~",
//            "!!!!!!!!",
            "rewardu",
            "qewrqwer",
            "iuzxycv",
            "CHAPTERX",
//            "~!@@#%&",
            "8762183476218934",
            "Pierre",
        };

        patterns.swap(p);
    }

    std::vector<std::string> patterns;
    std::string text;
};

struct PatternHandler {
    const int ALPH_SIZE = 26;

    PatternHandler(int cnt_pat = 1000, int max_len = 100) {
        for (int i = 0; i < cnt_pat; ++i) {
            int len = rand() % max_len + 1;
            std::string p;
            p.reserve(len);

            for (int j = 0; j < len; ++j) {
                p.push_back(char('a' + rand() % ALPH_SIZE));
            }

            patterns.push_back(p);

            if (rand() % 3 == 0)
                deleted.push_back(make_pair(p, i));
        }
    }

    vector<string> patterns;
    vector<pair<string, int>> deleted;
};

PatternHandler patternHandler;


using namespace StringAlgos;
using namespace std;

template<class PatternSearchT>
void BM_INSERT() {
    PatternSearchT ps;

    double start = clock();

    for (size_t i = 0; i < patternHandler.patterns.size(); ++i) {
        ps.Insert(patternHandler.patterns[i], i);
    }

    cerr << "  BM_INSERT: " << (clock() - start) / CLOCKS_PER_SEC << endl;
}


template<class PatternSearchT>
void BM_DELETE() {
    PatternSearchT ps;

    for (size_t i = 0; i < patternHandler.patterns.size(); ++i) {
        ps.Insert(patternHandler.patterns[i], i);
    }

    double start = clock();

    for (auto & pp: patternHandler.deleted) {
        ps.Delete(pp.first, pp.second);
    }

    cerr << "  BM_DELETE: " << (clock() - start) / CLOCKS_PER_SEC << endl;
}

template<class PatternSearchT>
void BM_BUILD() {
    PatternSearchT ps;

    for (size_t i = 0; i < patternHandler.patterns.size(); ++i) {
        ps.Insert(patternHandler.patterns[i], i);
    }

    double start = clock();

    ps.Build();

    cerr << "  BM_BUILD: " << (clock() - start) / CLOCKS_PER_SEC << endl;
}

PatternSearchBenchmark psb;

template<class PatternSearchT>
void BM_FIND() {
    if (psb.patterns.empty()) {
        return;
    }

    PatternSearchT ps;

    for (size_t i = 0; i < psb.patterns.size(); ++i) {
        ps.Insert(psb.patterns[i], i);
    }
    ps.Build();

    double start = clock();

    cerr << "  cnt: " << ps.Find(psb.text).size() << endl;
    cerr << "  BM_FIND: " << (clock() - start) / CLOCKS_PER_SEC << endl;
}

string text;
vector<string> words;
vector<pair<string, int>> deleted;

int generator()
{
    const int CNT_W = 100;
    const int LEN_W = 100;
    const int LEN_T = 1e8;
    const int ALPH_SIZE = 10;

    string word;
    word.reserve(LEN_W);
    deleted.clear();

    for (int j = 0; j < CNT_W; ++j) {
        const int lenW = rand() % LEN_W + 1;
        word.clear();

        for (int k = 0; k < lenW; ++k) {
            word.push_back(rand() % ALPH_SIZE + 'a');
        }

        words.push_back(word);

        if (rand() % 2)  deleted.push_back({word, j});
    }

    if (deleted.size()) deleted.pop_back();

    text.reserve(LEN_T);
    for (int k = 0; k < LEN_T; ++k) {
        text.push_back(rand() % ALPH_SIZE + 'a');
    }
}


template<class PatternSearchT>
void BM_RANDOM_FIND() {
    PatternSearchT ps;

    for (size_t i = 0; i < words.size(); ++i) {
        ps.Insert(words[i], i);
    }

    for (auto &pp : deleted) {
        ps.Delete(pp.first, pp.second);
    }

    ps.Build();

    double start = clock();
    cerr << "  cnt: " << ps.Find(text).size() << endl;
    cerr << "  BM_RANDOM_FIND: " << (clock() - start) / CLOCKS_PER_SEC << endl;
}

int x = generator();

template<template <typename> class PatternSearchT>
void startBM() {
    BM_INSERT<PatternSearchT<int>>();
    BM_DELETE<PatternSearchT<int>>();
    BM_BUILD<PatternSearchT<int>>();
    BM_FIND<PatternSearchT<int>>();

    if (!std::is_same<LinearSearch<int>, PatternSearchT<int>>::value) { // it's so hard test for LinearSearch
        BM_RANDOM_FIND<PatternSearchT<int>>();
    }
}

} // anonymous namespace
