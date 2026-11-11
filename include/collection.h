#pragma once
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include "exceptions/null_object_exception.h"
#include "exceptions/index_out_of_range_exception.h"

template <typename T>
class Collection {
private:
    std::vector<std::shared_ptr<T>> items;

    bool isValidIndex(int index) const;

public:
    typedef typename std::vector<std::shared_ptr<T>>::const_iterator ConstIterator;

    Collection() = default;

    void add(std::shared_ptr<T> item);
    bool removeAt(int index);
    bool remove(const T& value);
    void clear();
    void sort();

    std::shared_ptr<T> get(int index) const;
    std::shared_ptr<T> at(int index) const;

    int indexOf(const T& value) const;
    bool contains(const T& value) const;
    std::shared_ptr<T> find(const T& value) const;

    int getCount() const;
    bool isEmpty() const;

    void print(std::ostream& os = std::cout, const std::string& indent = "") const;

    ConstIterator begin() const;
    ConstIterator end() const;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Collection<T>& collection);

template <typename T, typename Predicate>
std::shared_ptr<T> findFirst(const Collection<T>& collection, Predicate condition);

template <typename T, typename Predicate, typename Argument>
std::shared_ptr<T> findFirst(const Collection<T>& collection, Predicate condition, const Argument& argument);

template <typename T, typename Predicate>
Collection<T> findAll(const Collection<T>& collection, Predicate condition);

template <typename T, typename Predicate, typename Argument>
Collection<T> findAll(const Collection<T>& collection, Predicate condition, const Argument& argument);

template <typename T, typename Predicate>
int countIf(const Collection<T>& collection, Predicate condition);

template <typename T, typename Predicate, typename Argument>
int countIf(const Collection<T>& collection, Predicate condition, const Argument& argument);

template <typename T>
std::shared_ptr<T> findMax(const Collection<T>& collection);

#include "collection.tpp"
