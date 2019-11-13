
#include "fracfast/fractals.h"
#include "locations.h"

#include <omp.h>

#include <fstream>
#include <chrono>


// typedef std::chrono::steady_clock::time_point steady_clock;
typedef std::chrono::duration<double, std::milli> duration_t;  // Omit ", std::milli" for second
#define CLOCKS std::chrono::steady_clock::time_point start; std::chrono::steady_clock::time_point end
#define ZERO std::chrono::steady_clock::duration::zero()
#define START start = std::chrono::steady_clock::now()
#define END end = std::chrono::steady_clock::now()
#define DURATION std::chrono::duration_cast<duration_t>(end - start)


const int TESTS = 25;

static const int LOCATIONS = 9;
static const Location locations[LOCATIONS] = {Locations::a, Locations::b, Locations::c, Locations::d, Locations::e, Locations::f, Locations::g, Locations::h, Locations::i};


void bruteforceSpeed() {
    std::cout << "Testing brute forcing" << std::endl;
    std::ofstream outfile("results/brute_force_home.txt", std::ofstream::app);
    CLOCKS;

    Mandelbrot* m = new Mandelbrot();
    uint32_t* p = new uint32_t[Locations::home.res.w * Locations::home.res.h];

    // Home domain
    m->setnMax(Locations::home.nMax);
    duration_t total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        memset(p, 0x0, Locations::home.res.w * Locations::home.res.h * sizeof(uint32_t));
        START;
        m->calcScreenBruteforce(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, p);
        END;
        outfile << DURATION.count() << std::endl;
        total += DURATION;
    }
    std::cout << "Home: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();


    // Average domain
    outfile.open("results/brute_force_average.txt", std::ofstream::app);
    p = new uint32_t[Locations::averageRes.w * Locations::averageRes.h];
    total = ZERO;

    duration_t sumLocs;
    for(int i = 0; i < TESTS; i++) {
        sumLocs = ZERO;
        for(int l = 0; l < LOCATIONS; l++) {
            memset(p, 0x0, Locations::averageRes.w * Locations::averageRes.h * sizeof(uint32_t));
            m->setnMax(locations[l].nMax);
            START;
            m->calcScreenBruteforce(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, p);
            END;
            sumLocs += DURATION;
            total += DURATION;
        }
        outfile << sumLocs.count() << std::endl;
    }
    std::cout << "Average: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();
    
    delete m;
    std::cout << std::endl;
}


void symRef(SDL_Renderer* const renderer) {
    std::ofstream outfile("results/symRef.txt", std::ofstream::app);
    CLOCKS;

    Mandelbrot* m = new Mandelbrot();
    m->setnMax(Locations::sym.nMax);

    Domain domain = Locations::sym.dom;
    Resolution res = Locations::sym.res;
    uint32_t* p = new uint32_t[res.w * res.h];
    memset(p, 0x0, res.w * res.h * sizeof(uint32_t));

    START;
    SDL_Texture* const texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, res.h);
    m->calcScreenBruteforce(domain, res, {0, res.w, 0, res.h}, nullptr, p);
    SDL_UpdateTexture(texture, NULL, p, res.w * sizeof(uint32_t));
    END;
    std::cout << DURATION.count() << " ms" << std::endl;
    outfile << DURATION.count() << std::endl;

    if(p != nullptr)
        delete[] p;

    SDL_DestroyTexture(texture);
    delete m;
    outfile.close();
}

void symCPU(SDL_Renderer* const renderer) {
    std::ofstream outfile("results/symCPU.txt", std::ofstream::app);
    CLOCKS;

    Mandelbrot* m = new Mandelbrot();
    m->setnMax(Locations::sym.nMax);

    Domain domain = Locations::sym.dom;
    Resolution res = Locations::sym.res;
    uint32_t* p = new uint32_t[res.w * res.h];
    memset(p, 0x0, res.w * res.h * sizeof(uint32_t));

    START;
    SDL_Texture* const texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, res.h);
    unsigned int yMin = 0;
    unsigned int yMax = res.h;
    bool sym = domain.iMin < 0 && domain.iMax > 0;
    if(sym) {
        if(domain.iMax + domain.iMin >= 0)          // Most of screen is above real axis, so calculate iMax to 0
            yMax = std::min((int)((domain.iMax * res.h) / (domain.iMax - domain.iMin)) + 1, (int)res.h);
        else //if(domain.iMax + domain.iMin < 0)    // Most of screen is below real axis, so calculate screenHeight to iMin
            yMin = ((domain.iMax * res.h) / (domain.iMax - domain.iMin)) + 1;
    }
 
    m->calcScreenBruteforce(domain, res, {0, res.w, yMin, yMax}, nullptr, p);

    // Copy to top half
    for(unsigned int y = 0; y < yMin; y++)  //for(int y = yMin - 1; y >= 0; y--)
        memcpy((void*)&p[y * res.w], (void*)&p[(yMin + yMin - y + 1) * res.w], res.w * sizeof(uint32_t));

    // Copy to bottom half
    for(unsigned int y = yMax--; y < res.h; y++)
        memcpy((void*)&p[y * res.w], (void*)&p[(yMax + yMax - y) * res.w], res.w * sizeof(uint32_t));

    SDL_UpdateTexture(texture, NULL, p, res.w * sizeof(uint32_t));
    END;
    std::cout << DURATION.count() << " ms" << std::endl;
    outfile << DURATION.count() << std::endl;

    if(p != nullptr)
        delete[] p;

    SDL_DestroyTexture(texture);
    delete m;
    outfile.close();
}

