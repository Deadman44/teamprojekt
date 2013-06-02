#include "cube.h"

void permanent_check()
{
	boost::thread workerThread(worker);

}

void worker()
{
	while(true)
	{
		int repsonse = check_license(user, user_password, license);
		conoutf(user.c_str());
		conoutf(user_password.c_str());
		conoutf(license.c_str());
		std::string s = boost::lexical_cast<std::string>(repsonse);
		conoutf(s.c_str());
		if(repsonse == 200)
			conoutf("Lizenz gueltig");
		else {
			conoutf("Lizenzpruefung fehlgeschlagen");
			boost::posix_time::seconds waiting(10);
			boost::this_thread::sleep(waiting);
			quit();
		}
		// Pruefungsintervall 20s	
		boost::posix_time::seconds waiting(20);
		boost::this_thread::sleep(waiting);
	}




}
