#ifndef COLLECTION_TPP
#define COLLECTION_TPP

#include "collection.h"

template <typename T>
bool Collection<T>::isValidIndex(int index) const {
    return index >= 0 && index < static_cast<int>(items.size());
}

template <typename T>
void Collection<T>::add(std::shared_ptr<T> item) {
    if (!item) throw NullObjectException("Collection::add", "item");
    items.push_back(item);
}

template <typename T>
bool Collection<T>::removeAt(int index) {
    if (!isValidIndex(index)) return false;
    items.erase(items.begin() + index);
    return true;
}

template <typename T>
bool Collection<T>::remove(const T& value) {
    return removeAt(indexOf(value));
}

template <typename T>
void Collection<T>::clear() {
    items.clear();
}

template <typename T>
void Collection<T>::sort() {
    int count = static_cast<int>(items.size());
    for (int i = 1; i < count; ++i) {
        std::shared_ptr<T> current = items[i];
        int j = i - 1;
        while (j >= 0 && *current < *items[j]) {
            items[j + 1] = items[j];
            j--;
        }
        items[j + 1] = current;
    }
}

template <typename T>
std::shared_ptr<T> Collection<T>::get(int index) const {
    if (!isValidIndex(index)) return nullptr;
    return items[index];
}

template <typename T>
std::shared_ptr<T> Collection<T>::at(int index) const {
    if (!isValidIndex(index)) {
        throw IndexOutOfRangeException(index, static_cast<int>(items.size()));
    }
    return items[index];
}

template <typename T>
int Collection<T>::indexOf(const T& value) const {
    int count = static_cast<int>(items.size());
    for (int i = 0; i < count; ++i) {
        if (*items[i] == value) return i;
    }
    return -1;
}

template <typename T>
bool Collection<T>::contains(const T& value) const {
    return indexOf(value) >= 0;
}

template <typename T>
std::shared_ptr<T> Collection<T>::find(const T& value) const {
    return get(indexOf(value));
}

template <typename T>
int Collection<T>::getCount() const {
    return static_cast<int>(items.size());
}

template <typename T>
bool Collection<T>::isEmpty() const {
    return items.empty();
}

template <typename T>
void Collection<T>::print(std::ostream& os, const std::string& indent) const {
    if (items.empty()) {
        os << indent << "Empty\n";
        return;
    }
    int count = static_cast<int>(items.size());
    for (int i = 0; i < count; ++i) {
        os << indent << i + 1 << ". " << *items[i] << "\n";
    }
}

template <typename T>
typename Collection<T>::ConstIterator Collection<T>::begin() const {
    return items.begin();
}

template <typename T>
typename Collection<T>::ConstIterator Collection<T>::end() const {
    return items.end();
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Collection<T>& collection) {
    collection.print(os);
    return os;
}

template <typename T, typename Predicate>
std::shared_ptr<T> findFirst(const Collection<T>& collection, Predicate condition) {
    for (const auto& item : collection) {
        if (condition(*item)) return item;
    }
    return nullptr;
}

template <typename T, typename Predicate, typename Argument>
std::shared_ptr<T> findFirst(const Collection<T>& collection, Predicate condition, const Argument& argument) {
    for (const auto& item : collection) {
        if (condition(*item, argument)) return item;
    }
    return nullptr;
}

template <typename T, typename Predicate>
Collection<T> findAll(const Collection<T>& collection, Predicate condition) {
    Collection<T> result;
    for (const auto& item : collection) {
        if (condition(*item)) result.add(item);
    }
    return result;
}

template <typename T, typename Predicate, typename Argument>
Collection<T> findAll(const Collection<T>& collection, Predicate condition, const Argument& argument) {
    Collection<T> result;
    for (const auto& item : collection) {
        if (condition(*item, argument)) result.add(item);
    }
    return result;
}

template <typename T, typename Predicate>
int countIf(const Collection<T>& collection, Predicate condition) {
    int total = 0;
    for (const auto& item : collection) {
        if (condition(*item)) total++;
    }
    return total;
}

template <typename T, typename Predicate, typename Argument>
int countIf(const Collection<T>& collection, Predicate condition, const Argument& argument) {
    int total = 0;
    for (const auto& item : collection) {
        if (condition(*item, argument)) total++;
    }
    return total;
}

template <typename T>
std::shared_ptr<T> findMax(const Collection<T>& collection) {
    std::shared_ptr<T> best = nullptr;
    for (const auto& item : collection) {
        if (!best || *item > *best) best = item;
    }
    return best;
}

#endif
