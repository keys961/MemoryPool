// STLAllocMemPool.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Allocator.h"
#include <iostream>
#include <vector>
#include <list>
#include <stack>

using std::cout;
using std::endl;

int main()
{
	
	//Allocator_MemPool::Allocator<int> test1;
	std::list<int, Allocator_MemPool::Allocator<int> > vect;
	for (int i = 1; i < 104; i++)
		vect.push_front(i);
	for (int i = 50; i < 104; i++)
	{
		//int x;
		//x = vect.top();
		cout << vect.front() << endl;
		vect.pop_front();
	}
	system("pause");
	return 0;
}

