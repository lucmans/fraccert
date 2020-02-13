
#include "select_scale.h"


void smallestARcenter(SDL_Rect& zoomTo, SDL_Rect& selectionBox, const Selection* const selection, const Resolution& res) {
    selectionBox = {(int)selection->xLast, (int)selection->yLast,
                    abs((int)(selection->xLast - selection->xInit) * 2),
                    abs((int)(selection->yLast - selection->yInit) * 2)};
    if(selection->xLast > selection->xInit)
        selectionBox.x = selection->xLast - selectionBox.w;
    if(selection->yLast > selection->yInit)
        selectionBox.y = selection->yLast - selectionBox.h;
    zoomTo = selectionBox;

    const double screenAR = (double)res.w / (double)res.h,
                 selectAR = (double)selectionBox.w / (double)selectionBox.h;
    if(screenAR > selectAR) {  // If screen is wider than selection
        // Decrease height to fit AR
        zoomTo.h = zoomTo.w / screenAR;
        zoomTo.y = selection->yInit - (zoomTo.h / 2);
    }
    else {  // If selection is wider than screen
        // Decrease width to fit AR
        zoomTo.w = zoomTo.h * screenAR;
        zoomTo.x = selection->xInit - (zoomTo.w / 2);
    }
}

void largestARtopleft(SDL_Rect& zoomTo, SDL_Rect& selectionBox, const Selection* const selection, const Resolution& res) {
    if(selection->xLast > selection->xInit) {
        selectionBox.x = selection->xInit;
        selectionBox.w = selection->xLast - selection->xInit;
    }
    else {
        selectionBox.x = selection->xLast;
        selectionBox.w = selection->xInit - selection->xLast;
    }
    if(selection->yLast > selection->yInit) {
        selectionBox.y = selection->yInit;
        selectionBox.h = selection->yLast - selection->yInit;
    }
    else {
        selectionBox.y = selection->yLast;
        selectionBox.h = selection->yInit - selection->yLast;
    }
    zoomTo = selectionBox;

    const double screenAR = (double)res.w / (double)res.h,
                 selectAR = (double)selectionBox.w / (double)selectionBox.h;
    if(screenAR > selectAR) {  // If screen is wider than selection
        // Increase width to fit AR
        zoomTo.w = zoomTo.h * screenAR;
        zoomTo.x -= (zoomTo.w - selectionBox.w) / 2;
    }
    else {  // If selection is wider than screen
        // Increase height to fit AR
        zoomTo.h = zoomTo.w / screenAR;
        zoomTo.y -= (zoomTo.h - selectionBox.h) / 2;
    }
}