void symGPU(SDL_Renderer* const renderer) {
    std::ofstream outfile("results/symGPU.txt", std::ofstream::app);
    CLOCKS;

    Domain domain = Locations::sym.dom;
    Resolution res = Locations::sym.res;
    
    Mandelbrot* m = new Mandelbrot();
    m->setnMax(Locations::sym.nMax);
    uint32_t* p = new uint32_t[res.w * res.h];
    memset(p, 0x0, res.w * res.h * sizeof(uint32_t));

    START;
    SDL_Texture* const texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, res.h);
    SDL_Rect symFrom, symTo;
    unsigned int yMin = 0;
    unsigned int yMax = res.h;
    bool sym = domain.iMin < 0 && domain.iMax > 0;
    if(sym) {
        if(domain.iMax + domain.iMin >= 0) {          // Most of screen is above real axis, so calculate iMax to 0
            yMax = std::min((int)((domain.iMax * res.h) / (domain.iMax - domain.iMin)) + 1, (int)res.h);
            symFrom = {0, (int)(yMax - (res.h - yMax)) - 1, (int)res.w, (int)(res.h - yMax)};
            symTo = {0, (int)yMax, (int)res.w, (int)(res.h - yMax)};
        }
        else { //if(domain.iMax + domain.iMin < 0)    // Most of screen is below real axis, so calculate screenHeight to iMin
            yMin = ((domain.iMax * res.h) / (domain.iMax - domain.iMin)) + 1;
            symFrom = {0, (int)yMin + 1, (int)res.w, (int)yMin};
            symTo = {0, 0, (int)res.w, (int)yMin};
        }
    }
 
    m->calcScreenBruteforce(domain, res, {0, res.w, yMin, yMax}, nullptr, p);

    if(sym) {
        SDL_Texture* const tempTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, yMax - yMin);
        SDL_UpdateTexture(tempTex, NULL, p + (yMin * res.w), res.w * sizeof(uint32_t));
        SDL_Rect dst = {0, (int)yMin, (int)res.w, (int)(yMax - yMin)};

        SDL_SetRenderTarget(renderer, texture);
        SDL_RenderCopy(renderer, tempTex, NULL, &dst);
        SDL_RenderCopyEx(renderer, texture, &symFrom, &symTo, 0, NULL, SDL_FLIP_VERTICAL);
        SDL_SetRenderTarget(renderer, NULL);

        SDL_DestroyTexture(tempTex);
    }
    else {
        SDL_UpdateTexture(texture, NULL, p, res.w * sizeof(uint32_t));
    }
    END;
    std::cout << DURATION.count() << " ms" << std::endl;
    outfile << DURATION.count() << std::endl;

    if(p != nullptr)
        delete[] p;

    SDL_DestroyTexture(texture);
    delete m;
    outfile.close();
}

void symmetrySpeed() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Fraccert test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Locations::sym.res.w, Locations::sym.res.h, SDL_WINDOW_HIDDEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    std::cout << "Testing symmetry copying reference" << std::endl;
    for(int i = 0; i < TESTS; i++)
        symRef(renderer);
    std::cout << "Testing symmetry copying on CPU" << std::endl;
    for(int i = 0; i < TESTS; i++)
        symCPU(renderer);
    std::cout << "Testing symmetry copying on GPU" << std::endl;
    for(int i = 0; i < TESTS; i++)
        symGPU(renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << std::endl;
}


void noShapeSpeed() {
    std::cout << "Testing no shape speed" << std::endl;
    std::ofstream outfile("results/noshape_home.txt", std::ofstream::app);
    CLOCKS;

    Mandelbrot* m = new Mandelbrot();
    uint32_t* p = new uint32_t[Locations::home.res.w * Locations::home.res.h];

    // Home domain
    m->setnMax(Locations::home.nMax);
    duration_t total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        memset(p, 0x0, Locations::home.res.w * Locations::home.res.h * sizeof(uint32_t));
        START;
        m->calcScreenBruteforceNoShape(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, p);
        END;
        outfile << DURATION.count() << std::endl;
        total += DURATION;
    }
    std::cout << "Home: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();


    // Average domain
    outfile.open("results/noshape_average.txt", std::ofstream::app);
    p = new uint32_t[Locations::averageRes.w * Locations::averageRes.h];
    total = ZERO;

    duration_t sumLocs;
    for(int i = 0; i < TESTS; i++) {
        sumLocs = ZERO;
        for(int l = 0; l < LOCATIONS; l++) {
            memset(p, 0x0, Locations::averageRes.w * Locations::averageRes.h * sizeof(uint32_t));
            m->setnMax(locations[l].nMax);
            START;
            m->calcScreenBruteforceNoShape(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, p);
            END;
            sumLocs += DURATION;
            total += DURATION;
        }
        outfile << sumLocs.count() << std::endl;
    }
    std::cout << "Average: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();
    
    delete m;
    std::cout << std::endl;
}

