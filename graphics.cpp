
#include "graphics.h"

#include "select_scale.h"
#include "fracfast/shapes.h"

#include <iostream>
#include <cstdlib>
#include <cstdint>


void complexToXY(Point& c, const Domain& dom, const Resolution& res, int& x, int& y) {
    const double pixelSize = (dom.rMax - dom.rMin) / (double)(res.w);

    x = (c[0] - dom.rMin) / pixelSize;
    y = (dom.iMax - c[1]) / pixelSize;
}


Graphics::Graphics() {
    coloring = Coloring::escapeTime;

    mpf_inits(newDomain.rMin, newDomain.rMax, newDomain.iMin, newDomain.iMax, pixelSize, NULL);
}

Graphics::~Graphics() {
    prev.destroy();

    SDL_DestroyRenderer(renderer);

    mpf_clears(newDomain.rMin, newDomain.rMax, newDomain.iMin, newDomain.iMax, pixelSize, NULL);
}


void Graphics::initRenderer(SDL_Window* const window) {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL) {
        printf("Renderer could not be created!\nSDL Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Set the scaling quality to nearest-pixel
    if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") < 0) {
        printf("Warning: Could not set renderer hints.\nSDL Error: %s\n", SDL_GetError());
    }

    setScreen();
    blit();  // Render black background
}


void Graphics::drawClick(const int x, const int y) {
    if(prev.pixels == NULL)
        return;
    SDL_RenderCopy(renderer, prev.pixels, NULL, NULL);

    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
    SDL_Rect rect = {x - 3, y - 3, 7, 7};
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0xff, 0xff);
    rect = {x - 1, y - 1, 3, 3};
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    rect = {x, y, 1, 1};
    SDL_RenderFillRect(renderer, &rect);

    blit();
}


// TODO: Max N orbits
void Graphics::drawOrbit(const Fractal* const fractal, const double c[2], const Domain& domain, const Resolution& res) {
    if(prev.pixels == NULL)
        return;
    SDL_RenderCopy(renderer, prev.pixels, NULL, NULL);
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);

    Orbit points;
    fractal->calcOrbit(c, points);

    int x, y;
    complexToXY(points.front(), domain, res, x, y);
    points.pop_front();
    int prevX = x, prevY = y;
    for(auto& p : points) {
        complexToXY(p, domain, res, x, y);
        SDL_RenderDrawLine(renderer, prevX, prevY, x, y);
        prevX = x; prevY = y;
    }

    blit();
}


void Graphics::setSymmetry(const bool sym) {
    symmetry = sym;
}


void Graphics::setLineDetail(Fractal* const fractal, const double lineDetail) {
    forceRedraw();
    fractal->setLineDetail(lineDetail);
}


void Graphics::nextColoring() {
    switch(coloring) {
        case Coloring::escapeTime:  coloring = Coloring::distance;      break;
        case Coloring::distance:    coloring = Coloring::escapeTime;    break;
    }
}


void Graphics::setScreen() {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);
}

void Graphics::blit() {
    SDL_RenderPresent(renderer);
}


void Graphics::draw(const Fractal* const fractal, const HighPrecDomain& domain, const Resolution& res) {
    SDL_Texture* texture = calculatePixels(fractal, domain, res);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // This deletes will delete the texture created in this function
    prev.update(domain, texture);
}

