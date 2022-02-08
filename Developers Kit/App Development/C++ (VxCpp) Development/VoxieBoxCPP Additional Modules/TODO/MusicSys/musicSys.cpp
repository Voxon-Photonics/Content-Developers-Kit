#define _CRT_SECURE_NO_WARNINGS

/* Music Sys

A music manager for Voxon development
by Matthew Vecchio (ReadyWolf)

*/

#include "musicSys.h"
#include "voxiebox.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>

const int SONG_MAX = 100;
const int DEFAULT_MAX_VOLUME = 100;
const float DEFAULT_PLAYBACK_SPEED = 1;
const int DEFAULT_PLAYBACK_CHANNEL = -1; // -2 is for STEREO samples, -1 for summed mono
const double DEFAULT_FADE_IN_TIME_SECONDS = 5;
const double DEFAULT_FADE_OUT_TIME_SECONDS = 2;

// TODO:

// Add Getters and Setters
// Add Cross Fade
// Add Cueing
// HOLD SHIFT to cue in a songs

// TODO Add cuelist


enum musicSys
{
    PLAYING, CUEING, FADE_IN, FADE_OUT, CROSS_FADE, FULL_STOP, STOPPED, MUTED, DISABLED 
};

 
typedef struct { char * name; char * filename; float length_s; } song_t;

typedef struct {song_t songList[SONG_MAX]; 
bool isEnabled = true,
                    isMuted = false,
                    isLooping = false,
                    cutToTrack = false,
                    enableCrossFade = false,
                    enableShuffle = false,
                    enableMultiCue = false,
                    handleProtection = true; // if your program runs a lot of sounds at once this projects the handle from being over written
                int noAddedSongs = 0,    
                    currentVolume = 0, 
                    previousCurrentVolume = 0,
                    deckAHandle = -1,
                    deckATrackNo = -1, 
                    deckBHandle = -1, 
                    deckBTrackNo = -1,
                    currentDeckHandle = -1,
                    currentDeck = -1,
                    previousDeck = -1,
                    maxVolume = DEFAULT_MAX_VOLUME, 
                    playbackChannel = DEFAULT_PLAYBACK_CHANNEL, 
                    previousPlaybackChannel = DEFAULT_PLAYBACK_CHANNEL,
                    playingStatus = musicSys::STOPPED,
                    previousTrackNo = -1,
                    currentTrackNo = -1, 
                    nextTrackNo = -1; 
                float playbackSpeed = DEFAULT_PLAYBACK_SPEED,
                    previousPlaybackSpeed = DEFAULT_PLAYBACK_SPEED,
                    version = 0.1;
                double trackStartTime = -1, 
                    currentTrackTime = -1,  
                    fadeInTime_s = DEFAULT_FADE_IN_TIME_SECONDS, 
                    fadeOutTime_s = DEFAULT_FADE_OUT_TIME_SECONDS, 
                    oldTime = 0,
                    time = 0,
                    volumeFragments;
                } mxSys_t;

static mxSys_t mx;


// clearDecks -
void mxClearCurrentDeck() {

    if (mx.deckATrackNo == mx.currentTrackNo && mx.currentDeck == mx.deckAHandle) { mx.deckAHandle = -1; }
    if (mx.deckBTrackNo == mx.currentTrackNo && mx.currentDeck == mx.deckBHandle) { mx.deckBHandle = -1; }
    mx.previousDeck = mx.currentDeck;
    mx.previousTrackNo = mx.currentTrackNo;
    mx.currentDeck = -1;
    mx.currentDeckHandle = -1;
    mx.currentTrackTime = 0;
    mx.currentTrackNo = -1;

}


// Adds a song to the songlist return the track number in the array
int mxAddSong ( char * _name, char * _filename, float _length_s) 
{

    char *a = _name;
    char *b = _filename;

    mx.songList[mx.noAddedSongs].name = a;
    mx.songList[mx.noAddedSongs].filename = b;
    mx.songList[mx.noAddedSongs].length_s = _length_s;
    
    mx.noAddedSongs++;
    
    return mx.noAddedSongs - 1;
}

