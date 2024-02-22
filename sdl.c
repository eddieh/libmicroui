/*
 * Copyright (c) 2020 rxi
 * Copyright (c) 2024 Eddie Hillenbrand
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <assert.h>
#include "sdl.h"
#include "res.h"

#define BUFFER_SIZE 16384

static GLfloat tex_buf[BUFFER_SIZE *  8];
static GLfloat vert_buf[BUFFER_SIZE *  8];
static GLubyte color_buf[BUFFER_SIZE * 16];
static GLuint index_buf[BUFFER_SIZE *  6];

static int width = 800;
static int height = 600;
static int buf_idx;

static SDL_Window *window;

static const char button_map[256] = {
    [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
    [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
    [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
    [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
    [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
    [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
    [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
    [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
    [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
    [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
    [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};

void
sdl_init(int w, int h)
{
    width = w;
    height = h;

    SDL_Init(SDL_INIT_EVERYTHING);

    /* init SDL window */
    window = SDL_CreateWindow(
        NULL, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_OPENGL);
    SDL_GL_CreateContext(window);

    /* init gl */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    /* init texture */
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, ATLAS_WIDTH, ATLAS_HEIGHT, 0,
        GL_ALPHA, GL_UNSIGNED_BYTE, atlas_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    assert(glGetError() == 0);
}


static void flush(void) {
    if (buf_idx == 0) { return; }

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTexCoordPointer(2, GL_FLOAT, 0, tex_buf);
    glVertexPointer(2, GL_FLOAT, 0, vert_buf);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, color_buf);
    glDrawElements(GL_TRIANGLES, buf_idx * 6, GL_UNSIGNED_INT, index_buf);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    buf_idx = 0;
}


static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
    if (buf_idx == BUFFER_SIZE) { flush(); }

    int texvert_idx = buf_idx *  8;
    int   color_idx = buf_idx * 16;
    int element_idx = buf_idx *  4;
    int   index_idx = buf_idx *  6;
    buf_idx++;

    /* update texture buffer */
    float x = src.x / (float) ATLAS_WIDTH;
    float y = src.y / (float) ATLAS_HEIGHT;
    float w = src.w / (float) ATLAS_WIDTH;
    float h = src.h / (float) ATLAS_HEIGHT;
    tex_buf[texvert_idx + 0] = x;
    tex_buf[texvert_idx + 1] = y;
    tex_buf[texvert_idx + 2] = x + w;
    tex_buf[texvert_idx + 3] = y;
    tex_buf[texvert_idx + 4] = x;
    tex_buf[texvert_idx + 5] = y + h;
    tex_buf[texvert_idx + 6] = x + w;
    tex_buf[texvert_idx + 7] = y + h;

    /* update vertex buffer */
    vert_buf[texvert_idx + 0] = dst.x;
    vert_buf[texvert_idx + 1] = dst.y;
    vert_buf[texvert_idx + 2] = dst.x + dst.w;
    vert_buf[texvert_idx + 3] = dst.y;
    vert_buf[texvert_idx + 4] = dst.x;
    vert_buf[texvert_idx + 5] = dst.y + dst.h;
    vert_buf[texvert_idx + 6] = dst.x + dst.w;
    vert_buf[texvert_idx + 7] = dst.y + dst.h;

    /* update color buffer */
    memcpy(color_buf + color_idx +  0, &color, 4);
    memcpy(color_buf + color_idx +  4, &color, 4);
    memcpy(color_buf + color_idx +  8, &color, 4);
    memcpy(color_buf + color_idx + 12, &color, 4);

    /* update index buffer */
    index_buf[index_idx + 0] = element_idx + 0;
    index_buf[index_idx + 1] = element_idx + 1;
    index_buf[index_idx + 2] = element_idx + 2;
    index_buf[index_idx + 3] = element_idx + 2;
    index_buf[index_idx + 4] = element_idx + 3;
    index_buf[index_idx + 5] = element_idx + 1;
}


void sdl_draw_rect(mu_Rect rect, mu_Color color) {
    push_quad(rect, atlas[ATLAS_WHITE], color);
}


void sdl_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
    mu_Rect dst = { pos.x, pos.y, 0, 0 };
    for (const char *p = text; *p; p++) {
        if ((*p & 0xc0) == 0x80) { continue; }
        int chr = mu_min((unsigned char) *p, 127);
        mu_Rect src = atlas[ATLAS_FONT + chr];
        dst.w = src.w;
        dst.h = src.h;
        push_quad(dst, src, color);
        dst.x += dst.w;
    }
}


void sdl_draw_icon(int id, mu_Rect rect, mu_Color color) {
    mu_Rect src = atlas[id];
    int x = rect.x + (rect.w - src.w) / 2;
    int y = rect.y + (rect.h - src.h) / 2;
    push_quad(mu_rect(x, y, src.w, src.h), src, color);
}


int sdl_get_text_width(const char *text, int len) {
    int res = 0;
    for (const char *p = text; *p && len--; p++) {
        if ((*p & 0xc0) == 0x80) { continue; }
        int chr = mu_min((unsigned char) *p, 127);
        res += atlas[ATLAS_FONT + chr].w;
    }
    return res;
}


int sdl_get_text_height(void) {
    return 18;
}


void sdl_set_clip_rect(mu_Rect rect) {
    flush();
    glScissor(rect.x, height - (rect.y + rect.h), rect.w, rect.h);
}

void
sdl_handle_events(mu_Context *ctx)
{
    SDL_Event e;
    int b, c;

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            exit(EXIT_SUCCESS);
            break;
        case SDL_MOUSEMOTION:
            mu_input_mousemove(ctx, e.motion.x, e.motion.y);
            break;
        case SDL_MOUSEWHEEL:
            mu_input_scroll(ctx, 0, e.wheel.y * -30);
            break;
        case SDL_TEXTINPUT:
            mu_input_text(ctx, e.text.text);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            b = button_map[e.button.button & 0xff];
            if (b && e.type == SDL_MOUSEBUTTONDOWN) {
                mu_input_mousedown(ctx, e.button.x, e.button.y, b);
            }
            if (b && e.type == SDL_MOUSEBUTTONUP) {
                mu_input_mouseup(ctx, e.button.x, e.button.y, b);
            }
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            c = key_map[e.key.keysym.sym & 0xff];
            if (c && e.type == SDL_KEYDOWN) {
                mu_input_keydown(ctx, c);
            }
            if (c && e.type == SDL_KEYUP) {
                mu_input_keyup(ctx, c);
            }
            break;
        }
    }
}

void
sdl_run_commands(mu_Context *ctx)
{
    mu_Command *cmd = NULL;
    while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
        case MU_COMMAND_TEXT:
            sdl_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color);
            break;
        case MU_COMMAND_RECT:
            sdl_draw_rect(cmd->rect.rect, cmd->rect.color);
            break;
        case MU_COMMAND_ICON:
            sdl_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
            break;
        case MU_COMMAND_CLIP:
            sdl_set_clip_rect(cmd->clip.rect);
            break;
        }
    }
}

void
sdl_clear(mu_Color clr) {
    flush();
    glClearColor(clr.r / 255., clr.g / 255., clr.b / 255., clr.a / 255.);
    glClear(GL_COLOR_BUFFER_BIT);
}

void
sdl_present(void) {
    flush();
    SDL_GL_SwapWindow(window);
}
