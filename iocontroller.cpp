
#include "iocontroller.h"

#include <iostream>


// If 1 is returned, the event is added to the event queue; otherwise it is dropped.
int eventFilter(void* userdata, SDL_Event* const e) {
    // Interrupt event
    if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE) {
        SDL_FlushEvent(SDL_MOUSEWHEEL);
        SDL_FlushEvent(SDL_KEYDOWN);
        SDL_FlushEvent(SDL_KEYUP);
        SDL_FlushEvent(SDL_MOUSEMOTION);

        return 0;
    }

    // Detect modifiers
    if(e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_LCTRL)
        ((IOController*)userdata)->ctrlHeldDown = true;
    if(e->type == SDL_KEYUP && e->key.keysym.sym == SDLK_LCTRL)
        ((IOController*)userdata)->ctrlHeldDown = false;

    // Dragging over Julia set without ctrl held down
    if(e->type == SDL_MOUSEMOTION && e->motion.state == SDL_BUTTON_LMASK && !((IOController*)userdata)->ctrlHeldDown) {
        SDL_FlushEvent(SDL_MOUSEWHEEL);
        SDL_FlushEvent(SDL_KEYDOWN);
        SDL_FlushEvent(SDL_KEYUP);
        SDL_FlushEvent(SDL_MOUSEMOTION);
    }

    // Dragging right mouse buttong without ctrl held down
    if(e->type == SDL_MOUSEMOTION && e->motion.state == SDL_BUTTON_RMASK && !((IOController*)userdata)->ctrlHeldDown) {
        SDL_FlushEvent(SDL_MOUSEWHEEL);
        SDL_FlushEvent(SDL_KEYDOWN);
        SDL_FlushEvent(SDL_KEYUP);
        SDL_FlushEvent(SDL_MOUSEMOTION);
    }

    return 1;

    // Prevent warning
    userdata = userdata;
}


IOController::IOController(Program* const p, Program* const p2, const uint32_t mainWinID, const uint32_t juliaWinID) : program(p), juliaWindow(p2), mainWindowID(mainWinID), juliaWindowID(juliaWinID) {
    // EventFilter checks event before adding to event queue
    SDL_SetEventFilter(eventFilter, this);

    console = new Console(p, p2);
    if(!console->startIOThread())
        printf("Couldn't start console thread. Console unavailable.%s\n", SDL_GetError());

    ctrlHeldDown = false;

    // Place mouse in the middle of the screen at start-up
    // When there is no mouse present, this will also make the default zoom location the middle of the screen
    SDL_Event fakeMouse;
    Resolution res;
    p->getResolution(res);

    fakeMouse.type = SDL_MOUSEMOTION;
    fakeMouse.motion.x = res.w / 2;
    fakeMouse.motion.y = res.h / 2;
    fakeMouse.motion.windowID = 0;
    
    SDL_PushEvent(&fakeMouse);

}

IOController::~IOController() {
    delete console;
}


void IOController::mainWindowKeyEvent(const SDL_Keycode& sym, bool& quit) {
    switch(sym) {
        case SDLK_q:            quit = true;                                break;
        case SDLK_EQUALS:       program->scale(1);                          break;
        case SDLK_MINUS:        program->scale(-1);                         break;
        case SDLK_t:            program->nextFractal();                     break;
        case SDLK_h:            program->home();                            break;
        case SDLK_r:            program->refresh();                         break;
        case SDLK_SPACE:        program->redraw();                          break;
        case SDLK_i:            program->translateJuliaParameter(0, 1);     break;
        case SDLK_k:            program->translateJuliaParameter(0, -1);    break;
        case SDLK_j:            program->translateJuliaParameter(-1, 0);    break;
        case SDLK_l:            program->translateJuliaParameter(1, 0);     break;
        case SDLK_LEFTBRACKET:  program->changenMax(-1);                    break;
        case SDLK_RIGHTBRACKET: program->changenMax(1);                     break;
        case SDLK_UP:           program->translate(0, 1);                   break;
        case SDLK_w:            program->translate(0, 1);                   break;
        case SDLK_DOWN:         program->translate(0, -1);                  break;
        case SDLK_s:            program->translate(0, -1);                  break;
        case SDLK_LEFT:         program->translate(-1, 0);                  break;
        case SDLK_a:            program->translate(-1, 0);                  break;
        case SDLK_RIGHT:        program->translate(1, 0);                   break;
        case SDLK_d:            program->translate(1, 0);                   break;
        case SDLK_p:            program->deepen();                          break;
        case SDLK_o:            program->unDeepen();                        break;
        case SDLK_g:            program->nextColoring();                    break;
        case SDLK_y:            program->toggleSymmetry();                  break;
    }
}

