#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "../util/threadpool.h"

using namespace MNET;

class TestTask: public Thread::RunnableTask
{
	int value;
public:
	TestTask(int _value) : Thread::RunnableTask(), value(_value)
	{}

	virtual void run()
	{
		int i = rand();
		printf("Task %d : %d\n", value, i);
		sleep(2);
		printf("Task %d end\n", value);
		delete this;
	}
};

int main()
{
    std::cout << "Test threadpool begin..." << std::endl;
	return 1;
}
