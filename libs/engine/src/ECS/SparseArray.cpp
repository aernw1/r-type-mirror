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
