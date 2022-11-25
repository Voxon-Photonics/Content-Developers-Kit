#pragma once
#include "vxCpp.h"
#include <time.h>
#include <vector>
#include <string>


// VXTimeLine - stores a date and timer 
// Go forwards in time
// Go backwards
// compare times 


class VxTimeLine : public IVoxiePtr
{
public:

	VxTimeLine();
	~VxTimeLine();

	// format is "YYYY-MM-DD-HH-MM-SS"
	tm strToTm(std::string time);

	void update(IVoxieBox* voxiePtr);


	char* getDateStr(tm* time);
	char* getTimeDateStr(tm* time);
	char* getTimeStr(tm* time);
	tm   getCurrentTime();
	void setCurrentTime(tm time);
	void setStartTime(tm time);
	void setStopTime(tm time);

	// format is "YYYY-MM-DD-HH-MM-SS"
	void setCurrentTime(std::string time);
	// format is "YYYY-MM-DD-HH-MM-SS"
	void setStartTime(std::string time);
	// format is "YYYY-MM-DD-HH-MM-SS"
	void setStopTime(std::string time);

	bool isWithinTimeLine(std::string queryTime, std::string startTime, std::string endTime);
	bool isWithinTimeLine(tm queryTime, tm startTime, tm endTime);
	bool isWithinTimeLine(tm* queryTime, tm* startTime, tm* endTime);

	// only goes forwards
	tm amendTime(tm currentTime, int inSeconds = 0, int inMins = 0, int inHours = 0, int inDays = 0, int inMonths = 0, int inYears = 0);

	bool isPlaying();
	void setPlaying(bool option);

	void setUpdateFrequency(float updateEveryInSecs);
	float getUpdateFrequency();

	double dateTimeToDays(tm queryTime);
	// format is "YYYY-MM-DD-HH-MM-SS"
	double dateTimeToDays(std::string queryTimestr);

	void setPlayBackSpeed(float playbackSpeed);

	float getPlayBackSpeed();

	float getElapsedTime();

	void report(int posX, int posY, IVoxieBox* voxiePtr);

private:

	//	void updateCurrentTime(int inSeconds = 0, int inMin = 0, int inHours = 0, int inDays = 0, int inMonths = 0, int inYears = 0);

	bool currentlyPlaying = false;
	double runningTime = 0;
	tm currentTime = { 0 };
	tm stopTime = { 0 };
	tm startTime = { 0 };
	float playBackSpeed = -35;
	double prevRuntime = 0;
	float updateRateInSecs = .01; // the threshold // delay that needs to occcur between the last update 1 means every second...

};

