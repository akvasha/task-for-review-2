#include <iostream>
#include <vector>
#include <utility>
#include <stdexcept>
#include <list>
#include <cassert>

const unsigned int EXPAND_COEFFICIENT = 4;
const unsigned int DEFAULT_SIZE = 100;

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>> class HashMap {
    using object = typename std::pair<const KeyType, ValueType>;
    using line = typename std::list<object>;
    using iter = typename line::iterator;
    using const_iter = typename line::const_iterator;
private:
    size_t max_size = DEFAULT_SIZE;
    size_t sz;
    std::vector<line> table;
    Hash hasher;
public:
    HashMap() : sz(0) {
        max_size = DEFAULT_SIZE;
        table = std::vector<line>(max_size + 1);
    }

    HashMap(const Hash &_h) : max_size(DEFAULT_SIZE), sz(0), hasher(_h) {
        max_size = DEFAULT_SIZE;
        table = std::vector<line>(max_size + 1);
    }

    void erase(const KeyType &key) {
        size_t i = hasher(key) % max_size;
        if (!table[i].empty()) {
            for (iter it = table[i].begin(); it != table[i].end(); ++it) {
                if (it->first == key) {
                    table[i].erase(it);
                    --sz;
                    return;
                }
            }
        }
    }

    void rebuild() {
        if (sz < DEFAULT_SIZE || max_size < DEFAULT_SIZE)
            return;
        if (sz > (EXPAND_COEFFICIENT / 2) * max_size) {
            size_t new_size = EXPAND_COEFFICIENT * max_size;
            std::list<object> res;
            for (size_t i = 0; i < max_size; ++i) {
                for (auto obj : table[i])
                   res.push_back(obj);
            }
            table.clear();
            sz = 0;
            table.resize(new_size + 1);
            max_size = new_size;
            for (auto obj : res) {
                size_t i = hasher(obj.first) % max_size;
                table[i].push_back(obj);
                ++sz;
            }
        }
    }

    void insert(const object &elem) {
        size_t i = hasher(elem.first) % max_size;
        for (iter it = table[i].begin(); it != table[i].end(); ++it) {
            if (it->first == elem.first)
                return;
        }
        table[i].push_back(elem);
        ++sz;
        rebuild();
    }

    template<class InputIterator>
    HashMap(InputIterator beg, InputIterator end) {
        *this = HashMap();
        while (beg != end)
            insert(*beg++);
    }

    template<class InputIterator>
    HashMap(InputIterator beg, InputIterator end, Hash &_h) {
        *this = HashMap();
        while (beg != end)
            insert(*beg++);
        hasher = _h;
    }

    HashMap(std::initializer_list<object> l) {
        *this = HashMap(l.begin(), l.end());
    }

    HashMap(std::initializer_list<object> l, Hash &_h) {
        *this = HashMap(l.begin(), l.end(), _h);
    }

    HashMap &operator = (const HashMap &other) {
        table = std::vector<line>(other.table);
        sz = other.sz;
        hasher = other.hasher;
        max_size = other.max_size;
        return *this;
    }

    size_t size() const {
        return sz;
    }

    bool empty() const {
        return sz == 0;
    }

    Hash hash_function() const {
        return hasher;
    }

    class iterator {
    private:
        size_t row;
        iter pos;
        HashMap *hash_table;
    public:
        iterator(size_t _row = 0, iter _pos = iter(), HashMap *t = 0) : row(_row), pos(_pos), hash_table(t) {}

        object &operator * () {
            return *pos;
        }

        iter operator -> () {
            return pos;
        }

        bool operator == (iterator other) {
            return row == other.row && pos == other.pos;
        }

        bool operator != (iterator other) {
            return !(*this == other);
        }

        iterator operator ++ () {
            ++pos;
            if (pos == hash_table->table[row].end() && row != hash_table->max_size) {
                ++row;
                while (row != hash_table->max_size && hash_table->table[row].empty())
                    ++row;
                if (row == hash_table->max_size)
                    pos = hash_table->table[row].end();
                else
                    pos = hash_table->table[row].begin();
            }
            return *this;
        }

        iterator operator ++ (int) {
            iterator curr = *this;
            ++(*this);
            return curr;
        }
    };

    class const_iterator {
    private:
        size_t row;
        const_iter pos;
        const HashMap *hash_table;
    public:
        const_iterator(size_t _row = 0, const_iter _pos = iter(), const HashMap *t = 0) : row(_row), pos(_pos), hash_table(t) {}

        const object &operator * () {
            return *pos;
        }

        const_iter operator -> () {
            return pos;
        }

        bool operator == (const_iterator other) {
            return row == other.row && pos == other.pos;
        }

        bool operator != (const_iterator other) {
            return !(*this == other);
        }

        const_iterator operator ++ () {
            ++pos;
            if (pos == hash_table->table[row].end() && row != hash_table->max_size) {
                ++row;
                while (row != hash_table->max_size && hash_table->table[row].empty())
                    ++row;
                if (row == hash_table->max_size)
                    pos = hash_table->table[row].end();
                else
                    pos = hash_table->table[row].begin();
            }
            return *this;
        }

        const_iterator operator ++ (int) {
            const_iterator curr = *this;
            ++(*this);
            return curr;
        }
    };

    iterator begin() {
        size_t first_empty = 0;
        while (first_empty != max_size && table[first_empty].empty())
            ++first_empty;
        if (first_empty == max_size)
            return iterator(max_size, table[max_size].end(), this);
        else
            return iterator(first_empty, table[first_empty].begin(), this);
    }

    iterator end() {
        return iterator(max_size, table[max_size].end(), this);
    }

    const_iterator begin() const {
        size_t first_empty = 0;
        while (first_empty != max_size && table[first_empty].empty())
            ++first_empty;
        if (first_empty == max_size)
            return const_iterator(max_size, table[max_size].end(), this);
        else
            return const_iterator(first_empty, table[first_empty].begin(), this);
    }

    const_iterator end() const {
        return const_iterator(max_size, table[max_size].end(), this);
    }

    iterator find(const KeyType &key) {
        size_t i = hasher(key) % max_size;
        size_t pos1 = max_size;
        iter pos2 = table[max_size].end();
        if (!table[i].empty()) {
            for (iter it = table[i].begin(); it != table[i].end(); ++it) {
                if (it->first == key) {
                    pos1 = i, pos2 = it;
                }
            }
        }
        return iterator(pos1, pos2, this);
    }

    const_iterator find(const KeyType &key) const {
        size_t i = hasher(key) % max_size;
        size_t pos1 = max_size;
        const_iter pos2 = table[max_size].end();
        if (!table[i].empty()) {
            for (const_iter it = table[i].begin(); it != table[i].end(); ++it) {
                if (it->first == key) {
                    pos1 = i, pos2 = it;
                }
            }
        }
        return const_iterator(pos1, pos2, this);
    }

    ValueType& operator [] (const KeyType &key) {
        if (find(key) == end())
            insert(std::make_pair(key, ValueType()));
        return (*find(key)).second;
    }

    const ValueType& at(const KeyType &key) const {
        if (find(key) == end())
            throw std::out_of_range("");
        return (*find(key)).second;
    }

    void clear() {
        *this = HashMap();
    }
};
