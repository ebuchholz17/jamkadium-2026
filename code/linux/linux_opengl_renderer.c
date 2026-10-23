#include <stdio.h>
#include <stdlib.h>

#include "linux_opengl_renderer.h"

#include "../game/gng_platform.h"
#include "shaders/sprite_shader.c"
#include "shaders/basic_3d_shader.c"

#define MAX_NUM_SPRITES 10000

void checkShaderCompilationSuccess (GLuint shader) {
    GLint compilationSucceeded;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compilationSucceeded);
    if (compilationSucceeded != GL_TRUE) {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar *infoLog = malloc(sizeof(GLchar) * infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, 0, infoLog);

        printf("%s\n", infoLog);

        ASSERT(false);
    }
}

shader_program compileAndLinkShader (const GLchar * const vertexShaderSource, 
                                     const GLchar * const fragmentShaderSource) 
{
    shader_program result = {0};

    result.vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(result.vertexShader, 1, &vertexShaderSource, 0);
    glCompileShader(result.vertexShader);

    checkShaderCompilationSuccess(result.vertexShader);

    result.fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(result.fragmentShader, 1, &fragmentShaderSource, 0);
    glCompileShader(result.fragmentShader);

    checkShaderCompilationSuccess(result.fragmentShader);

    result.program = glCreateProgram();
    glAttachShader(result.program, result.vertexShader);
    glAttachShader(result.program, result.fragmentShader);
    glLinkProgram(result.program);

    GLint linkSucceeded;
    glGetProgramiv(result.program, GL_LINK_STATUS, &linkSucceeded);
    if (linkSucceeded != GL_TRUE) {
        GLint infoLogLength;
        glGetProgramiv(result.program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar *infoLog = malloc(sizeof(GLchar) * infoLogLength);
        glGetProgramInfoLog(result.program, infoLogLength, 0, infoLog);

        printf("%s\n", infoLog);

        ASSERT(false);
    }

    return result;
}

void openGLRendererSetAttributes (void) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
}

GLuint createStaticBuffer (f32 *data, i32 size) {
    GLuint result = 0;
    glCreateBuffers(1, &result);
    glBindBuffer(GL_ARRAY_BUFFER, result);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    return result;
}

GLuint createIndexBuffer (u16 *data, i32 size) {
    GLuint result = 0;
    glCreateBuffers(1, &result);
    glBindBuffer(GL_ARRAY_BUFFER, result);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    return result;
}

void openGLRendererInit(opengl_renderer *renderer, SDL_Window *window) {
    renderer->gl = SDL_GL_CreateContext(window);
    ASSERT(renderer->gl);

    renderer->window = window;

    glewInit();

    renderer->spriteShader = compileAndLinkShader(spriteVertexSource, spriteFragmentSource);

    glCreateBuffers(1, &renderer->spriteVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->spriteVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * MAX_NUM_SPRITES * sizeof(opengl_vertex), NULL, GL_STATIC_DRAW);
    // NOTE: 40=sizeof(<sprite struct>), probably

    u16 indices[MAX_NUM_SPRITES * 6];
    for (i32 spriteIndex = 0; spriteIndex < MAX_NUM_SPRITES; spriteIndex++) {
        indices[spriteIndex * 6 + 0] = (spriteIndex * 4) + 0;
        indices[spriteIndex * 6 + 1] = (spriteIndex * 4) + 2;
        indices[spriteIndex * 6 + 2] = (spriteIndex * 4) + 1;
        indices[spriteIndex * 6 + 3] = (spriteIndex * 4) + 1;
        indices[spriteIndex * 6 + 4] = (spriteIndex * 4) + 2;
        indices[spriteIndex * 6 + 5] = (spriteIndex * 4) + 3;
    }

    renderer->spriteIndexBuffer = createIndexBuffer(indices, 6 * MAX_NUM_SPRITES * sizeof(u16));

    // 3D
    renderer->basic3DShader = compileAndLinkShader(basic3DVertexSource, basic3DFragmentSource);

    glCreateBuffers(1, &renderer->basic3DVertexBuffer);
    f32 basic3DData[] = {
        -1.0, 1.0, 0.0,
        0.0, 0.0,
        //0.0, 0.0, 1.0,

        1.0, 1.0, 0.0,
        1.0, 0.0,
        //0.0, 0.0, 1.0,

        -1.0, -1.0, 0.0,
        0.0, 1.0,
        //0.0, 0.0, 1.0,

        1.0, -1.0, 0.0,
        1.0, 1.0,
        //0.0, 0.0, 1.0,
    };

    glBindBuffer(GL_ARRAY_BUFFER, renderer->basic3DVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(basic3DData), basic3DData, GL_STATIC_DRAW);

    {
        u16 indices[6];
        indices[0] = 0;
        indices[1] = 2;
        indices[2] = 1;
        indices[3] = 1;
        indices[4] = 2;
        indices[5] = 3;
        renderer->basic3DIndexBuffer = createIndexBuffer(indices, 6 * sizeof(u16));
    }

    SDL_GL_SetSwapInterval(1);
}


