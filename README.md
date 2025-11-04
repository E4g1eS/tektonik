# Tektonik

TODO


```

#include <iostream>
#include <string>

template <size_t Length>
struct FixedString
{
  template<size_t InputLength>
  consteval FixedString(const char(&inputStr)[InputLength])
  {
    for (size_t i = 0; i < Length; ++i)
      str[i] = inputStr[i];
  }

  constexpr bool IsSameAs(const char* other) const
  {
    for (size_t i = 0; i < Length; ++i)
      if (str[i] != other[i])
        return false;

    return true;
  }

  size_t size() const { return Length; }

  char str[Length];
};

template <size_t N>
FixedString(const char(&)[N]) -> FixedString<N - 1>;

//template <FixedString str>
//class ReflectiveEnum
//{
//public:
//  std::string GetStr()
//  {
//    return std::string(str.str, str.size());
//  }
//};

template <FixedString ... enumValues>
class ReflectiveEnums
{
public:
  constexpr ReflectiveEnums() = default;
  constexpr ReflectiveEnums(const char* enumValue) : option(ToOption(enumValue)) {}

  constexpr operator int() const { return option; }

  explicit operator std::string() const { return std::string("unimplemented so far"); }

private:
  constexpr static int ToOption(const char* str)
  {
    int counter = 0;
    int option = -1;

    ([&]
      {

        if (enumValues.IsSameAs(str))
          option = counter;

        ++counter;

      }(), ...);

    return option;
  }

  int option = 0;
};

using Animal = ReflectiveEnums<"cat", "dog", "frog">;

int main()
{
  //ReflectiveEnum<"dog"> dogEnum;
  //std::string dogStr = dogEnum.GetStr();

  Animal animal = Animal("dog");

  switch (animal)
  {
    case Animal("dog"):
      std::cout << "animals is dog" << std::endl;
      break;
    default:
      std::cout << "animals is not zero" << std::endl;
      break;
  }

  return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

```