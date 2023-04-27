#ifndef _BIMAP_H_
#define _BIMAP_

#include <map>

using namespace std;

template <class TKey, class TValue>
class Bimap
{
public:
    bool containsKey(const TKey& key)
    {
        return (forwardMap.find(key) != forwardMap.end());
    }

    typename map<TKey, TValue>::iterator endKey()
    {
        return forwardMap.end();
    }

    bool containsValue(const TValue& value)
    {
        return (reverseMap.find(value) != reverseMap.end());
    }

    typename map<TValue, TKey>::iterator endValue()
    {
        return reverseMap.end();
    }

    bool insert(const TKey& key, const TValue& value)
    {
        if (!containsKey(key))
        {
            forwardMap.insert(pair<TKey, TValue>(key, value));
            reverseMap.insert(pair<TValue, TKey>(value, key));
            return true;
        }
        else
            return false;
    }

    void clear()
    {
        forwardMap.clear();
        reverseMap.clear();
    }

private:
    map<TKey, TValue> forwardMap;

    map<TValue, TKey> reverseMap;
};

#endif // _BIMAP_H
