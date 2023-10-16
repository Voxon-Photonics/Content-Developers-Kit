#pragma once
#include "vxCpp.h"


#define MAX_CHANNELS 4
/**
 *  VxBeeper is a class to use with the Voxon speaker which can be used by creating a custom audio callback.
 *  This class can be used to put in simple beep sounds in your VX++ app. See the example to see (and hear)  
 *  how it works. Matthew Vecchio 2-7-2021
 */


// holds all the values associated with playing a beeper note
typedef struct VxNote {

	double frequency	= 0;
	double volume		= 0;
	double startTime	= 0;
	double duration		= 0;
	double vibrato		= 0;
	double sweep		= 0;
	double fadeDuration = 0;
	double fadeLevel	= 0;


} beepNote_t;

class VxBeeper: public IActive, public IVoxiePtr
{
public:

	VxBeeper(IVoxieBox * voxiePtr);
	~VxBeeper();

	void	playNote(int noteFrequency = 44000, double noteAmplitude = 1, double noteDuration = 0.1f, int FX = 0);


//  TODO:
//	Fade notes, FX (chords, vibrato, sweeps, presets) 

//	void	sequenceNote(int noteChannel, double NoteFrequency, double NoteAmp, double NoteDuration = 0.1, double FX = 0);
//	void	sequenceClear();
//	void	muteAll();
//	void	fadeTo(double newVolume, double fadeDuration = 2);
//	void	beep(int preset = 0); // plays a bunch of presents

	int		playMix(int* sampleBuffer, int sampleRate);

private:

	beepNote_t	notes[MAX_CHANNELS] = { 0 };
	double		masterVolume = 0;
	int			noteOn = 0;
	int			gNote = 0;
	int			nextNote = 0;
	int			fade = 0;
	double		noteDuration = 0;
	IVoxieBox	*voxiePtr = 0;

	// TODO add note sequencer -- add for multiple notes

	int noteArray[78] = { B1,
					 C2, Db2, D2, Eb2, E2, F2, Gb2, G2, Ab2, A2, B2,
					 C3, Db3, D3, Eb3, E3, F3, Gb3, G3, Ab3, A3, B3,
					 C4, Db4, D4, Eb4, E4, F4, Gb4, G4, Ab4, A4, B4,
					 C5, Db5, D5, Eb5, E5, F5, Gb5, G5, Ab5, A5, B5,
					 C6, Db6, D6, Eb6, E6, F6, Gb6, G6, Ab6, A6, B6,
					 C7, Db7, D7, Eb7, E7, F7, Gb7, G7, Ab7, A7, B7,
					 C8, Db8, D8, Eb8, E8, F8, Gb8, G8, Ab8, A8, B8,
	};

};


