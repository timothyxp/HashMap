#pragma once
#include <iostream>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <map>
#include <list>
#include <vector>
#include <string>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap {
    typedef std::pair<KeyType, ValueType> MapValue;
    typedef std::pair<const KeyType, ValueType> MapValueConst;
    typedef std::list<typename std::list<MapValue>::iterator> ListMVIter;
    typedef std::list<typename std::list<MapValue>::const_iterator> ListMVIterConst;
    constexpr static int siz = 1;
    constexpr static float expansion = 0.5;
    std::vector<ListMVIter> m;
    std::list<MapValue> el;
    int len;
    Hash hasher;
    typename ListMVIter::iterator findKey(
        typename ListMVIter::iterator begin,
        typename ListMVIter::iterator end, const KeyType & key) {
        while (begin != end) {
            if ((*begin)->first == key)
                return begin;
            begin++;
        }
        return end;
    }
    std::pair<typename ListMVIter::iterator, bool> find(int hash, KeyType key) {
        typename ListMVIter::iterator res = findKey(m[hash].begin(), m[hash].end(), key);
        if (res == m[hash].end())
            return make_pair(res, false);
        return make_pair(res, true);
    }
    typename ListMVIter::const_iterator findKey(
        typename ListMVIter::const_iterator begin,
        typename ListMVIter::const_iterator end, const KeyType & key) const {
        while (begin != end) {
            if ((*begin)->first == key)
                return begin;
            begin++;
        }
        return end;
    }
    std::pair<typename ListMVIter::const_iterator, bool> find(const int hash, const KeyType & key) const {
        typename ListMVIter::const_iterator res = findKey(m[hash].begin(), m[hash].end(), key);
        if (res == m[hash].end())
            return make_pair(res, false);
        return make_pair(res, true);
    }
    int getHash(const KeyType & key) const {
        return hasher(key) % m.size();
    }
public:
    struct iterator {
        typename std::list<MapValue>::iterator iter;
        iterator() {}
        iterator(typename std::list<MapValue>::iterator it) :iter(it) {}
        MapValueConst & operator *() {
            return *reinterpret_cast<MapValueConst*>(&*iter);
        }
        MapValueConst * operator ->() {
            return reinterpret_cast<MapValueConst*>(&*iter);
        }
        iterator operator ++() {
            iter++;
            return *this;
        }
        iterator operator ++(int) {
            auto copy = *this;
            iter++;
            return copy;
        }
        bool operator ==(iterator d) const {
            return iter == d.iter;
        }
        bool operator !=(iterator d) const {
            return iter != d.iter;
        }
    };
    struct const_iterator {
        typename std::list<MapValue>::const_iterator iter;
        const_iterator() {}
        const_iterator(typename std::list<MapValue>::const_iterator it) :iter(it) {}
        const MapValueConst & operator *() const {
            return *reinterpret_cast<const MapValueConst*>(&*iter);
        }
        const MapValueConst * operator ->() const {
            return reinterpret_cast<const MapValueConst*>(&*iter);
        }
        const_iterator operator ++() {
            iter++;
            return *this;
        }
        const_iterator operator ++(int) {
            auto copy = *this;
            iter++;
            return copy;
        }
        bool operator ==(const_iterator d) const {
            return iter == d.iter;
        }
        bool operator !=(const_iterator d) const {
            return iter != d.iter;
        }
    };
    HashMap(const Hash & hash = std::hash<KeyType>()) :hasher(hash) {
        len = 0;
        m.resize(siz);
    }
    template<class Iterator>
    HashMap(Iterator begin, Iterator end, const Hash & hash = std::hash<KeyType>()) : hasher(hash) {
        len = 0;
        m.resize(siz);
        while (begin != end) {
            insert(*begin);
            begin++;
        }
    }
    HashMap(const std::initializer_list<MapValue> & l) :len(0) {
        m.resize(siz);
        auto p = l.begin();
        while (p != l.end()) {
            insert(*p);
            p++;
        }
    }
    int size() const {
        return len;
    }
    bool empty() const {
        return len == 0;
    }
    Hash hash_function() const {
        return hasher;
    }
    void rebuild() {
        for (int i = 0; i < int(m.size()); i++)
            m[i].clear();
        m.resize(m.size() * 2);
        for (auto p = el.begin(); p != el.end(); p++) {
            int hash = getHash(p->first);
            m[hash].push_back(p);
        }
    }
    void insert(MapValue q) {
        int hash = getHash(q.first);
        if (find(hash, q.first).second)
            return;
        el.push_back(q);
        m[hash].push_back(--el.end());
        len++;
        if (len*expansion >= m.size()) {
            rebuild();
        }
    }
    void erase(KeyType q) {
        int hash = getHash(q);
        std::pair<typename ListMVIter::iterator, bool> search = find(hash, q);
        if (search.second) {
            el.erase(*search.first);
            m[hash].erase(search.first);
            len--;
        }
    }
    iterator find(const KeyType & key) {
        int hash = getHash(key);
        std::pair<typename ListMVIter::iterator, bool> search = find(hash, key);
        if (search.second) {
            return iterator(*search.first);
        }
        return end();
    }
    const_iterator find(const KeyType & key) const {
        int hash = getHash(key);
        std::pair<typename ListMVIter::const_iterator, bool> search = find(hash, key);
        if (search.second) {
            return const_iterator(*search.first);
        }
        return end();
    }
    iterator begin() {
        return iterator(el.begin());
    }
    iterator end() {
        return iterator(el.end());
    }
    const_iterator begin() const {
        return const_iterator(el.begin());
    }
    const_iterator end() const {
        return const_iterator(el.end());
    }
    ValueType & operator [](KeyType key) {
        insert(std::make_pair(key, ValueType()));
        int hash = getHash(key);
        return (*(find(hash, key).first))->second;
    }
    const ValueType & at(const KeyType & key) const {
        int hash = getHash(key);
        std::pair<typename ListMVIter::const_iterator, bool> search = find(hash, key);
        if (search.second) {
            return (*search.first)->second;
        }
        throw std::out_of_range("Error key");
    }
    void clear() {
        for (auto p = el.begin(); p != el.end(); p++) {
            int hash = getHash(p->first);
            m[hash].clear();
        }
        el.clear();
        len = 0;
    }
};