#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "../util/threadpool.h"
#include "../util/timer.h"

using namespace MNET;

class TestTask: public Thread::RunnableTask
{
	int value;
public:
	TestTask(int _value) : Thread::RunnableTask(), value(_value)
	{}

	virtual void run()
	{
		time_t i = Timer::getTime();
		printf("Task %d : %ld\n", value, i);
		sleep(2);
		printf("Task %d end\n", value);
		delete this;
	}
};

class TestAddTask : public Thread::RunnableTask
{
    size_t count;
public:
    TestAddTask(size_t c) : Thread::RunnableTask(), count(c)
    {}

    virtual void run()
    {
        std::cout << "Add Task" << std::endl;
        Thread::Pool::addTask(new TestTask(10));
        delete this;
    }
};

void testThreadPool()
{
	for(int i = 0; i < 10; i++){
		Thread::Pool::addTask(new TestTask(i));
	}
    Thread::Pool::start();
}
int main()
{
    std::cout << "Test threadpool begin..." << std::endl;
    testThreadPool();
    std::cout << "Test threadpool end "<< std::endl;
	return 1;
}