void IOController::mainWindowScrollEvent(const SDL_MouseWheelEvent& eScroll) {
    if(eScroll.y > 0)
        program->scale(1);
    else if(eScroll.y < 0)  // Explicit check, because y=0 when scrolling sideways
        program->scale(-1);
}

void IOController::mainWindowWindowEvent(const SDL_WindowEvent& eWindow, bool& quit) {
    switch(eWindow.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            program->resize({(unsigned int)eWindow.data1, (unsigned int)eWindow.data2});
            break;

        case SDL_WINDOWEVENT_CLOSE:
            quit = true;
            break;

        case SDL_WINDOWEVENT_MOVED:
            program->refresh();
            juliaWindow->refresh();
            break;
    }
}

void IOController::mainWindowClick(const SDL_MouseButtonEvent& eClick) {
    switch(eClick.button) {
        case SDL_BUTTON_LEFT:
            program->click(eClick.x, eClick.y);
            break;

        case SDL_BUTTON_RIGHT:
            program->orbit(eClick.x, eClick.y);
            break;
    }
}

// void IOController::mainWindowUnclick(const SDL_MouseButtonEvent& eClick) {
// }

void IOController::mainWindowMouseMotion(const SDL_MouseMotionEvent& eMotion) {
    if(eMotion.state == SDL_BUTTON_LMASK && !program->isRendering()) {
        program->click(eMotion.x, eMotion.y, false);
    }
    if(eMotion.state == SDL_BUTTON_RMASK && !program->isRendering()) {
        program->orbit(eMotion.x, eMotion.y);
    }
}


void IOController::juliaWindowKeyEvent(const SDL_Keycode& sym, bool& quit) {
    switch(sym) {
        case SDLK_q:            quit = true;                                break;
        case SDLK_EQUALS:       juliaWindow->scale(1);                      break;
        case SDLK_MINUS:        juliaWindow->scale(-1);                     break;
        case SDLK_h:            juliaWindow->home();                        break;
        case SDLK_r:            juliaWindow->refresh();                     break;
        case SDLK_SPACE:        juliaWindow->redraw();                      break;
        case SDLK_UP:           juliaWindow->translate(0, 1);               break;
        case SDLK_w:            juliaWindow->translate(0, 1);               break;
        case SDLK_DOWN:         juliaWindow->translate(0, -1);              break;
        case SDLK_s:            juliaWindow->translate(0, -1);              break;
        case SDLK_LEFT:         juliaWindow->translate(-1, 0);              break;
        case SDLK_a:            juliaWindow->translate(-1, 0);              break;
        case SDLK_RIGHT:        juliaWindow->translate(1, 0);               break;
        case SDLK_d:            juliaWindow->translate(1, 0);               break;
        case SDLK_i:            program->translateJuliaParameter(0, 1);     break;
        case SDLK_k:            program->translateJuliaParameter(0, -1);    break;
        case SDLK_j:            program->translateJuliaParameter(-1, 0);    break;
        case SDLK_l:            program->translateJuliaParameter(1, 0);     break;
        case SDLK_LEFTBRACKET:  juliaWindow->changenMax(-1);                break;
        case SDLK_RIGHTBRACKET: juliaWindow->changenMax(1);                 break;
        case SDLK_g:            juliaWindow->nextColoring();                break;
        case SDLK_c:            program->hideJuliaWindow();                 break;

        // case SDLK_y:
        //     std::cout << std::endl << "Julia window: ";
        //     juliaWindow->toggleSymmetry();
        //     std::cout << "$ " << std::flush;
        //     break;
    }
}

