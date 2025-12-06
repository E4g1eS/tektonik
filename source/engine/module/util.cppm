module;
#include "common-defines.hpp"
export module util;

import std;

// Utility functions and classes that do not fit anywhere else.

import concepts;

template <typename T>
concept FormattableButNotOutStreamable = std::formattable<T, char> && !requires(T obj, std::ostream& os) {
    { os << obj } -> std::same_as<std::ostream&>;
};

export std::ostream& operator<<(std::ostream& os, const FormattableButNotOutStreamable auto& onlyFormattable)
{
    std::format_to(std::ostream_iterator<char>(os), "{}", onlyFormattable);
    return os;
}

export std::ostream& operator<<(std::ostream& os, const tektonik::concepts::Tiable auto& tiable)
{
    std::format_to(std::ostream_iterator<char>(os), "{}", tiable.Tie());
    return os;
}

export std::ostream& operator<<(std::ostream& os, const tektonik::concepts::ToStringable auto& toStringable)
{
    os << toStringable.ToString();
    return os;
}

namespace tektonik::util
{

export void MoveDelete(auto& object)
{
    auto temporary = std::move(object);
}

export template <concepts::Enum EnumType>
class Flags
{
  public:
    using Underlying = std::underlying_type_t<EnumType>;

    constexpr Flags() = default;
    constexpr Flags(EnumType e) : bits(ToBit(e)) {}
    constexpr explicit Flags(Underlying bits) : bits(bits) {}

    constexpr bool IsSet(EnumType e) const { return (bits & ToBit(e)) != 0; }
    constexpr void Set(EnumType e) { bits |= ToBit(e); }
    constexpr void Reset(EnumType e) { bits &= ~ToBit(e); }

    // Bitwise operators between Flags
    constexpr Flags operator|(const Flags& other) const { return Flags{static_cast<Underlying>(bits | other.bits)}; }
    constexpr Flags operator&(const Flags& other) const { return Flags{static_cast<Underlying>(bits & other.bits)}; }
    constexpr Flags operator^(const Flags& other) const { return Flags{static_cast<Underlying>(bits ^ other.bits)}; }

    // Compound assignment
    constexpr Flags& operator|=(const Flags& other)
    {
        bits |= other.bits;
        return *this;
    }
    constexpr Flags& operator&=(const Flags& other)
    {
        bits &= other.bits;
        return *this;
    }
    constexpr Flags& operator^=(const Flags& other)
    {
        bits ^= other.bits;
        return *this;
    }

    // Single operand operators
    constexpr Flags operator~() const { return Flags{static_cast<Underlying>(~bits)}; }

    // Comparison
    constexpr bool operator==(const Flags& other) const { return bits == other.bits; }
    constexpr bool operator!=(const Flags& other) const { return !(*this == other); }

    constexpr explicit operator bool() const { return bits != 0; }

    Underlying GetUnderlying() const { return bits; }

  private:
    static constexpr Underlying ToBit(EnumType e) { return static_cast<Underlying>(e); }

    Underlying bits{0};
};

namespace ranges
{

export template <concepts::Pointer ValuePointer, typename TargetType = std::remove_pointer_t<ValuePointer>>
auto MakeVector(ValuePointer pointer, size_t count)
{
    std::vector<TargetType> vec;
    vec.reserve(count);
    for (size_t i = 0; i < count; ++i)
        vec.push_back(static_cast<TargetType>(pointer[i]));

    return vec;
}

}  // namespace ranges

namespace string
{

export std::string_view Trim(const std::string_view& str, const std::locale& locale = std::locale::classic());

export std::vector<std::string_view> ParseCommandLineArgumentsToVector(int argc, char* argv[]);

export std::unordered_map<std::string_view, std::string_view> ParseCommandLineArgumentsToMap(int argc, char* argv[]);

export std::string ToString(const concepts::Tiable auto& tiable)
{
    std::stringstream ss;
    ss << tiable;
    return ss.str();
}

export std::string ToString(const std::formattable auto& formattable)
{
    return std::format("{}", formattable);
}

export enum class Case {
    Lower,
    Upper,
};

template <Case caseConvert>
std::string ToCaseFromStringView(const std::string_view& str)
{
    std::stringstream ss;
    for (char c : str)
    {
        if constexpr (caseConvert == Case::Lower)
            ss << static_cast<char>(std::tolower(c));
        else
            ss << static_cast<char>(std::toupper(c));
    }

    return ss.str();
}

export template <Case caseConvert>
std::string ToCase(const concepts::StaticCastableTo<std::string_view> auto& str)
{
    if constexpr (std::is_same_v<decltype(str), const std::string_view&>)
        return ToCaseFromStringView<caseConvert>(str);
    else
        return ToCaseFromStringView<caseConvert>(static_cast<std::string_view>(str));
}

// Checks that all wanted are available. Returns a set of wanted, but unavailable.
export std::unordered_set<std::string_view> HasAll(
    const concepts::RangeOfCastableTo<std::string_view> auto& availables,
    const concepts::RangeOfCastableTo<std::string_view> auto& wanteds)
{
    auto remainingWanteds = std::unordered_set<std::string_view>(wanteds.begin(), wanteds.end());

    for (const auto& available : availables)
        remainingWanteds.erase(available);

    return remainingWanteds;
}

}  // namespace string

}  // namespace tektonik::util
