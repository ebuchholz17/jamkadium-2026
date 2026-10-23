#ifndef LINUX_AUDIO_H
#define LINUX_AUDIO_H

#include <SDL3/SDL_audio.h>

#include "../game/gng_types.h"
#include "../game/gng_platform.h"

typedef struct linux_sound_output {
    SDL_AudioStream *audioStream;
    b32 soundIsValid;
    i32 samplesPerSecond;
    i32 bytesPerSample;
} linux_sound_output;

i32 sdlAudioGetNumSoundBytesToFill (linux_sound_output *soundOutput);
void sdlAudioFillAudioStreamData(linux_sound_output *soundOutput, game_sound_output *gameSoundOutput, 
                                 i32 numSoundBytesToFill);

#endif
