#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include "Machine.h"
#include "TuringMachine.h"

namespace {
const char* const kLogFlag = "-log";
const char* const kMenuFlag = "--menu";
const char* const kMachineFlag = "--machine";

const char* const kPostName = "post";
const char* const kTuringName = "turing";

enum ExitCode { kOk = 0, kUsageError = 1, kFileError = 2, kFormatError = 3, kRunError = 4 };

enum class MachineKind { Post, Turing };

struct Options {
    std::string path;
    MachineKind kind = MachineKind::Post;
    bool kindSpecified = false;
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
        } else if (argument == kMachineFlag && i + 1 < argc) {
            std::string kind = argv[++i];
            if (kind == kPostName) {
                options.kind = MachineKind::Post;
            } else if (kind == kTuringName) {
                options.kind = MachineKind::Turing;
            } else {
                return false;
            }
            options.kindSpecified = true;
        } else if (options.path.empty()) {
            options.path = argument;
        } else {
            return false;
        }
    }
    return !options.path.empty() && options.kindSpecified;
}

void printUsage() {
    std::cerr << "Usage: lab1_cli " << kMachineFlag << " post|turing <file> ["
              << kLogFlag << "] [" << kMenuFlag << "]\n"
              << "  " << kMachineFlag << " post|turing   choose machine type (required)\n"
              << "  <file>                path to file with machine data\n"
              << "  " << kLogFlag << "               print state after each step\n"
              << "  " << kMenuFlag << "              open interactive menu\n";
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// ==================== Машина Поста ====================

void printPostState(const Machine& machine, const char* prefix) {
    std::cout << prefix;
    machine.printState(std::cout);
    std::cout << '\n';
}

void runPostAll(Machine& machine, bool logEnabled) {
    if (logEnabled) {
        printPostState(machine, "start: ");
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
    printPostState(machine, "final: ");
}

void postMenu(Machine& machine, bool logEnabled) {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n=== Post Machine Menu ===\n"
                  << "1. Show state\n2. Show program\n3. Step\n4. Run all\n"
                  << "5. Add rule\n6. Remove rule\n7. Reset\n0. Exit\nChoice: ";
        if (!(std::cin >> choice)) {
            clearInput();
            continue;
        }
        switch (choice) {
            case 1:
                printPostState(machine, "state: ");
                break;
            case 2:
                std::cout << "program:\n" << machine.program();
                break;
            case 3:
                if (!machine.step()) {
                    std::cout << "cannot step\n";
                } else{
                    printPostState(machine, "state: ");
                }
                break;
            case 4:
                runPostAll(machine, logEnabled);
                break;
            case 5: {
                int number = 0;
                Command command;
                std::cout << "line and command: ";
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
                std::cout << "line: ";
                if ((std::cin >> number) && machine.program().removeRule(number)) {
                    machine.reset();
                    std::cout << "rule removed\n";
                } else {
                    clearInput();
                    std::cout << "not found\n";
                }
                break;
            }
            case 7:
                machine.reset();
                std::cout << "reset\n";
                break;
            case 0:
                std::cout << "bye\n";
                break;
            default:
                std::cout << "unknown\n";
                break;
        }
    }
}

int runPost(const Options& options) {
    std::ifstream file(options.path);
    if (!file) {
        std::cerr << "Cannot open file: " << options.path << '\n';
        return kFileError;
    }
    Machine machine;
    if (!(file >> machine)) {
        std::cerr << "Invalid file format\n";
        return kFormatError;
    }
    if (options.menuEnabled) {
        printPostState(machine, "loaded: ");
        postMenu(machine, options.logEnabled);
        return kOk;
    }
    runPostAll(machine, options.logEnabled);
    return machine.status() == MachineStatus::Stopped ? kOk : kRunError;
}

// ==================== Машина Тьюринга ====================

void printTuringState(const TuringMachine& machine, const char* prefix) {
    std::cout << prefix;
    machine.printState(std::cout);
    std::cout << '\n';
}

void runTuringAll(TuringMachine& machine, bool logEnabled) {
    if (logEnabled) {
        printTuringState(machine, "start: ");
    }
    std::size_t steps = 0;
    while (steps < TuringMachine::kDefaultStepLimit && machine.step()) {
        ++steps;
        if (logEnabled) {
            std::cout << "step " << steps << ": ";
            machine.printState(std::cout);
            std::cout << '\n';
        }
    }
    printTuringState(machine, "final: ");
}

void turingMenu(TuringMachine& machine, bool logEnabled) {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n=== Turing Machine Menu ===\n"
                  << "1. Show state\n2. Show program\n3. Step\n4. Run all\n"
                  << "5. Add rule\n6. Remove rule\n7. Reset\n0. Exit\nChoice: ";
        if (!(std::cin >> choice)) {
            clearInput();
            continue;
        }
        switch (choice) {
            case 1:
                printTuringState(machine, "state: ");
                break;
            case 2:
                std::cout << "program:\n" << machine.program();
                break;
            case 3:
                if (!machine.step()) {
                    std::cout << "cannot step\n";
                } else{
                    printTuringState(machine, "state: ");
                }
                break;
            case 4:
                runTuringAll(machine, logEnabled);
                break;
            case 5: {
                TuringRule rule;
                std::cout << "rule (state read write move next): ";
                if ((std::cin >> rule) && machine.program().addRule(rule)) {
                    machine.reset();
                    std::cout << "rule added\n";
                } else {
                    clearInput();
                    std::cout << "cannot add rule\n";
                }
                break;
            }
            case 6: {
                std::string state;
                char symbol = TuringTape::kBlank;
                std::cout << "state and symbol: ";
                if ((std::cin >> state >> symbol) && machine.program().removeRule(state, symbol)) {
                    machine.reset();
                    std::cout << "rule removed\n";
                } else {
                    clearInput();
                    std::cout << "not found\n";
                }
                break;
            }
            case 7:
                machine.reset();
                std::cout << "reset\n";
                break;
            case 0:
                std::cout << "bye\n";
                break;
            default:
                std::cout << "unknown\n";
                break;
        }
    }
}

int runTuring(const Options& options) {
    std::ifstream file(options.path);
    if (!file) {
        std::cerr << "Cannot open file: " << options.path << '\n';
        return kFileError;
    }
    TuringMachine machine;
    if (!(file >> machine)) {
        std::cerr << "Invalid file format\n";
        return kFormatError;
    }
    if (options.menuEnabled) {
        printTuringState(machine, "loaded: ");
        turingMenu(machine, options.logEnabled);
        return kOk;
    }
    runTuringAll(machine, options.logEnabled);
    return machine.status() == TuringStatus::Halted ? kOk : kRunError;
}
}  // namespace

int main(int argc, char* argv[]) {
    Options options;
    if (!parseArguments(argc, argv, options)) {
        printUsage();
        return kUsageError;
    }
    return options.kind == MachineKind::Post ? runPost(options) : runTuring(options);
}