void shapeSpeed() {
    std::cout << "Testing shape checking" << std::endl;
    std::ofstream outfile("results/shape_home.txt", std::ofstream::app);
    CLOCKS;

    ShapeVector shapes = {inCardioid, in2Bulb};
    Mandelbrot* m = new Mandelbrot();
    uint32_t* p = new uint32_t[Locations::home.res.w * Locations::home.res.h];

    // Home domain
    m->setnMax(Locations::home.nMax);
    duration_t total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        memset(p, 0x0, Locations::home.res.w * Locations::home.res.h * sizeof(uint32_t));
        START;
        m->calcScreenBruteforce(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, (void*)&shapes, p);
        END;
        outfile << DURATION.count() << std::endl;
        total += DURATION;
    }
    std::cout << "Home: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();


    // Average domain
    outfile.open("results/shape_average.txt", std::ofstream::app);
    p = new uint32_t[Locations::averageRes.w * Locations::averageRes.h];
    total = ZERO;

    duration_t sumLocs;
    for(int i = 0; i < TESTS; i++) {
        sumLocs = ZERO;
        for(int l = 0; l < LOCATIONS; l++) {
            memset(p, 0x0, Locations::averageRes.w * Locations::averageRes.h * sizeof(uint32_t));
            m->setnMax(locations[l].nMax);
            START;
            m->calcScreenBruteforce(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, (void*)&shapes, p);
            END;
            sumLocs += DURATION;
            total += DURATION;
        }
        outfile << sumLocs.count() << std::endl;
    }
    std::cout << "Average: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();
    
    delete m;
    std::cout << std::endl;
}


void borderCorrect() {
    std::cout << "Testing correctness of border tracing" << std::endl;

    Mandelbrot* m = new Mandelbrot();
    uint32_t* brute = new uint32_t[Locations::home.res.w * Locations::home.res.h];
    uint32_t* border = new uint32_t[Locations::home.res.w * Locations::home.res.h];
    memset(brute, 0x0, Locations::home.res.w * Locations::home.res.h * sizeof(uint32_t));
    memset(border, 0x0, Locations::home.res.w * Locations::home.res.h * sizeof(uint32_t));

    // Home domain
    std::cout << "Home domain:" << std::endl << "Rendering fractals with and without border tracing..." << std::flush;
    m->setnMax(Locations::home.nMax);
    m->calcScreenBruteforce(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, brute);
    m->calcScreen(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, border);

    std::cout << "\rCounting mistakes                                    " << std::endl;
    unsigned int mistakes = 0;
    for(unsigned int i = 0; i < Locations::home.res.w * Locations::home.res.h; i++) {
        if((brute[i] & 0xFFFFFF00) == (border[i] & 0xFFFFFF00))
            continue;

        // std::cout << "Mismatch at " << std::dec << i / r.w << ", " << i % r.w << ". p = " << std::hex << p[i] << "  q = " << q[i] << std::endl;
        mistakes++;
    }
    std::cout << "Mistakes = " << mistakes << " of " << Locations::home.res.w * Locations::home.res.h << " pixels (ratio " << (double)mistakes / (double)(Locations::home.res.w * Locations::home.res.h) << ")" << std::endl;

    delete[] brute;
    delete[] border;


    // Average domain
    std::cout << "\nAverage domain:" << std::endl << "Rendering fractals with and without border tracing and counting mistakes..." << std::flush;

    brute = new uint32_t[Locations::averageRes.w * Locations::averageRes.h];
    border = new uint32_t[Locations::averageRes.w * Locations::averageRes.h];
    mistakes = 0;
    for(int l = 0; l < LOCATIONS; l++) {
        memset(brute, 0x0, Locations::averageRes.w * Locations::averageRes.h * sizeof(uint32_t));
        memset(border, 0x0, Locations::averageRes.w * Locations::averageRes.h * sizeof(uint32_t));
        m->setnMax(locations[l].nMax);
        m->calcScreenBruteforce(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, brute);
        m->calcScreen(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, border);
        
        for(unsigned int i = 0; i < Locations::averageRes.w * Locations::averageRes.h; i++) {
            if((brute[i] & 0xFFFFFF00) == (border[i] & 0xFFFFFF00))
                continue;

            mistakes++;
        }
    }
    std::cout << std::endl << "Mistakes = " << mistakes << " of " << Locations::averageRes.w * Locations::averageRes.h * LOCATIONS << " pixels (ratio " << (double)mistakes / (double)(Locations::averageRes.w * Locations::averageRes.h * LOCATIONS) << ")" << std::endl;
    
    delete[] brute;
    delete[] border;

    delete m;
    std::cout << std::endl;
}

void borderSpeed() {
    std::cout << "Testing border tracing" << std::endl;
    std::ofstream outfile("results/border_trace_home.txt", std::ofstream::app);
    CLOCKS;

    Mandelbrot* m = new Mandelbrot();
    uint32_t* p = new uint32_t[Locations::home.res.w * Locations::home.res.h];

    // Home domain
    m->setnMax(Locations::home.nMax);
    duration_t total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        memset(p, 0x0, Locations::home.res.w * Locations::home.res.h * sizeof(uint32_t));
        START;
        m->calcScreen(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, p);
        END;
        outfile << DURATION.count() << std::endl;
        total += DURATION;
    }
    std::cout << "Home: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();


    // Average domain
    outfile.open("results/border_trace_average.txt", std::ofstream::app);
    p = new uint32_t[Locations::averageRes.w * Locations::averageRes.h];
    total = ZERO;

    duration_t sumLocs;
    for(int i = 0; i < TESTS; i++) {
        sumLocs = ZERO;
        for(int l = 0; l < LOCATIONS; l++) {
            memset(p, 0x0, Locations::averageRes.w * Locations::averageRes.h * sizeof(uint32_t));
            m->setnMax(locations[l].nMax);
            START;
            m->calcScreen(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, p);
            END;
            sumLocs += DURATION;
            total += DURATION;
        }
        outfile << sumLocs.count() << std::endl;
    }
    std::cout << "Average: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();
    
    delete m;
    std::cout << std::endl;
}