// TODO: Remove song from song list
void mxRemoveSong() {

}

// mxPlay just plays the song
int mxPlay( int trackNo) {

    if (mx.playingStatus == musicSys::FADE_IN || mx.playingStatus == musicSys::CROSS_FADE || mx.playingStatus == musicSys::PLAYING || mx.playingStatus == musicSys::FADE_OUT) {
        
        if (mx.playbackChannel == -2) {
            voxie_playsound_update(mx.currentDeckHandle+0, 0, 0, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.currentDeckHandle+1, 1, 0, 0, mx.playbackSpeed);
        } else {
            voxie_playsound_update(mx.currentDeckHandle, mx.playbackChannel, 0, 0, mx.playbackSpeed);
        }
        mxClearCurrentDeck();

    } 
    mx.currentTrackNo = trackNo;

    mx.playingStatus = musicSys::CUEING;
    

    return trackNo;
}

// mxCutTo - cuts abuptly too new song
int mxCutTo(int trackNo) {

    if (mx.playingStatus == musicSys::FADE_IN || mx.playingStatus == musicSys::CROSS_FADE || mx.playingStatus == musicSys::PLAYING || mx.playingStatus == musicSys::FADE_OUT) {
     
        if (mx.playbackChannel == -2) {
            voxie_playsound_update(mx.currentDeckHandle+0, 0, 0, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.currentDeckHandle+1, 1, 0, 0, mx.playbackSpeed);

        }
        else {
            voxie_playsound_update(mx.currentDeckHandle, mx.playbackChannel, 0, 0, mx.playbackSpeed);
        }

  

        mxClearCurrentDeck();

    }



    mx.currentTrackNo = trackNo;
    mx.cutToTrack = true;
    mx.playingStatus = musicSys::CUEING;


    return trackNo;
}

// fades out current song and crosses to the other song
int mxXPlay( int trackNo) {

    if (mx.playingStatus == musicSys::FADE_IN || mx.playingStatus == musicSys::CROSS_FADE || mx.playingStatus == musicSys::PLAYING || mx.playingStatus == musicSys::FADE_OUT) {


        mx.playingStatus = musicSys::FADE_OUT;
        mx.nextTrackNo = trackNo;


    } else {
    mx.currentTrackNo = trackNo;
    mx.playingStatus = musicSys::CUEING;
    }

    return trackNo;
}

// mxCue - sets 'cues' a track to player after the current track 
int mxCue( int trackNo) {

    // TODO: add multicue
    
    if (mx.playingStatus == musicSys::FADE_IN || mx.playingStatus == musicSys::CROSS_FADE || mx.playingStatus == musicSys::PLAYING || mx.playingStatus == musicSys::FADE_OUT) {

        mx.nextTrackNo = trackNo;


    } else {
    mx.currentTrackNo = trackNo;
    mx.playingStatus = musicSys::CUEING;
    }

    return trackNo;


}




//TODO:
// mxClearCue

// mxCueTop - adds cue to the top of the quecueLise

// mxCrossFade (crosses between two decks)

// nextTrack

// previousTrack


// returns the current track number playing
int mxGetCurrentTrackNo() {

    return mx.currentTrackNo;
}


// Stops the music on the spot - cancels out looping and shuffling
bool mxFullStop() {

    if (mx.playingStatus == musicSys::CROSS_FADE) {

        if (mx.playbackChannel == -2) {
            voxie_playsound_update(mx.deckAHandle+0, 0, 0, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.deckAHandle+1, 1, 0, 0, mx.playbackSpeed);

        }
        else {
            voxie_playsound_update(mx.deckAHandle, mx.playbackChannel, 0, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.deckBHandle, mx.playbackChannel, 0, 0, mx.playbackSpeed);
        }

   
    } else {

        if (mx.playbackChannel == -2) {
            voxie_playsound_update(mx.currentDeckHandle+0, 0, 0, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.currentDeckHandle+1, 1, 0, 0, mx.playbackSpeed);
        }
        else {
            voxie_playsound_update(mx.currentDeckHandle, mx.playbackChannel, 0, 0, mx.playbackSpeed);
        }
    }

    mx.playingStatus = musicSys::FULL_STOP;
    return true;

}

