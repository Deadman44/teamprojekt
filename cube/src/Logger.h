#pragma once

#include <time.h>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

class Logger
{
public:

	Logger(void);
	~Logger(void);
	void startLog();
	void endLog();
	void writeToLog(std::string s);
	void writeToConsole(std::string s);


	private:

	int starttime;
	int calcTime();
	std::string filename;
};

