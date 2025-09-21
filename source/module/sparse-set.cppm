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

        // Repoint the sparse pointing to the last element to
        // point to the to-be-removed element.
        sparse[dense.back().index] = sparse[index];
        // Repoint last dense pointer to the
        // to-be-removed index.
        dense.back().index = sparse[index];
        // Swap dense to-be-deleted element to remove with last.
        std::swap(dense[sparse[index]], dense.back());
        // Mark the removed item in sparse set as removed.
        sparse[index] = kInvalidIndex;
    }

    // Checks the validity of the whole data structure.
    // Basically just for debugging, it is not needed for production.
    bool IsValid() const
    {
        const size_t sparsePointerCount = std::ranges::count_if(sparse, [this](IndexType index) { return index != kInvalidIndex; });
        if (sparsePointerCount != dense.size())
            return false;

        // Dense and sparse pointers match
        for (IndexType i = 0; i < sparse.size(); ++i)
            if (dense[sparse[i]].index != i)
                return false;
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
