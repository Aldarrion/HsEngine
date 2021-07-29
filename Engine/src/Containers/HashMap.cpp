#include "Containers/HashMap.h"

namespace hs
{

const __m128i HashMapConstants::EMPTY_MASK_128 = _mm_set1_epi8(HashMapConstants::VALID_ELEMENT_MASK | HashMapConstants::TOMBSTONE_MASK);

}
