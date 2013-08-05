/***********************************************************************************
Teamprojekt
Lizenzierungstechnike am Beispiel von Cube
     ___          ___          ___          ___     
    /  /\        /  /\        /  /\        /  /\    
   /  /::\      /  /:/       /  /::\      /  /::\   
  /  /:/\:\    /  /:/       /  /:/\:\    /  /:/\:\  
 /  /:/  \:\  /  /:/       /  /::\ \:\  /  /::\ \:\ 
/__/:/ \  \:\/__/:/     /\/__/:/\:\_\:|/__/:/\:\ \:\
\  \:\  \__\/\  \:\    /:/\  \:\ \:\/:/\  \:\ \:\_\/ 
 \  \:\       \  \:\  /:/  \  \:\_\::/  \  \:\ \:\  
  \  \:\       \  \:\/:/    \  \:\/:/    \  \:\_\/  
   \  \:\       \  \::/      \__\::/      \  \:\    
    \__\/        \__\/           ~~        \__\/    

Feilen Markus,Wilde Hermann,Hoor Johannes,Schneider Florian

Beschreibung.:
Hilfsklasse zum Starten der SSL-Verbindungen in separaten Threads
************************************************************************************/
#include "cube.h"

void permanent_check()
{
	boost::thread workerThread(worker);	
}


void acquire_SAT()
{
	boost::thread workerThread(get_SAT);
	workerThread.join(); //besser an dieser stelle auf response warten, da auf ticket gewartet werden muss

}

void reacquire_SAT()
{
	boost::thread workerThread(get_SAT);
	secondSATrequired = true;
	//hier kein join, das würde das spiel zusätzlich unnötig blockieren, da mam bereits auf dem spielfeld ist
}


void worker()
{
	// Globale Variable, die das Beenden von Cube signalisiert
	// --> Ticketsstream wird gestoppt
	while(thread_exit==false)
	{
		int response = check_license(user, user_password, license);
		if(response == 200)
			std::cout << "Lizenz gueltig" << std::endl;
		else if(response == 403) {	// Autentifizierung fehlgeschlagen
			conoutf("Leider sind ihre Benutzerdaten oder ihre Lizenz ungueltig");
			boost::posix_time::seconds waiting(10);
			boost::this_thread::sleep(waiting);
			quit();
		} else if(response == 500) {	// Verbindungsfehler
			conoutf("Verbindungsproblem zum Server");
			boost::posix_time::seconds waiting(10);
			boost::this_thread::sleep(waiting);
			quit();
		} else {
			conoutf("Unbekannter Fehler");
			boost::posix_time::seconds waiting(10);
			boost::this_thread::sleep(waiting);
			quit();
		}
		// Pruefungsintervall 20s	
		boost::posix_time::seconds waiting(20);
		boost::this_thread::sleep(waiting);
	}


}
