#ifndef UTILS_H
#define UTILS_H

#include <SDL/SDL.h>
#include "SDL/SDL_ttf.h"
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
class Fl_Widget;

void runGUI(float* alpha, bool* exit);
void fill_circle(SDL_Surface *surface, int cx, int cy, int radius, Uint32 pixel);
void DrawGraphics(SDL_Surface *screen, int state, SDL_Rect* redRect);
void DrawGraphics_Augmented(SDL_Surface *screen, int state, float pasive_counter, SDL_Rect* redRect);
#endif // UTILS_H
