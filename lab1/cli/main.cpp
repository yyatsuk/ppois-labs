#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include "Machine.h"

namespace {
const char* const kLogFlag = "-log";

const int kExitOk = 0;
const int kExitUsageError = 1;
const int kExitFileError = 2;
const int kExitFormatError = 3;
const int kExitRunError = 4;

struct Options {
    std::string path;
    bool logEnabled = false;
};

bool parseArguments(int argc, char* argv[], Options& options) {
    for (int index = 1; index < argc; ++index) {
        std::string argument = argv[index];
        if (argument == kLogFlag) {
            options.logEnabled = true;
        } else if (options.path.empty()) {
            options.path = argument;
        } else {
            return false;
        }
    }
    return !options.path.empty();
}

void printUsage() {
    std::cerr << "Usage: machines <file> [" << kLogFlag << "]\n";
}

int loadMachine(const std::string& path, Machine& machine) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Cannot open file: " << path << '\n';
        return kExitFileError;
    }
    if (!(file >> machine)) {
        std::cerr << "Invalid file format: " << path << '\n';
        return kExitFormatError;
    }
    return kExitOk;
}

void runWithLog(Machine& machine, std::ostream& out) {
    out << "start: ";
    machine.printState(out);
    out << '\n';
    std::size_t executedSteps = 0;
    while (executedSteps < Machine::kDefaultStepLimit && machine.step()) {
        ++executedSteps;
        out << "step " << executedSteps << ": ";
        machine.printState(out);
        out << '\n';
    }
}

int printResult(const Machine& machine, std::ostream& out) {
    out << "final: ";
    machine.printState(out);
    out << '\n';
    return machine.status() == MachineStatus::Stopped ? kExitOk : kExitRunError;
}
}

int main(int argc, char* argv[]) {
    Options options;
    if (!parseArguments(argc, argv, options)) {
        printUsage();
        return kExitUsageError;
    }
    Machine machine;
    int loadResult = loadMachine(options.path, machine);
    if (loadResult != kExitOk) {
        return loadResult;
    }
    if (options.logEnabled) {
        runWithLog(machine, std::cout);
    } else {
        machine.run();
    }
    return printResult(machine, std::cout);
}