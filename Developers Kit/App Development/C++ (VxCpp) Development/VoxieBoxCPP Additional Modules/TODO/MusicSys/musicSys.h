/* Music Sys

A music manager for Voxon development
by Matthew Vecchio (ReadyWolf)

// Version 0.0.1 7-5-2020

features

*/
#ifndef MUSIC_SYSTEM
#define MUSIC_SYSTEM

// SONG
int mxAddSong ( char * _name, char * _filename, float _length_ms); 
void mxRemoveSong();


// Playing
int mxPlay  (int trackNo);
int mxXPlay (int trackNo);
int mxCue   (int trackNo); 
int mxCutTo (int trackNo);
bool mxStop();
bool mxFullStop();
bool mxFadeOut();

// Adjustments
bool mxAdjustVolume( int adjustment);
bool mxAdjustMaxVolume( int adjustment);
bool mxAdjustPlaybackSpeed(float adjustment);


// getters 
int     mxGetVolume();
int     mxGetMaxVolume();
float   mxGetPlaybackSpeed();
int		mxGetPlaybackChannel();
int     mxGetStatus();
int		mxGetCurrentTrackNo();

// setters
bool mxSetPlaybackSpeed(float newSpeed);
bool mxSetVolume( int newVolume);
bool mxSetMaxVolume( int newVolume);
bool mxSetPlaybackChannel(float newChannel);
bool mxSetFadeInTime( float secs);
bool mxSetFadeOutTime( float secs);
bool mxSetLoop(bool choice);
bool mxSetShuffle(bool choice);

// toggles
bool mxToggleLoop();
bool mxToggleShuffle();

// Interal
void mxClearCurrentDeck(); 


// Control
bool mxKill();
bool mxUpdate();

// DEBUG
bool mxDebug( float posX, float posY); // draws debug info onto the touch screen about music status



#endif //END MUSIC_SYSTEM