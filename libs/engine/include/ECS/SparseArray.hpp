/**
 * @file SparseArray.hpp
 * @brief Implémentation d'un sparse array pour le système ECS
 */

#pragma once

#include <optional>
#include <vector>
#include <stdexcept>
#include <algorithm>

/**
 * @class SparseArray
 * @brief Container sparse array pour stocker des composants ECS
 *
 * Un sparse array est un tableau creux où l'index correspond directement à l'ID d'une entité.
 * Les composants sont stockés dans un std::vector<std::optional<Component>>, permettant
 * d'avoir des "trous" (indices sans composant).
 *
 * @tparam Component Le type de composant à stocker
 *
 * @note Cette implémentation est optimisée pour les accès directs par index (O(1))
 *       et les itérations séquentielles (cache-friendly).
 *
 * @example
 * @code
 * SparseArray<Position> positions;
 * positions.insert_at(5, Position{10, 20});  // Entity 5
 * positions.insert_at(7, Position{30, 40});  // Entity 7
 *
 * // Accès direct
 * if (positions[5].has_value()) {
 *     Position& pos = positions[5].value();
 * }
 * @endcode
 */
template <typename Component>
class SparseArray {
    using value_type = std::optional<Component>;

    using reference_type = value_type&;
    using const_reference_type = value_type const&;

    using sparse_array_t = std::vector<value_type>;

    using size_type = typename sparse_array_t::size_type;
    using iterator = typename sparse_array_t::iterator;
    using const_iterator = typename sparse_array_t::const_iterator;
public:
    SparseArray() = default;

    SparseArray(SparseArray const& other) : _data(other._data) {
    }

    SparseArray(SparseArray&& other) noexcept : _data(std::move(other._data)) {
    }

    ~SparseArray() = default;

    SparseArray& operator=(SparseArray const& other) {
        if (this != &other) {
            _data = other._data;
        }
        return *this;
    }

    SparseArray& operator=(SparseArray&& other) noexcept {
        if (this != &other) {
            _data = std::move(other._data);
        }
        return *this;
    }

    iterator begin() {
        return _data.begin();
    }

    const_iterator begin() const {
        return _data.begin();
    }

    const_iterator cbegin() const {
        return _data.cbegin();
    }

    iterator end() {
        return _data.end();
    }

    const_iterator end() const {
        return _data.end();
    }

    const_iterator cend() const {
        return _data.cend();
    }

    reference_type operator[](size_t idx) {
        if (idx >= _data.size()) {
            ensure_capacity(idx + 1);
        }
        return _data[idx];
    }

    const_reference_type operator[](size_t idx) const {
        if (idx >= _data.size()) {
            throw std::out_of_range("SparseArray::operator[] const: index out of range");
        }
        return _data[idx];
    }

    size_type size() const {
        return _data.size();
    }

    reference_type insert_at(size_type pos, Component const& component) {
        if (pos >= _data.size()) {
            ensure_capacity(pos + 1);
        }
        _data[pos] = component;
        return _data[pos];
    }

    reference_type insert_at(size_type pos, Component&& component) {
        if (pos >= _data.size()) {
            ensure_capacity(pos + 1);
        }
        _data[pos] = std::move(component);
        return _data[pos];
    }

    template <class... Params>
    reference_type emplace_at(size_type pos, Params&&... params) {
        if (pos >= _data.size()) {
            ensure_capacity(pos + 1);
        }
        _data[pos].emplace(std::forward<Params>(params)...);
        return _data[pos];
    }

    void erase(size_type pos) {
        if (pos >= _data.size()) {
            return;
        }
        _data[pos].reset();
    }

    size_type capacity() const {
        return _data.capacity();
    }

    bool empty() const {
        for (const auto& opt : _data) {
            if (opt.has_value()) {
                return false;
            }
        }
        return true;
    }

    void clear() {
        _data.clear();
    }

    void reserve(size_type new_capacity) {
        _data.reserve(new_capacity);
    }

private:
    void ensure_capacity(size_type required_size) {
        if (required_size <= _data.size()) {
            return;
        }

        size_type new_capacity = std::max(required_size, _data.capacity() * 2);
        if (new_capacity < 8) {
            new_capacity = 8;
        }
        _data.reserve(new_capacity);
        _data.resize(required_size);
    }

    sparse_array_t _data;
};