void multiThreads() {
    std::cout << "Testing multi threads with brute force" << std::endl;
    std::ofstream outfile;
    CLOCKS;

    Mandelbrot* m = new Mandelbrot();
    uint32_t* p = nullptr;
    duration_t total;

    // Home domain
    m->setnMax(Locations::home.nMax);
    for(int t = 1; t < 17; t++) {
        outfile.open("results/threading/threads/brute_force_home" + std::to_string(t) + ".txt", std::ofstream::app);
        
        total = ZERO;
        for(int i = 0; i < TESTS; i++) {
            START;
            p = m->threadedRenderBruteforce(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, t, 8);
            END;
            outfile << DURATION.count() << std::endl;
            total += DURATION;
            delete[] p;
        }
        std::cout << "Home " << total.count() / TESTS << " ms on average with " << t << " threads" << std::endl;

        outfile.close();
    }


    // Average domain
    duration_t sumLocs;
    for(int t = 1; t < 17; t++) {
        outfile.open("results/threading/threads/brute_force_average" + std::to_string(t) + ".txt", std::ofstream::app);
        
        total = ZERO;
        for(int i = 0; i < TESTS; i++) {
            sumLocs = ZERO;
            for(int l = 0; l < LOCATIONS; l++) {
                m->setnMax(locations[l].nMax);
                START;
                p = m->threadedRenderBruteforce(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, t, 8);
                END;
                sumLocs += DURATION;
                total += DURATION;
                delete[] p;
            }
            outfile << sumLocs.count() << std::endl;
        }
        std::cout << "Average " << total.count() / TESTS << " ms on average with " << t << " threads" << std::endl;

        outfile.close();
    }



    // Border trace
    std::cout << std::endl << "Testing multi threads with border trace" << std::endl;
    // Home domain
    m->setnMax(Locations::home.nMax);
    for(int t = 1; t < 17; t++) {
        outfile.open("results/threading/threads/border_trace_home" + std::to_string(t) + ".txt", std::ofstream::app);
        
        total = ZERO;
        for(int i = 0; i < TESTS; i++) {
            START;
            p = m->threadedRender(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, t, 8);
            END;
            outfile << DURATION.count() << std::endl;
            total += DURATION;
            delete[] p;
        }
        std::cout << "Home " << total.count() / TESTS << " ms on average with " << t << " threads" << std::endl;

        outfile.close();
    }


    // Average domain
    for(int t = 1; t < 17; t++) {
        outfile.open("results/threading/threads/border_trace_average" + std::to_string(t) + ".txt", std::ofstream::app);
        
        total = ZERO;
        for(int i = 0; i < TESTS; i++) {
            sumLocs = ZERO;
            for(int l = 0; l < LOCATIONS; l++) {
                m->setnMax(locations[l].nMax);
                START;
                p = m->threadedRender(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, t, 8);
                END;
                sumLocs += DURATION;
                total += DURATION;
                delete[] p;
            }
            outfile << sumLocs.count() << std::endl;
        }
        std::cout << "Average " << total.count() / TESTS << " ms on average with " << t << " threads" << std::endl;

        outfile.close();
    }
    
    delete m;
    std::cout << std::endl;
}

void multiSplits() {
    std::cout << "Testing multi splits with brute force" << std::endl;
    std::ofstream outfile;
    CLOCKS;

    Mandelbrot* m = new Mandelbrot();
    uint32_t* p = nullptr;
    duration_t total;

    // Home domain
    m->setnMax(Locations::home.nMax);
    for(int s = 1; s < 17; s++) {
        outfile.open("results/threading/splits/brute_force_home" + std::to_string(s) + ".txt", std::ofstream::app);
        
        total = ZERO;
        for(int i = 0; i < TESTS; i++) {
            START;
            p = m->threadedRenderBruteforce(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, 8, s);
            END;
            outfile << DURATION.count() << std::endl;
            total += DURATION;
            delete[] p;
        }
        std::cout << "Home " << total.count() / TESTS << " ms on average with " << s << " splits" << std::endl;

        outfile.close();
    }


    // Average domain
    duration_t sumLocs;
    for(int s = 1; s < 17; s++) {
        outfile.open("results/threading/splits/brute_force_average" + std::to_string(s) + ".txt", std::ofstream::app);
        
        total = ZERO;
        for(int i = 0; i < TESTS; i++) {
            sumLocs = ZERO;
            for(int l = 0; l < LOCATIONS; l++) {
                m->setnMax(locations[l].nMax);
                START;
                p = m->threadedRenderBruteforce(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, 8, s);
                END;
                sumLocs += DURATION;
                total += DURATION;
                delete[] p;
            }
            outfile << sumLocs.count() << std::endl;
        }
        std::cout << "Average " << total.count() / TESTS << " ms on average with " << s << " splits" << std::endl;

        outfile.close();
    }



    // Border trace
    std::cout << std::endl << "Testing multi splits with border trace" << std::endl;
    // Home domain
    m->setnMax(Locations::home.nMax);
    for(int s = 1; s < 17; s++) {
        outfile.open("results/threading/splits/border_trace_home" + std::to_string(s) + ".txt", std::ofstream::app);
        
        total = ZERO;
        for(int i = 0; i < TESTS; i++) {
            START;
            p = m->threadedRender(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, 8, s);
            END;
            outfile << DURATION.count() << std::endl;
            total += DURATION;
            delete[] p;
        }
        std::cout << "Home " << total.count() / TESTS << " ms on average with " << s << " splits" << std::endl;

        outfile.close();
    }


    // Average domain
    for(int s = 1; s < 17; s++) {
        outfile.open("results/threading/splits/border_trace_average" + std::to_string(s) + ".txt", std::ofstream::app);
        
        total = ZERO;
        for(int i = 0; i < TESTS; i++) {
            sumLocs = ZERO;
            for(int l = 0; l < LOCATIONS; l++) {
                m->setnMax(locations[l].nMax);
                START;
                p = m->threadedRender(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, 8, s);
                END;
                sumLocs += DURATION;
                total += DURATION;
                delete[] p;
            }
            outfile << sumLocs.count() << std::endl;
        }
        std::cout << "Average " << total.count() / TESTS << " ms on average with " << s << " splits" << std::endl;

        outfile.close();
    }
    
    delete m;
    std::cout << std::endl;
}

