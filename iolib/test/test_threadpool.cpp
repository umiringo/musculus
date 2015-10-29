#include <iostream>
#include "../util/thread.h"
#include <stdlib.h>

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

	return 1；
}