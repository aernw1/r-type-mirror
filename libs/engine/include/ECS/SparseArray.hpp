#pragma once

#include <optional>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>


template <typename Component>
class SparseArray {
    using value_type = std::optional<Component>;
    
    using reference_type = value_type&;
    using const_reference_type = value_type const&;
    
    using sparse_array_t = std::vector<value_type>;
    
    using size_type = typename sparse_array_t::size_type;
    using iterator = typename sparse_array_t::iterator;
    using const_iterator = typename sparse_array_t::const_iterator;

    // Constructors
    public:
        SparseArray() = default;
 
        SparseArray(SparseArray const& other);

        SparseArray(SparseArray&& other) noexcept;

        ~SparseArray();

        SparseArray& operator=(SparseArray const& other);
        SparseArray& operator=(SparseArray&& other) noexcept;

        iterator begin();
        const_iterator begin() const;
        const_iterator cbegin() const;

        iterator end();
        const_iterator end() const;
        const_iterator cend() const;

        reference_type operator[](size_t idx);
        const_reference_type operator[](size_t idx) const;

        size_type size() const;

        reference_type insert_at(size_type pos, Component const & component);
        reference_type insert_at(size_type pos, Component && component);
        
        template <class ... Params >
        reference_type emplace_at(size_type pos, Params &&... params);
        
        void erase(size_type pos) ;
        size_type get_index(value_type const & component) const;

    private :
        sparse_array_t _data ;
};