#ifdef _GTEST
//#define cerr cout

#include <iostream>
#include <ctime>
#include <thread>
#include <map>

#include <LinearSearch.h>
#include <TrieSearch.h>
#include <Aho.h>
#include <Hyperscan.h>

#include <gtest/gtest.h>


namespace StringAlgos {

template <typename DataT>
struct HyperscanAddDotAll : public Hyperscan<DataT> {
    using Hyperscan<DataT>::Find;
    using Hyperscan<DataT>::Insert;
    using Hyperscan<DataT>::Delete;

    bool Insert(const std::string &pattern, const DataT& data) override {
        std::string temp = ".*" + pattern + ".*";
        return Insert(temp.c_str(), temp.size(), data);
    }

    bool Delete(const std::string &pattern, const DataT& data) override {
        std::string temp = ".*" + pattern + ".*";
        return Delete(temp.c_str(), temp.size(), data);
    }
};

template <typename DataT>
struct HyperscanWithEscapedCharacter : public Hyperscan<DataT> {
    using Hyperscan<DataT>::Find;
    using Hyperscan<DataT>::Insert;
    using Hyperscan<DataT>::Delete;

    bool Insert(const std::string &pattern, const DataT& data) override {
        std::string temp(CreateEscapedString(pattern));
        return Insert(temp.c_str(), temp.size(), data);
    }

    bool Delete(const std::string &pattern, const DataT& data) override {
        std::string temp(CreateEscapedString(pattern));
        return Delete(temp.c_str(), temp.size(), data);
    }

private:
    std::string CreateEscapedString(const std::string pattern) {
        const std::set<char> specials = {'-' ,'[' ,']' ,'/' ,'{' ,'}' ,'(' ,')' ,'*' ,'+' ,'?' ,'^' ,'$' ,'|', '.', ',', '#'};

        const std::map<char, std::string> converter = {{'*', ".*"},
                                                       {'?', "."}};


        std::string res;
        res.reserve(pattern.size() * 2);

        bool escaped = false;
        for (char c: pattern) {
            if (!escaped) {
                if (c == '\\') {
                    escaped = true;
                } else if (converter.count(c)) {
                    res.append(converter.at(c));
                    continue;
                } else  if (specials.count(c)) {
                    res.push_back('\\');
                }
            } else {
                escaped = false;
            }

            res.push_back(c);
        }

        return res;
    }
};

} // StringAlgos


using namespace std;
using namespace StringAlgos;

template<template <typename> class PatternSearchT>
void manualTest() {
    PatternSearchT<int> ps;

    vector<pair<string, int>> v {
        {"abcd", 11},
        {"abcd", 1},
        {"abc", 2},
        {"abce", 3},
        {"bcu", 4},
        {"bcdf", 5},
        {"bcde", 6},
        {"Z", 7},
        {"A", 8},
        {"AA", 9},
        {"AAA", 10}};

    for (auto & pp : v)
        ASSERT_TRUE(ps.Insert(pp.first, pp.second));

    ASSERT_FALSE(ps.Insert(v.front().first, v.front().second));

    ps.Build();

    ASSERT_EQ(ps.Size(), 11);

    {
        auto s = ps.Find("abcu");
        std::set<int> res{2, 4};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("abcd");
        std::set<int> res{1, 11, 2};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("bcdf");
        std::set<int> res{5};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("abceabcdfe");
        std::set<int> res{2, 3, 1, 11, 5};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("abceabcdfebcobcpabpbcdebcobcupp");
        std::set<int> res{1, 11, 2, 3, 4, 5, 6};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("ZZZZZZZZZZZZZZZZZZZZZZZZ");
        std::set<int> res{7};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("AAAAAAAAAAAAAAAAAAAAAAA");
        std::set<int> res{8, 9, 10};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("AA");
        std::set<int> res{8, 9};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("A");
        std::set<int> res{8};
        ASSERT_EQ(s, res);
    }

    ASSERT_TRUE(ps.Delete("bcu", 4));
    ASSERT_FALSE(ps.Delete("bcu", 4));

    ASSERT_TRUE(ps.Delete("abcd", 1));
    ASSERT_TRUE(ps.Delete("A", 8));
    ps.Build();

    ASSERT_EQ(ps.Size(), 8);

    {
        auto s = ps.Find("abcu");
        std::set<int> res{2};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("abcd");
        std::set<int> res{2, 11};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("bcdf");
        std::set<int> res{5};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("abceabcdfe");
        std::set<int> res{2, 3, 5, 11};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("abceabcdfebcobcpabpbcdebcobcupp");
        std::set<int> res{2, 3, 5, 6, 11};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("ZZZZZZZZZZZZZZZZZZZZZZZZ");
        std::set<int> res{7};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("AAAAAAAAAAAAAAAAAAAAAAA");
        std::set<int> res{9, 10};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("AA");
        std::set<int> res{9};
        ASSERT_EQ(s, res);
    }

    {
        auto s = ps.Find("A");
        std::set<int> res;
        ASSERT_EQ(s, res);
    }

    {
        PatternSearchT<int> ps2;
        {
            ASSERT_TRUE(ps2.Insert("a", 1));
            ps2.Insert("b", 2);
            ps2.Build();
            std::set<int> res {1};
            ASSERT_EQ(ps2.Find(";lkj;lkja"), res);
        }

        {
            ASSERT_TRUE(ps2.Delete("a", 1));
            ps2.Delete("b", 2);
            ps2.Build();
            std::set<int> res;
            ASSERT_EQ(ps2.Find(";lkj;lkja"), res);
            ASSERT_EQ(ps2.Size(), 0);
        }
    }

    {
        PatternSearchT<int> ps2;
        ps2.Insert("abc", 1);
        ps2.Insert("abcde", 2);

        ps2.Delete("abcde", 2);
        ps2.Delete("abc", 1);
        ps2.Build();

        std::set<int> res;
        ASSERT_EQ(ps2.Find("abc"), res);
        ASSERT_EQ(ps2.Size(), 0);
    }
}

