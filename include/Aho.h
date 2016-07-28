#ifndef AHO_GOOD_H
#define AHO_GOOD_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <cstring>
#include <cassert>

#include "PatternSearch.h"

namespace StringAlgos {

template <typename DataT>
class Aho : public PatternSearch<DataT>
{
private:
    struct TrieVertex;

    typedef TrieVertex * TrieVertexPtr;

    struct TrieVertex {
        static const int kAlphabetSize = 256;

        TrieVertex(TrieVertexPtr refToParent, uchar_t parentCharacter)
            : refToParent(refToParent)
            , link(nullptr)
            , goodLink(nullptr)
            , cntChilds(0)
            , parentCharacter(parentCharacter)
            , terminal(false)
            , existTerminal(false)
        {
            memset(next, 0, sizeof(next));
            memset(go, 0, sizeof(go));
        }

        ~TrieVertex() {
            for (int i = 0; i < kAlphabetSize; ++i) {
                if (next[i]) {
                    delete next[i];
                    next[i] = nullptr;
                }
            }
        }

        TrieVertexPtr  next[kAlphabetSize];
        TrieVertexPtr  go[kAlphabetSize];
        TrieVertexPtr refToParent;
        TrieVertexPtr link;
        TrieVertexPtr goodLink;

        size_t  cntChilds;
        uchar_t parentCharacter;
        bool terminal;
        bool existTerminal;

        std::vector<DataT> data;
        std::vector<DataT> data_link;
    };

public:
    using PatternSearch<DataT>::Insert;
    using PatternSearch<DataT>::Delete;
    using PatternSearch<DataT>::Find;

    Aho()
        : _root(new TrieVertex(nullptr, 0))
    {}

    ~Aho() {
        delete _root;
    }

    // bfs
    void Build() override {
        std::queue<TrieVertexPtr> q;
        q.push(_root);

        while (!q.empty()) {
            TrieVertexPtr v = q.front();
            q.pop();
            v->data_link.clear();

            for (int i = 0; i < TrieVertex::kAlphabetSize; ++i) {
                if (v->next[i]){
                    q.push(v->next[i]);
                }
            }

            if (v == _root || v->refToParent == _root) {
                v->link = _root;
            } else {
                TrieVertexPtr pr = v->refToParent;
                v->link = pr->link->go[v->parentCharacter];
                v->existTerminal = v->link->terminal | v->link->existTerminal;
            }

            TrieVertexPtr linkVer = v->link;
            for (int i = 0; i < TrieVertex::kAlphabetSize; ++i) {
                if (!v->next[i])
                    v->go[i] = (v == _root) ? _root : linkVer->go[i];
                else
                    v->go[i] = v->next[i];
            }

            if (v->existTerminal) {
                TrieVertexPtr fLinkVer = v->link;
                assert(fLinkVer != _root);
                v->goodLink = (fLinkVer->terminal) ? fLinkVer : fLinkVer->goodLink;

                v->data_link.insert(v->data_link.end(), v->goodLink->data.begin(), v->goodLink->data.end());
                v->data_link.insert(v->data_link.end(), v->goodLink->data_link.begin(), v->goodLink->data_link.end());
            } else {
                v->goodLink = _root;
            }
        }

        _builded = true;
    }

    size_t Size() const override {
        return _root->cntChilds;
    }

    bool Insert(const char * pattern, size_t len, const DataT& data) override {
        const uchar_t * first = (const uchar_t *) pattern;
        const uchar_t * last = first + len;

        TrieVertexPtr curVer = _root;

        for (uchar_ptr_t ptr = first; ptr != last; ++ptr) {
            uchar_t c = *ptr;

            curVer->cntChilds++;
            if (!(curVer->next[c])) {
                curVer->next[c] = new TrieVertex(curVer, c);
            }

            curVer = curVer->next[c];
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
                        curVer = curVer->next[c];
                    }
                    curVer->cntChilds--;

                    return false;
                }
            }
        }

        curVer->data.push_back(data);
        _builded = false;

        return true;
    }

    bool Delete(const char * pattern, size_t len, const DataT& data) override {
        const uchar_t * first = (const uchar_t *) pattern;
        const uchar_t * last = first + len;

        TrieVertexPtr curVer = _root;

        // assert that `pattern` was added earlier to the dict
        {
            if (len == 0) {
                return false;
            }

            for (uchar_ptr_t ptr = first; ptr != last; ++ptr) {
                uchar_t c = *ptr;
                curVer = curVer->next[c];

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
            if (curVer->next[c]->cntChilds == 1) {
                delete curVer->next[c];
                curVer->next[c] = nullptr;
                curVer = nullptr;
                break;
            }

            curVer = curVer->next[c];
        }

        if (curVer) {
            uchar_t c = *(last - 1);

            assert(len == 1 || curVer->cntChilds > 1);
            assert(curVer->next[c]->terminal && curVer->next[c]->data.size());

            curVer->cntChilds--;

            if (curVer->next[c]->cntChilds == 1) {
                delete curVer->next[c];
                curVer->next[c] = nullptr;
            } else {
                curVer = curVer->next[c];
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

        _builded = false;
        return true;
    }

    std::set<DataT> Find(const char *text, size_t len) const override {
        assert(("you should call `Build` function after modification (`Insert`, `Delete`)", _builded));

        const uchar_t * first = (const uchar_t *) text;
        const uchar_t * last = first + len;

        std::set<DataT> res;
        TrieVertexPtr curVer = _root;

        for (uchar_ptr_t ptr = first; ptr != last; ++ptr) {
            uchar_t c = *ptr;

            curVer = curVer->go[c];
            assert(curVer);

            // curVer has a data only when it's terminal vertex
            res.insert(curVer->data.begin(), curVer->data.end());

            // data_link is always good (terminal) data, pushed from another suffix verteces
            res.insert(curVer->data_link.begin(), curVer->data_link.end());

            if (res.size() == Size()) {
                break;
            }
        }

        return res;
    }


private:
    TrieVertexPtr _root;
    bool _builded;
};

} // StringAlgos

#endif // AHOGOOD_H