// TODO: Use range to simplify this function
void Graphics::extendDraw(const Fractal* const fractal, const HighPrecDomain& domain, const Resolution& res) {
    // TODO: Fix extendDraw for exterior distance estimation
    if(prev.pixels == NULL || coloring == Coloring::distance) {
        draw(fractal, domain, res);
        return;
    }

    SDL_Rect reusePixelsSrc, reusePixelsDst, newPixelsDst;
    SDL_Texture* newPixels;
    if(mpf_cmp(domain.rMin, prev.domain.rMin) < 0) {  // Left
        //pixelSize = (rMax - rMin) / res.w;
        mpf_sub(pixelSize, domain.rMax, domain.rMin);
        mpf_div_ui(pixelSize, pixelSize, res.w);
        
        // pixelsMoved = (prev.rMin - rMin) / pixelSize
        mpf_sub(newDomain.rMin, prev.domain.rMin, domain.rMin);  // Use newDomain.rMin as temp
        mpf_div(newDomain.rMin, newDomain.rMin, pixelSize);
        const unsigned int pixelsMoved = mpf_get_d(newDomain.rMin);
        
        reusePixelsSrc = {0, 0, (int)res.w - (int)pixelsMoved, (int)res.h};
        reusePixelsDst = {(int)pixelsMoved, 0, (int)res.w - (int)pixelsMoved, (int)res.h};
        newPixelsDst = {0, 0, (int)pixelsMoved, (int)res.h};

        // newDomain.rMax = rMin + (pixelsMoved * pixelSize)
        mpf_mul_ui(newDomain.rMax, pixelSize, pixelsMoved);
        mpf_add(newDomain.rMax, domain.rMin, newDomain.rMax);

        // Set the others
        mpf_set(newDomain.rMin, domain.rMin); mpf_set(newDomain.iMin, domain.iMin); mpf_set(newDomain.iMax, domain.iMax);
        newPixels = calculatePixels(fractal, newDomain, {pixelsMoved, res.h});
    }
    else if(mpf_cmp(domain.rMin, prev.domain.rMin) > 0) {  // Right
        //pixelSize = (rMax - rMin) / res.w;
        mpf_sub(pixelSize, domain.rMax, domain.rMin);
        mpf_div_ui(pixelSize, pixelSize, res.w);
        
        // pixelsMoved = (rMin - prev.rMin) / pixelSize
        mpf_sub(newDomain.rMin, domain.rMin, prev.domain.rMin);  // Use newDomain.rMin as temp
        mpf_div(newDomain.rMin, newDomain.rMin, pixelSize);
        const unsigned int pixelsMoved = mpf_get_d(newDomain.rMin);

        reusePixelsSrc = {(int)pixelsMoved, 0, (int)res.w - (int)pixelsMoved, (int)res.h};
        reusePixelsDst = {0, 0, (int)res.w - (int)pixelsMoved, (int)res.h};
        newPixelsDst = {(int)res.w - (int)pixelsMoved, 0, (int)pixelsMoved, (int)res.h};

        // newDomain.rMin = rMax - (pixelsMoved * pixelSize)
        mpf_mul_ui(newDomain.rMin, pixelSize, pixelsMoved);
        mpf_sub(newDomain.rMin, domain.rMax, newDomain.rMin);

        // Set the others
        mpf_set(newDomain.rMax, domain.rMax); mpf_set(newDomain.iMin, domain.iMin); mpf_set(newDomain.iMax, domain.iMax);
        
        newPixels = calculatePixels(fractal, newDomain, {pixelsMoved, res.h});
    }
    else if(mpf_cmp(domain.iMin, prev.domain.iMin) > 0) {  // Up
        //pixelSize = (iMax - iMin) / res.h;
        mpf_sub(pixelSize, domain.iMax, domain.iMin);
        mpf_div_ui(pixelSize, pixelSize, res.h);
        
        // pixelsMoved = (iMin - prev.iMin) / pixelSize
        mpf_sub(newDomain.rMin, domain.iMin, prev.domain.iMin);  // Use newDomain.rMin as temp
        mpf_div(newDomain.rMin, newDomain.rMin, pixelSize);
        const unsigned int pixelsMoved = mpf_get_d(newDomain.rMin);
        
        reusePixelsSrc = {0, 0, (int)res.w, (int)res.h - (int)pixelsMoved};
        reusePixelsDst = {0, (int)pixelsMoved, (int)res.w, (int)res.h - (int)pixelsMoved};
        newPixelsDst = {0, 0, (int)res.w, (int)pixelsMoved};

        // newDomain.iMin = iMax - (pixelsMoved * pixelSize)
        mpf_mul_ui(newDomain.iMin, pixelSize, pixelsMoved);
        mpf_sub(newDomain.iMin, domain.iMax, newDomain.iMin);

        // Set the others
        mpf_set(newDomain.rMin, domain.rMin); mpf_set(newDomain.rMax, domain.rMax); mpf_set(newDomain.iMax, domain.iMax);
        newPixels = calculatePixels(fractal, newDomain, {res.w, pixelsMoved});
    }
    else if(mpf_cmp(domain.iMin, prev.domain.iMin) < 0) {  // Down
        //pixelSize = (iMax - iMin) / res.h;
        mpf_sub(pixelSize, domain.iMax, domain.iMin);
        mpf_div_ui(pixelSize, pixelSize, res.h);
        
        // pixelsMoved = (prev.iMin - iMin) / pixelSize
        mpf_sub(newDomain.rMin, prev.domain.iMin, domain.iMin);  // Use newDomain.rMin as temp
        mpf_div(newDomain.rMin, newDomain.rMin, pixelSize);
        const unsigned int pixelsMoved = mpf_get_d(newDomain.rMin);

        reusePixelsSrc = {0, (int)pixelsMoved, (int)res.w, (int)res.h - (int)pixelsMoved};
        reusePixelsDst = {0, 0, (int)res.w, (int)res.h - (int)pixelsMoved};
        newPixelsDst = {0, (int)res.h - (int)pixelsMoved, (int)res.w, (int)pixelsMoved};

        // newDomain.iMax = iMin + (pixelsMoved * pixelSize)
        mpf_mul_ui(newDomain.iMax, pixelSize, pixelsMoved);
        mpf_add(newDomain.iMax, domain.iMin, newDomain.iMax);

        // Set the others
        mpf_set(newDomain.rMin, domain.rMin); mpf_set(newDomain.rMax, domain.rMax); mpf_set(newDomain.iMin, domain.iMin);
        newPixels = calculatePixels(fractal, newDomain, {res.w, pixelsMoved});
    }
    else {  // Prevents warning; really shouldn't happen
        newPixels = nullptr;
        std::cout << "Something has gone terribly wrong (position same after translating)." << std::endl;
        std::cout << "Program state may be corrupted. Pressing 'h' may fix it." << std::endl;
        return;
    }

    // Make a new texture to render the screen to and set renderer to this texture
    SDL_Texture* const screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, res.h);
    SDL_SetRenderTarget(renderer, screen);
    SDL_RenderCopy(renderer, prev.pixels, &reusePixelsSrc, &reusePixelsDst);

    // SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    // SDL_RenderFillRect(renderer, &newPixelsDst);
    SDL_RenderCopy(renderer, newPixels, NULL, &newPixelsDst);
    SDL_SetRenderTarget(renderer, NULL);

    SDL_DestroyTexture(newPixels);

    // Render texture to screen
    SDL_RenderCopy(renderer, screen, NULL, NULL);

    prev.update(domain, screen);
}