// Stops the music on the spot - cancels out looping and shuffling
bool mxStop() {

    if (mx.playingStatus == musicSys::CROSS_FADE) {
       
        if (mx.playbackChannel == -2) {

            voxie_playsound_update(mx.deckAHandle + 0, 0, 0, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.deckAHandle + 1, 1, 0, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.deckBHandle + 0, 0, 0, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.deckBHandle + 1, 1, 0, 0, mx.playbackSpeed);

        }
        else {
                
            voxie_playsound_update(mx.deckAHandle, mx.playbackChannel, 0, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.deckBHandle, mx.playbackChannel, 0, 0, mx.playbackSpeed);
            
        }
    }
    else {

        if (mx.playbackChannel == -2) {
            voxie_playsound_update(mx.currentDeckHandle+0, 0, 0, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.currentDeckHandle+1, 1, 0, 0, mx.playbackSpeed);
        }
        else {

            voxie_playsound_update(mx.currentDeckHandle, mx.playbackChannel, 0, 0, mx.playbackSpeed);
            
        }

    }

    mx.playingStatus = musicSys::STOPPED;
    return true;

}


bool mxAdjustVolume( int adjustment) {


    mx.currentVolume += adjustment;   
    if (mx.currentVolume > mx.maxVolume) mx.currentVolume = mx.maxVolume;
    if (mx.currentVolume < 1) mx.currentVolume = 1;
   

    return true;
}


bool mxAdjustMaxVolume( int adjustment) {

    mx.maxVolume += adjustment; 
    if (mx.maxVolume < 1 ) mx.maxVolume = 1;
    if (mx.currentVolume >  mx.maxVolume)   mx.currentVolume =  mx.maxVolume;   

 

    return true;
}

// Setters and Getters

bool mxSetVolume( int newVolume) {

    if (newVolume > mx.maxVolume) newVolume = mx.maxVolume;

    mx.currentVolume = newVolume;   

    return true;
}

int mxGetVolume() {

    return mx.currentVolume;  
}

bool mxSetMaxVolume( int newVolume) {

    if (mx.currentVolume > newVolume)   mx.currentVolume = newVolume;   

    mx.maxVolume = newVolume;   

    return true;
}

int mxGetMaxVolume() {

    return mx.maxVolume;  
}

float mxGetPlaybackSpeed() {

    return mx.playbackSpeed;
}

bool mxAdjustPlaybackSpeed(float adjustment) {

    mx.playbackSpeed += adjustment;

    return true;

}




bool mxSetPlaybackSpeed(float newSpeed) {

    

    mx.playbackSpeed = fabs(newSpeed);   

    return true;
}

int mxGetPlaybackChannel() {

    return mx.playbackChannel;
}

bool mxSetPlaybackChannel(float newChannel) {


    mx.playbackChannel =  newChannel;   

    return true;
}


bool mxSetFadeInTime( float secs) {

    mx.fadeInTime_s = secs;

    return true;

}

bool mxSetFadeOutTime( float secs) {

    mx.fadeOutTime_s = secs;

    return true;

}

int mxGetStatus() {

    return mx.playingStatus;
}

bool mxSetLoop(bool choice) {

    mx.isLooping = choice;

    return mx.isLooping;
}

bool mxToggleLoop() {

    mx.isLooping = !mx.isLooping;
    
    return true;

}

bool mxToggleShuffle() {

    mx.enableShuffle = !mx.enableShuffle;
    
    return true;

}

bool mxSetShuffle(bool choice) {

    mx.enableShuffle = choice;

    return mx.enableShuffle;
}

bool mxFadeOut() {

    mx.playingStatus = musicSys::FADE_OUT;

    return true;
}



