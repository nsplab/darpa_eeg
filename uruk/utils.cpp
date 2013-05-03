#include "utils.h"

#include <Fl/Fl.H>
#include <Fl/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>

#include <SDL/SDL_gfxPrimitives.h>

#include <cmath>
#include <iostream>

using namespace std;

//
static void close_cb(Fl_Widget*, void *w) {
  bool* exit = (bool*)w;
  (*exit) = true;
  cout<<"exit        "<<endl;
}

void runGUI(float* alpha, bool* exit) {
    char buffer[20]="0.9995";
    char buffer2[20]="0.1";
    Fl_Window* w = new Fl_Window(1800, 100, 330, 190, "Uruk - NSPLab");
    Fl_Button ok(110, 130, 100, 35, "Update");
    Fl_Input input(60, 40, 250, 25,"Alpha:");
    input.value(buffer);
    w->end();
    w->show();
    w->user_data(exit);
    w->callback((Fl_Callback*)close_cb, exit);

    while (!(*exit)) {
      Fl::wait();
      Fl_Widget *o;
      while (o = Fl::readqueue()) {
        if (o == &ok) {
            strcpy(buffer, input.value());
            (*alpha) = ::atof(buffer);
        }
      }
    }
}

void fill_circle(SDL_Surface *surface, int cx, int cy, int radius, Uint32 pixel) {
    static const int BPP = 4;

    double r = (double)radius;

    for (double dy = 1; dy <= r; dy += 1.0)
    {
        double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
        int x = cx - dx;

        // Grab a pointer to the left-most pixel for each half of the circle
        Uint8 *target_pixel_a = (Uint8 *)surface->pixels + ((int)(cy + r - dy)) * surface->pitch + x * BPP;
        Uint8 *target_pixel_b = (Uint8 *)surface->pixels + ((int)(cy - r + dy)) * surface->pitch + x * BPP;

        for (; x <= cx + dx; x++)
        {
            *(Uint32 *)target_pixel_a = pixel;
            *(Uint32 *)target_pixel_b = pixel;
            target_pixel_a += BPP;
            target_pixel_b += BPP;
        }
    }
}

void DrawGraphics(SDL_Surface *screen, int state, SDL_Rect* redRect) {
    SDL_FillRect(screen , NULL , 0x221122);

/*    SDL_Rect workRect;
    workRect.y = 90;
    workRect.x = 365;
    workRect.w = 70;
    workRect.h = 310;

    SDL_Rect workRect2;
    workRect2.y = 95;
    workRect2.x = 370;
    workRect2.w = 60;
    workRect2.h = 300;

    SDL_Rect hRect;
    hRect.y = 245;
    hRect.x = 350;
    hRect.w = 100;
    hRect.h = 2;*/


    //SDL_FillRect(screen , &workRect , SDL_MapRGB(screen->format , 200 , 200 , 200 ) );
    //SDL_FillRect(screen , &workRect2 , SDL_MapRGB(screen->format , 34 , 17 , 34 ) );
    //SDL_FillRect(screen , &hRect , SDL_MapRGB(screen->format , 34 , 100 , 34 ) );

    switch(state) {
    case 0:
        roundedRectangleColor(screen, 645, 400, 1275, 600, 20, SDL_MapRGB(screen->format , 200 , 250 , 250 ));
        rectangleColor(screen, 15, 400, 645, 600, SDL_MapRGB(screen->format , 200 , 200 , 200 ));
        rectangleColor(screen, 1275, 400, 1905, 600, SDL_MapRGB(screen->format , 200 , 200 , 200 ));
        break;
    case -1:
        roundedRectangleColor(screen, 15, 400, 645, 600, 20, SDL_MapRGB(screen->format , 200 , 250 , 250 ));
        rectangleColor(screen, 645, 400, 1275, 600, SDL_MapRGB(screen->format , 200 , 200 , 200 ));
        rectangleColor(screen, 1275, 400, 1905, 600, SDL_MapRGB(screen->format , 200 , 200 , 200 ));
        break;
    case 1:
        roundedRectangleColor(screen, 1275, 400, 1905, 600, 20, SDL_MapRGB(screen->format , 200 , 250 , 250 ));
        rectangleColor(screen, 645, 400, 1275, 600, SDL_MapRGB(screen->format , 200 , 200 , 200 ));
        rectangleColor(screen, 15, 400, 645, 600, SDL_MapRGB(screen->format , 200 , 200 , 200 ));
        break;
    }

    SDL_FillRect(screen , redRect , SDL_MapRGB(screen->format , 200 , 20 , 200 ) );

    SDL_Flip(screen);
}
