#if 1

#include "interproc_rw_lock.h"
#include <thread>
#include <iostream>
#include <chrono>
#include <conio.h>

#define COUT(x) cout << x << endl

using namespace tepp;

void ww()
{
	std::cout << ">>> w\n";
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << '.';
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << '.';
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << '.';
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << '.';
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "<<< w\n";

}
void rr()
{
	std::cout << ">>> r\n";
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "<<< r\n";

}
int main()
{
	auto il = interproc_rw_lock::try_create_interproc_rw_lock("toto",2);
	if (!il)
	{
		std::cout << "cant start\n";
		return 1;
	}


	std::cout << "press r, w or . to leave\n";
	std::string er;
	while (1)
	{
		char c = (char)_getch();
		switch (c)
		{
		case 'x':
		case '.':
			return 0;

		case 'w':
			std::cout << "\nwriting\n";
			if (!il->write_under_lock([] {ww(); }, nullptr, 10, 10, &er))
				std::cout << "err w:" << er << "\n";
			continue;

		case 'r':
			std::cout << "\nreading\n";
			if (!il->read_under_lock([] {rr(); }, nullptr, 10, 10, &er))
				std::cout << "err r:" << er << "\n";
			continue;

		case 'W':
			std::cout << "\nthrowing W\n";
			if (!il->write_under_lock([] {throw 1; }, nullptr, 10, 10, &er))
				std::cout << "err W:" << er << "\n";
			continue;
		case 'R':
			std::cout << "\nthrowing R\n";
			if (!il->read_under_lock([] {throw 1; }, nullptr, 10, 10, &er))
				std::cout << "err R:" << er << "\n";
			continue;

		};
	}

	
	std::cout << "bye\n";
	 
}



#endif
