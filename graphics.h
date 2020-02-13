
#ifndef GRAPHICS_H
#define GRAPHICS_H


#include "fracfast/fractals.h"
#include "fracfast/types.h"

#include <SDL2/SDL.h>


// const unsigned int SCALEFRAMES = 30,
//                    SCALETIME = 1500;  // milliseconds


enum class Coloring {
    escapeTime,
    distance
};


struct GraphicsState {
    HighPrecDomain domain;
    SDL_Texture* pixels;


    GraphicsState() {
        pixels = NULL;

        mpf_inits(domain.rMin, domain.rMax, domain.iMin, domain.iMax, NULL);
    }

    ~GraphicsState() {
        if(pixels != NULL)
            SDL_DestroyTexture(pixels);
        
        mpf_clears(domain.rMin, domain.rMax, domain.iMin, domain.iMax, NULL);
    }

    void update(const HighPrecDomain& d, SDL_Texture* p) {
        if(pixels != NULL)
            SDL_DestroyTexture(pixels);

        pixels = p;

        mpf_set(domain.rMin, d.rMin); mpf_set(domain.rMax, d.rMax); mpf_set(domain.iMin, d.iMin); mpf_set(domain.iMax, d.iMax);
    }

    void forceRedraw() {
        pixels = NULL;
    }

    void destroy() {
        if(pixels != NULL)
            SDL_DestroyTexture(pixels);
    }
};


class Graphics {
    public:
        Graphics();
        ~Graphics();

        void initRenderer(SDL_Window* const window);

        void drawClick(const int x, const int y);
        void drawOrbit(const Fractal* const fractal, const double c[2], const Domain& domain, const Resolution& res);

        void setSymmetry(const bool sym);

        void setLineDetail(Fractal* const fractal, const double lineDetail);
        
        void nextColoring();

        // Sets the screen up for new frame
        void setScreen();

        // Updates screen; finish frame
        void blit();

        void draw(const Fractal* const fractal, const HighPrecDomain& domain, const Resolution& res);
        void extendDraw(const Fractal* const fractal, const HighPrecDomain& domain, const Resolution& res);
        void deepenDraw(const Fractal* const fractal, const HighPrecDomain& domain, const Resolution& res);

        SDL_Texture* calculatePixels(const Fractal* const fractal, const HighPrecDomain& domain, const Resolution& res);

        SDL_Texture* calculateMandelbrot(const Mandelbrot* const fractal, const HighPrecDomain& domain, const Resolution& res) const;
        SDL_Texture* calculateJulia(const Julia* const fractal, const HighPrecDomain& domain, const Resolution& res);

        void forceRedraw();

        void refresh();

        void drawCurrentC(const double xRatio, const double yRatio, const Resolution& res);

        // void smoothScale(unsigned int x, unsigned int y);

        void select(const Selection* const selection, const Resolution& res);



    private:
        SDL_Renderer* renderer;

        bool symmetry;  // Should use symmetry optimization

        Coloring coloring;

        GraphicsState prev;

        // These are members so these GMP floats only have to be inited once
        HighPrecDomain newDomain;
        mpf_t pixelSize;
};


#endif  // GRAPHICS_H
