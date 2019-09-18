
#include "fracfast/types.h"
#include "console.h"
#include "locations.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <csignal>


void signalHandler(const int signum) {
    std::cout << "Interrupt signal (" << signum << ") received." << std::endl;

    // cleanup and close up stuff here
    // terminate program

    exit(signum);
}


inline bool checkInt(const std::string number) {
    char* end;
    strtol(number.c_str(), &end, 10);
    return *end == 0;
}

inline bool checkFloat(const std::string number) {
    char* end;
    strtod(number.c_str(), &end);
    return *end == 0;
}


void split(const std::string& s, Strings& tokens, const char delim) {
    std::stringstream ss(s);
    std::string token;

    while(std::getline(ss, token, delim)) {
        tokens.push_back(std::move(token));  // move() is faster in this case
    }
}


int inputThread(void* const c) {
    signal(SIGINT, signalHandler);

    const Console* const console = (Console*)c;
    std::string read;
    Strings tokens;
    
    console->printHelp();
    while(true){
        std::cout << "$ ";

        std::getline(std::cin, read);

        tokens.clear();
        split(read, tokens, ' ');

        if(tokens.size() == 0)
            continue;

        else if(tokens[0].compare("c") == 0)
            console->parseC(tokens);

        // else if(tokens[0].compare("deepen") == 0)
        //     console->parseDeepen(tokens);

        else if(tokens[0].compare("help") == 0)
            console->parseHelp(tokens);

        else if(tokens[0].compare("loc") == 0)
            console->parseLoc(tokens);

        else if(tokens[0].compare("nmax") == 0)
            console->parseNmax(tokens);

        else if(tokens[0].compare("line") == 0)
            console->parseLine(tokens);

        else if(tokens[0].compare("res") == 0)
            console->parseRes(tokens);

        else if(tokens[0].compare("sym") == 0)
            console->parseSym();

        else if(tokens[0].compare("stop") == 0)
            console->parseStop();

        else if(tokens[0].compare("exit") == 0)
            console->parseExit();

        else
            std::cout << "Invalid command." << std::endl
                      << "Use help for a list of command and help <command> for info about command" << std::endl;
    }
}


Console::Console(Program* const p, Program* const p2) : program(p), juliaProgram(p2) {
    
}

Console::~Console() {

}


Program* Console::getProgram() const {
    return program;
}


void Console::parseC(const Strings& tokens) const {
    // TODO: High precision support
    if(tokens.size() == 1) {
        double c[2];
        program->getC(c);

        std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1) << "c = [" << c[0] << ", " << c[1] << "]" << std::endl;
    }
    else if(tokens.size() == 3) {
        if(!checkFloat(tokens[1]) || !checkFloat(tokens[2])) {
            std::cout << "One or more of the arguments is invaled." << std::endl;
            return;
        }

        const double c[2] = {atof(tokens[1].c_str()), atof(tokens[2].c_str())};
        program->setC(c);
        std::cout << "Set c to [" << c[0] << ", " << c[1] << "]" << std::endl;
    }
    else
        std::cout << "Invalid number of arguments" << std::endl;
}


void Console::parseDeepen(const Strings& tokens) const {
    if(tokens.size() == 1) {
        std::cout << "Deepen = " << program->getDeepen() << std::endl;
    }
    else if(tokens.size() == 2) {
        int n = atoi(tokens[1].c_str());
        if(n < 1)
            std::cout << "Error: Value to small (< 1)." << std::endl;
        else
            program->setDeepen(n);
    }
}


void Console::parseHelp(const Strings& tokens) const {
    if(tokens.size() == 1)
        printHelp();

    else if(tokens.size() == 2) {
        if(tokens[1].compare("c") == 0)
            printHelpC();
        else if(tokens[1].compare("deepen") == 0)
            printHelpDeepen();
        else if(tokens[1].compare("help") == 0)
            printHelpHelp();
        else if(tokens[1].compare("loc") == 0)
            printHelpLoc();
        else if(tokens[1].compare("nmax") == 0)
            printHelpNmax();
        else if(tokens[1].compare("line") == 0)
            printHelpLine();
        else if(tokens[1].compare("res") == 0)
            printHelpRes();
        else if(tokens[1].compare("sym") == 0)
            printHelpSym();
        else if(tokens[1].compare("stop") == 0)
            printHelpStop();
        else if(tokens[1].compare("exit") == 0)
            printHelpExit();
    }
}

void Console::parseLine(const Strings& tokens) const {
    if(tokens.size() == 1) {
        //getLineWidth();
    }
    else if(tokens.size() == 2) {
        const int n = atoi(tokens[1].c_str());
        if(n < 1)
            std::cout << "Invalid value" << std::endl;
        else
            program->setLineDetail(n);
    }
    else
        std::cout << "Invalid number of arguments" << std::endl;
}

