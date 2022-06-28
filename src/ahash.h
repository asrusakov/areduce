#pragma once

#include <unordered_map>
//
// abstraction for a hash. We may want to switch to more efficient hash
//
template <class Key, class T> class ahash : public std::unordered_map<Key, T>
{
};

