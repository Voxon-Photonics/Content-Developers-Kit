#include "VxBeeper.h"


VxBeeper::VxBeeper(IVoxieBox* voxiePtr)
{
	this->voxiePtr = voxiePtr;
}

VxBeeper::~VxBeeper()
{
}

//! Plays a note
/**
 * @param	noteFrequency	the frequency of the note to play see vx.inputTypes::MUSICAL_NOTES enum. Value is expressed as a whole number so middle C would be 26163
 * @param	note
 *
 *
 */
void VxBeeper::playNote(int noteFrequency, double noteAmplitude, double noteDuration, int FX)
{
	if (voxiePtr == 0) return;
	if (nextNote >= MAX_CHANNELS) return;
	notes[nextNote].frequency = noteFrequency;
	notes[nextNote].duration = noteDuration + voxiePtr->getTime();
	notes[nextNote].startTime = voxiePtr->getTime();
	notes[nextNote].volume = noteAmplitude;
	nextNote++;
}

int VxBeeper::playMix(int* sampleBuffer, int sampleRate)
{
	if (!active) return 0;
	if (!voxiePtr) return -1;

	double tim = voxiePtr->getTime();
	int i = 0;
	nextNote = 0;
	int playSound = 0; // if this is set to 0 it will mute. It will play when set to -1 
	VxNote nplay[MAX_CHANNELS] = { 0 }; //
 
	for (i = 0; i < MAX_CHANNELS; i++) {

		if (notes[i].frequency != 0 && notes[i].startTime <= tim && notes[i].duration >= tim) {
			nplay[nextNote].duration		= notes[i].duration;
			nplay[nextNote].fadeDuration	= notes[i].fadeDuration;
			nplay[nextNote].fadeLevel		= notes[i].fadeLevel;
			nplay[nextNote].frequency		= notes[i].frequency;
			nplay[nextNote].startTime		= notes[i].startTime;
			nplay[nextNote].sweep			= notes[i].sweep;
			nplay[nextNote].vibrato			= notes[i].vibrato;
			nplay[nextNote].volume			= notes[i].volume;
			nextNote++;
			playSound = -1;
		}

	}

	

	// note array to pass in cull to just active notes playing.
	//duration is made from the LONGEST duration

	int noChannelsToPlay = 2; // set via vw.playnchans (default is 2)
	static int cnt[2];
	
	for (i = 0; i < sampleRate; i++, sampleBuffer+=noChannelsToPlay) {

		if (playSound == 0) { // write nothing so it is actually slient otherwise sound leaks through
			sampleBuffer[0] = ((cnt[0] & (1 << 20)) - (1 << 19)) & playSound; cnt[0] += 0;
			sampleBuffer[1] = ((cnt[1] & (1 << 20)) - (1 << 19)) & playSound; cnt[1] += 0;
			continue;
		}
		
		if (nextNote == 2) {
			sampleBuffer[0] = (((cnt[0] & (1 << 20)) - (1 << 19)) & playSound) * nplay[0].volume; cnt[0] += nplay[0].frequency;
			sampleBuffer[1] = (((cnt[1] & (1 << 20)) - (1 << 19)) & playSound) * nplay[1].volume; cnt[1] += nplay[1].frequency;
		}
		else {
			sampleBuffer[0] = (((cnt[0] & (1 << 20)) - (1 << 19)) & playSound) * nplay[(gNote) % nextNote].volume;		cnt[0] += nplay[(gNote) % nextNote].frequency;
			sampleBuffer[1] = (((cnt[1] & (1 << 20)) - (1 << 19)) & playSound) * nplay[(gNote + 1) % nextNote].volume;	cnt[1] += nplay[(gNote + 1) % nextNote].frequency;
		}
		
	}
	gNote++;
	if (gNote >= nextNote) gNote = 0;
	return 1;
}
