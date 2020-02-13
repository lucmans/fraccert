
#ifndef IOCONTROLLER_H
#define IOCONTROLLER_H


#include "program.h"
#include "console.h"

#include <SDL2/SDL.h>


class IOController {
    public:
        IOController(Program* const p, Program* const p2, const uint32_t mainWinID, const uint32_t juliaWinID);
        ~IOController();

        void mainWindowKeyEvent(const SDL_Keycode& sym, bool& quit);
        void mainWindowScrollEvent(const SDL_MouseWheelEvent& eScroll);
        void mainWindowWindowEvent(const SDL_WindowEvent& eWindow, bool& quit);
        void mainWindowClick(const SDL_MouseButtonEvent& eClick);
        void mainWindowUnclick(const SDL_MouseButtonEvent& eClick);
        void mainWindowMouseMotion(const SDL_MouseMotionEvent& eMotion);

        void juliaWindowKeyEvent(const SDL_Keycode& sym, bool& quit);
        void juliaWindowScrollEvent(const SDL_MouseWheelEvent& eScroll);
        void juliaWindowWindowEvent(const SDL_WindowEvent& eWindow, bool& quit);
        void juliaWindowClick(const SDL_MouseButtonEvent& eClick);
        // void juliaWindowUnclick(const SDL_MouseButtonEvent& eClick);
        void juliaWindowMouseMotion(const SDL_MouseMotionEvent& eMotion);

        void mainLoop();


        bool ctrlHeldDown;

    private:
        Program* const program;
        Program* const juliaWindow;
        const Console* console;

        const uint32_t mainWindowID;
        const uint32_t juliaWindowID;
};


#endif  // IOCONTROLLER_H
