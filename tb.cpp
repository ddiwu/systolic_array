#include "systolic_array.h"
#include <cstdlib>
#include <iostream>

using namespace std;

#define edge	256
#define SIZE	(edge*edge)

DataType a[SIZE], b[SIZE], res[SIZE];

void init();

int main()
{
	init();

	systolic_array(a, b, res);

	//cout << "\n" << (compare() ? "TEST PASSED!" : "TEST FAILED!") << "\n\n";

	for(int i=0; i<256*256; i++)
    {
        std::cout << res[i] << " ";
        if((i+1)%256 == 0)
        std::cout << std::endl;
	}
	return 0;
}

void init()
{
	for (int i=0; i<SIZE; i++)
	{
		if(i < 128*256)
			a[i] = 1;
		else
			b[i] = 0;
	}
	for (int i=0; i<SIZE; i++)
		b[i] = 1;
}