void IOController::juliaWindowScrollEvent(const SDL_MouseWheelEvent& eScroll) {
    if(eScroll.y > 0)
        juliaWindow->scale(1);
    else if(eScroll.y < 0)  // Explicit check, because y=0 when scrolling sideways
        juliaWindow->scale(-1);
}

void IOController::juliaWindowWindowEvent(const SDL_WindowEvent& eWindow, bool& quit) {
    switch(eWindow.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            juliaWindow->resize({(unsigned int)eWindow.data1, (unsigned int)eWindow.data2});
            break;

        case SDL_WINDOWEVENT_CLOSE:
            quit = true;
            break;

        case SDL_WINDOWEVENT_MOVED:
            program->refresh();
            juliaWindow->refresh();
            break;
    }
}

void IOController::juliaWindowClick(const SDL_MouseButtonEvent& eClick) {
    switch(eClick.button) {
        case SDL_BUTTON_LEFT:
            double c0, c1;
            juliaWindow->xyToComplex(eClick.x, eClick.y, c0, c1);
            program->setJuliaParameter(c0, c1);
            break;
    }
}

// void IOController::juliaWindowUnclick(const SDL_MouseButtonEvent& eClick) {
// }

void IOController::juliaWindowMouseMotion(const SDL_MouseMotionEvent& eMotion) {
    if(eMotion.state == SDL_BUTTON_LMASK && !program->isRendering()) {
        double c0, c1;
        juliaWindow->xyToComplex(eMotion.x, eMotion.y, c0, c1);
        program->setJuliaParameter(c0, c1);
    }
}


void IOController::mainLoop() {
    bool quit = false;
    SDL_Event e;

    while(!quit) {
        SDL_WaitEvent(&e);

        switch(e.type) {
            case SDL_QUIT:
                quit = true;
                break;

            case SDL_KEYDOWN:
                if(e.key.windowID == mainWindowID)
                    mainWindowKeyEvent(e.key.keysym.sym, quit);
                else if(e.key.windowID == juliaWindowID)
                    juliaWindowKeyEvent(e.key.keysym.sym, quit);
                break;
            
            case SDL_MOUSEWHEEL:
                if(e.wheel.windowID == mainWindowID)
                    mainWindowScrollEvent(e.wheel);
                else if(e.wheel.windowID == juliaWindowID)
                    juliaWindowScrollEvent(e.wheel);
                break;

            case SDL_WINDOWEVENT:
                if(e.window.windowID == mainWindowID)
                    mainWindowWindowEvent(e.window, quit);
                else if(e.window.windowID == juliaWindowID)
                    juliaWindowWindowEvent(e.window, quit);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(e.button.windowID == mainWindowID)
                    mainWindowClick(e.button);
                else if(e.button.windowID == juliaWindowID)
                    juliaWindowClick(e.button);
                break;

            case SDL_MOUSEBUTTONUP:
                // if(e.button.windowID == mainWindowID)
                //     mainWindowUnclick(e.button);
                // else if(e.button.windowID == juliaWindowID)
                //     juliaWindowUnclick(e.button);
                break;

            case SDL_MOUSEMOTION:
                if(e.motion.windowID == mainWindowID)
                    mainWindowMouseMotion(e.motion);
                else if(e.motion.windowID == juliaWindowID)
                    juliaWindowMouseMotion(e.motion);
                break;
        }
    }
}