void multiSpeed(int threads = 8, int splits = 7) {
    std::cout << "Testing multi threads+splits with brute force" << std::endl;
    std::ofstream outfile;
    CLOCKS;

    Mandelbrot* m = new Mandelbrot();
    uint32_t* p = nullptr;
    duration_t total;

    // Home domain
    m->setnMax(Locations::home.nMax);
    outfile.open("results/threading/multi_brute_force_home.txt", std::ofstream::app);
    
    total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        START;
        p = m->threadedRenderBruteforce(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, threads, splits);
        END;
        outfile << DURATION.count() << std::endl;
        total += DURATION;
        delete[] p;
    }
    std::cout << "Home " << total.count() / TESTS << " ms on average with " << threads << " threads and " << splits << " splits" << std::endl;

    outfile.close();


    // Average domain
    duration_t sumLocs;
    outfile.open("results/threading/multi_brute_force_average.txt", std::ofstream::app);
    
    total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        sumLocs = ZERO;
        for(int l = 0; l < LOCATIONS; l++) {
            m->setnMax(locations[l].nMax);
            START;
            p = m->threadedRenderBruteforce(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, threads, splits);
            END;
            sumLocs += DURATION;
            total += DURATION;
            delete[] p;
        }
        outfile << sumLocs.count() << std::endl;
    }
    std::cout << "Average " << total.count() / TESTS << " ms on average with " << threads << " threads and " << splits << " splits" << std::endl;

    outfile.close();



    // Border trace
    std::cout << std::endl << "Testing multi threads+splits with border trace" << std::endl;
    // Home domain
    m->setnMax(Locations::home.nMax);
    outfile.open("results/threading/multi_border_trace_home.txt", std::ofstream::app);
    
    total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        START;
        p = m->threadedRender(Locations::home.dom, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, threads, splits);
        END;
        outfile << DURATION.count() << std::endl;
        total += DURATION;
        delete[] p;
    }
    std::cout << "Home " << total.count() / TESTS << " ms on average with " << threads << " threads and " << splits << " splits" << std::endl;

    outfile.close();


    // Average domain
    outfile.open("results/threading/multi_border_trace_average.txt", std::ofstream::app);
    
    total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        sumLocs = ZERO;
        for(int l = 0; l < LOCATIONS; l++) {
            m->setnMax(locations[l].nMax);
            START;
            p = m->threadedRender(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, threads, splits);
            END;
            sumLocs += DURATION;
            total += DURATION;
            delete[] p;
        }
        outfile << sumLocs.count() << std::endl;
    }
    std::cout << "Average " << total.count() / TESTS << " ms on average with " << threads << " threads and " << splits << " splits" << std::endl;

    outfile.close();
    
    delete m;
    std::cout << std::endl;
}