void Console::parseLoc(const Strings& tokens) const {
    // TODO: High precision support
    // std::cout << "Currently broken..." << std::endl;

    if(tokens.size() == 1) {
        const Domain d = program->getDomain();
        std::cout << std::setprecision(std::numeric_limits<double>::digits10 + 1) << "Re = [" << d.rMin << ", " << d.rMax << ']' << "  Im = [" << d.iMin << ", " << d.iMax << ']' << std::endl;
    }
    else if(tokens.size() == 2) {
        if(tokens[1].compare("limit") == 0) {
            program->setnMax(Locations::limit.nMax, false);
            program->setDomain(Locations::limit.dom);
        }
        else if(tokens[1].compare("a") == 0) {
            program->setnMax(Locations::a.nMax, false);
            program->setDomain(Locations::a.dom);
        }
        else if(tokens[1].compare("b") == 0) {
            program->setnMax(Locations::b.nMax, false);
            program->setDomain(Locations::b.dom);
        }
        else if(tokens[1].compare("c") == 0) {
            program->setnMax(Locations::c.nMax, false);
            program->setDomain(Locations::c.dom);
        }
        else if(tokens[1].compare("d") == 0) {
            program->setnMax(Locations::d.nMax, false);
            program->setDomain(Locations::d.dom);
        }
        else if(tokens[1].compare("e") == 0) {
            program->setnMax(Locations::e.nMax, false);
            program->setDomain(Locations::e.dom);
        }
        else if(tokens[1].compare("f") == 0) {
            program->setnMax(Locations::f.nMax, false);
            program->setDomain(Locations::f.dom);
        }
        else if(tokens[1].compare("g") == 0) {
            program->setnMax(Locations::g.nMax, false);
            program->setDomain(Locations::g.dom);
        }
        else if(tokens[1].compare("h") == 0) {
            program->setnMax(Locations::h.nMax, false);
            program->setDomain(Locations::h.dom);
        }
        else if(tokens[1].compare("i") == 0) {
            program->setnMax(Locations::i.nMax, false);
            program->setDomain(Locations::i.dom);
        }
        else {
            std::cout << "Unknown location." << std::endl;
        }
    }
    else if(tokens.size() == 4) {
        for(int i = 1; i < 4; i++) {
            if(!checkFloat(tokens[i])) {
                std::cout << "One or more of the arguments is invaled." << std::endl;
                return;
            }
        }

        Resolution res;
        program->getResolution(res);

        Domain d = {atof(tokens[1].c_str()), atof(tokens[2].c_str()), 0, 0};
        const double iHeight = (d.rMax - d.rMin) * (res.h / (double)res.w);
        d.iMax = (iHeight / 2.0) + atof(tokens[3].c_str());
        d.iMin = (iHeight / -2.0) + atof(tokens[3].c_str());

        if(program->setDomain(d)) {
            d = program->getDomain();
            std::cout << "Set loc to [" << d.rMin << ", " << d.rMax << "]  [" << d.iMin << ", " << d.iMax << "]" << std::endl;
        }
        else {
            std::cout << "Location not valid." << std::endl;
        }
    }
    else if(tokens.size() == 5) {
        for(int i = 1; i < 5; i++) {
            if(!checkFloat(tokens[i])) {
                std::cout << "One or more of the arguments is invaled." << std::endl;
                return;
            }
        }

        Domain d;
        d = {atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str()), atof(tokens[4].c_str())};

        if(program->setDomain(d)) {
            d = program->getDomain();
            std::cout << "Set loc to [" << d.rMin << ", " << d.rMax << "]  [" << d.iMin << ", " << d.iMax << "]" << std::endl;
        }
        else {
            std::cout << "Location not valid." << std::endl;
        }
    }
    else
        std::cout << "Invalid number of arguments" << std::endl;
}

void Console::parseNmax(const Strings& tokens) const {
    if(tokens.size() == 1) {
        std::cout << "NMAX = " << program->getnMax() - 2 << std::endl;
    }
    else if(tokens.size() == 2) {
        if(!checkInt(tokens[1])) {
            std::cout << "Argument is not a valid number" << std::endl;
            return;
        }

        const int n = atoi(tokens[1].c_str());
        if(n < 0)
            std::cout << "Error: Value to small (< 0)" << std::endl;
        else
            program->setnMax(n + 2);
    }
    else if(tokens.size() == 3 && tokens[2].compare("notick") == 0) {
        if(!checkInt(tokens[1])) {
            std::cout << "Argument is not a valid number" << std::endl;
            return;
        }

        const int n = atoi(tokens[1].c_str());
        if(n < 0)
            std::cout << "Error: Value to small (< 0)" << std::endl;
        else
            program->setnMax(n + 2, false);
    }
    else
        std::cout << "Invalid number of arguments" << std::endl;
}

