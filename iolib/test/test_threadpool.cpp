#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "threadpool.h"
#include "timer.h"
#include "jsonconf.h"

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
    {
        std::cout << "Test Json file..." << std::endl;
        std::ifstream ifs;
        ifs.open("test.json");

        Json::Reader reader;
        Json::Value root;
        if( !reader.parse(ifs, root, false)){
            std::cout << "Parse json failed !" << std::endl;
            return -1;
        }
        std::cout << root["address"]["name"].asString() << std::endl;
    }
    Json::Value value; 
    if(!JsonConf::digConfFromFile("test.json", "address", "name", value)){
        std::cout << "Dig json failed !" <<std::endl;
        return -1;
    }
    
    std::cout << "Test conf..." << std::endl;
    JsonConf::getInstance("test.json");
    std::string name = JsonConf::getInstance()->find("address", "name");
    if( name.empty() ){
        std::cout << "no such key!" << std::endl;
        return -1;
    }
    std::cout << name << std::endl;


    std::cout << "Test threadpool begin..." << std::endl;
    testThreadPool();
    std::cout << "Test threadpool end "<< std::endl;
	return 1;
}
