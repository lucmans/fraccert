
#ifndef CONSOLE_H
#define CONSOLE_H


#include "program.h"

#include <string>
#include <vector>


typedef std::vector<std::string> Strings;


inline bool checkInt(const std::string number);
inline bool checkFloat(const std::string number);

void split(const std::string& s, Strings& tokens, const char delim);

int inputThread(void* const p);


class Console {
    public:
        Console(Program* const p, Program* const p2);
        ~Console();

        Program* getProgram() const;

        void parseC(const Strings& tokens) const;
        void parseDeepen(const Strings& tokens) const;
        void parseHelp(const Strings& tokens) const;
        void parseLine(const Strings& tokens) const;
        void parseLoc(const Strings& tokens) const;
        void parseNmax(const Strings& tokens) const;
        void parseRes(const Strings& tokens) const;
        void parseSym() const;
        void parseStop() const;
        void parseExit() const;

        void printHelp() const;

        void printHelpC() const;
        void printHelpDeepen() const;
        void printHelpHelp() const;
        void printHelpLine() const;
        void printHelpLoc() const;
        void printHelpNmax() const;
        void printHelpRes() const;
        void printHelpSym() const;
        void printHelpStop() const;
        void printHelpExit() const;

        bool startIOThread() const;



    private:
        Program* const program;
        Program* const juliaProgram;
};


#endif  // CONSOLE_H
