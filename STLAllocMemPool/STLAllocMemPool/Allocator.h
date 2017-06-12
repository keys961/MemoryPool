#pragma once

#include <cstddef>
#include <utility>
#include "stdafx.h"

namespace Allocator_MemPool
{
	union obj
	{
		obj* link; //link free list
		char data[1]; //data
	};
	const static std::size_t ALIGN = 1024;
	const static std::size_t MAX_BYTES = ALIGN << 4;
	const static std::size_t NUM_FREE_LIST = 16;
	static obj* freeList[NUM_FREE_LIST] = { 0 };
	static char* startFree;
	static char* endFree;
	static size_t heapSize = 0;


	template <class T>
	class allocator
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type *const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		allocator() noexcept
		{
		}
		allocator(const allocator<T>& refAllocator) noexcept : allocator()
		{
		}
		~allocator()
		{
		}
		//Rebind
		template <class U> allocator(const allocator<U>& refAllocator) noexcept: allocator()
		{

		}
		template <class U> struct rebind
		{
			typedef allocator<U> other;
		};

		//Get address
		inline pointer address(reference _Val) const noexcept
		{
			return &_Val;
		}
		inline const_pointer address(const_reference _Val) const noexcept
		{
			return &_Val;
		}
		//Alloc/Dealloc
		void deallocate(pointer _Ptr, size_type _Count)
		{
			obj* q = (obj*)_Ptr;
			obj** myFreeList;
			const size_type deleteSize = _Count * sizeof(value_type);
			if (deleteSize > (size_t)MAX_BYTES)
			{
				//deallocate
				::operator delete (_Ptr);
				return;
			}
			myFreeList = freeList + freeListIndex(deleteSize);
			q->link = *myFreeList;
			*myFreeList = q;
		}
		pointer allocate(size_type _Count)
		{
			obj* res = NULL;
			const size_type needSize = _Count * sizeof(value_type);
			if (needSize > MAX_BYTES)
				return static_cast<pointer>(::operator new (needSize));
			else
			{
				obj** myFreeList = (freeList + freeListIndex(needSize));
				res = *myFreeList;
				if (res == 0)
				{
					void* r = refill(roundUp(needSize));
					return static_cast<pointer>(r);
				}
				*myFreeList = res->link;
				return reinterpret_cast<pointer>(res);
			}

		}
		template <class U>inline void destroy(U* _Ptr)//destruct obj on mem
		{
			_Ptr->~U();
		}
		template <class U, class... Args>inline void construct(U* _Ptr, Args&&... _Args)//construct obj on mem
		{
			new(_Ptr) U(std::forward<Args>(_Args)...);
		}
		/*
		template <class U, class... Args> void construct(U* _Ptr, Args&&... _Args)//construct obj on mem
		{
		new(_Ptr) U(std::forward<Args>(_Args)..._Args);
		}
		*/
		inline size_type max_size() const
		{
			/*size_type maxSize = -1 / blockSize;
			return
			(blockSize - sizeof(data_ptr)) / sizeof(position_ptr) * maxSize;*/
			return size_type(-1) / sizeof(value_type);
		}
		//Mem pool
	private:

		inline static size_t roundUp(size_t bytes)
		{
			return ((bytes + ALIGN) & ~(ALIGN - 1));
		}

		inline static size_t freeListIndex(size_t bytes)
		{
			return ((bytes + ALIGN - 1) / ALIGN - 1);
		}

		static void* refill(size_t n)
		{
			int numObjs = 20;
			char* chunk = chunkAlloc(n, numObjs);
			obj **myFreeList, *current, *next;
			void* result;
			if (numObjs == 1)
				return chunk;
			myFreeList = freeList + freeListIndex(n);
			result = chunk;
			*myFreeList = next = reinterpret_cast<obj*>(chunk + n);
			numObjs -= 2;
			for (int i = 0; ; ++i)
			{
				current = next;
				next = reinterpret_cast<obj*>(reinterpret_cast<char*>(next) + n);
				if (numObjs == i)//Finish link
				{
					current->link = nullptr;
					break;
				}
				else//Link
					current->link = next;
			}
			return result;
		}

		static char* chunkAlloc(size_t size, int& numObjs)
		{
			char* result;

			size_t totalBytes = size * numObjs;
			size_t bytesLeft = endFree - startFree;
			if (bytesLeft >= totalBytes)
			{
				result = startFree;
				startFree += totalBytes;
				return result;
			}
			else if (bytesLeft >= size)
			{
				numObjs = bytesLeft / size;
				totalBytes = size * numObjs;
				result = startFree;
				startFree += totalBytes;
				return result;
			}
			else
			{
				size_t byteGet = 2 * totalBytes + roundUp(heapSize >> 4);
				if (bytesLeft > 0)
				{
					obj** myFreeList = freeList + freeListIndex(bytesLeft);
					reinterpret_cast<obj*>(startFree)->link = *myFreeList;
					*myFreeList = reinterpret_cast<obj*>(startFree);
				}
				try {
					startFree = reinterpret_cast<char*>(::operator new (byteGet));
				}
				catch (const std::exception& e) {
					obj* * myFreeList, *p;
					for (int i = size; i <= MAX_BYTES; i += ALIGN)
					{
						myFreeList = freeList + freeListIndex(i);
						p = *myFreeList;
						if (p)
						{
							*myFreeList = p->link;
							startFree = (char*)p;
							endFree = startFree + i;
							return (chunkAlloc(size, numObjs));
						}
					}
					endFree = NULL;
					startFree = static_cast<char*>(::operator new(byteGet));
				}
				heapSize += byteGet;
				endFree = startFree + byteGet;
				// endFree = startFree + totalBytes;
				return chunkAlloc(size, numObjs);
			}
		}
	};
}

