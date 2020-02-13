
#include "iocontroller.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <cstring>
#include <cstdlib>


#include "tests.cpp"
void tests() {
    // bruteforceSpeed();
    // symmetrySpeed();
    // shapeSpeed();
    // noShapeSpeed();
    // borderCorrect();
    // borderSpeed();
    // multiThreads();
    // multiSplits();
    // multiSpeed(/*8, 7*/);
    // allSpeeds();
    // gmpBruteforceSpeed(/*64*/);
    // gmpBordertraceSpeed(/*64*/);
    // for(int t = 0; t < 25; t++) {
    //     gmpFractalSpeedAll(t);
    // }
    // gmpScaleSpeed();
    // doublePrec();
}


void parseArgs(unsigned int argc, char* argv[], unsigned int& width, unsigned int& height) {
    for(unsigned int i = 1; i < argc; i++) {
        if((strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--precision") == 0) && argc > i + 1) {
            // TODO: Start with given arbitrary precision
            std::cout << "Setting precision on startup is not implemented yet!" << std::endl;
        }
        else if((strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--resolution") == 0) && argc > i + 2) {
            width = atoi(argv[i + 1]);
            height = atoi(argv[i + 2]);

            if(height == 0 || width == 0) {
                std::cout << "Incorrect value for width and/or height. Skipping " << argv[i] << ' ' << argv[i + 1] << ' ' << argv[i + 2] << '.' << std::endl;
                width = DEFAULTWIDTH;
                height = DEFAULTHEIGHT;
            }

            i += 2;  // Advance 2 extra arguments
        }
        else if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--test") == 0) {
            tests();
            exit(EXIT_SUCCESS);
        }
        else {
            if(strcmp(argv[i], "-h") != 0 && strcmp(argv[i], "--help") != 0)
                std::cout << "Incorrect usage.\n" << std::endl;

            std::cout << "---[ Fraccert ]---\n"
                      << "An interactive fractal viewer\n"
                      << "Author: Luc de Jonckheere\n"
                      << "\n"
                      << "Flags:\n"
                      << "  (-p | --precision) [p]        - Sets precision to p bits\n"
                      << "  (-r | --resolution) [x] [y]   - Run fraccert in x by y pixels\n"
                      << "  (-t | --tests)                - Run tests\n"
                      << "  (-h | --help)                 - Prints help\n"
                      << "\n"
                      << "Usage:\n"
                      << "Use scroll wheel or +/- keys to scale.\n"
                      << "WASD or arrow keys to translate.\n"
                      << "T to toggle between Mandelbrot and Julia.\n"
                      << "G to toggle coloring method.\n"
                      << "H to return to the starting location\n"
                      << "IJKL to translate Julia c value.\n"
                      << "[] to change NMAX.\n"
                      << "Use escape ('esc') to remove any queued actions.\n" << std::endl;
            exit(EXIT_SUCCESS);
        }
    }
}


int main(int argc, char* argv[]) {
    // Delfault settings
    unsigned int width = DEFAULTWIDTH;
    unsigned int height = DEFAULTHEIGHT;

    parseArgs(argc, argv, width, height);

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("SDL could not initialize!\nSDL Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Julia window
    Graphics* juliaGraphics = new Graphics();
    Program* juliaWindow = new Program(juliaGraphics, DEFAULTWIDTH / 2, DEFAULTHEIGHT / 2, SDL_WINDOW_HIDDEN);

    // Init model-view-controller
    Graphics* graphics = new Graphics();
    Program* program = new Program(graphics, width, height, SDL_WINDOW_SHOWN);
    IOController* ioController = new IOController(program, juliaWindow, program->getWindowID(), juliaWindow->getWindowID());

    program->setJuliaWindow(juliaWindow);
    juliaWindow->setJuliaWindow(program);  // So it can request c

    program->redraw();  // Initial draw
    ioController->mainLoop();

    std::cout << std::endl;

    delete juliaGraphics;
    delete juliaWindow;

    delete graphics;
    delete program;
    delete ioController;
    SDL_Quit();

    return EXIT_SUCCESS;
}
