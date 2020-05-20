#include <iostream>
#include "timer.h"

using namespace std;




int main(int argc, char const *argv[])
{
	YTimer yt;
	srand(time(NULL));
	Millisecs period(5000);

	const TPredicate pred = [&]()
	{
		int i = rand();
		if (i%2 == 0)
			return true;
		else
			return false;
	};


	const TTimerCallback cb = [&]()
	{
		cout << "cb() function" << endl;
	};

	const Timepoint tp = CLOCK::now();

	std::thread thread_1( [&]
	{	
		yt.registerTimer(tp, cb);	
	});
	
	std::thread thread_2( [&]
	{	
		yt.registerTimer(period, cb);	
	});

	std::thread thread_3( [&]
	{	
		yt.registerTimer(tp, period, cb);
	});

	std::thread thread_4( [&]
	{	
		yt.registerTimer(pred, period, cb);	
	});

	thread_1.join();
	thread_2.join();
	thread_3.join();
	thread_4.join();

	return 0;
}