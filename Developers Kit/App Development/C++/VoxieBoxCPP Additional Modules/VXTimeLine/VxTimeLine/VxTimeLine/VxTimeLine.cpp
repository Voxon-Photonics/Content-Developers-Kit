#include "VxTimeLine.h"

VxTimeLine::VxTimeLine()
{
	currentTime.tm_hour = 0;
	currentTime.tm_min = 0;
	currentTime.tm_sec = 0;
	currentTime.tm_mday = 1;
	currentTime.tm_mon = 0;
	currentTime.tm_year = 2000;

	startTime.tm_hour = 0;
	startTime.tm_min = 0;
	startTime.tm_sec = 0;
	startTime.tm_mday = 1;
	startTime.tm_mon = 0;
	startTime.tm_year = 2000;

	stopTime.tm_hour = 0;
	stopTime.tm_min = 0;
	stopTime.tm_sec = 0;
	stopTime.tm_mday = 1;
	stopTime.tm_mon = 0;
	stopTime.tm_year = 2001;

}

VxTimeLine::~VxTimeLine()
{

}
// format is "YYYY-MM-DD-HH-MM-SS"
tm VxTimeLine::strToTm(std::string str)
{

	tm tdate = { 0 };
	std::string buff;

	buff = str.substr(0, 4);
	tdate.tm_year = stoi(buff);

	buff = str.substr(5, 7);
	tdate.tm_mon = stoi(buff);

	buff = str.substr(8, 10);
	tdate.tm_mday = stoi(buff);

	buff = str.substr(11, 13);
	tdate.tm_hour = stoi(buff);

	buff = str.substr(14, 16);
	tdate.tm_min = stoi(buff);

	buff = str.substr(17, 19);
	tdate.tm_sec = stoi(buff);

	return tdate;
}


void VxTimeLine::update(IVoxieBox* voxiePtr)
{
	if (!currentlyPlaying) return;

	float f = playBackSpeed;

	//	updateCurrentTime(1 * f);


	if (voxiePtr->getTime() - prevRuntime >= updateRateInSecs) {
		prevRuntime = voxiePtr->getTime();
		currentTime = amendTime(currentTime, f);
	}

	/*
	if (isWithinTimeLine(currentTime, startTime, stopTime) == false) {
		currentlyPlaying = false;
	}
	*/


	/*
	time_t t1 = mktime(&currentTime);
	time_t t2 = mktime(&stopTime);
	double diffSecs = difftime(t1, t2); // If positive, then tm1 > tm2

	if (diffSecs) {

	}
	*/

}
/*
char* VxTimeLine::getCurrentTimeFullStr()
{
	char buff[50];


	if (currentTime.tm_hour		== 0	&&
		currentTime.tm_isdst	== 0	&&
		currentTime.tm_mday		== 0	&&
		currentTime.tm_min		== 0	&&
		currentTime.tm_sec		== 0	&&
		currentTime.tm_wday		== 0	&&
		currentTime.tm_yday		== 0	&&
		currentTime.tm_year		== 0	&&
		currentTime.tm_mon		== 0)	return buff;




	strftime(buff, 50, "%d-%m-%y | %X", &currentTime);

	return buff;
}
*/

char* VxTimeLine::getTimeDateStr(tm* time)
{
	char buff[50];

	// check if time is valid
	if (time->tm_hour >= 0 && time->tm_hour <= 23 &&
		time->tm_isdst >= 0 && time->tm_isdst <= 1 &&
		time->tm_mday >= 1 && time->tm_mday <= 31 &&
		time->tm_min >= 0 && time->tm_min <= 59 &&
		time->tm_sec >= 0 && time->tm_sec <= 60 &&
		time->tm_wday >= 0 && time->tm_wday <= 6 &&
		time->tm_yday >= 0 && time->tm_yday <= 365 &&
		time->tm_mon >= 0 && time->tm_mon <= 11 &&
		time->tm_year >= 0
		) {

		strftime(buff, 50, "%d-%m-%y | %X", time);
	}

	return buff;
}

