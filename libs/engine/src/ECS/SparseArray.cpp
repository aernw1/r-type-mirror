#include "ECS/SparseArray.hpp"

template <typename Component>
typename SparseArray<Component>::size_type SparseArray<Component>::size() const {
    return _data.size();
}

template <typename Component>
typename SparseArray<Component>::reference_type SparseArray<Component>::operator[](size_t idx) {
    if (idx >= _data.size()) {
        _data.resize(idx + 1);
    }
    return _data[idx];
}

template <typename Component>
typename SparseArray<Component>::const_reference_type SparseArray<Component>::operator[](size_t idx) const {
    return _data[idx];
}

template <typename Component>
SparseArray<Component>::SparseArray(SparseArray const& other) : _data(other._data) {
}

template <typename Component>
SparseArray<Component>::SparseArray(SparseArray&& other) noexcept : _data(std::move(other._data)) {
}

template <typename Component>
SparseArray<Component>::~SparseArray() = default;

template <typename Component>
SparseArray<Component>& SparseArray<Component>::operator=(SparseArray const& other) {
    if (this != &other) {
        _data = other._data;
    }
    return *this;
}

template <typename Component>
SparseArray<Component>& SparseArray<Component>::operator=(SparseArray&& other) noexcept {
    if (this != &other) {
        _data = std::move(other._data);
    }
    return *this;
}

template <typename Component>
typename SparseArray<Component>::iterator SparseArray<Component>::begin() {
    return _data.begin();
}

template <typename Component>
typename SparseArray<Component>::const_iterator SparseArray<Component>::begin() const {
    return _data.begin();
}

template <typename Component>
typename SparseArray<Component>::const_iterator SparseArray<Component>::cbegin() const {
    return _data.cbegin();
}

template <typename Component>
typename SparseArray<Component>::iterator SparseArray<Component>::end() {
    return _data.end();
}

template <typename Component>
typename SparseArray<Component>::const_iterator SparseArray<Component>::end() const {
    return _data.end();
}

template <typename Component>
typename SparseArray<Component>::const_iterator SparseArray<Component>::cend() const {
    return _data.cend();
}

template <typename Component>
typename SparseArray<Component>::reference_type SparseArray<Component>::insert_at(size_type pos, Component const& component) {
    if (pos >= _data.size()) {
        _data.resize(pos + 1);
    }
    _data[pos] = component;
    return _data[pos];
}

template <typename Component>
typename SparseArray<Component>::reference_type SparseArray<Component>::insert_at(size_type pos, Component&& component) {
    if (pos >= _data.size()) {
        _data.resize(pos + 1);
    }
    _data[pos] = std::move(component);
    return _data[pos];
}

template <typename Component>
template <class... Params>
typename SparseArray<Component>::reference_type SparseArray<Component>::emplace_at(size_type pos, Params&&... params) {
    if (pos >= _data.size()) {
        _data.resize(pos + 1);
    }
    _data[pos].emplace(std::forward<Params>(params)...);
    return _data[pos];
}