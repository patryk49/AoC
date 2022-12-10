#pragma once

#include "Utils.hpp"


template<class T, class A> static 
Span<T> make_range(A &al, size_t size) noexcept{
	static_assert(alignof(T) <= A::Alignment, "this alignment is not supported by allocator");

	Memblock blk;
	if constexpr (A::Alignment)
		blk = alloc(al, size*sizeof(T));
	else
		blk = alloc(al, size*sizeof(T), alignof(T));
	return Span<T>{(T *)blk.ptr, blk.size / sizeof(T)};
}


// ALLOCATOR INTERFACE THOUGHT A POINTER
template<class A> static constexpr
bool contains(T *al, Memblock blk) noexcept{ return contains(*al, blk); }

template<class A> static constexpr
Memblock alloc(T *al, size_t size) noexcept{ return alloc(*al, size); }

template<class A> static constexpr
Memblock realloc(T *al, Memblock blk, size_t size) noexcept{ return realloc(*al, blk, size); }



// NULL ALLOCATOR
struct NullAllocator{
	static constexpr size_t Alignment = 0;
	static constexpr bool IsAware = true;
	static constexpr bool HasMassFree = true;
};

static constexpr
bool contains(NullAllocator, Memblock blk) noexcept{ return blk.ptr == nullptr; }

static constexpr
Memblock alloc(NullAllocator, size_t) noexcept{ return Memblock{nullptr, 0}; }

static constexpr
void free(NullAllocator, Memblock = Memblock{}) noexcept{}

static constexpr
Memblock realloc(NullAllocator, Memblock, size_t) noexcept{
	return Memblock{nullptr, 0};
}



// FALLBACK ALLOCATOR
template<auto MA, auto SA>
struct FallbackAllocator{
	typedef std::remove_pointer_t<decltype(MA)> MainAllocator;
	typedef std::remove_pointer_t<decltype(SA)> SpareAllocator;

	static constexpr size_t Alignment = gcd(MainAllocator::Alignment, SpareAllocator::Alignment);

	static constexpr bool IsAware = SpareAllocator::IsAware;
	
	static constexpr bool HasMassFree = MainAloocator::HasMassFree && SpareAllocator::HasMassFree;

	static_assert(
		MainAllocator::IsAware,
		"main allocator of fallback allocator must be aware of what memory belongs to it"
	);
};

template<auto MA, auto SA> static constexpr
bool contains(const FallbackAllocator<MA, SA> &al, Memblock blk) noexcept{
	return contains(decltype(al)::MainAllocator, blk) || contains(decltype(al)::SpareAllocator, blk);
}

template<auto MA, auto SA> static
Memblock alloc(FallbackAllocator<MA, SA> &al, size_t size) noexcept{
	Memblock blk;
	blk = alloc(decltype(al)::MainAllocator, size);
	if (blk.ptr) return blk;
	
	return alloc(decltype(al)::SpareAllocator, size);
}

template<auto MA, auto SA> static
void free(FallbackAllocator<MA, SP> &al, Memblock blk) noexcept{
	if (contains(decltype(al)::MainAllocator, blk))
		free(decltype(al)::MainAllocator, blk);
	else
		free(decltype(al)::SpareAllocator, blk);
}

template<auto MA, auto SA> static
void free(FallbackAllocator<MA, SP> &al) noexcept{
	free(decltype(al)::MainAllocator);
	free(decltype(al)::SpareAllocator);
}

template<auto MA, auto SA> static
Memblock realloc(
	FallbackAllocator<MA, SA> &al, Memblock blk, size_t size
) noexcept{
	if (contains(decltype(al)::MainAllocator, blk)){
		Memblock newblk;
		newblk = realloc(decltype(al)::MainAllocator, blk, size);
	
		if (newblk.ptr) return newblk;
		
		newblk = alloc(decltype(al)::SpareAllocator, size);
		
		if (newblk.ptr){
			for (uint8_t *I=blk.ptr, *J=newblk.ptr; I!=blk.ptr+blk.size; ++I, ++J) *J=*I;
			free(decltype(al)::MainAllocator, blk);
		}
		return newblk;
	}
	return realloc(decltype(al)::SpareAllocator, blk, size);
}



// MALLOC ALLOCATOR
template<
	void *(*M)(size_t) = (void *(*)(size_t))::malloc,
	void (*F)(void *) = nullptr,
	void *(*R)(void *, size_t) = nullptr
>
struct MallocAllocator{
	static constexpr size_t Alignment = alignof(max_align_t);
	static constexpr bool IsAware = false;
	static constexpr bool HasMassFree = false;
	
