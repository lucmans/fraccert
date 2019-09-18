
#ifndef PROGRAM_H
#define PROGRAM_H


#include "graphics.h"
#include "fracfast/fractals.h"
#include "fracfast/types.h"

#include <SDL2/SDL.h>
#include <gmp.h>

#include <mutex>


const unsigned int DEFAULTWIDTH = 800;
const unsigned int DEFAULTHEIGHT = 600;

const unsigned int MINWIDTH = 400;
const unsigned int MINHEIGHT = 300;

const double SCALEFACTOR = 0.8;

const unsigned int DEFAULTDEEPEN = 50;


// All non-const public functions must lock/unlock 'rendering"
// TODO: Seperate Window class
class Program {
    public:
        Program(Graphics* const g, const unsigned int w, const unsigned int h, const uint32_t flags);
        ~Program();

        void click(const unsigned int x, const unsigned int y, const bool print = true);

        inline void lock(std::mutex& m);
        inline void unlock(std::mutex& m);
        bool isRendering() const;

        unsigned int getWindowID() const;

        void setC(const double c[2]);
        void getC(double c[2]) const;

        bool setDomain(const Domain& d);
        Domain getDomain() const;

        void setDeepen(const unsigned int d);
        unsigned int getDeepen() const;

        void deepen();
        void unDeepen();

        void setLineDetail(const double lineDetail);
        void nextColoring();

        void setJuliaWindow(Program* const p);

        void getResolution(Resolution& res) const;
        void resize(const Resolution& res);

        void refresh();
        void redraw();

        void scale(const int scaleDirection);

        void translate(const int realDirection, const int imagDirection);

        void translateJuliaParameter(const int realDirection, const int imagDirection);
        void setJuliaParameter(const double real, const double imag);

        void nextFractal();

        void home();

        void setnMax(const unsigned long n, const bool doTick = true);
        unsigned long getnMax() const;
        void changenMax(const long n);

        void toggleSymmetry();

        void xyToComplex(const unsigned int x, const unsigned int y, double& c0, double& c1) const;
        void xyToComplex(const unsigned int x, const unsigned int y, double c[2]) const;

        void drawJuliaC(const bool juliaWin = false) const;
        void drawJuliaC(const double c0, const double c1) const;

        void showJuliaWindow();
        void hideJuliaWindow();

        void showWindow();
        void hideWindow();

        void orbit(const unsigned int x, const unsigned int y);



    private:
        SDL_Window* window;
        Resolution res;
        unsigned int nDeepen;
        
        Graphics* const graphics;
        Fractal* fractal;

        // Mutex for signaling then fractal is rendering. When fractal is rendering, no changes may be made to the program state and in extension, no changes to the fractal
        std::mutex renderingMutex;
        bool rendering;

        bool symmetry = true;

        // For hinding/showing the Julia window
        Program* juliaWindow = nullptr;
        bool juliaWinUp = false;
        bool isJuliaWin = false;

        HighPrecDomain domain;
        
        // These are members, because they are expensive to initialize.
        // Now, the constructor can initialize them once and every member function can use them with little overhead.
        mpf_t xRatio, yRatio, dReal, dImag, t, scaleFactor;
        

        // Scale at mouse position (x, y)
        void scaleXY(const int scaleDirection, const unsigned int x, const unsigned int y);

        // Draw fractal with current program state
        void tick();
        void translateTick();
        void deepenTick();

        void resetView();

        void setResolution(const unsigned int w, const unsigned int h);
};


#endif  // PROGRAM_H