char* VxTimeLine::getDateStr(tm* time)
{
	char buff[50];

	// check if time is valid
	if (time->tm_hour >= 0 && time->tm_hour <= 23 &&
		time->tm_isdst >= 0 && time->tm_isdst <= 1 &&
		time->tm_mday >= 1 && time->tm_mday <= 31 &&
		time->tm_min >= 0 && time->tm_min <= 59 &&
		time->tm_sec >= 0 && time->tm_sec <= 60 &&
		time->tm_wday >= 0 && time->tm_wday <= 6 &&
		time->tm_yday >= 0 && time->tm_yday <= 365 &&
		time->tm_mon >= 0 && time->tm_mon <= 11 &&
		time->tm_year >= 0
		) {

		strftime(buff, 50, "%d-%m-%y", time);
	}

	return buff;
}

char* VxTimeLine::getTimeStr(tm* time) {

	char buff[50];

	// check if time is valid
	if (time->tm_hour >= 0 && time->tm_hour <= 23 &&
		time->tm_isdst >= 0 && time->tm_isdst <= 1 &&
		time->tm_mday >= 1 && time->tm_mday <= 31 &&
		time->tm_min >= 0 && time->tm_min <= 59 &&
		time->tm_sec >= 0 && time->tm_sec <= 60 &&
		time->tm_wday >= 0 && time->tm_wday <= 6 &&
		time->tm_yday >= 0 && time->tm_yday <= 365 &&
		time->tm_mon >= 0 && time->tm_mon <= 11 &&
		time->tm_year >= 0
		) {

		strftime(buff, 50, "%X", time);
	}

	return buff;
}

tm VxTimeLine::getCurrentTime()
{
	return this->currentTime;
}

void VxTimeLine::setCurrentTime(tm time)
{
	this->currentTime = time;
}

void VxTimeLine::setStartTime(tm time)
{
	this->startTime = time;
}

void VxTimeLine::setStopTime(tm time)
{
	this->stopTime = time;
}

void VxTimeLine::setCurrentTime(std::string time)
{
	tm tdate = strToTm(time);

	this->currentTime = tdate;

}

void VxTimeLine::setStartTime(std::string time)
{
	tm tdate = strToTm(time);

	this->startTime = tdate;
}

void VxTimeLine::setStopTime(std::string time)
{
	tm tdate = strToTm(time);

	this->stopTime = tdate;

}

bool VxTimeLine::isWithinTimeLine(std::string queryTime, std::string startTime, std::string endTime)
{
	tm tdateQuery = strToTm(queryTime);
	tm tdateStart = strToTm(startTime);
	tm tdateEnd = strToTm(endTime);

	return isWithinTimeLine(tdateQuery, tdateStart, tdateEnd);
}

bool VxTimeLine::isWithinTimeLine(tm queryTime, tm startTime, tm endTime)
{
	bool result = false;


	double qt = dateTimeToDays(queryTime);
	double st = dateTimeToDays(startTime);
	double et = dateTimeToDays(endTime);

	if (qt >= st && qt <= et) result = true;

	return result;


}