	static constexpr auto Malloc = M;
	static constexpr auto Free = (
		M==(void *(*)(size_t))::malloc ? (void (*)(void *))::free : F
	);
	static constexpr auto Realloc = (
		M==(void *(*)(size_t))::malloc ? (void *(*)(void *, size_t))::realloc : R
	);

	static_assert(
		F || M==(void *(*)(size_t))::malloc,
		"\"free\" function must come in pair with costum malloc"
	);
};

template<void *(*M)(size_t), void (*F)(void *), void *(*R)(void *, size_t)> static
bool contains(MallocAllocator<M, F, R>, Memblock) noexcept{
	return true;
}

template<void *(*M)(size_t), void (*F)(void *), void *(*R)(void *, size_t)> static
Memblock alloc(MallocAllocator<M, F, R>, size_t size) noexcept{
	return Memblock{(uint8_t *)MallocAllocator<M, F, R>::Malloc(size), size};
}

template<void *(*M)(size_t), void (*F)(void *), void *(*R)(void *, size_t)> static
void free(MallocAllocator<M, F, R>, Memblock blk) noexcept{
	MallocAllocator<M, F, R>::Free(blk.ptr);
}

template<void *(*M)(size_t), void (*F)(void *), void *(*R)(void *, size_t)> static
Memblock realloc(
	MallocAllocator<M, F, R>, Memblock blk, size_t size
) noexcept{
//	if constexpr (MallocAllocator<M, F, R>::Realloc){
		return Memblock{(uint8_t *)MallocAllocator<M, F, R>::Realloc(blk.ptr, size), size};
//	} else{
//		Memblock newBlk;
//		newBlk.ptr = (uint8_t *)MallocAllocator<M, F, R>::Malloc(size);
//		newBlk.size = size;
//		if (newBlk.ptr){
//			for (uint8_t *I=beg(blk), *J=beg(newBlk); I!=end(blk); ++I, ++J) *J = *I;
//			MallocAllocator<M, F, R>::Free(blk.ptr);
//		}
//		return newBlk;
//	}
}



// BUMP ALLOCATOR
template<size_t N, size_t A = alignof(max_align_t)>
struct BumpAllocator{
	static constexpr size_t Alignment = A;
	static constexpr bool IsAware = true;
	static constexpr bool HasMassFree = true;
	
	static constexpr size_t Size = N;

	static_assert(!((Alignment-1) & -Alignment), "alignment must be a power of 2");
	static_assert(N % Alignment == 0, "storage size must be a multiple of the alignment");

	uint8_t *back = storage;
	alignas(A) uint8_t storage[N];
};

template<size_t N, size_t A> static
bool contains(const BumpAllocator<N, A> &al, Memblock blk) noexcept{
	return al.storage <= blk.ptr && blk.ptr < al.back;
}

template<size_t N, size_t A> static
Memblock alloc(BumpAllocator<N, A> &al, size_t size) noexcept{
	uint8_t *newPointer = alignptr(al.back+size, A);
	size_t allocSize = newPointer - al.back;

	Memblock blk{al.back, allocSize};

	if (newPointer > al.storage+N) return Memblock{nullptr, allocSize};
	
	al.back = newPointer;
	return blk;
}

template<size_t N, size_t A> static
void free(BumpAllocator<N, A> &al) noexcept{ al.back = al.storage; }

template<size_t N, size_t A> static
void free(BumpAllocator<N, A> &al, Memblock blk) noexcept{
	if (blk.ptr+blk.size == al.back) al.back = blk.ptr;
}

template<size_t N, size_t A> static
Memblock realloc(
	BumpAllocator<N, A> &al, Memblock blk, size_t size
) noexcept{
	if (blk.ptr+blk.size == al.back){
		uint8_t *newBack = alignptr(blk.ptr+size, A);
		if (newBack <= al.storage+N){
			al.back = newBack;
			return Memblock{blk.ptr, newBack-blk.ptr};
		}
	}
	return Memblock{nullptr, 0};
}



// STACK ALLOCATOR
template<size_t N, size_t A = alignof(max_align_t)>
struct StackAllocator{
	static constexpr size_t Alignment = A;
	static constexpr bool IsAware = true;
	static constexpr bool HasMassFree = true;
	
	static constexpr size_t Size = N;

	static_assert(!((Alignment-1) & -Alignment), "alignment must be a power of 2");
	static_assert(N % Alignment == 0, "storage size must be a multiple of the alignment");

	uint8_t *back = storage;
	uint8_t *front = storage;
	alignas(A) uint8_t storage[N];
};

