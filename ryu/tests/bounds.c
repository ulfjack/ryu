#include "ryu/ryu.h"
#include<assert.h>
#include<stdio.h>

int main()
{
	char arr[30];
	assert(d2s_maximum_size(12147832473248742.389732489324793247923487234980,arr,10)==0);
	assert(d2s_maximum_size(12147832473248742.389732489324793247923487234980e218,arr,21)==0);
	assert(d2s_maximum_size(12147832473248742.389732489324793247923487234980,arr,28)!=0);
	puts(arr);
}