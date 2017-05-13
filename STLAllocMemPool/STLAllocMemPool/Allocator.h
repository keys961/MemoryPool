#pragma once

namespace Allocator_MemPool
{
	union obj
	{
		obj* link; //link free list
		char data[1]; //data
	};
	const static size_t ALIGN = 8;
	const static size_t MAX_BYTES = 128;
	const static size_t NUM_FREE_LIST = 16;
	static obj* freeList[NUM_FREE_LIST] = { 0, };
	static char* startFree;
	static char* endFree;
	static size_t heapSize;
	

	template <class T>
	class Allocator
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type *const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		Allocator() noexcept
		{
		}
		Allocator(const Allocator& allocator) noexcept : Allocator()
		{		
		}
		~Allocator()
		{
		}
		//Rebind
		template <class U> Allocator(const Allocator<U>& allocator) noexcept: Allocator()
		{
			
		}
		template <class U> struct rebind
		{
			typedef Allocator<U> other;
		};

		/*Allocator<T>& operator=(Allocator&& allocator) noexcept
		{
			if (this != &allocator)
			{
				std::swap(blockPtr, allocator.blockPtr);
				positionPtr = allocator.positionPtr;
				lastPosition = allocator.lastPosition;
				freeNode = allocator.freeNode;
			}
			return *this;
		}*/
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
			//if (_Ptr)
			//{
			//	position_ptr deletePtr = reinterpret_cast<position_ptr>(_Ptr);
			//	deletePtr->next = freeNode;//link freenode
			//	freeNode = deletePtr;//note free as head
			//}
			
			obj* q = (obj*)_Ptr;
			obj** myFreeList;
			if (_Count > (size_t)MAX_BYTES)
			{
				//deallocate
				operator delete(_Ptr, _Count);
				return;
			}
			myFreeList = freeList + freeListIndex(_Count);
			q->link = *myFreeList;
			*myFreeList = q;
		}
		pointer allocate(size_type _Count)
		{
			/*pointer tmp = (pointer)(::operator new((size_t)(((difference_type)_Count) * sizeof(T))));
			if (tmp == 0)
			{
				std::cerr << "Out of memory" << endl;
				exit(1);
			}
			return tmp;*/
			//pointer res = 0;
			//if (freeNode != nullptr)//First use free node
			//{
			//	res = reinterpret_cast<pointer>(freeNode);
			//	freeNode = freeNode->next;
			//}
			//else
			//{
			//	if (lastPosition <= positionPtr)
			//		allocateMem();
			//	res = reinterpret_cast<pointer>(positionPtr);
			//	positionPtr++;
			//}
			//return res;
			obj* res = NULL;
			if (_Count > (size_t)MAX_BYTES)
				return (pointer)malloc(_Count);
			else
			{
				obj** myFreeList = (freeList + freeListIndex(_Count));
				res = *myFreeList;
				if (res == 0)
				{
					void* r = refill(roundUp(_Count));
					return (pointer)r;
				}
				*myFreeList = res->link;
				return (pointer)res;
			}

		}
		void destroy(T* _Ptr)//destruct obj on mem
		{
			_Ptr->~T();
		}
		template <class T, class... Args> void construct(T* _Ptr, Args&&... _Args)//construct obj on mem
		{
			new(_Ptr) T(std::forward<Args>(_Args)...);
		}

		inline size_type max_size() const
		{
			/*size_type maxSize = -1 / blockSize;
			return
				(blockSize - sizeof(data_ptr)) / sizeof(position_ptr) * maxSize;*/
			return size_type(-1) / sizeof(value_type);
		}
		//Mem pool	
	private:
		
		static size_t roundUp(size_t bytes)
		{
			return ((bytes + ALIGN) & ~(ALIGN - 1));
		}

		static size_t freeListIndex(size_t bytes)
		{
			return ((bytes + ALIGN - 1) / ALIGN - 1);
		}

		static void* refill(size_t n)
		{
			int numObjs = 20;
			char* chunk = chunkAlloc(n, numObjs);
			obj*  *myFreeList;
			obj* result;
			obj* current;//Current obj
			obj* next;//Next obj
			if (numObjs == 1)
				return chunk;
			myFreeList = freeList + freeListIndex(n);
			result = (obj*)chunk;
			*myFreeList = next = (obj*)(chunk + n);
			for (int i = 0; ; i++)
			{
				current = next;
				next = (obj*)((char*)next + n);
				if (numObjs - 1 == i)//Finish link
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
					obj*  *myFreeList = freeList + freeListIndex(bytesLeft);
					((obj*)startFree)->link = *myFreeList;
					*myFreeList = (obj*)startFree;
				}
				startFree = (char*)malloc(byteGet);
				if (startFree == nullptr)
				{
					obj*  * myFreeList, *p;
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
					startFree = (char*)malloc(byteGet);
				}
				heapSize += byteGet;
				endFree = startFree + byteGet;
				return chunkAlloc(size, numObjs);
			}
		}

		

		

		//size_type blockSize;
		//union Position
		//{
		//	value_type element;
		//	Position* next;
		//};

		//typedef Position position_type;
		//typedef Position* position_ptr;
		//typedef Position* block_ptr;
		//typedef char* data_ptr;

		//block_ptr blockPtr = 0;//block head
		//position_ptr positionPtr = 0;//current position
		//position_ptr freeNode = 0;//free list
		//position_ptr lastPosition = 0;//last postion

		//void allocateMem()
		//{
		//	blockSize = 2048;
		//	//int blockSize = 4096;//1 block size
		//	void* newPtr = ::operator new(blockSize);
		//	//memset(newPtr, 0, blockSize);
		//	block_ptr newBlock = reinterpret_cast<block_ptr>(newPtr);//new block
		//	newBlock->next = blockPtr;//link, add it to the top
		//	blockPtr = newBlock;//change block to the top

		//	//calculate padding
		//	data_ptr body = reinterpret_cast<data_ptr>(newBlock) + sizeof(position_ptr);
		//	uintptr_t result = reinterpret_cast<uintptr_t>(body);
		//	size_type align = alignof(position_type);
		//	size_type padding = (align - result) % align;

		//	positionPtr = reinterpret_cast<position_ptr>(body + padding);
		//	lastPosition = reinterpret_cast<position_ptr>(newBlock + blockSize
		//		- sizeof(position_type) + 1);
		//}
		//
		//void releaseMem()
		//{
		//	position_ptr current = blockPtr, nextPtr;
		//	while (current)
		//	{
		//		nextPtr = current->next;
		//		operator delete(reinterpret_cast<void*>(current));
		//		current = nextPtr;
		//	}
		//}
		//void operator=(const Allocator&) {}

	};
}

