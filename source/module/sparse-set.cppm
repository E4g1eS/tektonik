module;
#include "std.hpp"
export module sparse_set;

// Sparse set pagination.
// TODO restrict IndexType
export template <typename ContainedType, typename IndexType = size_t>
class SparseSet
{
  public:
    SparseSet() = default;
    SparseSet(size_t maxElements) : sparse(maxElements, kInvalidIndex) {}

    bool Contains(IndexType index) const { return sparse[index] != kInvalidIndex; }

    void Add(IndexType index, ContainedType element)
    {
        assert(dense.size() < sparse.size());
        assert(IsValid(index));
        assert(!Contains(index));

        sparse[index] = dense.size();
        dense.push_back(DenseElement{.index = index, .value = std::move(element)});
    }

    void Remove(IndexType index)
    {
        assert(IsValid(index));
        assert(Contains(index));

        // Swap dense element to remove with last
        std::swap(dense[sparse[index]], dense.back());
        // Set sparse index to swapped element index
        // TODO sparse[index] = 
    }

  private:
    struct DenseElement
    {
        IndexType index;
        ContainedType value;
    };

    bool IsValid(IndexType index) const { return index >= 0 && index < sparse.size(); }

    inline static constexpr IndexType kInvalidIndex = std::numeric_limits<IndexType>::max();

    std::vector<IndexType> sparse{};
    std::vector<DenseElement> dense{};
};