template<template <typename> class PatternSearchT, typename T = int>
void randomTest(const int LEN_T = 10000, const int CNT_W = 100, const int CNT_T = 100, const int LEN_W = 100, const int ALPH_SIZE = 10, const int CNT_TESTS = 100) {
    if (std::is_same<PatternSearchT<T>, LinearSearch<T>>::value) return;


    string word;
    word.reserve(LEN_W);

    string text;
    text.reserve(LEN_T);

    vector<pair<string, int>> deleted;

    for (int i = 0; i < CNT_TESTS; ++i) {
        LinearSearch<T> ls;
        PatternSearchT<T> ps;

        const int cntWords = rand() % CNT_W + 1;
        const int cntTexts = rand() % CNT_T + 1;
//        cerr << "test #" << i << "; words: " << cntWords << "; texts: " << cntTexts;

        deleted.clear();
        for (int j = 0; j < cntWords; ++j) {
            const int lenW = rand() % LEN_W + 1;
            word.clear();

            for (int k = 0; k < lenW; ++k) {
                word.push_back(rand() % ALPH_SIZE + 'a');
            }

            ASSERT_TRUE(ls.Insert(word, j));
            ASSERT_TRUE(ps.Insert(word, j));

            if (rand() % 2)  deleted.push_back({word, j});
        }

        if (deleted.size()) deleted.pop_back();

//        cerr << "; deleted: " << deleted.size() << endl;
        for (pair<string, int>& p: deleted) {
            ASSERT_TRUE(ls.Delete(p.first, p.second));
            ASSERT_TRUE(ps.Delete(p.first, p.second));
        }

        ls.Build();
        ps.Build();

        ASSERT_EQ(ps.Size(), ls.Size());
        ASSERT_EQ(ps.Size(), cntWords - deleted.size());

        for (int j = 0; j < cntTexts; ++j) {
            const int lenT = rand() % LEN_T + 1;
            text.clear();

            for (int k = 0; k < lenT; ++k) {
                text.push_back(rand() % ALPH_SIZE + 'a');
            }
//            cerr << "; text.size() = " << text.size() << endl;

            ASSERT_EQ(ps.Find(text), ls.Find(text));
        }
    }
}

template<template <typename> class PatternSearchT, typename T = int>
void WorstCaseTest(bool notMatchTest = false) {
    PatternSearchT<T> ps;
    int CNT_W = 1e3;
    int LEN_T = 1e8;

    if (notMatchTest) { // TrieSearch so slow in this case, O(max_len * |text|) ~ (1e3 * 1e6) iterations
        LEN_T = 1e6;
    }

    std::string a;
    a.reserve(CNT_W);
    for (int i = 0; i < CNT_W; ++i) {
        a.push_back('a');
        if (notMatchTest) {
            ps.Insert(a + "b", i);
        } else {
            ps.Insert(a, i);
        }
    }

    std::string text;
    text.reserve(LEN_T);
    for (int i = 0; i < LEN_T; ++i) {
        text.push_back('a');
    }

    ps.Build();

    double start = clock();
    auto res = ps.Find(text);

#ifdef BENCHMARK
    std::cerr << "worst case: " << ps.Size() << " " << text.size() << std::endl;
    std::cerr << "WorstCase find time: " << (clock() - start) / CLOCKS_PER_SEC << std::endl;
#endif

    if (notMatchTest) {
        ASSERT_EQ(res.size(), 0);
    } else {
        ASSERT_EQ(res.size(), CNT_W);
        int i = 0;
        for (int r: res) {
            ASSERT_EQ(r, i++);
        }
    }
}

