#pragma once

#include "Utils.hpp"



template<class K> static constexpr
size_t _impl_DefaultHash(const K &key) noexcept{
	union{
		size_t hash;
		K key;
	} u{.key = key};

	if constexpr (sizeof(K) < sizeof(size_t))
		return u.hash & (SIZE_MAX >> (sizeof(K)*8-1));
	else
		return u.hash;
}

template<class K> constexpr
auto DefaultHash = _impl_DefaultHash<K>;



template<class K, class V, size_t C, K M, auto H = DefaultHash<K>>
struct FiniteHashmap{
	constexpr
	V &operator [](const K &key) noexcept{
		for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1))
			if (this->keys[i] == key) return this->values[i];
	}

	constexpr
	const V &operator [](const K &key) const noexcept{
		for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1))
			if (this->keys[i] == key) return this->values[i];
	}

	constexpr
	FiniteHashmap() noexcept{ for (auto &it : this->keys) it = M; }


	typedef K KeyType;
	typedef V ValueType;
	static constexpr size_t Capacity = C;
	static_assert(std::has_single_bit(C), "capacity of open hash table must be a power of 2");

	union{
		K keys[C];
	};
	union{
		V values[C];
	};
};


template<class K, class V, size_t C, K M, auto H> static constexpr
K empty_mark(const FiniteHashmap<K, V, C, M, H> &) noexcept{ return M; }

template<class K, class V, size_t C, K M, auto H> static constexpr
size_t cap(const FiniteHashmap<K, V, C, M, H> &) noexcept{ return C; }

template<class K, class V, size_t C, K M, auto H> static
size_t len(const FiniteHashmap<K, V, C, M, H> &hm) noexcept{
	size_t count = 0;
	for (auto &it : hm.keys) count += it != M;
	return count;
}

template<class K, class V, size_t C, K M, auto H>
bool contains(const FiniteHashmap<K, V, C, M, H> &hm, const K &key) noexcept{
	for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1)){
		if (hm.keys[i] == M) return false;
		if (hm.keys[i] == key) return true;
	}
}

template<class K, class V, size_t C, K M, auto H> static constexpr
V *get(FiniteHashmap<K, V, C, M, H> &hm, const K &key) noexcept{
	for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1)){
		if (hm.keys[i] == M) return nullptr;
		if (hm.keys[i] == key) return (V *)hm.values + i;
	}
}

template<class K, class V, size_t C, K M, auto H> static constexpr
const V *get(const FiniteHashmap<K, V, C, M, H> &hm, const K &key) noexcept{
	for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1)){
		if (hm.keys[i] == M) return nullptr;
		if (hm.keys[i] == key) return (const V *)hm.values + i;
	}
}


template<class K, class V, size_t C, K M, auto H> static
bool add(FiniteHashmap<K, V, C, M, H> &hm, const K &key) noexcept{
	size_t i = H(key) & (C-1);
	size_t j = 0;
	while (hm.keys[i] != M && hm.keys[i] != key){
		i = (i+1) & (C-1);
		[[unlikely]] if (j == C-1) return true;
		++j;
	}
	if (hm.keys[i] == M) hm.keys[i] = key;
	if constexpr (needs_init<V>) init((V *)hm.values + i);
	return false;
}

template<class K, class V, size_t C, K M, auto H> static
bool add(FiniteHashmap<K, V, C, M, H> &hm, const K &key, const V &value) noexcept{
	size_t i = H(key) & (C-1);
	size_t j = 0;
	while (hm.keys[i] != M && hm.keys[i] != key){
		i = (i+1) & (C-1);
		[[unlikely]] if (j == C-1) return true;
		++j;
	}
	if (hm.keys[i] == M) hm.keys[i] = key;
	if constexpr (needs_init<V>) init((V *)hm.values + i);
	hm.values[i] = value;	
	return false;
}

template<class K, class V, size_t C, K M, auto H> static
bool remove(FiniteHashmap<K, V, C, M, H> &hm, const K &key) noexcept{
	size_t i = H(key) & (C-1);
	if (hm.keys[i] == M) return true;
	for (size_t j=i;;){
		hm.keys[i] = M;
		if constexpr (needs_deinit<V>) deinit((V *)hm.values + i);
		for (;;){
			j = (j+1) & (C-1);
			if (hm.keys[i] == M) return false;
			size_t k = H(hm.keys[j]) & (C-1);
			if (!(i<=j ? (i<k && k<=j) : (i<k || k<=j))) break;
		}
		hm.keys[i] = hm.keys[j];
		hm.values[i] = hm.values[j];
		i = j;
	}
}

template<class K, class V, size_t C, K M, auto H> static
void reset(FiniteHashmap<K, V, C, M, H> &hm) noexcept{
	for (auto &it : hm.keys) it = M;
	if constexpr (needs_init<V>) for (auto &it : hm.values) deinit(it);
}






template<class K, size_t C, K M, auto H = DefaultHash<K>>
struct FiniteHashset{

	constexpr
	FiniteHashset() noexcept{ for (auto &it : this->keys) it = M; }

	typedef K KeyType;
	static constexpr size_t Capacity = C;
	static_assert(std::has_single_bit(C), "capacity of open hash table must be a power of 2");

	union{
		K keys[C];
	};
};


template<class K, size_t C, K M, auto H> static constexpr
K empty_mark(const FiniteHashset<K, C, M, H> &) noexcept{ return M; }

template<class K, size_t C, K M, auto H> static constexpr
size_t cap(const FiniteHashset<K, C, M, H> &) noexcept{ return C; }

template<class K, size_t C, K M, auto H> static
size_t len(const FiniteHashset<K, C, M, H> &hs) noexcept{
	size_t count = 0;
	for (auto &it : hs.keys) count += it != M;
	return count;
}

template<class K, size_t C, K M, auto H>
bool contains(const FiniteHashset<K, C, M, H> &hs, const K &key) noexcept{
	for (size_t i = H(key) & (C-1);; i = (i+1) & (C-1)){
		if (hs.keys[i] == M) return false;
		if (hs.keys[i] == key) return true;
	}
}

template<class K, size_t C, K M, auto H> static
bool add(FiniteHashset<K, C, M, H> &hm, const K &key) noexcept{
	size_t i = H(key) & (C-1);
	size_t j = 0;
	while (hm.keys[i] != M && hm.keys[i] != key){
		i = (i+1) & (C-1);
		[[unlikely]] if (j == C-1) return true;
		++j;
	}
	if (hm.keys[i] == M) hm.keys[i] = key;
	return false;
}

template<class K, size_t C, K M, auto H> static
bool remove(FiniteHashset<K, C, M, H> &hs, const K &key) noexcept{
	size_t i = H(key) & (C-1);
	if (hs.keys[i] == M) return true;
	for (size_t j=i;;){
		hs.keys[i] = M;
		for (;;){
			j = (j+1) & (C-1);
			if (hs.keys[i] == M) return false;
			size_t k = H(hs.keys[j]) & (C-1);
			if (!(i<=j ? (i<k && k<=j) : (i<k || k<=j))) break;
		}
		hs.keys[i] = hs.keys[j];
		i = j;
	}
}

template<class K, size_t C, K M, auto H> static
void reset(FiniteHashset<K, C, M, H> &hs) noexcept{
	for (auto &it : hs.keys) it = M;
}