void Graphics::deepenDraw(const Fractal* const fractal, const HighPrecDomain& domain, const Resolution& res) {
    SDL_Texture* const texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, res.w, res.h);
    uint32_t* pixels = new uint32_t[res.w * res.h];  //calculatePixels(fractal, domain, res);

    SDL_SetRenderTarget(renderer, prev.pixels);

    // std::clock_t start = std::clock();
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA8888, pixels, res.w * sizeof(uint32_t));

    // fractal->deepenRender(pixels, domain, res);
    for(unsigned int i = 0; i < res.w * res.h; i++) {
        if(pixels[i] != 0)
            std::cout << "kleur" << std::endl;
        else
            std::cout << "geen kleur" << std::endl;
    }

    SDL_UpdateTexture(texture, NULL, pixels, res.w * sizeof(uint32_t));
    SDL_SetRenderTarget(renderer, NULL);

    SDL_RenderCopy(renderer, texture, NULL, NULL);

    if(pixels != nullptr)
        delete[] pixels;

    prev.update(domain, texture);

    return;

    // Prevent warning
    fractal->render({mpf_get_d(domain.rMin), mpf_get_d(domain.rMax), mpf_get_d(domain.iMin), mpf_get_d(domain.iMax)}, res, {0, res.w, 0, res.h}, nullptr);
}


SDL_Texture* Graphics::calculatePixels(const Fractal* const fractal, const HighPrecDomain& domain, const Resolution& res) {
    switch(fractal->fractalType) {
        case Fractals::Mandelbrot:  return calculateMandelbrot((Mandelbrot*)fractal, domain, res);  break;
        case Fractals::Julia:       return calculateJulia((Julia*)fractal, domain, res);            break;
        case Fractals::None:        break;
    }

    std::cout << "Error: Incorrect fractal type!" << std::endl;
    return NULL;
}


