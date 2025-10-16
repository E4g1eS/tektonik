export module consteval_map;

template <typename Key, typename Value>
export class ConstevalMap
{
  public:
    consteval void Add(Key key, Value value)
    {
        keys.push_back(key);
        values.push_back(value);
    }

    consteval const Value& Get(Key key)
    {
        for (size_t i = 0; i < keys.size(); ++i)
            if (keys[i] == key)
                return values[i];

        keys.push_back(key);
        return values.back();
    }

  private:
    std::vector<Key> keys;
    std::vector<Value> values;
};
