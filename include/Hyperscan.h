
#ifndef HYPERSCAN_H
#define HYPERSCAN_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <thread>

#include <hs.h>
#include <PatternSearch.h>
#include <psc/utils/thread.h>
#include <psc/utils/smart_ptr.h>

namespace StringAlgos {

template <typename DataT>
class Hyperscan : public PatternSearch<DataT> {
private:
    struct Context {
        std::set<DataT> * res;
        const std::vector<DataT> * data;
    };

    class DatabaseWrapper {
    public:
        DatabaseWrapper(const std::vector<char *>& patterns, const std::vector<DataT>& data)
            : data(data)
        {
            assert(!patterns.empty());

            // flags is a vector = {HS_FLAG_SINGLEMATCH, HS_FLAG_SINGLEMATCH, ...} n times
            // ids = 1..n
            // n = max of _patterns.size() from all instances of Hyperscan
            static std::vector<unsigned> flags;
            static std::vector<unsigned> ids;

            // if we inserted new patterns to `_paterrns` from previous Init call
            if (patterns.size() > flags.size()) {
                int prev_sz = flags.size();

                flags.resize(patterns.size());
                ids.resize(patterns.size());

                std::fill_n(flags.begin() + prev_sz, flags.size() - prev_sz, HS_FLAG_SINGLEMATCH);
                std::iota(ids.begin() + prev_sz, ids.end(), prev_sz);
            }

            static const unsigned int mode = HS_MODE_BLOCK;

            hs_compile_error_t *compileErr;
            hs_error_t err = hs_compile_multi(patterns.data(), flags.data(), ids.data(),
                                              patterns.size(), mode, nullptr, &db, &compileErr);

            if (err != HS_SUCCESS) {
                if (compileErr->expression < 0) {
                    // The error does not refer to a particular expression.
                    std::cerr << "ERROR: " << compileErr->message << std::endl;

                } else {
                    std::cerr << "ERROR: Pattern '" << patterns[compileErr->expression]
                         << "' failed compilation with error: " << compileErr->message
                         << std::endl;
                }


                // As the compileErr pointer points to dynamically allocated memory, if
                // we get an error, we must be sure to release it. This is not
                // necessary when no error is detected.
                hs_free_compile_error(compileErr);
                db = nullptr;
            }

            if (db && hs_alloc_scratch(db, &scratch) != HS_SUCCESS) {
                std::cerr << "ERROR: Unable to allocate scratch space." << std::endl;
                scratch = nullptr;
            }
        }

        ~DatabaseWrapper() {
            hs_free_database(db);
            hs_free_scratch(scratch);
        }

        hs_database_t * db = nullptr;
        hs_scratch_t * scratch = nullptr;
        std::vector<DataT> data;
    };

    struct ScratchWrapper {
        ScratchWrapper(hs_scratch_t * s) {
            hs_error_t err = hs_clone_scratch(s, &scratch);

            if (err != HS_SUCCESS) {
                printf("hs_clone_scratch failed!");
                scratch = nullptr;
            }
        }

        ~ScratchWrapper() {
            hs_free_scratch(scratch);
        }

        hs_scratch_t * scratch = nullptr;
    };

public:
    using PatternSearch<DataT>::Find;
    using PatternSearch<DataT>::Insert;
    using PatternSearch<DataT>::Delete;

    ~Hyperscan() {
        for (char* c: _patterns) {
            delete[] c;
        }
    }

    void Build() override {
        psc::smart_ptr<DatabaseWrapper> dw;

        if (!_patterns.empty())
            dw = new DatabaseWrapper(_patterns, _data);

        _m.lock();
        _dw = dw;
        _m.unlock();
    }

    size_t Size() const override {
        return _patterns.size();
    }

    bool Insert(const char *pattern, size_t len, const DataT& data) override {
        for (size_t i = 0; i < _patterns.size(); ++i) {
            if (strcmp(_patterns[i], pattern) == 0 && _data[i] == data) {
                return false;
            }
        }

        char * cpy = new char[len + 1];
        strcpy(cpy, pattern);
        _patterns.push_back(cpy);
        _data.push_back(data);

        return true;
    }

    bool Delete(const char *pattern, size_t len, const DataT& data) override {
        for (size_t i = 0; i < _patterns.size(); ++i) {
            if (strcmp(_patterns[i], pattern) == 0 && _data[i] == data) {

                std::swap(_patterns[i], _patterns.back());
                std::swap(_data[i], _data.back());

                delete[] _patterns.back();
                _patterns.pop_back();
                _data.pop_back();

                return true;
            }
        }

        return false;
    }

    std::set<DataT> Find(const char *text, size_t len) const override {
        _m.lock();
        psc::smart_ptr<DatabaseWrapper> dw = _dw;
        _m.unlock();

        std::set<DataT> res;
        if (!dw) return res;

        assert(dw->scratch);
        ScratchWrapper sw(dw->scratch);
        Context ctx{&res, &dw->data};

        if (hs_scan(dw->db, text, len, 0, sw.scratch, FindHandler, (void*) &ctx) != HS_SUCCESS) {
            std::cerr << "ERROR: Unable to scan input buffer" << std::endl;
            std::cerr << text << " " << len << std::endl;
            res.clear();
        }

        return res;
    }

private:
    static int FindHandler(unsigned int id, unsigned long long from,
                            unsigned long long to, unsigned int flags, void * ctx) {
        Context * context = reinterpret_cast<Context *>(ctx);
        context->res->insert((*context->data)[id]);

        return 0;
    }

private:
    std::vector<char *> _patterns;
    std::vector<DataT> _data;
    psc::smart_ptr<DatabaseWrapper> _dw;
    mutable psc::threads::mutex _m;
};

} // StringAlgos

#endif // HYPERSCAN_H