void Console::parseRes(const Strings& tokens) const {
    if(tokens.size() == 1) {
        Resolution res;
        program->getResolution(res);
        std::cout << "Res = " << res.w << ", " << res.h << std::endl;
    }
    // else if(tokens.size() == 2 && tokens[2].compare("reset") == 0) {

    // }
    // else if(tokens.size() == 3) {
    //     if(!checkInt(tokens[1]) || !checkInt(tokens[2])) {
    //         std::cout << "One or more of the arguments is invaled." << std::endl;
    //         return;
    //     }

    //     unsigned int w = atoi(tokens[1].c_str());
    //     unsigned int h = atoi(tokens[1].c_str());
    //     Resolution res = {w, h};
    //     program->resize(res);
    // }
    else
        std::cout << "Invalid number of arguments" << std::endl;
}

void Console::parseSym() const {
    program->toggleSymmetry();
}

void Console::parseStop() const {
    SDL_FlushEvent(SDL_MOUSEWHEEL);
    SDL_FlushEvent(SDL_KEYDOWN);
    SDL_FlushEvent(SDL_KEYUP);
    SDL_FlushEvent(SDL_MOUSEMOTION);
}

void Console::parseExit() const {
    exit(-1);
}


void Console::printHelp() const {
    printHelpC();
    // printHelpDeepen();
    printHelpHelp();
    printHelpLoc();
    printHelpNmax();
    printHelpLine();
    printHelpSym();
    printHelpStop();
    printHelpExit();
    std::cout << std::endl;
}


void Console::printHelpC() const {
    std::cout << "  - c\n"
              << "        Prints the currect value for c\n"
              << '\n'
              << "  - c <real> <imag>\n"
              << "        Sets c to [real, imag]\n"
              << '\n';
}

void Console::printHelpDeepen() const {
    std::cout << "  - deepen\n"
              << "        Prints current deepen stepsize\n"
              << '\n'
              << "  - deepen <d>\n"
              << "        Sets deepen stepsize to d\n"
              << '\n';
}

void Console::printHelpHelp() const {
    std::cout << "  - help\n"
              << "        Prints the help for all commands\n"
              << '\n'
              << "  - help <command>\n"
              << "        Prints the help for command\n"
              << '\n';
}

void Console::printHelpLoc() const {
    std::cout << "  - loc\n"
              << "        Prints the current location\n"
              << '\n'
              << "  - loc limit\n"
              << "        Sets location to close to the limit of double precision\n"
              << '\n'
              << "  - loc <rMin> <rMax> <iBase>\n"
              << "        Sets location to [rMin, rMax] and centered around iBase\n"
              << '\n'
              << "  - loc <rMin> <rMax>  <iMin> <iMax>\n"
              << "        Sets location to [rMin, rMax]  [iMin, iMax]\n"
              << '\n';
}

void Console::printHelpNmax() const {
    std::cout << "  - nmax\n"
              << "        Gets NMAX\n"
              << '\n'
              << "  - nmax <n>\n"
              << "        Sets NMAX to n\n"
              << "        If third argument is 'notick', the fractal won't be rendered after setting NMAX\n"
              << '\n';
}

void Console::printHelpLine() const {
    std::cout << "  - line\n"
              << "        Sets location to close to the limit of double precision\n"
              << '\n'
              << "  - line <lineDetail>\n"
              << "        Sets distance exterior line detail to lineDetail\n"
              << "        Higher values for lineDetail make the line thinner\n"
              << '\n';
}

void Console::printHelpRes() const {
    std::cout << "  - res\n"
              << "        Prints current resolution\n"
              << '\n';
              // << "  - res <width> <height>\n"
              // << "        Sets the resolution to width by height\n"
              // << '\n';
}

void Console::printHelpSym() const {
    std::cout << "  - sym\n"
              << "        Toggles symmetry\n"
              << '\n';
}

void Console::printHelpStop() const {
    std::cout << "  - stop\n"
              << "        Removes all queued events\n"
              << '\n';
}

void Console::printHelpExit() const {
    std::cout << "  - exit\n"
              << "        Forces exit with exitcode -1\n"
              << '\n' << std::endl;
}



bool Console::startIOThread() const {
    SDL_Thread* const thread = SDL_CreateThread(inputThread, "consoleThread", (void *)this);
    SDL_DetachThread(thread);
    return thread != NULL;
}
