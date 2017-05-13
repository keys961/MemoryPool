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
	std::vector<int, Allocator_MemPool::Allocator<int> > vect;
	for (int i = 1; i < 104; i++)
		vect.push_back(i);
	for (int i = 50; i < 104; i++)
	{
		//int x;
		//x = vect.top();
		cout << vect[i] << endl;
	}
	system("pause");
	return 0;
}

