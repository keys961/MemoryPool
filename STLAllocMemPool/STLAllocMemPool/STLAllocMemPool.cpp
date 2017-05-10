// STLAllocMemPool.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Allocator.h"
#include <iostream>
#include <vector>

using std::cout;
using std::endl;

int main()
{
	//Allocator_MemPool::Allocator<int> test1;
	std::vector<int, Allocator_MemPool::Allocator<int> > vect;
	for (int i = 0; i < 10000; i++)
		vect.push_back(i);
	for (int i = 0; i < 10000; i++)
		std::cout << vect[i] << endl;
	system("pause");
	return 0;
}

