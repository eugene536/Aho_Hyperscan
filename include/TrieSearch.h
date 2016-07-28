#ifndef TRIESEARCH_H
#define TRIESEARCH_H

#include <memory>
#include <cstring>
#include <cstdint>
#include <cassert>

#include "PatternSearch.h"

namespace StringAlgos {

template <typename DataT>
class TrieSearch : public PatternSearch<DataT>
{
    struct TrieVertex {
        static const int kAlphabetSize = 256; // 128? - ~utf

        TrieVertex()
            : cntChilds(0)
            , terminal(false)
        {
            memset(child, 0, sizeof(child));
        }

        ~TrieVertex() {
            for (int i = 0; i < kAlphabetSize; ++i) {
                if (child[i])
                    delete child[i];
            }
        }

        TrieVertex * child[kAlphabetSize];
        size_t  cntChilds;
        bool terminal;

        std::vector<DataT> data;
    };

public:
    using PatternSearch<DataT>::Insert;
    using PatternSearch<DataT>::Delete;
    using PatternSearch<DataT>::Find;

    TrieSearch()
        : _root(new TrieVertex)
    {}

    ~TrieSearch() {
        delete _root;
    }

    size_t Size() const override {
        return _root->cntChilds;
    }

    bool Insert(const char * pattern, size_t len, const DataT& data) override {
        const uchar_t * first = (const uchar_t *) pattern;
        const uchar_t * last = first + len;

        TrieVertex * curVer = _root;

        for (uchar_ptr_t ptr = first; ptr != last; ++ptr) {
            uchar_t c = *ptr;

            curVer->cntChilds++;
            if (!(curVer->child[c])) {
                curVer->child[c] = new TrieVertex;
            }

            curVer = curVer->child[c];
        }
        curVer->cntChilds++;

        curVer->terminal = true;

        // assert that pair <pattern, data> is unique in the dictionary
        {
            for (DataT& d: curVer->data) {
                if (d == data) {
                    curVer = _root;

                    for (uchar_ptr_t ptr = first; ptr != last; ++ptr) {
                        uchar_t c = *ptr;

                        assert (curVer);
                        curVer->cntChilds--;
                        curVer = curVer->child[c];
                    }
                    curVer->cntChilds--;

                    return false;
                }
            }
        }

        curVer->data.push_back(data);

        return true;
    }

    bool Delete(const char * pattern, size_t len, const DataT& data) override {
        const uchar_t * first = (const uchar_t *) pattern;
        const uchar_t * last = first + len;

        TrieVertex * curVer = _root;

        // assert that `pattern` was added earlier to the dict
        {
            if (len == 0) {
                return false;
            }

            for (uchar_ptr_t ptr = first; ptr != last; ++ptr) {
                uchar_t c = *ptr;
                curVer = curVer->child[c];

                if (!curVer) {
                    return false;
                }
            }

            bool found = false;
            for (DataT& d: curVer->data) {
                if (d == data) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                return false;
            }
        }

        curVer = _root;
        for (uchar_ptr_t ptr = first; ptr + 1 != last; ++ptr) {
            uchar_t c = *ptr;

            curVer->cntChilds--;
            if (curVer->child[c]->cntChilds == 1) {
                delete curVer->child[c];
                curVer->child[c] = nullptr;
                curVer = nullptr;

                break;
            }

            curVer = curVer->child[c];
        }

        if (curVer) {
            uchar_t c = *(last - 1);

            assert(len == 1 || curVer->cntChilds > 1);
            assert(curVer->child[c]->terminal && curVer->child[c]->data.size());

            curVer->cntChilds--;

            if (curVer->child[c]->cntChilds == 1) {
                delete curVer->child[c];
                curVer->child[c] = nullptr;
            } else {
                curVer = curVer->child[c];
                for (auto it = curVer->data.begin(); it != curVer->data.end(); ++it) {
                    if (*it == data) {
                        curVer->data.erase(it);
                        break;
                    }
                }

                if (curVer->data.empty()) {
                    curVer->terminal = false;
                }

                curVer->cntChilds--;
            }
        }

        return true;
    }

    std::set<DataT> Find(const char *text, size_t len) const override {
        const uchar_t * first = (const uchar_t *) text;
        const uchar_t * last = first + len;

        std::set<DataT> res;
        for (size_t i = 0; i < len; ++i) {
            TrieVertex * curVer = _root;

            for (uchar_ptr_t ptr = first + i; ptr != last; ++ptr) {
                uchar_t c = *ptr;

                curVer = curVer->child[c];
                if (!curVer) break;

                if (curVer->terminal) {
                    res.insert(curVer->data.begin(), curVer->data.end());

                    if (res.size() == Size()) {
                        break;
                    }
                }
            }
        }

        return res;
    }

private:
    TrieVertex * _root;
};

} // StringAlgos

#endif // TRIESEARCH_H
