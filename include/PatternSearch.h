#ifndef PATTERNSEARCH_H
#define PATTERNSEARCH_H

#include <string>
#include <vector>
#include <set>

namespace StringAlgos {

typedef unsigned char uchar_t;
typedef const unsigned char * uchar_ptr_t;

template<typename DataT>
class PatternSearch
{
public:
    PatternSearch() {}
    virtual ~PatternSearch() {}

    virtual void Build() {}
    virtual size_t Size() const = 0;

    virtual bool Insert(const std::string &pattern, const DataT& data) {
        return Insert(pattern.c_str(), pattern.size(), data);
    }

    virtual bool InsertAndBuild(const std::string& pattern, const DataT& data) {

    }

    virtual bool Delete(const std::string &pattern, const DataT& data) {
        return Delete(pattern.c_str(), pattern.size(), data);
    }

    virtual std::set<DataT> Find(const std::string &text) const {
        return Find(text.c_str(), text.size());
    }

    virtual bool Insert(char const * pattern, size_t len, const DataT& data) = 0;
    virtual bool Delete(char const * pattern, size_t len, const DataT& data) = 0;
    virtual std::set<DataT> Find(char const * text, size_t len) const = 0;
};

} // StringAlgos

#endif // PATTERNSEARCH_H
