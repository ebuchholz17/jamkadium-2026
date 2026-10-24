#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include <assert.h>
#include <bits/time.h>
#include <stdio.h>
#include <dlfcn.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#include "./linux_platform.h"
#include "../game/gng_platform.h"

#include "./linux_audio.c"
#include "./linux_opengl_renderer.c"

static b32 gameIsRunning = true;
static b32 justResized = false;
static i32 windowWidth = 1280;
static i32 windowHeight = 720;

static b32 isFullscreen = false;
static b32 windowHidden = false;
//static RECT windowedRect = {};

unsigned int numFilesToLoad;
linux_file_to_load filesToLoad[LINUX_FILES_TO_LOAD_MAX];

void processSDLMessages (game_input *input) {
    // TODO: more messages
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:{
                if (event.key.key == SDLK_A) {
                    if (!input->aKey.down && event.key.down) {
                        input->aKey.justPressed = true;
                    }
                    input->aKey.down = event.key.down;
                }
                if (event.key.key == SDLK_S) {
                    if (!input->sKey.down && event.key.down) {
                        input->sKey.justPressed = true;
                    }
                    input->sKey.down = event.key.down;
                }
                if (event.key.key == SDLK_Z) {
                    if (!input->zKey.down && event.key.down) {
                        input->zKey.justPressed = true;
                    }
                    input->zKey.down = event.key.down;
                }
                if (event.key.key == SDLK_X) {
                    if (!input->xKey.down && event.key.down) {
                        input->xKey.justPressed = true;
                    }
                    input->xKey.down = event.key.down;
                }
                if (event.key.key == SDLK_UP) {
                    if (!input->upArrow.down && event.key.down) {
                        input->upArrow.justPressed = true;
                    }
                    input->upArrow.down = event.key.down;
                }
                if (event.key.key == SDLK_DOWN) {
                    if (!input->downArrow.down && event.key.down) {
                        input->downArrow.justPressed = true;
                    }
                    input->downArrow.down = event.key.down;
                }
                if (event.key.key == SDLK_LEFT) {
                    if (!input->leftArrow.down && event.key.down) {
                        input->leftArrow.justPressed = true;
                    }
                    input->leftArrow.down = event.key.down;
                }
                if (event.key.key == SDLK_RIGHT) {
                    if (!input->rightArrow.down && event.key.down) {
                        input->rightArrow.justPressed = true;
                    }
                    input->rightArrow.down = event.key.down;
                }
            } break;
            case SDL_EVENT_MOUSE_MOTION: {
                input->pointerX = event.motion.x;
                input->pointerY = event.motion.y;
            } break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                if (event.button.button == 1) {
                    if (!input->pointerDown) {
                        input->pointerJustDown = true;
                    }
                    input->pointerDown = true;
                }
                else if (event.button.button == 3) {
                    if (!input->pointer2Down) {
                        input->pointer2JustDown = true;
                    }
                    input->pointer2Down = true;
                }
            } break;
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                if (event.button.button == 1) {
                    input->pointerDown = false;
                }
                else if (event.button.button == 3) {
                    input->pointer2Down = false;
                }
            } break;
            case SDL_EVENT_WINDOW_RESIZED: {
                justResized = true;
            } break;
            case SDL_EVENT_QUIT: {
                gameIsRunning = false;
            } break;
        }
    }
}

game_api loadGameLibrary (void **libHandle) {
    void *handle = dlopen("./game.so", RTLD_NOW);
    assert(handle);

    dlerror();

    game_api gameAPI = {0};
    gameAPI.updateGNGGame = (update_gng_game *)dlsym(handle, "updateGNGGame");
    assert(gameAPI.updateGNGGame);
    gameAPI.getSoundSamplesGNGGame = (get_sound_samples_gng_game *)dlsym(handle, "getSoundSamplesGNGGame");
    assert(gameAPI.getSoundSamplesGNGGame);

    *libHandle = handle;
    return gameAPI;
}

void consoleLog (char *msg) {
    printf("%s", msg);
    printf("\n");
}

u64 rngSeedFromTime () {
    return time(NULL);
}

void readFile (char *id, char *path) {
    assert(numFilesToLoad < LINUX_FILES_TO_LOAD_MAX);

    linux_file_to_load file = {};
    file.id = id;
    file.path = path;
    filesToLoad[numFilesToLoad++] = file;
}

f32 clockTimeSecondsF32 (void) {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    f32 seconds = time.tv_sec + (time.tv_nsec / 1000000000.0f);
    return seconds;
}

