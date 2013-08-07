#include "Logger.h"


Logger::Logger(void)
{
	//Dateiname festlegen

	time_t simpletime;
	time (&simpletime);
	struct tm *timeinfo;

	timeinfo = localtime(&simpletime);
	char currentdate[30];
	sprintf(currentdate,"%i-%i-%i-LOG.txt",timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900);

	filename = std::string(currentdate);
	std::cout << filename;



	//Datei erstellen
	std::fstream fs;
	fs.open(filename,std::ios_base::out || std::fstream::app);
	fs << "LOG ERSTELLT " << "\n";
	fs.flush();
	fs.close();

}


Logger::~Logger(void)
{
}


void Logger::startLog()
{
	this->writeToLog("BEGIN AUFZEICHNUNG");
}

void Logger::endLog()
{
	this->writeToLog("ENDE DES LOGS");
}
void Logger::writeToLog(std::string s)
{
	std::fstream fs;
	fs.open(filename,std::fstream::app);

	//Zeit formatieren

	time_t simpletime;
	time (&simpletime);
	struct tm *timeinfo;

	timeinfo = localtime(&simpletime);
	char currentdate[30];
	sprintf(currentdate,"%i:%i -- ",timeinfo->tm_hour,timeinfo->tm_sec);


	fs<<currentdate<<s << " \n";
	fs.flush();
	fs.close();
}
		
void Logger::writeToConsole(std::string s)
{

	std::cout << s << "\n";
}

//nicht verwendet
int Logger::calcTime()
{
	return this->starttime - time(NULL);
}

