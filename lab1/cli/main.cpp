#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include "Machine.h"

namespace {
const char* const kLogFlag = "-log";
const char* const kMenuFlag = "--menu";

enum ExitCode { kOk = 0, kUsageError = 1, kFileError = 2, kFormatError = 3, kRunError = 4 };

struct Options {
    std::string path;
    bool logEnabled = false;
    bool menuEnabled = false;
};

bool parseArguments(int argc, char* argv[], Options& options) {
    for (int i = 1; i < argc; ++i) {
        std::string argument = argv[i];
        if (argument == kLogFlag) {
            options.logEnabled = true;
        } else if (argument == kMenuFlag) {
            options.menuEnabled = true;
        } else if (options.path.empty()) {
            options.path = argument;
        } else {
            return false;
        }
    }
    return !options.path.empty();
}

void printUsage() {
    std::cerr << "Usage: machines <file> [" << kLogFlag << "] [" << kMenuFlag << "]\n";
}

void printState(const Machine& machine, const char* prefix) {
    std::cout << prefix;
    machine.printState(std::cout);
    std::cout << '\n';
}

int loadMachine(const std::string& path, Machine& machine) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Cannot open file: " << path << '\n';
        return kFileError;
    }
    if (!(file >> machine)) {
        std::cerr << "Invalid file format: " << path << '\n';
        return kFormatError;
    }
    return kOk;
}

void runAll(Machine& machine, bool logEnabled) {
    if (logEnabled) {
        printState(machine, "start: ");
    }
    std::size_t steps = 0;
    while (steps < Machine::kDefaultStepLimit && machine.step()) {
        ++steps;
        if (logEnabled) {
            std::cout << "step " << steps << ": ";
            machine.printState(std::cout);
            std::cout << '\n';
        }
    }
    printState(machine, "final: ");
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void printMenu() {
    std::cout << "\n=== Menu ===\n"
              << "1. Show state\n"
              << "2. Show program\n"
              << "3. Step\n"
              << "4. Run all\n"
              << "5. Add rule\n"
              << "6. Remove rule\n"
              << "7. Reset\n"
              << "0. Exit\n"
              << "Choice: ";
}

void menuLoop(Machine& machine, bool logEnabled) {
    int choice = -1;
    while (choice != 0) {
        printMenu();
        if (!(std::cin >> choice)) {
            clearInput();
            continue;
        }
        switch (choice) {
            case 1:
                printState(machine, "state: ");
                break;
            case 2:
                std::cout << "program:\n" << machine.program();
                break;
            case 3:
                if (!machine.step()) {
                    std::cout << "cannot step: machine stopped or failed\n";
                } else if (logEnabled) {
                    printState(machine, "state: ");
                }
                break;
            case 4:
                runAll(machine, logEnabled);
                break;
            case 5: {
                int number = 0;
                Command command;
                std::cout << "line and command (V n, X n, R n, L n, ? a b, !): ";
                if ((std::cin >> number >> command) && machine.program().addRule(number, command)) {
                    machine.reset();
                    std::cout << "rule added\n";
                } else {
                    clearInput();
                    std::cout << "cannot add rule\n";
                }
                break;
            }
            case 6: {
                int number = 0;
                std::cout << "line to remove: ";
                if ((std::cin >> number) && machine.program().removeRule(number)) {
                    machine.reset();
                    std::cout << "rule removed\n";
                } else {
                    clearInput();
                    std::cout << "rule not found\n";
                }
                break;
            }
            case 7:
                machine.reset();
                std::cout << "machine reset\n";
                break;
            case 0:
                std::cout << "bye\n";
                break;
            default:
                std::cout << "unknown command\n";
                break;
        }
    }
}
}  // namespace

int main(int argc, char* argv[]) {
    Options options;
    if (!parseArguments(argc, argv, options)) {
        printUsage();
        return kUsageError;
    }
    Machine machine;
    int loadResult = loadMachine(options.path, machine);
    if (loadResult != kOk) {
        return loadResult;
    }
    if (options.menuEnabled) {
        printState(machine, "loaded: ");
        menuLoop(machine, options.logEnabled);
        return kOk;
    }
    runAll(machine, options.logEnabled);
    return machine.status() == MachineStatus::Stopped ? kOk : kRunError;
}