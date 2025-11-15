module;
#include "common-defines.hpp"
export module sparse_set;

import std;

namespace tektonik
{

// Sparse set pagination.
// TODO restrict IndexType
export template <typename ContainedType, typename IndexType = size_t>
class SparseSet
{
  public:
    SparseSet() = default;
    SparseSet(size_t initSize) : sparse(initSize, kInvalidIndex) {}
    virtual ~SparseSet() = default;

    bool Contains(IndexType index) const
    {
        if (!IsAllocated(index))
            return false;

        return sparse[index] != kInvalidIndex;
    }

    void Add(IndexType index, ContainedType element)
    {
        if (index >= sparse.size())
            sparse.resize(index + 1, kInvalidIndex);

        ASSUMERT(dense.size() < sparse.size());
        ASSUMERT(IsAllocated(index));
        ASSUMERT(!Contains(index));

        sparse[index] = dense.size();
        dense.push_back(DenseElement{.index = index, .value = std::move(element)});
    }

    void Remove(IndexType index)
    {
        ASSUMERT(IsAllocated(index));
        ASSUMERT(Contains(index));

        // Repoint the sparse pointing to the last element to
        // point to the to-be-removed element.
        sparse[dense.back().index] = sparse[index];
        // Swap dense to-be-deleted element to remove with last.
        std::swap(dense[sparse[index]], dense.back());
        // Mark the removed item in sparse set as removed.
        sparse[index] = kInvalidIndex;
        // Pop the unreferenced dense element at the back.
        dense.pop_back();
    }

    ContainedType& Get(IndexType index)
    {
        ASSUMERT(Contains(index));
        return dense[sparse[index]].value;
    }

    const ContainedType& Get(IndexType index) const
    {
        ASSUMERT(Contains(index));
        return dense[sparse[index]].value;
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
        {
            if (sparse[i] == kInvalidIndex)
                continue;

            const auto& denseElement = dense[sparse[i]];
            if (denseElement.index != i)
                return false;
        }

        return true;
    }

    auto begin() noexcept { return dense.begin(); }
    auto cbegin() const noexcept { return dense.cbegin(); }
    auto end() noexcept { return dense.end(); }
    auto cend() const noexcept { return dense.cend(); }
    auto size() const noexcept { return dense.size(); }
    auto empty() const noexcept { return dense.empty(); }

  private:
    struct DenseElement
    {
        IndexType index;
        ContainedType value;
    };

    bool IsAllocated(IndexType index) const { return index >= 0 && index < sparse.size(); }

    inline static constexpr IndexType kInvalidIndex = std::numeric_limits<IndexType>::max();

    std::vector<IndexType> sparse{};
    std::vector<DenseElement> dense{};
};

}  // namespace tektonik
