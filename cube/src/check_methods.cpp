#include "cube.h"

void permanent_check()
{
	boost::thread workerThread(worker);

}

void worker()
{
	while(true)
	{
		conoutf("WORKING WORKING");
		boost::posix_time::seconds waiting(10);
		boost::this_thread::sleep(waiting);
	}




}
