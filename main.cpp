#include "MultiQueueThreadPool.h"
#include <iostream>
#include <unistd.h>


class A {
public:
	int a;
public:
	A() { std::cout << this << " construct" << std::endl; }
	~A() { std::cout << this << " destruct" << std::endl; }
	A(const A& rhs) { std::cout << this << " copy construct" << std::endl; }
	A& operator=(const A& rhs) { std::cout << this << " assign" << std::endl; }
	void show() const { std::cout << this << ":" << a << std::endl;  }
};

void task(unsigned i) 
{
	std::cout << "task " << i << "done" << std::endl;
}

void task2(std::shared_ptr<A>& a) 
{
	a->show();
	return;
}

int main()
{
	MultiQueueThreadPool tp(3);

	{
		std::shared_ptr<A> p = std::make_shared<A>();

		std::cout << "====================" << std::endl;

		unsigned i = 0;
		while(1) {
			sched_yield();
			tp.enqueue(i, task2, p);
			i++;
			break;
		}
	}
	while(1) {
		sched_yield();
	}
	return 0;
}
