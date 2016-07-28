#ifndef LINEARSEARCH_H
#define LINEARSEARCH_H

#include <set>
#include <cassert>

#include "PatternSearch.h"

namespace StringAlgos {

template <typename DataT>
class LinearSearch : public PatternSearch<DataT>
{
public:
    LinearSearch() {}

    size_t Size() const override {
        return _patterns.size();
    }

    bool Insert(const char * pattern, size_t len, const DataT& data) override {
        return Insert(std::string(pattern, pattern + len), data);
    }

    bool Insert(const std::string &pattern, const DataT& data) override {
        auto r = _patterns.insert(std::make_pair(pattern, data));
        return r.second;
    }

    bool Delete(const char * pattern, size_t len, const DataT& data) override {
        return Delete(std::string(pattern, pattern + len), data);
    }

    bool Delete(const std::string &pattern, const DataT& data) override {
        auto r = _patterns.erase(std::make_pair(pattern, data));
        return r;
    }

    std::set<DataT> Find(const char *text, size_t len) const override {
        return Find(std::string(text, text + len));
    }

    std::set<DataT> Find(const std::string &text) const override {
        std::set<DataT> res;

        for (auto& pp: _patterns) {
            const std::string& pattern = pp.first;

            if (text.find(pattern) != std::string::npos) {
                res.insert(pp.second);
            }
        }

        return res;
    }

private:
    std::set<std::pair<std::string, DataT>> _patterns;
};

} // StringAlgos

#endif // LINEARSEARCH_H