bool mxUpdate() {

    if (mx.isEnabled == false) {
        mx.playingStatus = musicSys::DISABLED;
        return false; 
    }
    if (mx.isMuted == true) 
    { 
        if (mx.playbackChannel == -2) {
            voxie_playsound_update(mx.currentDeckHandle+0, 0, 1, 0, mx.playbackSpeed);
            voxie_playsound_update(mx.currentDeckHandle+1, 1, 0, 1, mx.playbackSpeed);
        }
        else {

            voxie_playsound_update(mx.currentDeckHandle, mx.playbackChannel, 1, 1, mx.playbackSpeed);
       
        }

 
        mx.playingStatus = musicSys::MUTED;
        return true;
    }
    
    mx.oldTime = mx.time;
    mx.time = voxie_klock();
    double dtim = mx.time - mx.oldTime;    
    int i = mx.currentTrackNo;
    int j = 0;

    // update current track time.
    if (mx.playingStatus == musicSys::PLAYING || mx.playingStatus == musicSys::FADE_IN ||
        mx.playingStatus == musicSys::FADE_OUT || mx.playingStatus == musicSys::CROSS_FADE) mx.currentTrackTime = mx.time - mx.trackStartTime;

    switch (mx.playingStatus) {
    case musicSys::CUEING:
        // volume needs to be at 1 or higher as if it is at 0 it will kill the sound from memory
        mx.currentVolume = 1;

        // brute force way to get the handle up
        if (mx.handleProtection) for (j = 0; j <= 256; j++) voxie_playsound_update(j, 1, 0, 0, 10);
        if (mx.handleProtection) for (j = 0; j < 200; j++) voxie_playsound("InvCapture.wav", 1, 1, 1, -1);



        if (mx.playbackChannel == -2) { // stereo
            // check if the 1st handler is free if not load it in the second
            if (mx.previousDeck != 0)
            {
                mx.deckAHandle = voxie_playsound(mx.songList[i].filename, mx.playbackChannel, mx.currentVolume, mx.currentVolume, mx.playbackSpeed);
                mx.deckATrackNo = mx.currentTrackNo;
                mx.currentDeck = 0;
                mx.deckAHandle = mx.deckAHandle;
                mx.currentDeckHandle = mx.deckAHandle;
           



            }
            else
            {
                mx.deckBHandle = voxie_playsound(mx.songList[i].filename, mx.playbackChannel, mx.currentVolume, mx.currentVolume, mx.playbackSpeed);
                mx.deckBTrackNo = mx.currentTrackNo;
                mx.currentDeck = 1;
                mx.deckBHandle = mx.deckAHandle;
                mx.currentDeckHandle = mx.deckAHandle;

            }


        }
        else { // mono 
            // check if the 1st handler is free if not load it in the second
            if (mx.previousDeck != 0)
            {
                mx.deckAHandle = voxie_playsound(mx.songList[i].filename, mx.playbackChannel, mx.currentVolume, mx.currentVolume, mx.playbackSpeed);
              //  mx.deckAHandle = voxie_playsound("test.wav", -2, mx.currentVolume, mx.currentVolume, mx.playbackSpeed);

                mx.deckATrackNo = mx.currentTrackNo;
                mx.currentDeck = 0;
                mx.currentDeckHandle = mx.deckAHandle;


            }
            else
            {
                mx.deckBHandle = voxie_playsound(mx.songList[i].filename, mx.playbackChannel, mx.currentVolume, mx.currentVolume, mx.playbackSpeed);
                mx.deckBTrackNo = mx.currentTrackNo;
                mx.currentDeck = 1;
                mx.currentDeckHandle = mx.deckBHandle;

            }
        }

        mx.trackStartTime = voxie_klock();
        mx.volumeFragments = 0;
        mx.playingStatus = (mx.enableCrossFade == true) ? musicSys::CROSS_FADE : musicSys::FADE_IN;

        if (mx.cutToTrack == true) {
            mx.playingStatus = musicSys::PLAYING; 
            mx.currentVolume = mx.maxVolume;
            mx.cutToTrack = false;
            if (mx.playbackChannel == -2) { // stereo
                voxie_playsound_update(mx.currentDeckHandle+0, 0, mx.currentVolume, 0, mx.playbackSpeed);
                voxie_playsound_update(mx.currentDeckHandle+1, 1, 0, mx.currentVolume, mx.playbackSpeed);
            }
            else {
                voxie_playsound_update(mx.currentDeckHandle, mx.playbackChannel, mx.currentVolume, mx.currentVolume, mx.playbackSpeed);
            }
         }

            if (mx.handleProtection) for (j = 0; j < 200; j++) voxie_playsound_update(j, 1, 0, 0, 10);
            
   
        break;
        case musicSys::FADE_IN:
            // because volume is an Int we use volumeFragments (Double) for finer adjustment so we can slowly fade over time   
            mx.volumeFragments += (mx.maxVolume / mx.fadeInTime_s ) * dtim ;
           
            if (mx.volumeFragments > 1) {
                mx.currentVolume += int(mx.volumeFragments);
                mx.volumeFragments -= int(mx.volumeFragments);
                
            }
            if (mx.playbackChannel == -2) { // stereo
                voxie_playsound_update(mx.currentDeckHandle+0, 0, mx.currentVolume, 0, mx.playbackSpeed);
                voxie_playsound_update(mx.currentDeckHandle+1, 1, 0, mx.currentVolume, mx.playbackSpeed);
            }
            else {
                voxie_playsound_update(mx.currentDeckHandle, mx.playbackChannel, mx.currentVolume, mx.currentVolume, mx.playbackSpeed);
            }
            
            if (mx.currentVolume >= mx.maxVolume) {
                mx.currentVolume = mx.maxVolume;              
                mx.playingStatus = musicSys::PLAYING;
            }


        //break; no break here because we want to still check weather to fade out even during a long fade in
        case musicSys::PLAYING:

            if (mx.currentTrackTime > mx.songList[mx.currentTrackNo].length_s - mx.fadeOutTime_s) {
                mx.playingStatus = musicSys::FADE_OUT;
                mx.volumeFragments = 0;
            }

            if (mx.previousCurrentVolume != mx.currentVolume || mx.previousPlaybackSpeed != mx.playbackSpeed || mx.playbackChannel != mx.previousPlaybackChannel) {
                mx.previousCurrentVolume = mx.currentVolume;
                mx.previousPlaybackSpeed = mx.playbackSpeed;
                mx.previousPlaybackChannel = mx.playbackChannel;
                if (mx.playbackChannel == -2) { // stereo
                    voxie_playsound_update(mx.currentDeckHandle+0, 0, mx.currentVolume, 0, mx.playbackSpeed);
                    voxie_playsound_update(mx.currentDeckHandle+1, 1, 0, mx.currentVolume, mx.playbackSpeed);
                }
                else {
                    voxie_playsound_update(mx.currentDeckHandle, mx.playbackChannel, mx.currentVolume, mx.currentVolume, mx.playbackSpeed);

                }
            }
            break;
        case musicSys::FADE_OUT:
            
       
            // because volume is an Int we use volumeFragments (Double) for finer adjustment so we can slowly fade over time     
            mx.volumeFragments += (mx.maxVolume / mx.fadeOutTime_s ) * dtim ;
           
            if (mx.volumeFragments > 1) {
                mx.currentVolume -= int(mx.volumeFragments);
                mx.volumeFragments -= int(mx.volumeFragments);             
            }
            if (mx.playbackChannel == -2) { // stereo
                voxie_playsound_update(mx.currentDeckHandle+0, 0, mx.currentVolume, 0, mx.playbackSpeed);
                voxie_playsound_update(mx.currentDeckHandle+1, 1, 0, mx.currentVolume, mx.playbackSpeed);
            }
            else { // mono
                voxie_playsound_update(mx.currentDeckHandle, mx.playbackChannel, mx.currentVolume, mx.currentVolume, mx.playbackSpeed);
            }
            if (mx.currentVolume <= 0) {
                mx.currentVolume = 0;
                if (mx.playbackChannel == -2) { // stereo
                    voxie_playsound_update(mx.currentDeckHandle+0, 0, mx.currentVolume, 0, mx.playbackSpeed);
                    voxie_playsound_update(mx.currentDeckHandle+1, 1, 0, mx.currentVolume, mx.playbackSpeed);
                }
                else {
                    voxie_playsound_update(mx.currentDeckHandle, mx.playbackChannel, mx.currentVolume, mx.currentVolume, mx.playbackSpeed);
                }
                mx.playingStatus = musicSys::STOPPED;
               
            }
        break;
        case musicSys::STOPPED:
          
            mxClearCurrentDeck();

            if (mx.nextTrackNo != -1) {

                mx.currentTrackNo = mx.nextTrackNo;
                mx.nextTrackNo = -1;
                mx.playingStatus = musicSys::CUEING;

            }
            else if (mx.enableShuffle && mx.previousTrackNo != -1)
            {
              
                mx.nextTrackNo = mx.previousTrackNo;

                while (mx.nextTrackNo == mx.previousTrackNo ) {
                    mx.nextTrackNo = rand() % mx.noAddedSongs;
                }

                mxPlay(mx.nextTrackNo);

            }
            else if (mx.isLooping && mx.previousTrackNo != -1 ) mxPlay(mx.previousTrackNo);
           

        break;
        case musicSys::FULL_STOP:
            mx.currentDeck = -1;
        break;
    }

    return true;
}