SDL_Texture* Graphics::calculateMandelbrot(const Mandelbrot* const fractal, const HighPrecDomain& domain, const Resolution& res) const {
    SDL_Texture* const texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, res.h);
    ShapeVector shapes = {inCardioid, in2Bulb};  // TODO: Only add shape if in screen
    Domain lpDom = {mpf_get_d(domain.rMin), mpf_get_d(domain.rMax), mpf_get_d(domain.iMin), mpf_get_d(domain.iMax)};

    // Symmetry checking
    SDL_Rect symFrom, symTo;
    unsigned int yMin = 0;
    unsigned int yMax = res.h;
    bool sym = lpDom.iMin < 0 && lpDom.iMax > 0 && symmetry;
    if(sym) {
    //     //yAlignPixel(iMin, iMax);

        if(lpDom.iMax + lpDom.iMin >= 0) {          // Most of screen is above real axis, so calculate iMax to 0
            yMax = std::min((int)((lpDom.iMax * res.h) / (lpDom.iMax - lpDom.iMin)) + 1, (int)res.h);
            symFrom = {0, (int)(yMax - (res.h - yMax)) - 1, (int)res.w, (int)(res.h - yMax)};
            symTo = {0, (int)yMax, (int)res.w, (int)(res.h - yMax)};
        }
        else { //if(lpDom.iMax + lpDom.iMin < 0)    // Most of screen is below real axis, so calculate screenHeight to iMin
            yMin = ((lpDom.iMax * res.h) / (lpDom.iMax - lpDom.iMin)) + 1;
            symFrom = {0, (int)yMin + 1, (int)res.w, (int)yMin};
            symTo = {0, 0, (int)res.w, (int)yMin};
        }
    }

    const Range r = {0, res.w, yMin, yMax};
    // std::cout << r.yMin << ' ' << r.yMax << std::endl;
    uint32_t* pixels = nullptr;
    if(coloring == Coloring::escapeTime)
        // pixels = fractal->render(lpDom, res, r, (void*)&shapes);
        // ((Mandelbrot*)fractal)->calcScreenGMP(domain, res, r, nullptr, pixels);
        pixels = fractal->threadedRender(lpDom, res, r, (void*)&shapes);
        // fractal->calcScreen(lpDom, res, r, (void*)&shapes, pixels);
        // pixels = fractal->calcScreen(lpDom, res, r, (void*)&shapes);
    else if(coloring == Coloring::distance) {
        pixels = new uint32_t[res.w * res.h]; memset(pixels, 0x0, res.w * res.h * sizeof(uint32_t));
        fractal->calcScreenDistance(lpDom, res, r, (void*)&shapes, pixels);
        // pixels = fractal->threadedRenderBruteforce(lpDom, res, r, (void*)&shapes);
        // fractal->calcScreenBruteforce(lpDom, res, r, (void*)&shapes, pixels);
    }
        // ((Mandelbrot*)fractal)->calcScreenGMPBruteforce(domain, res, r, nullptr, pixels);
        // pixels = fractal->calcScreen(lpDom, res, r, (void*)&shapes);
        // pixels = fractal->calcScreen(lpDom, res, r, (void*)&shapes);

    if(sym) {
        SDL_Texture* const tempTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, yMax - yMin);
        SDL_UpdateTexture(tempTex, NULL, pixels + (yMin * res.w), res.w * sizeof(uint32_t));
        SDL_Rect dst = {0, (int)yMin, (int)res.w, (int)(yMax - yMin)};

        SDL_SetRenderTarget(renderer, texture);
        SDL_RenderCopy(renderer, tempTex, NULL, &dst);
        SDL_RenderCopyEx(renderer, texture, &symFrom, &symTo, 0, NULL, SDL_FLIP_VERTICAL);
        SDL_SetRenderTarget(renderer, NULL);

        SDL_DestroyTexture(tempTex);
    }
    else {
        SDL_UpdateTexture(texture, NULL, pixels, res.w * sizeof(uint32_t));
    }

    /*  ---Old symmetry copy---  */
    // // Copy to top half
    // for(unsigned int y = 0; y < yMin; y++)  //for(int y = yMin - 1; y >= 0; y--)
    //     memcpy((void*)&pixels[y * res.w], (void*)&pixels[(yMin + yMin - y + 1) * res.w], res.w * sizeof(uint32_t));

    // // Copy to bottom half
    // for(unsigned int y = yMax--; y < res.h; y++)
    //     memcpy((void*)&pixels[y * res.w], (void*)&pixels[(yMax + yMax - y) * res.w], res.w * sizeof(uint32_t));

    // SDL_UpdateTexture(texture, NULL, pixels, res.w * sizeof(uint32_t));

    if(pixels != nullptr)
        delete[] pixels;

    return texture;
}

