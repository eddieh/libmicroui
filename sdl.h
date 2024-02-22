#ifndef SDL_H
#define SDL_H

#include <microui.h>

void sdl_init(int w, int h);
void sdl_draw_rect(mu_Rect rect, mu_Color color);
void sdl_draw_text(const char *text, mu_Vec2 pos, mu_Color color);
void sdl_draw_icon(int id, mu_Rect rect, mu_Color color);
void sdl_set_clip_rect(mu_Rect rect);
void sdl_clear(mu_Color color);
void sdl_handle_events(mu_Context *ctx);
void sdl_run_commands(mu_Context *ctx);
void sdl_present(void);

int sdl_get_text_width(const char *text, int len);
int sdl_get_text_height(void);

#endif