void allSpeeds() {
    std::cout << "Testing everything" << std::endl;
    std::ofstream outfile("results/all_home.txt", std::ofstream::app);
    CLOCKS;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Fraccert test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Locations::sym.res.w, Locations::sym.res.h, SDL_WINDOW_HIDDEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    Domain domain;
    Resolution res;

    ShapeVector shapes = {inCardioid, in2Bulb};
    Mandelbrot* m = new Mandelbrot();
    uint32_t* p = nullptr;

    // Home domain
    domain = Locations::home.dom;
    res = Locations::home.res;
    m->setnMax(Locations::home.nMax);
    duration_t total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        START;
        SDL_Texture* const texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, res.h);
        SDL_Rect symFrom, symTo;
        unsigned int yMin = 0;
        unsigned int yMax = res.h;
        bool sym = domain.iMin < 0 && domain.iMax > 0;
        if(sym) {
            if(domain.iMax + domain.iMin >= 0) {          // Most of screen is above real axis, so calculate iMax to 0
                yMax = std::min((int)((domain.iMax * res.h) / (domain.iMax - domain.iMin)) + 1, (int)res.h);
                symFrom = {0, (int)(yMax - (res.h - yMax)) - 1, (int)res.w, (int)(res.h - yMax)};
                symTo = {0, (int)yMax, (int)res.w, (int)(res.h - yMax)};
            }
            else { //if(domain.iMax + domain.iMin < 0)    // Most of screen is below real axis, so calculate screenHeight to iMin
                yMin = ((domain.iMax * res.h) / (domain.iMax - domain.iMin)) + 1;
                symFrom = {0, (int)yMin + 1, (int)res.w, (int)yMin};
                symTo = {0, 0, (int)res.w, (int)yMin};
            }
        }

        p = m->threadedRender(domain, res, {0, res.w, yMin, yMax}, (void*)&shapes, 8, 7);

        if(sym) {
            SDL_Texture* const tempTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res.w, yMax - yMin);
            SDL_UpdateTexture(tempTex, NULL, p + (yMin * res.w), res.w * sizeof(uint32_t));
            SDL_Rect dst = {0, (int)yMin, (int)res.w, (int)(yMax - yMin)};

            SDL_SetRenderTarget(renderer, texture);
            SDL_RenderCopy(renderer, tempTex, NULL, &dst);
            SDL_RenderCopyEx(renderer, texture, &symFrom, &symTo, 0, NULL, SDL_FLIP_VERTICAL);
            SDL_SetRenderTarget(renderer, NULL);

            SDL_DestroyTexture(tempTex);
        }
        else {
            SDL_UpdateTexture(texture, NULL, p, res.w * sizeof(uint32_t));
        }

        END;
        outfile << DURATION.count() << std::endl;
        total += DURATION;
        delete[] p;
        SDL_DestroyTexture(texture);
    }
    std::cout << "Home: " << total.count() / TESTS << " ms on average" << std::endl;

    outfile.close();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    // Average domain
    outfile.open("results/all_average.txt", std::ofstream::app);
    p = new uint32_t[Locations::averageRes.w * Locations::averageRes.h];
    total = ZERO;

    duration_t sumLocs;
    for(int i = 0; i < TESTS; i++) {
        sumLocs = ZERO;
        for(int l = 0; l < LOCATIONS; l++) {
            m->setnMax(locations[l].nMax);
            START;
            p = m->threadedRender(locations[l].dom, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, (void*)&shapes, 8, 7);
            END;
            sumLocs += DURATION;
            total += DURATION;
            delete[] p;
        }
        outfile << sumLocs.count() << std::endl;
    }
    std::cout << "Average: " << total.count() / TESTS << " ms on average" << std::endl;

    outfile.close();
    
    delete m;
    std::cout << std::endl;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
void doublePrec() {
    int n;

    float f = 1.0;
    for(n = 0; 1.0 + f != 1.0; n++)
        f /= 2;
    std::cout << "Float has " << n << " bits precision" << std::endl;

    double d = 1.0;
    for(n = 0; 1.0 + d != 1.0; n++)
        d /= 2;
    std::cout << "Double has " << n << " bits precision" << std::endl;

    long double ld = 1.0;
    for(n = 0; 1.0 + ld != 1.0; n++)
        ld /= 2;
    std::cout << "Long double has " << n << " bits precision" << std::endl;


    // float a = 1.f / 81;
    // float b = 0;
    // for (int i = 0; i < 729; ++ i)
    //     b += a;
    // printf("%.15g\n", b); // prints 9.000023

    // double c = 1.0 / 81;
    // double d = 0;
    // for (int i = 0; i < 729; ++ i)
    //     d += c;
    // printf("%.15g\n", d); // prints 8.99999999999996
}
#pragma GCC diagnostic pop

void gmpBruteforceSpeed(int prec = 0) {
    mpf_set_default_prec(prec);

    // Brute force
    std::cout << "Testing GMP, brute force with " << mpf_get_default_prec() << " bits precision" << std::endl;
    std::ofstream outfile("results/gmp/brute_force_home" + std::to_string(mpf_get_default_prec()) + ".txt", std::ofstream::app);
    CLOCKS;

    Mandelbrot* m = new Mandelbrot();
    uint32_t* p = new uint32_t[Locations::home.res.w * Locations::home.res.h];

    HighPrecDomain d;
    mpf_inits(d.rMin, d.rMax, d.iMin, d.iMax, NULL);

    // Home domain
    mpf_set_d(d.rMin, Locations::home.dom.rMin); mpf_set_d(d.rMax, Locations::home.dom.rMax); mpf_set_d(d.iMin, Locations::home.dom.iMin); mpf_set_d(d.iMax, Locations::home.dom.iMax);
    m->setnMax(Locations::home.nMax);
    duration_t total = ZERO;
    // for(int i = 0; i < TESTS; i++) {
        memset(p, 0x0, Locations::home.res.w * Locations::home.res.h * sizeof(uint32_t));
        START;
        m->calcScreenGMPBruteforce(d, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, p);
        END;
        outfile << DURATION.count() << std::endl;
        total += DURATION;
    // }
    std::cout << "Home: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();


    // Average domain
    outfile.open("results/gmp/brute_force_average" + std::to_string(mpf_get_default_prec()) + ".txt", std::ofstream::app);
    p = new uint32_t[Locations::averageRes.w * Locations::averageRes.h];
    total = ZERO;

    duration_t sumLocs;
    // for(int i = 0; i < TESTS; i++) {
        sumLocs = ZERO;
        for(int l = 0; l < LOCATIONS; l++) {
            memset(p, 0x0, Locations::averageRes.w * Locations::averageRes.h * sizeof(uint32_t));
            mpf_set_d(d.rMin, locations[l].dom.rMin); mpf_set_d(d.rMax, locations[l].dom.rMax); mpf_set_d(d.iMin, locations[l].dom.iMin); mpf_set_d(d.iMax, locations[l].dom.iMax);
            m->setnMax(locations[l].nMax);
            START;
            m->calcScreenGMPBruteforce(d, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, p);
            END;
            sumLocs += DURATION;
            total += DURATION;
        }
        outfile << sumLocs.count() << std::endl;
    // }
    std::cout << "Average: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();

    delete m;
    mpf_clears(d.rMin, d.rMax, d.iMin, d.iMax, NULL);
    std::cout << std::endl;
}