SDL_Texture* Graphics::calculateJulia(const Julia* const fractal, const HighPrecDomain& domain, const Resolution& res) {
    SDL_Texture* const texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, res.h);
    ShapeVector shapes = {inCardioid, in2Bulb};  // TODO: Only add shape if in screen
    Domain lpDom = {mpf_get_d(domain.rMin), mpf_get_d(domain.rMax), mpf_get_d(domain.iMin), mpf_get_d(domain.iMax)};

    // Symmetry checking
    SDL_Rect symFrom, symTo;
    unsigned int yMin = 0;
    unsigned int yMax = res.h;
    bool sym = false; //lpDom.iMin < 0 && lpDom.iMax > 0 && lpDom.rMin < 0 && lpDom.rMax > 0 && symmetry;
    if(sym) {
    //     //yAlignPixel(iMin, iMax);

        if(lpDom.iMax + lpDom.iMin >= 0 && lpDom.rMax + lpDom.rMin >= 0) {          // Most of screen is above real axis, so calculate iMax to 0
            yMax = std::min((int)((lpDom.iMax * res.h) / (lpDom.iMax - lpDom.iMin)) + 1, (int)res.h);
            symFrom = {0, (int)(yMax - (res.h - yMax)) - 1, (int)res.w, (int)(res.h - yMax)};
            symTo = {0, (int)yMax, (int)res.w, (int)(res.h - yMax)};
        }
        else { //if(lpDom.iMax + lpDom.iMin < 0)    // Most of screen is below real axis, so calculate screenHeight to iMin
            yMin = ((lpDom.iMax * res.h) / (lpDom.iMax - lpDom.iMin)) + 1;
            symFrom = {0, (int)yMin + 1, (int)res.w, (int)yMin};
            symTo = {0, 0, (int)res.w, (int)yMin};
        }
    }

    const Range r = {0, res.w, yMin, yMax};
    // std::cout << r.yMin << ' ' << r.yMax << std::endl;
    uint32_t* pixels = nullptr;
    if(coloring == Coloring::escapeTime)
        // pixels = fractal->render(lpDom, res, r, (void*)&shapes);
        // ((Mandelbrot*)fractal)->calcScreenGMP(domain, res, r, nullptr, pixels);
        pixels = fractal->threadedRender(lpDom, res, r, (void*)&shapes);
        // fractal->calcScreen(lpDom, res, r, (void*)&shapes, pixels);
        // pixels = fractal->calcScreen(lpDom, res, r, (void*)&shapes);
    else if(coloring == Coloring::distance) {
        pixels = new uint32_t[res.w * res.h]; memset(pixels, 0x0, res.w * res.h * sizeof(uint32_t));
        fractal->calcScreenDistance(lpDom, res, r, (void*)&shapes, pixels);
        // pixels = fractal->threadedRenderBruteforce(lpDom, res, r, (void*)&shapes);
        // fractal->calcScreenBruteforce(lpDom, res, r, (void*)&shapes, pixels);
    }
        // ((Mandelbrot*)fractal)->calcScreenGMPBruteforce(domain, res, r, nullptr, pixels);
        // pixels = fractal->calcScreen(lpDom, res, r, (void*)&shapes);
        // pixels = fractal->calcScreen(lpDom, res, r, (void*)&shapes);

    if(sym) {
        SDL_Texture* const tempTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, yMax - yMin);
        SDL_UpdateTexture(tempTex, NULL, pixels + (yMin * res.w), res.w * sizeof(uint32_t));
        SDL_Rect dst = {0, (int)yMin, (int)res.w, (int)(yMax - yMin)};

        SDL_SetRenderTarget(renderer, texture);
        SDL_RenderCopy(renderer, tempTex, NULL, &dst);
        SDL_RenderCopyEx(renderer, texture, &symFrom, &symTo, 0, NULL, SDL_FLIP_VERTICAL);
        SDL_SetRenderTarget(renderer, NULL);

        SDL_DestroyTexture(tempTex);
    }
    else {
        SDL_UpdateTexture(texture, NULL, pixels, res.w * sizeof(uint32_t));
    }

    /*  ---Old symmetry copy---  */
    // // Copy to top half
    // for(unsigned int y = 0; y < yMin; y++)  //for(int y = yMin - 1; y >= 0; y--)
    //     memcpy((void*)&pixels[y * res.w], (void*)&pixels[(yMin + yMin - y + 1) * res.w], res.w * sizeof(uint32_t));

    // // Copy to bottom half
    // for(unsigned int y = yMax--; y < res.h; y++)
    //     memcpy((void*)&pixels[y * res.w], (void*)&pixels[(yMax + yMax - y) * res.w], res.w * sizeof(uint32_t));

    // SDL_UpdateTexture(texture, NULL, pixels, res.w * sizeof(uint32_t));

    if(pixels != nullptr)
        delete[] pixels;

    return texture;
    
    // SDL_Texture* const texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, res.h);

    // const Range r = {0, res.w, 0, res.h};
    // uint32_t* pixels = nullptr;
    // if(coloring == Coloring::escapeTime)
    //     pixels = fractal->threadedRender({mpf_get_d(domain.rMin), mpf_get_d(domain.rMax), mpf_get_d(domain.iMin), mpf_get_d(domain.iMax)}, res, r, (void*)nullptr);
    //     // pixels = fractal->calcScreen({mpf_get_d(domain.rMin), mpf_get_d(domain.rMax), mpf_get_d(domain.iMin), mpf_get_d(domain.iMax)}, res, r, (void*)nullptr);
    // else if(coloring == Coloring::distance) {
    //     pixels = new uint32_t[res.w * res.h]; memset(pixels, 0x0, res.w * res.h * sizeof(uint32_t));
    //     fractal->calcScreenDistance({mpf_get_d(domain.rMin), mpf_get_d(domain.rMax), mpf_get_d(domain.iMin), mpf_get_d(domain.iMax)}, res, r, (void*)nullptr, pixels);
    // }
    //     // pixels = fractal->threadedRenderBruteforce({mpf_get_d(domain.rMin), mpf_get_d(domain.rMax), mpf_get_d(domain.iMin), mpf_get_d(domain.iMax)}, res, r, (void*)nullptr);
    //     // pixels = fractal->render({mpf_get_d(domain.rMin), mpf_get_d(domain.rMax), mpf_get_d(domain.iMin), mpf_get_d(domain.iMax)}, res, r, (void*)nullptr);
    
    // SDL_UpdateTexture(texture, NULL, pixels, res.w * sizeof(uint32_t));

    // if(pixels != nullptr)
    //     delete[] pixels;

    // return texture;
}