template<size_t N, size_t A> static
bool contains(const StackAllocator<N, A> &al, Memblock blk) noexcept{
	return al.front <= blk.ptr && blk.ptr < al.back;
}

template<size_t N, size_t A> static
Memblock alloc(StackAllocator<N, A> &al, size_t size) noexcept{
	uint8_t *newPointer = alignptr(al.back+size, A);
	size_t allocSize = newPointer - al.back;

	Memblock blk{al.back, allocSize};

	if (newPointer > al.storage+N){
		if (allocSize < (size_t)(al.front-al.storage))
			al.front -= allocSize;
		else
			blk.ptr = nullptr;
		return blk;
	}
	
	al.back = newPointer;
	return blk;
}

template<size_t N, size_t A> static
void free(StackAllocator<N, A> &al) noexcept{
	al.back = al.storage;
	al.front = al.storage;
}

template<size_t N, size_t A> static
void free(StackAllocator<N, A> &al, Memblock blk) noexcept{
	if (blk.ptr+blk.size == al.back)
		al.back = blk.ptr;
	else if (blk.ptr == al.front)
		al.front = blk.ptr + blk.size;
}

template<size_t N, size_t A> static
Memblock realloc(
	StackAllocator<N, A> &al, Memblock blk, size_t size
) noexcept{
	if (blk.ptr+blk.size == al.back){
		uint8_t *newBack = alignptr(blk.ptr+size, A);
		if (newBack <= al.storage+N){		
			al.back = newBack;
			return Memblock{blk.ptr, newBack-blk.ptr};
		}
	} // else if (blk.ptr == al.front) // maybe add this

	return Memblock{nullptr, 0};
}



// FREE LIST
template<class A>
struct FreeListAllocator{
	static constexpr size_t Alignment = std::remove_pointer_t<A>::Alignment;
	static constexpr bool IsAware = std::remove_pointer_t<A>::IsAware;
	static constexpr bool HasMassFree = std::remove_pointer_t<A>::HasMassFree;
	
	static_assert(
		alignof(uint8_t *) <= std::remove_pointer_t<A>::Alignment,
		"aglignment of underlying allocator must be"
		"compltible with the alignment of the pointer"
	);

	struct Node{
		Node *next;
		size_t size;
	};

	typedef std::remove_pointer_t<A> BaseType;

	[[no_unique_address]] A allocator;
	Node *head = nullptr;
};


template<class A> static
bool contains(const FreeListAllocator<A> &al, Memblock blk) noexcept{
	for (const typename FreeListAllocator<A>::Node *node=al.head; node; node=node->next)
		if (blk.ptr == (uint8_t *)node) return true;
	return contains(al.allocator, blk);
}

template<class A> static
Memblock alloc(FreeListAllocator<A> &al, size_t size) noexcept{
	typename FreeListAllocator<A>::Node **nodePtr = &al.head;
	for (typename FreeListAllocator<A>::Node *node=al.head; node; node=node->next){
		if (size <= node->size){
			if (size <= node->size/2){
				auto end = (typename FreeListAllocator<A>::Node *)alignptr(
					(uint8_t *)node+size, std::remove_pointer_t<A>::Alignment
				);
				size = (uint8_t *)end - (uint8_t *)node;
				end->next = node->next;
				end->size = node->size - size;
				*nodePtr = end;
				return Memblock{(uint8_t *)node, size};
			} else{
				*nodePtr = node->next;
				return Memblock{(uint8_t *)node, node->size};
			}
		}
		nodePtr = &node->next;
	}
	return alloc(deref(al.allocator), size);
}

template<class A> static
void free(FreeListAllocator<A> &al, Memblock blk) noexcept{
	typename FreeListAllocator<A>::Node *prevHead = al.head;
	al.head = (typename FreeListAllocator<A>::Node *)blk.ptr;
	((typename FreeListAllocator<A>::Node *)blk.ptr)->next = prevHead;
	((typename FreeListAllocator<A>::Node *)blk.ptr)->size = blk.size;
}

template<class A> static
void free(std::enable_if_t<A::HasMassFree, FreeListAllocator<A>> &al) noexcept{
	free(deref(al.allocator));
	al.head = nullptr;
}

template<class A> static
Memblock realloc(FreeListAllocator<A> &al, Memblock blk, size_t size) noexcept{
	Memblock newBlk = alloc(al, size);
	if (newBlk.ptr){
		for (uint8_t *I=beg(blk), *J=beg(newBlk); I!=end(blk); ++I, ++J) *J = *I;
		free(al, blk);
	}
	return newBlk;
}