void gmpBordertraceSpeed(int prec = 0) {
    mpf_set_default_prec(prec);

    std::cout << "Testing GMP, border tracing with " << mpf_get_default_prec() << " bits precision" << std::endl;
    std::ofstream outfile("results/gmp/border_trace_home" + std::to_string(mpf_get_default_prec()) + ".txt", std::ofstream::app);
    CLOCKS;

    Mandelbrot* m = new Mandelbrot();
    uint32_t* p = new uint32_t[Locations::home.res.w * Locations::home.res.h];

    HighPrecDomain d;
    mpf_inits(d.rMin, d.rMax, d.iMin, d.iMax, NULL);

    // Home domain
    p = new uint32_t[Locations::home.res.w * Locations::home.res.h];
    mpf_set_d(d.rMin, Locations::home.dom.rMin); mpf_set_d(d.rMax, Locations::home.dom.rMax); mpf_set_d(d.iMin, Locations::home.dom.iMin); mpf_set_d(d.iMax, Locations::home.dom.iMax);
    m->setnMax(Locations::home.nMax);
    duration_t total = ZERO;
    // for(int i = 0; i < TESTS; i++) {
        memset(p, 0x0, Locations::home.res.w * Locations::home.res.h * sizeof(uint32_t));
        START;
        m->calcScreenGMP(d, Locations::home.res, {0, Locations::home.res.w, 0, Locations::home.res.h}, nullptr, p);
        END;
        outfile << DURATION.count() << std::endl;
        total += DURATION;
    // }
    std::cout << "Home: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();


    // Average domain
    outfile.open("results/gmp/border_trace_average" + std::to_string(mpf_get_default_prec()) + ".txt", std::ofstream::app);
    p = new uint32_t[Locations::averageRes.w * Locations::averageRes.h];
    total = ZERO;

    // for(int i = 0; i < TESTS; i++) {
        duration_t sumLocs = ZERO;
        for(int l = 0; l < LOCATIONS; l++) {
            memset(p, 0x0, Locations::averageRes.w * Locations::averageRes.h * sizeof(uint32_t));
            mpf_set_d(d.rMin, locations[l].dom.rMin); mpf_set_d(d.rMax, locations[l].dom.rMax); mpf_set_d(d.iMin, locations[l].dom.iMin); mpf_set_d(d.iMax, locations[l].dom.iMax);
            m->setnMax(locations[l].nMax);
            START;
            m->calcScreenGMP(d, Locations::averageRes, {0, Locations::averageRes.w, 0, Locations::averageRes.h}, nullptr, p);
            END;
            sumLocs += DURATION;
            total += DURATION;
        }
        outfile << sumLocs.count() << std::endl;
    // }
    std::cout << "Average: " << total.count() / TESTS << " ms on average" << std::endl;

    delete[] p;
    outfile.close();

    delete m;
    mpf_clears(d.rMin, d.rMax, d.iMin, d.iMax, NULL);
    std::cout << std::endl;
}


void gmpFractalSpeedAll(int t) {
    // Get step size of GMP precision
    mpf_set_default_prec(0);
    const int step = mpf_get_default_prec();

    // Perform test with every precision in [step, 1024]
    for(int i = 1; i < 1025; i += step) {
        std::cout << "Test " << t << std::endl;
        gmpBruteforceSpeed(i);
        gmpBordertraceSpeed(i);
    }
}


void lowPrecScale() {
    const double scaleFactor = 0.8;
    const int x = 250, y = 350;
    const int w = 600, h = 800;
    double rMin = -2, rMax = 1, iMin = -2, iMax = 2;

    const double xRatio = x / (double)w,
                 yRatio = y / (double)h;

    double dReal = rMax - rMin,
           dImag = iMax - iMin;

    // if(scaleDirection == 1) {
    //     dReal = (scaleFactor * dReal) - dReal;
    //     dImag = (scaleFactor * dImag) - dImag;
    // }
    // else {
        dReal = ((1 / scaleFactor) * dReal) - dReal;
        dImag = ((1 / scaleFactor) * dImag) - dImag;
    // }

    // Domain oldDomain = {rMin, rMax, iMin, iMax};
    rMin = rMin - (xRatio * dReal);
    rMax = rMax + ((1.0 - xRatio) * dReal);
    iMax = iMax + (yRatio * dImag);
    iMin = iMin - ((1.0 - yRatio) * dImag);
}


