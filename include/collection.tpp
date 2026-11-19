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
    std::stable_sort(items.begin(), items.end(),
        [](const std::shared_ptr<T>& first, const std::shared_ptr<T>& second) { return *first < *second; });
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
    typename std::vector<std::shared_ptr<T>>::const_iterator found = std::find_if(items.begin(), items.end(),
        [&value](const std::shared_ptr<T>& item) { return *item == value; });
    if (found == items.end()) return -1;
    return static_cast<int>(found - items.begin());
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
    auto found = std::find_if(collection.begin(), collection.end(),
        [&condition](const std::shared_ptr<T>& item) { return condition(*item); });
    return found == collection.end() ? nullptr : *found;
}

template <typename T, typename Predicate, typename Argument>
std::shared_ptr<T> findFirst(const Collection<T>& collection, Predicate condition, const Argument& argument) {
    auto found = std::find_if(collection.begin(), collection.end(),
        [&condition, &argument](const std::shared_ptr<T>& item) { return condition(*item, argument); });
    return found == collection.end() ? nullptr : *found;
}

template <typename T, typename Predicate>
Collection<T> findAll(const Collection<T>& collection, Predicate condition) {
    Collection<T> result;
    std::for_each(collection.begin(), collection.end(),
        [&result, &condition](const std::shared_ptr<T>& item) { if (condition(*item)) result.add(item); });
    return result;
}

template <typename T, typename Predicate, typename Argument>
Collection<T> findAll(const Collection<T>& collection, Predicate condition, const Argument& argument) {
    Collection<T> result;
    std::for_each(collection.begin(), collection.end(),
        [&result, &condition, &argument](const std::shared_ptr<T>& item) {
            if (condition(*item, argument)) result.add(item);
        });
    return result;
}

template <typename T, typename Predicate>
int countIf(const Collection<T>& collection, Predicate condition) {
    return static_cast<int>(std::count_if(collection.begin(), collection.end(),
        [&condition](const std::shared_ptr<T>& item) { return condition(*item); }));
}

template <typename T, typename Predicate, typename Argument>
int countIf(const Collection<T>& collection, Predicate condition, const Argument& argument) {
    return static_cast<int>(std::count_if(collection.begin(), collection.end(),
        [&condition, &argument](const std::shared_ptr<T>& item) { return condition(*item, argument); }));
}

template <typename T>
std::shared_ptr<T> findMax(const Collection<T>& collection) {
    auto best = std::max_element(collection.begin(), collection.end(),
        [](const std::shared_ptr<T>& first, const std::shared_ptr<T>& second) { return *first < *second; });
    return best == collection.end() ? nullptr : *best;
}

template <typename T>
std::shared_ptr<T> findMin(const Collection<T>& collection) {
    auto worst = std::min_element(collection.begin(), collection.end(),
        [](const std::shared_ptr<T>& first, const std::shared_ptr<T>& second) { return *first < *second; });
    return worst == collection.end() ? nullptr : *worst;
}

#endif
