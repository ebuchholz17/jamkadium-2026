#ifndef LINUX_OPENGL_RENDERER_H
#define LINUX_OPENGL_RENDERER_H

#include <GL/glew.h>
#include <SDL3/SDL_video.h>

#include "../game/gng_types.h"
#include "../game/gng_memory.h"

#define MAX_NUM_OPENGL_TEXTURES 1000
#define UNUSED_TEXTURE_VAL ((u32)(-1))

typedef struct shader_program {
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
} shader_program;

// opengl format vertex data
#pragma pack(push, 1)
typedef struct opengl_vertex {
    f32 pos[3];
    f32 texCoords[2];
    f32 color[4];
    float textureID; // NOTE: webgl only uses floats
} opengl_vertex;
#pragma pack(pop)

typedef struct texture_size {
    f32 width;
    f32 height;
} texture_size;

typedef struct opengl_renderer {
    SDL_GLContext gl;
    SDL_Window *window;

    shader_program spriteShader;
    GLuint spriteVertexBuffer;
    GLuint spriteIndexBuffer;

    shader_program basic3DShader;
    GLuint basic3DVertexBuffer;
    GLuint basic3DIndexBuffer;

    i32 windowWidth;
    i32 windowHeight;

    GLuint textures[MAX_NUM_OPENGL_TEXTURES];
    texture_size textureSizes[MAX_NUM_OPENGL_TEXTURES];
    i32 numTextures;

    u16 numSpritesDrawnThisFrame;

    u32 spriteBatchSeenTextures[MAX_NUM_OPENGL_TEXTURES];
    u32 spriteBatchNumSeenTextures;
    u16 spriteBatchNumSpritesDrawn;
    opengl_vertex *batchStartOriginal;
    opengl_vertex *batchStart;
    opengl_vertex *currentVertex;
} opengl_renderer;


void openGLSetAttributes(void);
void openGLRendererInit(opengl_renderer *renderer, SDL_Window *window);
void openGLRender(opengl_renderer *renderer, mem_arena *renderMemory);

#endif
