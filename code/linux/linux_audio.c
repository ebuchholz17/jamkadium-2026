#include "linux_audio.h"

#include "../game/gng_types.h"

i32 sdlAudioGetNumSoundBytesToFill (linux_sound_output *soundOutput) {
    f32 secondsOfSoundToBuffer = 0.050f;
    i32 minBytesOfAudio = secondsOfSoundToBuffer * soundOutput->samplesPerSecond * 
                            soundOutput->bytesPerSample;

    i32 queuedBytes = SDL_GetAudioStreamQueued(soundOutput->audioStream);
    i32 bytesToFill = minBytesOfAudio - queuedBytes;
    if (bytesToFill < 0) {
        bytesToFill = 0;
    }

    return bytesToFill;
}

void sdlAudioFillAudioStreamData (linux_sound_output *soundOutput, game_sound_output *gameSoundOutput, 
                                  i32 numSoundBytesToFill) 
{
    static i16 sdlSamples[512];

    i32 totalBytesFilled = 0;
    sound_sample *sourceSample = gameSoundOutput->samples;
    while (totalBytesFilled < numSoundBytesToFill) {
        i32 chunkSampleIndex = 0;
        for (chunkSampleIndex = 0; chunkSampleIndex < ARRAY_COUNT(sdlSamples) && totalBytesFilled < numSoundBytesToFill; chunkSampleIndex++) {
            i16 sdlSample = (i16)(sourceSample->value * 32767);
            sdlSamples[chunkSampleIndex] = sdlSample;
            sourceSample++;
            totalBytesFilled += 2;
        }
        SDL_PutAudioStreamData(soundOutput->audioStream, sdlSamples, chunkSampleIndex * sizeof(i16));
    }
}