template<template <typename> class PatternSearchT, typename T = int>
void SimpleMultiThreadingTest(const int CNT_W = 100, const int CNT_T = 100, const int LEN_W = 100, const int LEN_T = 10000, const int ALPH_SIZE = 10, const int CNT_TESTS = 100) {
    string word;
    word.reserve(LEN_W);

    string text;
    text.reserve(LEN_T);

    vector<pair<string, int>> deleted;

    for (int i = 0; i < CNT_TESTS; ++i) {
        LinearSearch<T> ls;
        PatternSearchT<T> ps;

        const int cntWords = rand() % CNT_W + 1;
        const int cntTexts = rand() % CNT_T + 1;
    //        cerr << "test #" << i << "; words: " << cntWords << "; texts: " << cntTexts;

        deleted.clear();
        for (int j = 0; j < cntWords; ++j) {
            const int lenW = rand() % LEN_W + 1;
            word.clear();

            for (int k = 0; k < lenW; ++k) {
                word.push_back(rand() % ALPH_SIZE + 'a');
            }

            ASSERT_TRUE(ls.Insert(word, j));
            ASSERT_TRUE(ps.Insert(word, j));

            if (rand() % 2)  deleted.push_back({word, j});
        }

        if (deleted.size()) deleted.pop_back();

    //        cerr << "; deleted: " << deleted.size() << endl;
        for (pair<string, int>& p: deleted) {
            ASSERT_TRUE(ls.Delete(p.first, p.second));
            ASSERT_TRUE(ps.Delete(p.first, p.second));
        }

        ls.Build();
        ps.Build();

        ASSERT_EQ(ps.Size(), ls.Size());
        ASSERT_EQ(ps.Size(), cntWords - deleted.size());

        std::vector<std::thread> threads;
        for (int j = 0; j < cntTexts; ++j) {
            const int lenT = rand() % LEN_T + 1;
            text.clear();

            for (int k = 0; k < lenT; ++k) {
                text.push_back(rand() % ALPH_SIZE + 'a');
            }

            threads.emplace_back([text, &ps, &ls]() {
                ASSERT_EQ(ps.Find(text), ls.Find(text));
            });
        }

        for (int j = 0; j < cntTexts; ++j) {
            threads[j].join();
        }
    }
}

