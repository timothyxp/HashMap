#pragma once
#include <iostream>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <map>
#include <list>
#include <vector>
#include <string>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
    typedef std::pair<KeyType, ValueType> MapValue;
    typedef std::pair<const KeyType, ValueType> MapValueConst;
    typedef std::list<typename std::list<MapValue>::iterator> ListMVIter;
    typedef std::list<typename std::list<MapValue>::const_iterator> ListMVIterConst;

    constexpr static int siz = 1;
    constexpr static float expansionFactor = 0.5;

    std::vector<ListMVIter> Map;
    std::list<MapValue> elements;
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
        auto res = findKey(Map[hash].begin(), Map[hash].end(), key);
        if (res == Map[hash].end())
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
        auto res = findKey(Map[hash].begin(), Map[hash].end(), key);
        if (res == Map[hash].end())
            return make_pair(res, false);
        return make_pair(res, true);
    }

    int getHash(const KeyType & key) const {
        return hasher(key) % Map.size();
    }
public:
    struct iterator {
        typename std::list<MapValue>::iterator iter;

        iterator() {}

        iterator(typename std::list<MapValue>::iterator it) :iter(it) {}

        MapValueConst & operator *() {
            MapValueConst iterator_const = make_pair((*const_cast<KeyType const *>(&iter->first)), iter->second);
            return iterator_const;
        }

        MapValueConst * operator ->() {
            MapValueConst iterator_const = make_pair((*const_cast<KeyType const *>(&iter->first)), iter->second);
            return &iterator_const;
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

        bool operator ==(const iterator & other) const {
            return iter == other.iter;
        }

        bool operator !=(const iterator & other) const {
            return iter != other.iter;
        }
    };
    struct const_iterator {
        typename std::list<MapValue>::const_iterator iter;

        const_iterator() {}

        const_iterator(typename std::list<MapValue>::const_iterator it) :iter(it) {}

        const MapValueConst & operator *() const {
            const MapValueConst iterator_const = make_pair((*const_cast<KeyType const *>(&iter->first)), iter->second);
            return iterator_const;
        }

        const MapValueConst * operator ->() const {
            MapValueConst iterator_const = make_pair((*const_cast<KeyType const *>(&iter->first)), iter->second);
            return &iterator_const;
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

        bool operator ==(const const_iterator & other) const {
            return iter == other.iter;
        }

        bool operator !=(const const_iterator & other) const {
            return iter != other.iter;
        }
    };

    HashMap(const Hash & hash = std::hash<KeyType>()) :hasher(hash) {
        len = 0;
        Map.resize(siz);
    }

    template<class Iterator>
    HashMap(Iterator begin, Iterator end, const Hash & hash = std::hash<KeyType>()) : hasher(hash) {
        len = 0;
        Map.resize(siz);
        while (begin != end) {
            insert(*begin);
            begin++;
        }
    }

    HashMap(const std::initializer_list<MapValue> & l) :len(0) {
        Map.resize(siz);
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
        for (size_t i = 0; i < Map.size(); i++)
            Map[i].clear();
        Map.resize(Map.size() * 2);
        for (auto p = elements.begin(); p != elements.end(); p++) {
            int hash = getHash(p->first);
            Map[hash].push_back(p);
        }
    }

    void insert(const MapValue & q) {
        int hash = getHash(q.first);
        if (find(hash, q.first).second)
            return;
        elements.push_back(q);
        Map[hash].push_back(--elements.end());
        len++;
        if (len*expansionFactor >= Map.size()) {
            rebuild();
        }
    }

    void erase(const KeyType & q) {
        int hash = getHash(q);
        auto search = find(hash, q);
        if (search.second) {
            elements.erase(*search.first);
            Map[hash].erase(search.first);
            len--;
        }
    }

    iterator find(const KeyType & key) {
        int hash = getHash(key);
        auto search = find(hash, key);
        if (search.second) {
            return iterator(*search.first);
        }
        return end();
    }

    const_iterator find(const KeyType & key) const {
        int hash = getHash(key);
        auto search = find(hash, key);
        if (search.second) {
            return const_iterator(*search.first);
        }
        return end();
    }

    iterator begin() {
        return iterator(elements.begin());
    }

    iterator end() {
        return iterator(elements.end());
    }

    const_iterator begin() const {
        return const_iterator(elements.begin());
    }

    const_iterator end() const {
        return const_iterator(elements.end());
    }

    ValueType & operator [](const KeyType & key) {
        insert(std::make_pair(key, ValueType()));
        int hash = getHash(key);
        return (*(find(hash, key).first))->second;
    }

    const ValueType & at(const KeyType & key) const {
        int hash = getHash(key);
        auto search = find(hash, key);
        if (search.second) {
            return (*search.first)->second;
        }
        throw std::out_of_range("Error key");
    }

    void clear() {
        for (auto p = elements.begin(); p != elements.end(); p++) {
            int hash = getHash(p->first);
            Map[hash].clear();
        }
        elements.clear();
        len = 0;
    }
};