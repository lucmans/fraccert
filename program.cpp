
#include "program.h"

#include "select_scale.h"

#include <iostream>
#include <iomanip>


Program::Program(Graphics* const g, const unsigned int w, const unsigned int h, const uint32_t flags) : nDeepen(DEFAULTDEEPEN), graphics(g) {
    mpf_inits(domain.rMin, domain.rMax, domain.iMin, domain.iMax, xRatio, yRatio, dReal, dImag, t, scaleFactor, NULL);
    mpf_set_d(scaleFactor, SCALEFACTOR);

    setResolution(w, h);

    window = SDL_CreateWindow("Fraccert", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, res.w, res.h, flags | SDL_WINDOW_RESIZABLE | SDL_WINDOW_UTILITY);
    if(window == NULL) {
        printf("Window could not be created!\nSDL Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    g->initRenderer(window);
    g->setSymmetry(symmetry);

    fractal = new Mandelbrot();
    rendering = false;
    selection = nullptr;

    resetView();
}

Program::~Program() {
    mpf_clears(domain.rMin, domain.rMax, domain.iMin, domain.iMax, xRatio, yRatio, dReal, dImag, t, scaleFactor, NULL);

    delete fractal;
    SDL_DestroyWindow(window);
}


void Program::click(const unsigned int x, const unsigned int y, const bool print) {
    lock(renderingMutex);

    graphics->drawClick(x, y);

    double c[2];
    xyToComplex(x, y, c);
    if(print) {
        std::cout << "\rClicked on point [" << std::setprecision(std::numeric_limits<double>::digits10 + 1) << c[0] << ", " << c[1] << "]" << std::endl;
        std::cout << "$ " << std::flush;
    }

    unlock(renderingMutex);
}


inline void Program::lock(std::mutex& m) {
    rendering = true;
    m.lock();
}

inline void Program::unlock(std::mutex& m) {
    m.unlock();
    rendering = false;
}

bool Program::isRendering() const {
    return rendering;
}


unsigned int Program::getWindowID() const {
    return SDL_GetWindowID(window);
}


void Program::setC(const double c[2]) {
    if(fractal->fractalType != Fractals::Julia)
        return;

    lock(renderingMutex);

    ((Julia*)fractal)->setC(c[0], c[1]);
    tick();

    unlock(renderingMutex);
}

void Program::getC(double c[2]) const {
    if(fractal->fractalType == Fractals::Julia)
        ((Julia*)fractal)->getC(c[0], c[1]);
    else
        memset(c, 0, 2 * sizeof(double));
}


// TODO: Support high precision
bool Program::setDomain(const Domain& d) {
    if(d.rMin >= d.rMax || d.iMin >= d.iMax)
        return false;

    lock(renderingMutex);

    const double iCenter = (d.iMax + d.iMin) / 2.0;
    const double iHeight = (d.rMax - d.rMin) * (res.h / (double)res.w);

    mpf_set_d(domain.rMin, d.rMin);
    mpf_set_d(domain.rMax, d.rMax);
    mpf_set_d(domain.iMin, iCenter - (iHeight / 2.0));
    mpf_set_d(domain.iMax, iCenter + (iHeight / 2.0));

    tick();
        
    unlock(renderingMutex);

    return true;
}

// TODO: Support high precision
Domain Program::getDomain() const {
    return {mpf_get_d(domain.rMin), mpf_get_d(domain.rMax), mpf_get_d(domain.iMin), mpf_get_d(domain.iMax)};
}


void Program::setDeepen(const unsigned int d) {
    nDeepen = d;
}

unsigned int Program::getDeepen() const {
    return nDeepen;
}


void Program::deepen() {
    lock(renderingMutex);

    std::cout << "\rNot implemented!" << std::endl;
    std::cout << "$ " << std::flush;

    // fractal->changenMax(nDeepen);
    // deepenTick();
    // tick();
    
    unlock(renderingMutex);
}

void Program::unDeepen() {
    lock(renderingMutex);

    std::cout << "\rNot implemented!" << std::endl;
    std::cout << "$ " << std::flush;

    // fractal->changenMax(-nDeepen);
    // tick();
    
    unlock(renderingMutex);
}


void Program::setLineDetail(const double lineDetail) {
    lock(renderingMutex);

    graphics->setLineDetail(fractal, lineDetail);
    tick();

    unlock(renderingMutex);
}


void Program::nextColoring() {
    lock(renderingMutex);

    graphics->nextColoring();
    tick();

    unlock(renderingMutex);
}


void Program::setJuliaWindow(Program* const p) {
    juliaWindow = p;
}


void Program::getResolution(Resolution& r) const {
    r.w = res.w;
    r.h = res.h;
}

void Program::resize(const Resolution& newRes) {
    lock(renderingMutex);

    setResolution(newRes.w, newRes.h);
    // graphics->forceRedraw();  // Prevents extend drawing, because otherwise old resolution would be used when pixel recycling

    // Alias xRatio as iCenter and yRatio as iHeight to save initializing extra mpf_t members
    mpf_t& iCenter = xRatio; mpf_t& iHeight = yRatio;

    // const double iCenter = (iMax + iMin) / 2.0;
    mpf_add(t, domain.iMax, domain.iMin);
    mpf_div_ui(iCenter, t, 2);

    // const double iHeight = (rMax - rMin) * (res.h / (double)res.w);
    mpf_sub(t, domain.rMax, domain.rMin);
    mpf_set_d(iHeight, res.h / (double)res.w);
    mpf_mul(iHeight, t, iHeight);

    // iMin = iCenter - (iHeight / 2.0);
    mpf_div_ui(t, iHeight, 2);
    mpf_sub(domain.iMin, iCenter, t);

    // iMax = iCenter + (iHeight / 2.0);
    // t still set correctly
    mpf_add(domain.iMax, iCenter, t);

    tick();

    unlock(renderingMutex);
}


void Program::setResolution(const unsigned int w, const unsigned int h) {
    // if(w < MINWIDTH) {
    //     res.w = DEFAULTWIDTH;
    //     printf("Width under minimum [%i]. Width set to default [%i].\n", MINWIDTH, res.w);
    //     return;
    // }
    // if(h < MINHEIGHT) {
    //     res.h = DEFAULTHEIGHT;
    //     printf("Height under minimum [%i]. Height set to default [%i].\n", MINHEIGHT, res.h);
    //     return;
    // }

    res.w = w;
    res.h = h;
}
// void Program::setResolution(const Resolution& newRes) {
//     if(newRes.w < MINWIDTH) {
//         res.w = DEFAULTWIDTH;
//         printf("Width under minimum [%i]. Width set to default [%i].\n", MINWIDTH, res.w);
//         return;
//     }
//     if(newRes.h < MINHEIGHT) {
//         res.h = DEFAULTHEIGHT;
//         printf("Height under minimum [%i]. Height set to default [%i].\n", MINHEIGHT, res.h);
//         return;
//     }

//     res.w = newRes.w;
//     res.h = newRes.h;
// }


void Program::refresh() {
    lock(renderingMutex);
    graphics->refresh();

    unlock(renderingMutex);
}

void Program::redraw() {
    lock(renderingMutex);
    tick();
    unlock(renderingMutex);
}


void Program::tick() {
    graphics->setScreen();

    graphics->draw(fractal, domain, {res.w, res.h});

    if(juliaWinUp)
        drawJuliaC();

    graphics->blit();
}

void Program::translateTick() {
    graphics->setScreen();

    graphics->extendDraw(fractal, domain, {res.w, res.h});

    if(juliaWinUp)
        drawJuliaC();

    graphics->blit();
}

void Program::deepenTick() {
    graphics->setScreen();

    graphics->deepenDraw(fractal, domain, {res.w, res.h});

    if(juliaWinUp)
        drawJuliaC();

    graphics->blit();
}


void Program::scaleXY(const int scaleDirection, const unsigned int x, const unsigned int y) {
    // dReal = rMax - rMin, dImag = iMax - iMin;
    mpf_sub(dReal, domain.rMax, domain.rMin);
    mpf_sub(dImag, domain.iMax, domain.iMin);

    if(scaleDirection == 1) {
        mpf_set_d(scaleFactor, SCALEFACTOR);

        //dReal = (SCALEFACTOR * dReal) - dReal;
        mpf_mul(t, scaleFactor, dReal);
        mpf_sub(dReal, t, dReal);
        
        //dImag = (SCALEFACTOR * dImag) - dImag;
        mpf_mul(t, scaleFactor, dImag);
        mpf_sub(dImag, t, dImag);
    }
    else {
        // scaleFactor = 1 / SCALEFACTOR, because zooming the other direction
        mpf_set_d(scaleFactor, 1 / SCALEFACTOR);

        // dReal = ((1 / SCALEFACTOR) * dReal) - dReal;
        mpf_mul(t, scaleFactor, dReal);
        mpf_sub(dReal, t, dReal);

        // dImag = ((1 / SCALEFACTOR) * dImag) - dImag;
        mpf_mul(t, scaleFactor, dImag);
        mpf_sub(dImag, t, dImag);
    }

    // const double xRatio = x / (double)res.w, yRatio = y / (double)res.h;
    mpf_set_d(xRatio, x / (double)res.w);
    mpf_set_d(yRatio, y / (double)res.h);

    // rMin = rMin - (xRatio * dReal);
    mpf_mul(t, xRatio, dReal);
    mpf_sub(domain.rMin, domain.rMin, t);

    // rMax = rMax + ((1.0 - xRatio) * dReal);
    mpf_ui_sub(t, 1, xRatio);
    mpf_mul(t, t, dReal);
    mpf_add(domain.rMax, domain.rMax, t);

    // iMax = iMax + (yRatio * dImag);
    mpf_mul(t, yRatio, dImag);
    mpf_add(domain.iMax, domain.iMax, t);

    // iMin = iMin - ((1.0 - yRatio) * dImag);
    mpf_ui_sub(t, 1, yRatio);
    mpf_mul(t, t, dImag);
    mpf_sub(domain.iMin, domain.iMin, t);

    tick();
}

void Program::scale(const int scaleDirection) {
    lock(renderingMutex);

    int x, y;
    SDL_GetMouseState(&x, &y);

    // SDL_GetMouseState returns 0 if there was no mouse update.
    if(x < 0 || y < 0 || x > (int)res.w || y > (int)res.h)
        scaleXY(scaleDirection, res.w / 2, res.h / 2);
    else if(x != 0 && y != 0)
        scaleXY(scaleDirection, x, y);
    else
        scaleXY(scaleDirection, res.w / 2, res.h / 2);

    unlock(renderingMutex);
}


// TODO: Translate with pixel size
void Program::translate(const int realDirection, const int imagDirection) {
    lock(renderingMutex);

    // const double dReal = (rMax - rMin) / 10.0;
    mpf_sub(t, domain.rMax, domain.rMin);
    mpf_div_ui(dReal, t, 10);

    // const double dImag = (iMax - iMin) / 10.0;
    mpf_sub(t, domain.iMax, domain.iMin);
    mpf_div_ui(dImag, t, 10);

    // Because mpf only supports unsigned ints for arithmetic, check direction with if
    if(realDirection == 1) {
        //rMin += dReal * realDirection; rMax += dReal * realDirection;
        mpf_add(domain.rMin, domain.rMin, dReal);
        mpf_add(domain.rMax, domain.rMax, dReal);
    }
    else if(realDirection == -1) {
        //rMin += dReal * realDirection; rMax += dReal * realDirection;
        mpf_sub(domain.rMin, domain.rMin, dReal);
        mpf_sub(domain.rMax, domain.rMax, dReal);
    }

    if(imagDirection == 1) {
        //rMin += dReal * realDirection; rMax += dReal * realDirection;
        mpf_add(domain.iMin, domain.iMin, dImag);
        mpf_add(domain.iMax, domain.iMax, dImag);
    }
    else if(imagDirection == -1) {
        //rMin += dReal * realDirection; rMax += dReal * realDirection;
        mpf_sub(domain.iMin, domain.iMin, dImag);
        mpf_sub(domain.iMax, domain.iMax, dImag);
    }

    translateTick();

    unlock(renderingMutex);
}


void Program::translateJuliaParameter(const int realDirection, const int imagDirection) {
    if(fractal->fractalType != Fractals::Julia)
        return;

    lock(renderingMutex);

    const double dReal = (mpf_get_d(domain.rMax) - mpf_get_d(domain.rMin)) / 100.0 * realDirection,
                 dImag = (mpf_get_d(domain.iMax) - mpf_get_d(domain.iMin)) / 100.0 * imagDirection;

    ((Julia*)fractal)->moveC(dReal, dImag);

    tick();

    unlock(renderingMutex);
}

void Program::setJuliaParameter(const double real, const double imag) {
    if(fractal->fractalType != Fractals::Julia)
        return;

    lock(renderingMutex);

    ((Julia*)fractal)->setC(real, imag);

    tick();

    unlock(renderingMutex);
}


void Program::nextFractal() {
    lock(renderingMutex);

    const Fractal* const oldFrac = fractal;
    switch(fractal->fractalType) {
        case Fractals::Mandelbrot:
            fractal = new Julia();
            showJuliaWindow();
            break;

        case Fractals::Julia:
            hideJuliaWindow();
            fractal = new Mandelbrot();
            break;

        // Add other fractals here

        case Fractals::None:
            fractal = new Mandelbrot();
            break;
    }
    delete oldFrac;
    
    resetView();
    tick();

    unlock(renderingMutex);
}


void Program::resetView() {
    double dd[3];
    fractal->getDefaultDomain(dd);
    const double iHeight = (dd[1] - dd[0]) * (res.h / (double)res.w);

    mpf_set_d(domain.rMin, dd[0]);
    mpf_set_d(domain.rMax, dd[1]);
    mpf_set_d(domain.iMax, (iHeight / 2.0) + dd[2]);
    mpf_set_d(domain.iMin, (iHeight / -2.0) + dd[2]);
}

void Program::home() {
    lock(renderingMutex);

    resetView();
    tick();

    unlock(renderingMutex);
}


void Program::setnMax(const unsigned long n, const bool doTick) {
    lock(renderingMutex);

    fractal->setnMax(n);
    if(doTick)
        tick();

    unlock(renderingMutex);
}

unsigned long Program::getnMax() const {
    return fractal->getnMax();
}

void Program::changenMax(const long n) {
    lock(renderingMutex);

    const unsigned long nMax = fractal->getnMax();
    if(nMax + n <= 1) {
        std::cout << std::endl << "\rWarning: NMAX underflow. NMAX is set to 0" << std::endl;
        std::cout << "$ " << std::flush;

        fractal->setnMax(2);
    }
    else
        fractal->setnMax(nMax + n);

    tick();

    unlock(renderingMutex);
}


void Program::toggleSymmetry() {
    lock(renderingMutex);

    symmetry = !symmetry;
    graphics->setSymmetry(symmetry);

    tick();
    std::cout << "\rSymmetry is " << (symmetry ? "on" : "off") << std::endl;
    std::cout << "$ " << std::flush;

    unlock(renderingMutex);
}


void Program::xyToComplex(const unsigned int x, const unsigned int y, double& c0, double& c1) const {
    const double pixelSize = (mpf_get_d(domain.rMax) - mpf_get_d(domain.rMin)) / (double)(res.w);

    c0 = mpf_get_d(domain.rMin) + (x * pixelSize);
    c1 = mpf_get_d(domain.iMax) - (y * pixelSize);
}

void Program::xyToComplex(const unsigned int x, const unsigned int y, double c[2]) const {
    const double pixelSize = (mpf_get_d(domain.rMax) - mpf_get_d(domain.rMin)) / (double)(res.w);

    c[0] = mpf_get_d(domain.rMin) + (x * pixelSize);
    c[1] = mpf_get_d(domain.iMax) - (y * pixelSize);
}


void Program::drawJuliaC(const bool juliaWin /*= false*/) const {
    double c0, c1;

    if(juliaWin) {
        juliaWindow->drawJuliaC();
        return;
    }

    ((Julia*)fractal)->getC(c0, c1);
    juliaWindow->drawJuliaC(c0, c1);
}

void Program::drawJuliaC(const double c0, const double c1) const {
    graphics->drawCurrentC(((c0 - mpf_get_d(domain.rMin)) / (mpf_get_d(domain.rMax) - mpf_get_d(domain.rMin))), ((c1 - mpf_get_d(domain.iMax)) / (mpf_get_d(domain.iMin) - mpf_get_d(domain.iMax))), {res.w, res.h});
}


void Program::showJuliaWindow() {
    juliaWinUp = true;
    juliaWindow->showWindow();
}

void Program::hideJuliaWindow() {
    juliaWindow->hideWindow();
    juliaWinUp = false;
}


void Program::showWindow() {
    if(window == NULL)  // NULL because SDL returns NULL on window creation failure
        return;

    isJuliaWin = true;

    SDL_ShowWindow(window);
    SDL_RaiseWindow(window);
    tick();

    // drawJuliaC(true);
}

void Program::hideWindow() {
    if(window == NULL)  // NULL because SDL returns NULL on failure
        return;

    SDL_HideWindow(window);
}


void Program::orbit(const unsigned int x, const unsigned int y) {
    lock(renderingMutex);
    
    double c[2];
    xyToComplex(x, y, c);

    graphics->drawOrbit(fractal, c, {mpf_get_d(domain.rMin), mpf_get_d(domain.rMax), mpf_get_d(domain.iMin), mpf_get_d(domain.iMax)}, res);
    
    unlock(renderingMutex);
}


void Program::beginRegionSelect(const unsigned int x, const unsigned int y) {
    lock(renderingMutex);

    if(selection != nullptr)
        delete selection;

    selection = new Selection();
    selection->xInit = selection->xLast = x;
    selection->yInit = selection->yLast = y;

    graphics->select(selection, res);

    unlock(renderingMutex);
}

void Program::updateRegionSelect(const unsigned int x, const unsigned int y) {
    if(selection == nullptr)
        return;

    lock(renderingMutex);

    selection->xLast = x;
    selection->yLast = y;

    graphics->select(selection, res);

    unlock(renderingMutex);
}

void Program::setSelectedRegion() {
    if(selection == nullptr)
        return;

    lock(renderingMutex);

    SDL_Rect zoomTo, selectionBox;
    largestARtopleft(zoomTo, selectionBox, selection, res);

    // const double pixelSize = (domain.rMax - domain.rMin) / (double)(res.w);
    mpf_t& pixelSize = scaleFactor;  // Alias to reuse gmp floats
    mpf_sub(pixelSize, domain.rMax, domain.rMin);
    mpf_div_ui(pixelSize, pixelSize, res.w);

    // Update rMax
    mpf_mul_ui(dReal, pixelSize, zoomTo.x + zoomTo.w);
    mpf_add(domain.rMax, domain.rMin, dReal);

    // Update iMin
    mpf_mul_ui(dImag, pixelSize, zoomTo.y + zoomTo.h);
    mpf_sub(domain.iMin, domain.iMax, dImag);

    // Update rMin
    mpf_mul_ui(dReal, pixelSize, zoomTo.x);
    mpf_add(domain.rMin, domain.rMin, dReal);

    // Update iMax
    mpf_mul_ui(dImag, pixelSize, zoomTo.y);
    mpf_sub(domain.iMax, domain.iMax, dImag);

    tick();

    delete selection;
    selection = nullptr;

    unlock(renderingMutex);
}

void Program::cancelSelect() {
    if(selection == nullptr)
        return

    lock(renderingMutex);

    delete selection;
    selection = nullptr;

    unlock(renderingMutex);
}