void Graphics::forceRedraw() {
    prev.forceRedraw();
}


void Graphics::refresh() {
    if(prev.pixels == NULL)
        return;

    SDL_RenderCopy(renderer, prev.pixels, NULL, NULL);
    SDL_RenderPresent(renderer);
}


void Graphics::drawCurrentC(const double xRatio, const double yRatio, const Resolution& res) {
    drawClick(xRatio * res.w, yRatio * res.h);
}


void largestARcenter(SDL_Rect& zoomTo, SDL_Rect& selectionBox, const Selection* const selection, const Resolution& res) {
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
        // Increase width to fit AR
        zoomTo.w = zoomTo.h * screenAR;
        zoomTo.x = selection->xInit - (zoomTo.w / 2);
    }
    else {  // If selection is wider than screen
        // Increase height to fit AR
        zoomTo.h = zoomTo.w / screenAR;
        zoomTo.y = selection->yInit - (zoomTo.h / 2);
    }
}


void dot(int x, int y, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
    SDL_Rect dot = {x - 3, y - 3, 7, 7};
    SDL_RenderFillRect(renderer, &dot);

    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0xff, 0xff);
    dot = {x - 1, y - 1, 3, 3};
    SDL_RenderFillRect(renderer, &dot);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    dot = {x, y, 1, 1};
    SDL_RenderFillRect(renderer, &dot);
}

void Graphics::select(const Selection* const selection, const Resolution& res) {
    if(prev.pixels == NULL)
        return;
    SDL_RenderCopy(renderer, prev.pixels, NULL, NULL);

    SDL_Rect zoomTo;  // Largest box with same aspect ratio as screen fitting the selection
    SDL_Rect selectionBox;
    // largestARcenter(zoomTo, selectionBox, selection, res);
    // smallestARcenter(zoomTo, selectionBox, selection, res);
    largestARtopleft(zoomTo, selectionBox, selection, res);
    
    // SDL_SetRenderDrawColor(renderer, 0x77, 0x77, 0x77, 0xff);
    // SDL_RenderDrawRect(renderer, &selectionBox);
    
    SDL_SetRenderDrawColor(renderer, 0xee, 0xee, 0xee, 0xff);
    SDL_RenderDrawRect(renderer, &zoomTo);

    // Dot in the middle if ARcenter zooming
    // dot(selection->xInit, selection->yInit, renderer);

    // Dot in corners of ARtopleft zooming
    dot(selection->xInit, selection->yInit, renderer);
    dot(selection->xLast, selection->yLast, renderer);

    blit();
}
