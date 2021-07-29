#include "UnitTests.h"
#include "Containers/HashMap.h"
#include "Containers/Array.h"

#include "TestContainersCommon.h"

#include <unordered_map>

using namespace hs;
using namespace hsTest;

//------------------------------------------------------------------------------
template<class ElementT, class HashMapT>
void AddToHashMap(int N, ElementT start, HashMapT& hashmap)
{
    ElementT item = start;
    for (int i = 0; i < N; ++i)
    {
        hashmap.Insert(item, item + 1000);
        item++;
    }
}

using HashMapT = HashMap<int, int>;
using HashMapPtrT = HashMap<int, int>*;

//------------------------------------------------------------------------------
TEST_DEF(HashMap_Ctor)
{
    HashMapT m;
    __m128i v = HashMapConstants::EMPTY_MASK_128;
    TEST_UNUSED(v);
}

//------------------------------------------------------------------------------
TEST_DEF(HashMap_FindOrInsert)
{
    HashMapT m;
    auto result = m.FindOrInsert(0, 10);

    TEST_FALSE(result.first);
    TEST_TRUE(result.second == 10);

    auto result2 = m.FindOrInsert(0, 10);

    TEST_TRUE(result2.first);
    TEST_TRUE(result2.second == 10);
}

//------------------------------------------------------------------------------
TEST_DEF(HashMap_FindOrInsert_ValueCanBeModified)
{
    HashMapT m;
    auto empty = m.FindOrDefault(5);
    empty.second = 15;

    auto filled = m.FindOrDefault(5);
    TEST_TRUE(filled.first);
    TEST_TRUE(filled.second == 15);
}

//------------------------------------------------------------------------------
TEST_DEF(HashMap_InsertMany)
{
    HashMapT m;
    constexpr int count = 100;
    AddToHashMap(count, 0, m);

    for (int i = 0; i < count; ++i)
    {
        TEST_TRUE(m.Contains(i));
    }
}

//------------------------------------------------------------------------------
TEST_DEF(HashMap_Iterators)
{
    HashMapT m;
    const HashMapT& cpm = m;
    m.Insert(5, 1005);

    auto begin = m.begin();
    TEST_TRUE(begin->Key() == 5);
    TEST_TRUE(begin->Value() == 1005);

    auto cbegin = cpm.begin();
    static_assert(std::is_same_v<decltype(cbegin), HashMapT::ConstIterator_t>);
    TEST_TRUE(cbegin->Key() == 5);
    TEST_TRUE(cbegin->Value() == 1005);

    auto end = m.end();
    TEST_UNUSED(end);
    auto cend = cpm.end();
    TEST_UNUSED(cend);

    ++cbegin;
    TEST_TRUE(cbegin == cend);
    --cbegin;
    TEST_TRUE(cbegin == m.cbegin());
}

//------------------------------------------------------------------------------
TEST_DEF(HashMap_Remove)
{
    HashMapT m;
    constexpr int count = 100;
    AddToHashMap(count, 0, m);

    for (int i = 0; i < count; ++i)
    {
        m.Remove(i);
        TEST_FALSE(m.Contains(i));

        m.Insert(-1, -1);
        TEST_FALSE(m.Contains(i));
        TEST_TRUE(m.Contains(-1));

        m.Remove(-1);
        TEST_FALSE(m.Contains(-1));
    }
}

//------------------------------------------------------------------------------
TEST_DEF(HashMap_Find)
{
    HashMapT m;
    AddToHashMap(100, 0, m);

    HashMapPtrT pm = &m;
    for (int i = 0; i < 100; ++i)
    {
        auto it = pm->Find(i);
        TEST_TRUE(it != pm->end());
        TEST_TRUE(it->Key() == i);
        TEST_TRUE(it->Value() == 1000 + i);
    }
}

//------------------------------------------------------------------------------
TEST_DEF(HashMap_Iteration)
{
    HashMapT m;
    constexpr int count = 100;
    AddToHashMap(count, 0, m);

    HashMapPtrT pm = &m;
    int iter = 0;
    Array<int> foundArr;
    for (const auto& kvPair : *pm)
    {
        auto found = pm->Find(kvPair.Key());
        TEST_TRUE(kvPair.Key() == found->Key());
        TEST_TRUE(kvPair.Value() == found->Value());
        ++iter;
        foundArr.Add(kvPair.Key());
    }
    TEST_TRUE(iter == count);

    iter = 0;
    for (auto it = pm->begin(); it != pm->end(); ++it)
    {
        auto found = pm->Find(it->Key());
        TEST_TRUE(it->Key() == found->Key());
        TEST_TRUE(it->Value() == found->Value());
        ++iter;
    }
    TEST_TRUE(iter == count);

    iter = 0;
    auto it = pm->end();
    do
    {
        --it;
        auto found = pm->Find(it->Key());
        TEST_TRUE(it->Key() == found->Key());
        TEST_TRUE(it->Value() == found->Value());
        ++iter;
    } while(it != pm->begin());
    TEST_TRUE(iter == count);
}