tm VxTimeLine::amendTime(tm currentTime, int inSeconds, int inMins, int inHours, int inDays, int inMonths, int inYears)
{
	tm newTime = currentTime;

	// at the moment can only go forwards in time

	int monthDayMax[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int isLeap = 0;
	int toAmend = 0;
	int i = newTime.tm_year % 4;
	if (i == 0) monthDayMax[1] = 29; // is a leap  year

	// check to add or take time...
	if (inSeconds + inMins + inHours + inDays + inMonths + inYears >= 0) {


		newTime.tm_sec += inSeconds;
		if (newTime.tm_sec > 60) {
			toAmend = (newTime.tm_sec) / 60;
			newTime.tm_sec -= (60 * toAmend);
		}

		// minute
		if (toAmend != 0 || inMins != 0) {
			newTime.tm_min += toAmend + inMins;
			toAmend = 0;
			if (newTime.tm_min > 59) {
				toAmend = (newTime.tm_min) / 59;
				newTime.tm_min -= (59 * toAmend);

			}
		}

		// hours 
		if (toAmend != 0 || inHours != 0) {

			newTime.tm_hour += toAmend + inHours;
			toAmend = 0;

			if (newTime.tm_hour > 23) {
				toAmend = (newTime.tm_hour) / 23;
				newTime.tm_hour -= (23 * toAmend);
			}
		}

		// day
		if (toAmend != 0 || inDays != 0) {

			newTime.tm_mday += toAmend + inDays;
			toAmend = 0;

			// leap year check
			i = newTime.tm_year % 4;
			if (i == 0) monthDayMax[1] = 29; // is a leapyear
			else monthDayMax[1] = 28; // isn't a leap year

			if (newTime.tm_mday > monthDayMax[newTime.tm_mon]) {
				toAmend = (newTime.tm_mday) / monthDayMax[newTime.tm_mon];
				newTime.tm_mday -= (monthDayMax[newTime.tm_mon] * toAmend);

			}
		}

		// month
		if (toAmend != 0 || inMonths != 0) {

			// add year
			newTime.tm_mon += toAmend + inMonths;
			toAmend = 0;

			if (newTime.tm_mon > 11) {
				toAmend = (newTime.tm_mon) / 11;
				newTime.tm_mon -= (11 * toAmend);

			}

		}

		// year
		if (toAmend != 0 || inYears != 0) {

			newTime.tm_year += toAmend + inYears;
			toAmend = 0;

		}


	}
	// going backwards
	else {

		newTime.tm_sec += inSeconds;
		if (newTime.tm_sec < 0) {

			toAmend = (fabs(newTime.tm_sec) / 60) + 1;
			newTime.tm_sec += (60 * toAmend);
		}

		// minute
		if (toAmend != 0 || inMins != 0) {
			newTime.tm_min += (-toAmend) + inMins;
			toAmend = 0;
			if (newTime.tm_min < 0) {
				toAmend = (fabs(newTime.tm_min) / 59) + 1;
				newTime.tm_min += (59 * toAmend);

			}
		}

		// hours 
		if (toAmend != 0 || inHours != 0) {

			newTime.tm_hour += (-toAmend) + inHours;
			toAmend = 0;

			if (newTime.tm_hour < 0) {
				toAmend = (fabs(newTime.tm_hour) / 23) + 1;
				newTime.tm_hour += (23 * toAmend);
			}
		}


		// day
		if (toAmend != 0 || inDays != 0) {

			newTime.tm_mday += (-toAmend) + inDays;
			toAmend = 0;

			// leap year check
			i = newTime.tm_year % 4;
			if (i == 0) monthDayMax[1] = 29; // is a leapyear
			else monthDayMax[1] = 28; // isn't a leap year

			if (newTime.tm_mday < 0) {
				toAmend = (fabs(newTime.tm_mday) / monthDayMax[newTime.tm_mon] + 1);
				newTime.tm_mday += (monthDayMax[newTime.tm_mon] * toAmend);
			}
		}

		// month
		if (toAmend != 0 || inMonths != 0) {

			// add year
			newTime.tm_mon += (-toAmend) + inMonths;
			toAmend = 0;

			if (newTime.tm_mon < 0) {
				toAmend = (fabs(newTime.tm_mon) / 11) + 1;
				newTime.tm_mon += (11 * toAmend);

			}

		}

		// year
		if (toAmend != 0 || inYears != 0) {

			newTime.tm_year += (-toAmend) + inYears;
			toAmend = 0;

		}








	}

	return newTime;
}

bool VxTimeLine::isWithinTimeLine(tm* queryTime, tm* startTime, tm* endTime)
{
	bool result = false;

	double qt = dateTimeToDays(*queryTime);
	double st = dateTimeToDays(*startTime);
	double et = dateTimeToDays(*endTime);

	if (qt >= st && qt <= et) result = true;

	return result;
}

bool VxTimeLine::isPlaying()
{
	return this->currentlyPlaying;
}

void VxTimeLine::setPlaying(bool option)
{
	currentlyPlaying = option;



}


void VxTimeLine::setUpdateFrequency(float updateEveryInSecs)
{
	this->updateRateInSecs = updateEveryInSecs;
}

float VxTimeLine::getUpdateFrequency()
{
	return this->updateRateInSecs;
}

void VxTimeLine::setPlayBackSpeed(float playBackSpeed)
{
	this->playBackSpeed = playBackSpeed;
}

float VxTimeLine::getPlayBackSpeed()
{
	return playBackSpeed;
}

double VxTimeLine::dateTimeToDays(std::string queryTimestr)
{
	tm tdate = strToTm(queryTimestr);

	return dateTimeToDays(tdate);
}



double VxTimeLine::dateTimeToDays(tm queryTime)
{
	double answer = 0;

	answer = (queryTime.tm_year - 1) * 365;

	int monthDayMax[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	int i = queryTime.tm_year % 4;
	if (i == 0) monthDayMax[1] = 29; // is a leap  year
	int j = 0;

	// work out what years have been leap years
	for (i = 0; i < queryTime.tm_year; i++) {

		j = i % 4;
		if (j == 0) answer += 1;

	}

	for (i = 0; i < queryTime.tm_mon - 1; i++) {
		answer += monthDayMax[i];
	}

	answer += queryTime.tm_mday;

	// turn everything into seconds
	double timeFraction = ((queryTime.tm_hour * 60) * 60);

	timeFraction += (queryTime.tm_min) * 60;

	timeFraction += queryTime.tm_sec;

	double fractionWas = timeFraction;
	//	normalise formula toMin + (num - fromMin) / (fromMax - fromMin) * (toMax - toMin) 86400 seconds in a day.
	timeFraction = (timeFraction) / (86400 - 0) * (1 - 0);

	answer += timeFraction;

	return answer;







}

float VxTimeLine::getElapsedTime()
{
	return runningTime;
}

void VxTimeLine::report(int posX, int posY, IVoxieBox* voxiePtr)
{
	char buff[50];

	strcpy(buff, getTimeDateStr(&currentTime));
	voxiePtr->debugText(posX, posY, 0x00ff00, -1, "isPlaying %d Playback Speed %1.2f Playback %s", currentlyPlaying, this->playBackSpeed, buff);
	posY += 8;
	strcpy(buff, getTimeDateStr(&startTime));
	voxiePtr->debugText(posX, posY, 0x00ffff, -1, "Start Time %s", buff);
	posY += 8;
	strcpy(buff, getTimeDateStr(&stopTime));
	voxiePtr->debugText(posX, posY, 0xff0000, -1, "Stop Time %s", buff);

}
/*
void VxTimeLine::updateCurrentTime(int inSeconds, int inMins, int inHours, int inDays, int inMonths, int inYears)
{
	// at the moment can only go forwards in time

	int monthDayMax[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int isLeap = 0;

	int i = currentTime.tm_year % 4;
	if (i == 0) monthDayMax[1] = 29; // is a leap  year

	int toAmend = 0;

	currentTime.tm_sec += inSeconds;


	if (currentTime.tm_sec > 60 ) {
		toAmend = (currentTime.tm_sec) / 60;
		currentTime.tm_sec -= (60 * toAmend);
	}

	// second
	if (toAmend != 0 || inMins != 0 ) {

		currentTime.tm_min += toAmend + inMins;
		toAmend = 0;
		if (currentTime.tm_min > 59) {
			toAmend = (currentTime.tm_min) / 59;
			currentTime.tm_min -= (59 * toAmend);

		}

	}

	// minute
	if (toAmend != 0 || inHours  != 0) {

		currentTime.tm_hour += toAmend + inHours;
		toAmend = 0;

		if (currentTime.tm_hour > 23) {
			toAmend = (currentTime.tm_hour) / 23;
			currentTime.tm_hour -= (23 * toAmend);

		}

	}

	// day
	if (toAmend != 0 || inDays != 0) {


		currentTime.tm_mday += toAmend + inDays;
		toAmend = 0;

		// leap year check
		i = currentTime.tm_year % 4;
		if (i == 0) monthDayMax[1] = 29; // is a leapyear
		else monthDayMax[1] = 28; // isn't a leap year

		if (currentTime.tm_mday > monthDayMax[currentTime.tm_mon]) {
			toAmend = (currentTime.tm_mday) / monthDayMax[currentTime.tm_mon];
			currentTime.tm_mday -= (monthDayMax[currentTime.tm_mon] * toAmend);
			currentTime.tm_mday = 1;
		}

	}

	// month
	if (toAmend != 0 || inMonths != 0) {

		// add year
		currentTime.tm_mon += toAmend + inMonths;
		toAmend = 0;

		if (currentTime.tm_mon > 11) {
			toAmend = (currentTime.tm_mon) / 11;
			currentTime.tm_mon -= (11 * toAmend);

		}

	}

	// year
	if (toAmend != 0 || inYears !=  0) {

		currentTime.tm_year += toAmend + inYears;
		toAmend = 0;

	}


}
*/