void resizeOpenGL (opengl_renderer *renderer, i32 windowWidth, i32 windowHeight) {
    renderer->windowWidth = windowWidth;
    renderer->windowHeight = windowHeight;
}



void openGLOnRenderStart (opengl_renderer *renderer) {
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, renderer->windowWidth, renderer->windowHeight);
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glClearColor(0.00, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void openGLLoadTexture (opengl_renderer *renderer, i32 id, i32 width, i32 height, u8 *pixelData) {
    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    ASSERT(renderer->numTextures == id);
    renderer->textures[renderer->numTextures] = texture;
    renderer->textureSizes[renderer->numTextures] = (texture_size){
        .width = width,
        .height = height
    };
    renderer->numTextures++;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void openGLOnRenderSpritesStart (opengl_renderer *renderer) {
    glViewport(0, 0, renderer->windowWidth, renderer->windowHeight);

    f32 gameRatio = 356.0f / 200.0f;
    f32 windowRatio = (f32)renderer->windowWidth / (f32)renderer->windowHeight;

    glEnable(GL_SCISSOR_TEST);
    if (gameRatio > windowRatio) {
        f32 viewportHeight = renderer->windowWidth / gameRatio;
        f32 offset = (renderer->windowHeight - viewportHeight) / 2;
        glScissor(0, offset, renderer->windowWidth, viewportHeight);
    }
    else {
        f32 viewportWidth = renderer->windowHeight * gameRatio;
        f32 offset = (renderer->windowWidth - viewportWidth) / 2;
        glScissor(offset, 0, viewportWidth, renderer->windowHeight);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    //gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void openGLOnRender3DStart (opengl_renderer *renderer) {
    // NOTE(ebuchholz): hardcoded game size
    f32 gameRatio = 356.0 / 200.0;
    f32 windowRatio = (f32)renderer->windowWidth / (f32)renderer->windowHeight;

    if (gameRatio > windowRatio) {
        f32 viewportHeight = renderer->windowWidth / gameRatio;
        f32 offset = (renderer->windowHeight - viewportHeight) / 2;
        glViewport(0, offset, renderer->windowWidth, viewportHeight);
        glScissor(0, offset, renderer->windowWidth, viewportHeight);
    }
    else {
        f32 viewportWidth = renderer->windowHeight * gameRatio;
        f32 offset = (renderer->windowWidth - viewportWidth) / 2;
        glViewport(offset, 0, viewportWidth, renderer->windowHeight);
        glScissor(offset, 0, viewportWidth, renderer->windowHeight);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_SCISSOR_TEST);
    glClearColor(0.00, 0.294, 0.839, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void openGLSpriteBatchStart (opengl_renderer *renderer) {
    GLuint program = renderer->spriteShader.program;
    glUseProgram(program);

    GLint screenWidthLoc = glGetUniformLocation(program, "screenDims.width");
    glUniform1f(screenWidthLoc, renderer->windowWidth);
    GLint screenHeightLoc = glGetUniformLocation(program, "screenDims.height");
    glUniform1f(screenHeightLoc, renderer->windowHeight);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->spriteVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->spriteIndexBuffer);

    u32 stride = sizeof(opengl_vertex);
    GLint positionLocation = glGetAttribLocation(program, "pos");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, false, stride, 0);

    GLint texCoordLocation = glGetAttribLocation(program, "texCoords");
    glEnableVertexAttribArray(texCoordLocation);
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, false, stride, (u8 *)12);

    GLint colorCoordLocation = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorCoordLocation);
    glVertexAttribPointer(colorCoordLocation, 4, GL_FLOAT, false, stride, (u8 *)20);

    GLint texIndexLocation = glGetAttribLocation(program, "textureID");
    glEnableVertexAttribArray(texIndexLocation);
    glVertexAttribPointer(texIndexLocation, 1, GL_FLOAT, false, stride, (u8 *)36);
}

void openGLSpriteBatchFlush (opengl_renderer *renderer, f32 *spriteData, i32 numSpritesBatched, 
                             i32 vertexSize, i32 numSpritesDrawn, i32 *textureIDs, i32 numTextures) 
{
    GLuint program = renderer->spriteShader.program;
    i32 dataSize = numSpritesBatched * vertexSize * 4;
    i32 bufferOffset = numSpritesDrawn * vertexSize * 4;

    glBufferSubData(GL_ARRAY_BUFFER, bufferOffset, dataSize, spriteData);

    for (i32 i = 0; i < numTextures; i++) {
        GLuint texture = renderer->textures[textureIDs[i]];
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        char textureName[16];
        sprintf(textureName, "texture%d", i);
        GLint textureLocation = glGetUniformLocation(program, textureName);
        glUniform1i(textureLocation, i);

        char textureSizeName[16];
        sprintf(textureSizeName, "texture%dSize", i);
        GLint textureSizeLocation = glGetUniformLocation(program, textureSizeName);
        glUniform2fv(textureSizeLocation, 1, &renderer->textureSizes[textureIDs[i]].width);
    }

    glDrawElements(GL_TRIANGLES, numSpritesBatched * 6, GL_UNSIGNED_SHORT, (u8 *)(numSpritesDrawn * 6));
}

void openGLBasic3D (opengl_renderer *renderer, f32 *modelMatrix, f32 *viewMatrix, f32 *projMatrix, u32 textureID) {
    GLuint program = renderer->basic3DShader.program;
    glUseProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->basic3DVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->basic3DIndexBuffer);

    GLuint positionLocation = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, false, 20, 0);

    GLuint texCoordLocation = glGetAttribLocation(program, "texCoords");
    glEnableVertexAttribArray(texCoordLocation);
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, false, 20, (u8 *)12);

    //GLuint normalLocation = glGetAttribLocation(program, "normal");
    //glEnableVertexAttribArray(normalLocation);
    //glVertexAttribPointer(normalLocation, 3, GL_FLOAT, false, 32, 20);

    GLuint modelMatrixLocation = glGetUniformLocation(program, "modelMatrix");
    glUniformMatrix4fv(modelMatrixLocation, 1, true, modelMatrix);

    GLuint viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, true, viewMatrix);

    GLuint projMatrixLocation = glGetUniformLocation(program, "projMatrix");
    glUniformMatrix4fv(projMatrixLocation, 1, true, projMatrix);

    GLuint texture = renderer->textures[textureID];
    GLuint textureLocation = glGetUniformLocation(program, "texture");
    glUniform1i(textureLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

void openGLSpriteBatchOnStart (opengl_renderer *renderer) {
    renderer->spriteBatchNumSpritesDrawn = 0;
    renderer->spriteBatchNumSeenTextures = 0;
    for (u32 textureIndex = 0; textureIndex < MAX_NUM_OPENGL_TEXTURES; textureIndex++) {
        renderer->spriteBatchSeenTextures[textureIndex] = UNUSED_TEXTURE_VAL;
    }
}

void openGLFlushSprites (opengl_renderer *renderer) {
    u32 textureIDs[8];
    u32 numTextureIDsToBind = 0;
    for (u32 textureIndex = 0; textureIndex < MAX_NUM_OPENGL_TEXTURES; textureIndex++) {
        u32 shaderTextureIndex = renderer->spriteBatchSeenTextures[textureIndex];
        if (shaderTextureIndex != UNUSED_TEXTURE_VAL) {
            textureIDs[shaderTextureIndex] = textureIndex;
            numTextureIDsToBind++;
        }
    }
    openGLSpriteBatchFlush(renderer, (void *)renderer->batchStart, renderer->spriteBatchNumSpritesDrawn, sizeof(opengl_vertex), 
                          renderer->numSpritesDrawnThisFrame, textureIDs, numTextureIDsToBind);
    renderer->batchStart = renderer->currentVertex;
}


void openGLRenderGame (opengl_renderer *renderer, mem_arena *renderMemory) {

    openGLOnRenderStart(renderer);

    renderer->numSpritesDrawnThisFrame = 0;

    static f32 triT = 0.0f;
    triT += 1.0f / 144.0f;

    void *renderCursor = renderMemory->base;
    while (renderCursor < renderMemory->current) {
        render_cmd_header *header = (render_cmd_header *)renderCursor;
        renderCursor = (u8 *)renderCursor + sizeof(render_cmd_header);
        switch (header->type) {
            default: {
                ASSERT(false);
            } break;
            case RENDER_CMD_TYPE_LOAD_TEXTURE: {
                render_cmd_load_texture *cmd = (render_cmd_load_texture *)renderCursor;
                renderCursor = (u8 *)renderCursor + sizeof(render_cmd_load_texture);

                ASSERT(renderer->numTextures < MAX_NUM_OPENGL_TEXTURES);

                i32 textureID = cmd->id;

                openGLLoadTexture(renderer, textureID, cmd->width, cmd->height, cmd->pixels);
            } break;
            case RENDER_CMD_TYPE_SPRITE_BATCH_START: {
                // NOTE: no data in render cmd
                openGLOnRenderSpritesStart(renderer);
                openGLSpriteBatchOnStart(renderer);
                openGLSpriteBatchStart(renderer);
            } break;
            case RENDER_CMD_TYPE_SPRITE_BATCH_DRAW: {
                render_cmd_sprite_batch_draw *cmd = (render_cmd_sprite_batch_draw *)renderCursor;
                renderCursor = (u8 *)renderCursor + sizeof(render_cmd_sprite_batch_draw);

                ASSERT(cmd->numSprites + renderer->numSpritesDrawnThisFrame <= MAX_NUM_SPRITES);

                // TODO: alloc big working space to copy vertices into the right format
                // use opengl_vertex, then send pointer to web side
                if (renderer->batchStart == 0) {
                    renderer->batchStart = (opengl_vertex *)malloc(4 * MAX_NUM_SPRITES * sizeof(opengl_vertex));
                    renderer->batchStartOriginal = renderer->batchStart;
                }
                else {
                    renderer->batchStart = renderer->batchStartOriginal;
                }

                renderer->currentVertex = renderer->batchStart;
                for (u32 spriteIndex = 0; spriteIndex < cmd->numSprites; spriteIndex++) {
                    render_cmd_sprite_data *sprite = &cmd->sprites[spriteIndex];

                    ASSERT(sprite->textureID < MAX_NUM_OPENGL_TEXTURES);

                    u32 shaderTextureIndex = UNUSED_TEXTURE_VAL;
                    if (renderer->spriteBatchSeenTextures[sprite->textureID] == UNUSED_TEXTURE_VAL) {
                        if (renderer->spriteBatchNumSeenTextures >= 8) {
                            openGLFlushSprites(renderer);
                            openGLSpriteBatchOnStart(renderer);
                        }
                        renderer->spriteBatchSeenTextures[sprite->textureID] = renderer->spriteBatchNumSeenTextures;
                        shaderTextureIndex = renderer->spriteBatchNumSeenTextures;
                        renderer->spriteBatchNumSeenTextures++;
                    }
                    else {
                        shaderTextureIndex = renderer->spriteBatchSeenTextures[sprite->textureID];
                    }

                    for (u32 vIndex = 0; vIndex < 4; vIndex++) {
                        renderer->currentVertex[vIndex].pos[0] = sprite->positions[vIndex * 3 + 0];
                        renderer->currentVertex[vIndex].pos[1] = sprite->positions[vIndex * 3 + 1];
                        renderer->currentVertex[vIndex].pos[2] = sprite->positions[vIndex * 3 + 2];
                        renderer->currentVertex[vIndex].texCoords[0] = sprite->texCoords[vIndex * 2 + 0];
                        renderer->currentVertex[vIndex].texCoords[1] = sprite->texCoords[vIndex * 2 + 1];
                        renderer->currentVertex[vIndex].color[0] = sprite->colors[vIndex * 4 + 0];
                        renderer->currentVertex[vIndex].color[1] = sprite->colors[vIndex * 4 + 1];
                        renderer->currentVertex[vIndex].color[2] = sprite->colors[vIndex * 4 + 2];
                        renderer->currentVertex[vIndex].color[3] = sprite->colors[vIndex * 4 + 3];
                        renderer->currentVertex[vIndex].textureID = (f32)shaderTextureIndex;
                    }

                    renderer->currentVertex += 4;

                    renderer->spriteBatchNumSpritesDrawn++;
                }

                // advance past the list of sprites
                renderCursor = (u8 *)renderCursor + sizeof(render_cmd_sprite_data) * cmd->numSprites;
            } break;
            case RENDER_CMD_TYPE_SPRITE_BATCH_END: {
                // NOTE: no data in render cmd

                // flush leftover sprites
                openGLFlushSprites(renderer);
            } break;
            case RENDER_CMD_TYPE_BASIC_3D: {
                openGLOnRender3DStart(renderer);
                render_cmd_basic_3d *cmd = (render_cmd_basic_3d *)renderCursor;
                renderCursor = (u8 *)renderCursor + sizeof(render_cmd_basic_3d);
                openGLBasic3D(renderer, cmd->model, cmd->view, cmd->proj, cmd->textureID);
            } break;
        }
    }

    SDL_GL_SwapWindow(renderer->window);
}