int main (void) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        return -1;
    }

    openGLRendererSetAttributes();

    SDL_WindowFlags windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL;
    SDL_Window *window = SDL_CreateWindow("GNG", windowWidth, windowHeight, windowFlags);

    // Load game shared library
    void *gameLibHandle = 0;
    game_api gameApi = loadGameLibrary(&gameLibHandle);

    opengl_renderer renderer = {0};
    openGLRendererInit(&renderer, window);
    SDL_GetWindowSizeInPixels(window, &windowWidth, &windowHeight);
    resizeOpenGL(&renderer, windowWidth, windowHeight);

    // Set up platform for game
    plat_api platAPI = {0};

    platAPI.consoleLog = (plat_console_log *)consoleLog;
    platAPI.loadFile = (plat_load_file *)readFile;
    platAPI.rngSeedFromTime = (plat_rng_seed_from_time *)rngSeedFromTime;
    platAPI.mainMemorySize = 64 * 1024 * 1024;
    platAPI.mainMemory = malloc(platAPI.mainMemorySize);
    memset(platAPI.mainMemory, 0, platAPI.mainMemorySize);
    platAPI.assetMemorySize = 192 * 1024 * 1024;
    platAPI.assetMemory = malloc(platAPI.assetMemorySize);
    platAPI.scratchMemorySize = 24 * 1024 * 1024;
    platAPI.scratchMemory = malloc(platAPI.scratchMemorySize);

    // Memory for renderer
    mem_arena renderMemory = {};
    renderMemory.capacity = 64 * 1024 * 1024;
    renderMemory.base = malloc(renderMemory.capacity);
    renderMemory.current = renderMemory.base;

    // sound
    linux_sound_output soundOutput = {
        .audioStream = SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, 
            &((SDL_AudioSpec){
                .channels = 1,
                .format = SDL_AUDIO_S16LE, // 2 bytes, signed, little-endian
                .freq = 44100,
            }), NULL, NULL),
        .samplesPerSecond = 44100,
        .bytesPerSample = 2
    };
    soundOutput.soundIsValid = soundOutput.audioStream != NULL;
    SDL_ResumeAudioStreamDevice(soundOutput.audioStream);

    sound_sample *soundSamples = (sound_sample *)malloc(2 * soundOutput.samplesPerSecond * sizeof(sound_sample));
    ASSERT(soundSamples);

    // input TODO controllers
    game_input input = {};

    f32 lastTime = clockTimeSecondsF32();
    f32 lastGameUpdateTime = clockTimeSecondsF32();
    f32 lastControllerCheckTime = clockTimeSecondsF32();

    // Keep track of when the game library was last modified
    struct stat libWriteTime;
    stat("./game.so", &libWriteTime);
    struct timespec lastLibWriteTime = libWriteTime.st_mtim;

    while (gameIsRunning) {
        processSDLMessages(&input);
        
        f32 workTimeSeconds = clockTimeSecondsF32();
        f32 controllerCheckTime = clockTimeSecondsF32();

        // Reload game library if it has been modified
        stat("./game.so", &libWriteTime);
        if (libWriteTime.st_mtim.tv_sec != lastLibWriteTime.tv_sec) {
            dlclose(gameLibHandle);
            gameApi = loadGameLibrary(&gameLibHandle);
            lastLibWriteTime = libWriteTime.st_mtim;
        }

        #define MAX_FILES_LOADED_PER_FRAME 10000
        plat_loaded_file loadedFiles[MAX_FILES_LOADED_PER_FRAME] = {};
        unsigned int numLoadedFiles = 0;

        if (numFilesToLoad > 0) {
            f32 fileReadStartSeconds = clockTimeSecondsF32();

            float fileReadElapsed = 0.0f;

            while (numFilesToLoad > 0 && 
                   numLoadedFiles < MAX_FILES_LOADED_PER_FRAME && 
                   fileReadElapsed < 0.010f) 
            {
                linux_file_to_load fileToLoad = filesToLoad[0];

                FILE *file = fopen(fileToLoad.path, "rb");
                assert(file);

                fseek(file, 0, SEEK_END);
                i32 fileSize = ftell(file);
                fseek(file, 0, SEEK_SET);

                plat_loaded_file *platLoadedFile = &loadedFiles[numLoadedFiles++];

                platLoadedFile->id = fileToLoad.id;
                platLoadedFile->data = malloc(fileSize);
                platLoadedFile->dataSize = fileSize;

                size_t elementReadCount = fread(platLoadedFile->data, fileSize, 1, file);
                if (elementReadCount < 1) {
                    b32 fileReadSuccess = !feof(file) && !ferror(file);
                    assert(fileReadSuccess);
                }
                fclose(file);

                filesToLoad[0] = filesToLoad[numFilesToLoad-- - 1];

                f32 fileReadCurrentTimeSeconds = clockTimeSecondsF32();
                fileReadElapsed = fileReadCurrentTimeSeconds - fileReadStartSeconds;
            }
        }

        // Set up platform api
        renderMemory.current = renderMemory.base;
        platAPI.windowWidth = windowWidth;
        platAPI.windowHeight = windowHeight;
        platAPI.audioSampleRate = soundOutput.samplesPerSecond;

        // Resize renderer buffers/etc. if window was just resized
        if (justResized && !windowHidden) {
            SDL_GetWindowSizeInPixels(window, &windowWidth, &windowHeight);
            resizeOpenGL(&renderer, windowWidth, windowHeight);
        }
        justResized = false;

        // controller update
        f32 currentGameUpdateTime = clockTimeSecondsF32();
        f32 elapsedTime = currentGameUpdateTime - lastGameUpdateTime;
        lastGameUpdateTime = currentGameUpdateTime;

        gameApi.updateGNGGame(platAPI, &renderMemory, &input, elapsedTime, numLoadedFiles, loadedFiles);

        i32 numSoundBytesToFill = sdlAudioGetNumSoundBytesToFill(&soundOutput);
        if (numSoundBytesToFill > 0) {
            game_sound_output gameSoundOutput = {};
            gameSoundOutput.samplesPerSecond = soundOutput.samplesPerSecond;
            gameSoundOutput.sampleCount = numSoundBytesToFill / soundOutput.bytesPerSample;
            gameSoundOutput.samples = soundSamples;

            gameApi.getSoundSamplesGNGGame(platAPI, &gameSoundOutput);
            sdlAudioFillAudioStreamData(&soundOutput, &gameSoundOutput, numSoundBytesToFill);
        }

        openGLRenderGame(&renderer, &renderMemory);
    }

    //openGLRendererShutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