mpf_t rMin, rMax, iMin, iMax, xRatio, yRatio, dReal, dImag, t, scaleFactor;
void highPrecScale() {
    const int x = 250, y = 350;
    const int w = 600, h = 800;

    // dReal = rMax - rMin, dImag = iMax - iMin;
    mpf_sub(dReal, rMax, rMin);
    mpf_sub(dImag, iMax, iMin);

    // dReal = ((1 / scaleFactor) * dReal) - dReal;
    mpf_ui_div(xRatio, 1, scaleFactor);  // Use xRatio as extra temp
    mpf_mul(t, xRatio, dReal);
    mpf_sub(dReal, t, dReal);

    // dImag = ((1 / scaleFactor) * dImag) - dImag;
    // xRatio (temp) is still set correctly
    mpf_mul(t, xRatio, dImag);
    mpf_sub(dImag, t, dImag);

    // xRatio = x / (double)w, yRatio = y / (double)h;
    mpf_set_d(xRatio, x / (double)w);
    mpf_set_d(yRatio, y / (double)h);

    // rMin = rMin - (xRatio * dReal);
    mpf_mul(t, xRatio, dReal);
    mpf_sub(rMin, rMin, t);

    // rMax = rMax + ((1.0 - xRatio) * dReal);
    mpf_ui_sub(t, 1, xRatio);
    mpf_mul(t, t, dReal);
    mpf_add(rMax, rMax, t);

    // iMax = iMax + (yRatio * dImag);
    mpf_mul(t, yRatio, dImag);
    mpf_add(iMax, iMax, t);

    // iMin = iMin - ((1.0 - yRatio) * dImag);
    mpf_ui_sub(t, 1, yRatio);
    mpf_mul(t, t, dImag);
    mpf_sub(iMin, iMin, t);
}

void gmpScaleSpeed() {
    std::cout << "Testing scaling using normal and GMP floats." << std::endl;
    std::ofstream outfile("results/gmpScale.txt", std::ofstream::app);
    CLOCKS;

    mpf_inits(rMin, rMax, iMin, iMax, xRatio, yRatio, dReal, dImag, t, scaleFactor, NULL);
    mpf_set_d(rMin, -2.0); mpf_set_d(rMax, 1.0); mpf_set_d(iMin, -2.0); mpf_set_d(iMax, 2.0); mpf_set_d(scaleFactor, 0.8);

    duration_t total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        START;
        lowPrecScale();
        END;
        outfile << DURATION.count() << " 0" << std::endl;
        total += DURATION;
    }
    std::cout << total.count() / TESTS << " ms" << std::endl;

    mpf_set_default_prec(1);
    total = ZERO;
    for(int i = 0; i < TESTS; i++) {
        START;
        highPrecScale();
        END;
        outfile << DURATION.count() << " " << mpf_get_default_prec() << std::endl;
        total += DURATION;
    }
    std::cout << total.count() / TESTS << " ms" << std::endl;

    mpf_clears(rMin, rMax, iMin, iMax, xRatio, yRatio, dReal, dImag, t, scaleFactor, NULL);
    outfile.close();
    std::cout << std::endl;
}


/*
void lowPrecSetDom() {
    const int x = 600, y = 800;
    double rMin = -2, rMax = 1, iMin = -2, iMax = 2;

    const double iCenter = (iMax + iMin) / 2.0;
    const double iHeight = (rMax - rMin) * (y / (double)x);

    rMin = rMin;
    rMax = rMax;
    iMin = iCenter - (iHeight / 2.0);
    iMax = iCenter + (iHeight / 2.0);
}

// Alias to existing mpf_t to save initializations
mpf_t& iCenter = xRatio; mpf_t& iHeight = yRatio;
void highPrecSetDom() {
    const int x = 600, y = 800;

    // const double iCenter = (iMax + iMin) / 2.0;
    mpf_add(t, iMax, iMin);
    mpf_div_ui(iCenter, t, 2);

    // const double iHeight = (rMax - rMin) * (y / (double)x);
    mpf_sub(t, rMax, rMin);
    mpf_set_d(iHeight, y / (double)x);
    mpf_mul(iHeight, t, iHeight);

    // rMin = rMin; rMax = rMax;
    mpf_set(rMin, rMin);
    mpf_set(rMax, rMax);

    // iMin = iCenter - (iHeight / 2.0);
    mpf_div_ui(t, iHeight, 2);
    mpf_sub(iMin, iCenter, t);

    // iMax = iCenter + (iHeight / 2.0);
    // t still set correctly
    mpf_add(iMax, iCenter, t);
}

void speedPrecSetDom() {
    mpf_inits(rMin, rMax, iMin, iMax, iCenter, iHeight, t, NULL);
    mpf_set_d(rMin, -2.0); mpf_set_d(rMax, 1.0); mpf_set_d(iMin, -2.0); mpf_set_d(iMax, 2.0); mpf_set_d(scaleFactor, 0.8);

    std::clock_t low = std::clock();
    lowPrecSetDom();
    double lowTime = (std::clock() - low) / (double)(CLOCKS_PER_SEC / 1000);
    std::cout << "Time: " << lowTime << " ms  with " << sizeof(double) * 8 << " bits precision" << std::endl;

    std::clock_t high = std::clock();
    highPrecSetDom();
    double highTime = (std::clock() - high) / (double)(CLOCKS_PER_SEC / 1000);
    std::cout << "Time: " << highTime << " ms  with " << mpf_get_default_prec() << " bits precision" << std::endl;

    std::cout << "High precision speeddown = " << highTime / lowTime << std::endl;

    mpf_clears(rMin, rMax, iMin, iMax, iCenter, iHeight, t, NULL);
}*/