template<template <typename> class PatternSearchT, typename T = int>
void manualSwmrThreadingTest() {
    PatternSearchT<T> ps;

    vector<pair<string, int>> v {
        {"abcd", 1},
        {"abc", 2},
        {"abce", 3},
        {"bcdf", 5},
        {"bcde", 6},
        {"A", 8},
        {"AAA", 10}};

    for (auto & pp : v)
        ASSERT_TRUE(ps.Insert(pp.first, pp.second));
    ps.Build();

    std::set<int> res{1, 2, 8, 10, 6};

    char const * text = "bcu abcd AA Z AAA bcdef";
    ASSERT_EQ(ps.Find(text), res);

    std::thread writer([&ps]() {
        ps.Insert("bcu", 4);
        ps.Build();
        std::this_thread::sleep_for(std::chrono::seconds(3));

        ps.Insert("abcd", 11);
        ps.Insert("Z", 7);
        ps.Insert("AA", 9);
        ps.Build();

        std::this_thread::sleep_for(std::chrono::seconds(2));
        ps.Delete("abcd", 1);
        ps.Delete("abc", 2);
        ps.Build();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    const int CNT_READERS = 10;
    std::vector<std::thread> readers;
    for (int i = 0; i < CNT_READERS; ++i) {
        readers.emplace_back([&ps, res, text]() {
            std::set<int> r(res);
            r.insert(4);
            ASSERT_EQ(ps.Find(text), r);

            std::this_thread::sleep_for(std::chrono::seconds(3));

            r.insert({11, 7, 9});
            ASSERT_EQ(ps.Find(text), r);

            std::this_thread::sleep_for(std::chrono::seconds(3));
            r.erase(1);
            r.erase(2);

            ASSERT_EQ(ps.Find(text), r);
        });
    }

    for (int i = 0; i < CNT_READERS; ++i) {
        readers[i].join();
    }

    writer.join();
}

template<template <typename> class PatternSearchT, typename T = int>
void checkManualRegexs(PatternSearchT<T>& ps) {
    {
        std::set<int> res;
        ASSERT_EQ(ps.Find("za"), res);
    }

    {
        std::set<int> res{6};
        ASSERT_EQ(ps.Find("z."), res);
    }

    {
        std::set<int> res;
        ASSERT_EQ(ps.Find("asd fasd fasd fasdf asdfa sd"), res);
    }

    {
        std::set<int> res{1};
        ASSERT_EQ(ps.Find("Putin"), res);
    }

    {
        std::set<int> res;
        ASSERT_EQ(ps.Find("\\teeeeeract\\"), res);
    }

    {
        std::set<int> res{3, 4};
        ASSERT_EQ(ps.Find("\\teract\\"), res);
    }

    {
        std::set<int> res{1, 3, 4, 2};
        ASSERT_EQ(ps.Find("ads as dasd PutAn asd fas d\\teRacI\\ Booooooomba"), res);
    }

    {
        std::set<int> res{1, 2};
        ASSERT_EQ(ps.Find("ads as dasd PutAn asd fas Booooooomba \\teraCTTT\\"), res);
    }

    {
        std::set<int> res{1, 2, 0};
        ASSERT_EQ(ps.Find("ads as dasd PutAn .*Put.n.* fas Booooooomba \\teraCTTT\\"), res);
    }
}

template<template <typename> class PatternSearchT, typename T = int>
void manualRegexTests() {
    PatternSearchT<T> ps;

    ps.Insert("\\.\\*Put\\.n\\.\\*", 0);
    ps.Insert(".*Put.n.*", 1);
    ps.Insert(".*Boo.*mba.*", 2);
    ps.Insert(".*\\\\te.ac.\\\\.*", 3);
    ps.Insert(".*\\\\te.ac.\\\\.*", 4);
    ps.Insert(".*z\\..*", 6);
    ps.Build();
    checkManualRegexs(ps);
}

template<template <typename> class PatternSearchT, typename T = int>
void manualRegexEscapedTests() {
    PatternSearchT<T> ps;

    ps.Insert(".\\*Put.n.\\*", 0);
    ps.Insert("*Put?n*", 1);
    ps.Insert("*Boo*mba*", 2);
    ps.Insert("*\\\\te?ac?\\\\*", 3);
    ps.Insert("*\\\\te?ac?\\\\*", 4);
    ps.Insert("*z.*", 6);

    ps.Build();
    checkManualRegexs(ps);
}

TEST (Hyperscan, ManualTests) {
    manualTest<HyperscanAddDotAll>();
}

TEST (Hyperscan, ManualRegexTests) {
    manualRegexTests<Hyperscan>();
}

TEST (Hyperscan, ManualRegexEscapedTests) {
    manualRegexEscapedTests<HyperscanWithEscapedCharacter>();
}


TEST (Hyperscan, RandomTests) {
    randomTest<HyperscanAddDotAll>(100);
}

TEST (Hyperscan, SimpleMultiThreadingTest) {
    SimpleMultiThreadingTest<HyperscanAddDotAll>();
}

TEST (Hyperscan, manualSwmrThreadingTest) {
    manualSwmrThreadingTest<HyperscanAddDotAll>();
}

TEST (Hyperscan, WorstCaseTest) {
    WorstCaseTest<HyperscanAddDotAll>();
}

TEST (Hyperscan, WorstCaseNotMatchTest) {
    WorstCaseTest<HyperscanAddDotAll>(true);
}

TEST (LinearSearch, ManualTests) {
    manualTest<LinearSearch>();
}

TEST (LinearSearch, WorstCaseTest) {
    WorstCaseTest<LinearSearch>();
}

// so hard for LinearSearch
//TEST (LinearSearch, WorstCaseNotMatchTest) {
//    WorstCaseTest<LinearSearch>(true);
//}

TEST (Aho, ManualTests) {
    manualTest<Aho>();
}

TEST (Aho, SimpleMultiThreadingTest) {
    SimpleMultiThreadingTest<Aho>();
}

TEST (Aho, RandomTests) {
    randomTest<Aho>();
}

TEST (Aho, WorstCaseTest) {
    WorstCaseTest<Aho>();
}

TEST (Aho, WorstCaseNotMatchTest) {
    WorstCaseTest<Aho>(true);
}

TEST (TrieSearch, ManualTests) {
    manualTest<TrieSearch>();
}

TEST (TrieSearch, RandomTests) {
    randomTest<TrieSearch>();
}

TEST (TrieSearch, WorstCaseTest) {
    WorstCaseTest<TrieSearch>();
}

TEST (TrieSearch, WorstCaseNotMatchTest) {
    WorstCaseTest<TrieSearch>(true);
}

#endif