// kill the mx System
bool mxKill() {

    if (mx.playbackChannel == -2) { // stereo
        voxie_playsound_update(mx.deckAHandle+0, 0, -1, -1, 0);
        voxie_playsound_update(mx.deckAHandle+1, 1, -1, -1, 0);
        voxie_playsound_update(mx.deckBHandle+0, 0, -1, -1, 0);
        voxie_playsound_update(mx.deckBHandle+1, 1, -1, -1, 0);
    }
    else {
        voxie_playsound_update(mx.deckAHandle, mx.playbackChannel, -1, -1, 0);
        voxie_playsound_update(mx.deckBHandle, mx.playbackChannel, -1, -1, 0);
    }



    return true;
}


// shows MusicSys 
bool mxDebug( float posX, float posY) 
{

    int col = 0xFF0099;

    int lineSpace = 10;
    int i = 0;

	voxie_debug_print6x8_(posX, posY, col, -1, "Music Sys | Version %1.2f | Status %d | Looping %d | Shuffle %d", mx.version, mx.playingStatus, mx.isLooping, mx.enableShuffle );
	posY += lineSpace;
	voxie_debug_print6x8_(posX, posY, col, -1, "Current Volume : %d  %1.3f | MaxVolume : %d | Current Track : %s | Current Time : %1.2f Current Deck %d ", mx.currentVolume, mx.volumeFragments, mx.maxVolume, mx.songList[mx.currentTrackNo].name, mx.currentTrackTime, mx.currentDeck ); 
 	posY += lineSpace;   
    voxie_debug_print6x8_(posX, posY, col, -1, "Track Start Time : %1.2f |  deckAHandle : %d | deckBHandle : %d | Playback Channel : %d", 
    mx.trackStartTime, mx.deckAHandle, mx.deckBHandle, mx.playbackChannel);
	posY += lineSpace;
	posY += lineSpace;
  
    for (i = 0; i < mx.noAddedSongs; i++) 
    {
     
        voxie_debug_print6x8_(posX + 15, posY , col - 0x202020, -1, "Song %d \"%s\" File: \"%s\" Length: %1.2f", i,
        mx.songList[i].name, mx.songList[i].filename, mx.songList[i].length_s );

        if (i == mx.currentTrackNo)     voxie_debug_print6x8_(posX - 30, posY, 0xFFFFFF, -1, "-->");
        if (i == mx.deckATrackNo)       voxie_debug_print6x8_(posX - 5, posY, 0xFF0000, -1, "A");
        if (i == mx.deckBTrackNo)       voxie_debug_print6x8_(posX, posY, 0x0000FF, -1, "B");
        if (i == mx.nextTrackNo)        voxie_debug_print6x8_(posX - 10, posY, 0xFFFF00, -1, "N");
        if (i == mx.previousTrackNo)    voxie_debug_print6x8_(posX - 10, posY, 0xFF00AA, -1, "P");
        posY += lineSpace;
    
    }  
    voxie_debug_print6x8_(posX + 30, posY, col, -1, "No Songs %d", mx.noAddedSongs );
	posY += lineSpace;    

	return true;

}


