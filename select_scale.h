
#ifndef SELECT_SCALE
#define SELECT_SCALE


#include "fracfast/types.h"

#include <SDL2/SDL.h>


void largestARcenter(SDL_Rect& zoomTo, SDL_Rect& selectionBox, const Selection* const selection, const Resolution& res);
void smallestARcenter(SDL_Rect& zoomTo, SDL_Rect& selectionBox, const Selection* const selection, const Resolution& res);
void largestARtopleft(SDL_Rect& zoomTo, SDL_Rect& selectionBox, const Selection* const selection, const Resolution& res);


#endif  // SELECT_SCALE
