#pragma once
namespace Allocator_MemPool
{

	template <class T>
	class Allocator
	{
	public:
		typedef void _Not_user_specialized;
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type *const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		Allocator() noexcept
		{
			blockPtr = positionPtr = freeNode = lastPosition = NULL;	
		}
		Allocator(const Allocator& allocator) noexcept : Allocator()
		{
			
		}
		Allocator(const Allocator&& allocator) noexcept
		{
			blockPtr = allocator.blockPtr;
			positionPtr = allocator.positionPtr;
			lastPosition = allocator.lastPosition;
			freeNode = allocator.freeNode;
			allocator.blockPtr = nullptr;
		}
		~Allocator()
		{
			//Release block
			releaseMem();
		}
		//Rebind
		template <class U> Allocator(const Allocator<U>& allocator) noexcept: Allocator()
		{
			
		}
		template <class U> struct rebind
		{
			typedef Allocator<U> other;
		};

		Allocator<T>& operator=(Allocator&& allocator) noexcept
		{
			if (this != &allocator)
			{
				std::swap(blockPtr, allocator.blockPtr);
				positionPtr = allocator.positionPtr;
				lastPosition = allocator.lastPosition;
				freeNode = allocator.freeNode;
			}
			return *this;
		}
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
		inline void deallocate(pointer _Ptr, size_type _Count)
		{
			if (_Ptr)
			{
				position_ptr deletePtr = reinterpret_cast<position_ptr>(_Ptr);
				deletePtr->next = freeNode;//link freenode
				freeNode = deletePtr;//note free as head
			}
		}
		inline pointer allocate(size_type _Count)
		{
			/*pointer tmp = (pointer)(::operator new((size_t)(((difference_type)_Count) * sizeof(T))));
			if (tmp == 0)
			{
				std::cerr << "Out of memory" << endl;
				exit(1);
			}
			return tmp;*/
			pointer res = 0;
			if (freeNode != nullptr)//First use free node
			{
				res = reinterpret_cast<pointer>(freeNode);
				freeNode = freeNode->next;
			}
			else
			{
				if (lastPosition <= positionPtr)
					allocateMem();
				res = reinterpret_cast<pointer>(positionPtr);
				positionPtr++;
			}
			return res;
		}
		template <class U> void destroy(U* _Ptr)//destruct obj on mem
		{
			_Ptr->~U();
		}
		template <class U, class... Args> void construct(U* _Ptr, Args&&... _Args)//construct obj on mem
		{
			new(_Ptr) U(std::forward<Args>(_Args)...);
		}

		inline size_type max_size() const
		{
			size_type maxSize = -1 / blockSize;
			return
				(blockSize - sizeof(data_ptr)) / sizeof(position_ptr) * maxSize;
		}
		//Mem pool
		

	private:
		size_type blockSize;
		union Position
		{
			value_type element;
			Position* next;
		};

		typedef Position position_type;
		typedef Position* position_ptr;
		typedef Position* block_ptr;
		typedef char* data_ptr;

		block_ptr blockPtr = 0;//block head
		position_ptr positionPtr = 0;//current position
		position_ptr freeNode = 0;//free list
		position_ptr lastPosition = 0;//last postion

		void allocateMem()
		{
			blockSize = 2048;
			//int blockSize = 4096;//1 block size
			void* newPtr = ::operator new(blockSize);
			//memset(newPtr, 0, blockSize);
			block_ptr newBlock = reinterpret_cast<block_ptr>(newPtr);//new block
			newBlock->next = blockPtr;//link, add it to the top
			blockPtr = newBlock;//change block to the top

			//calculate padding
			data_ptr body = reinterpret_cast<data_ptr>(newBlock) + sizeof(position_ptr);
			uintptr_t result = reinterpret_cast<uintptr_t>(body);
			size_type align = alignof(position_type);
			size_type padding = (align - result) % align;

			positionPtr = reinterpret_cast<position_ptr>(body + padding);
			lastPosition = reinterpret_cast<position_ptr>(newBlock + blockSize
				- sizeof(position_type) + 1);
		}
		
		void releaseMem()
		{
			position_ptr current = blockPtr, nextPtr;
			while (current)
			{
				nextPtr = current->next;
				operator delete(reinterpret_cast<void*>(current));
				current = nextPtr;
			}
		}
		//void operator=(const Allocator&) {}

	};
}

