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

/// Пункты интерактивного меню (общие для обеих машин).
enum class MenuItem : int {
    Exit = 0,
    ShowState = 1,
    ShowProgram = 2,
    Step = 3,
    RunAll = 4,
    AddRule = 5,
    RemoveRule = 6,
    Reset = 7
};

struct Options {
    std::string path;
    MachineKind kind = MachineKind::Post;
    bool kindSpecified = false;
    bool logEnabled = false;
    bool menuEnabled = false;
};

//Разбор аргументов

bool parseMachineKind(const std::string& name, Options& options) {
    if (name == kPostName) {
        options.kind = MachineKind::Post;
    } else if (name == kTuringName) {
        options.kind = MachineKind::Turing;
    } else {
        return false;
    }
    options.kindSpecified = true;
    return true;
}

bool parseArgument(int argc, char* argv[], int& index, Options& options) {
    std::string argument = argv[index];
    if (argument == kLogFlag) {
        options.logEnabled = true;
        return true;
    }
    if (argument == kMenuFlag) {
        options.menuEnabled = true;
        return true;
    }
    if (argument == kMachineFlag && index + 1 < argc) {
        ++index;
        return parseMachineKind(argv[index], options);
    }
    if (options.path.empty()) {
        options.path = argument;
        return true;
    }
    return false;
}

bool parseArguments(int argc, char* argv[], Options& options) {
    for (int index = 1; index < argc; ++index) {
        if (!parseArgument(argc, argv, index, options)) {
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

// Всё, чем машины Поста и Тьюринга различаются в CLI, собрано в перегрузках ниже.
// Остальной код работает с обеими машинами через шаблоны.

const char* menuTitle(const Machine&) { return "Post Machine"; }

const char* menuTitle(const TuringMachine&) { return "Turing Machine"; }

bool isSuccess(const Machine& machine) { return machine.status() == MachineStatus::Stopped; }

bool isSuccess(const TuringMachine& machine) { return machine.status() == TuringStatus::Halted; }

bool readAndAddRule(Machine& machine) {
    int number = 0;
    Command command;
    std::cout << "line and command: ";
    return (std::cin >> number >> command) && machine.program().addRule(number, command);
}

bool readAndAddRule(TuringMachine& machine) {
    TuringRule rule;
    std::cout << "rule (state read write move next): ";
    return (std::cin >> rule) && machine.program().addRule(rule);
}

bool readAndRemoveRule(Machine& machine) {
    int number = 0;
    std::cout << "line: ";
    return (std::cin >> number) && machine.program().removeRule(number);
}

bool readAndRemoveRule(TuringMachine& machine) {
    std::string state;
    char symbol = TuringTape::kBlank;
    std::cout << "state and symbol: ";
    return (std::cin >> state >> symbol) && machine.program().removeRule(state, symbol);
}

//Общая логика для обеих машин

template <typename MachineT>
void printMachineState(const MachineT& machine, const char* prefix) {
    std::cout << prefix;
    machine.printState(std::cout);
    std::cout << '\n';
}

template <typename MachineT>
void runAll(MachineT& machine, bool logEnabled) {
    if (logEnabled) {
        printMachineState(machine, "start: ");
    }
    std::size_t steps = 0;
    while (steps < MachineT::kDefaultStepLimit && machine.step()) {
        ++steps;
        if (logEnabled) {
            std::cout << "step " << steps << ": ";
            machine.printState(std::cout);
            std::cout << '\n';
        }
    }
    printMachineState(machine, "final: ");
}

template <typename MachineT>
void stepOnce(MachineT& machine) {
    if (machine.step()) {
        printMachineState(machine, "state: ");
    } else {
        std::cout << "cannot step\n";
    }
}

template <typename MachineT>
void addRule(MachineT& machine) {
    if (readAndAddRule(machine)) {
        machine.reset();
        std::cout << "rule added\n";
    } else {
        clearInput();
        std::cout << "cannot add rule\n";
    }
}

template <typename MachineT>
void removeRule(MachineT& machine) {
    if (readAndRemoveRule(machine)) {
        machine.reset();
        std::cout << "rule removed\n";
    } else {
        clearInput();
        std::cout << "not found\n";
    }
}

void printMenu(const char* title) {
    std::cout << "\n=== " << title << " Menu ===\n"
              << "1. Show state\n2. Show program\n3. Step\n4. Run all\n"
              << "5. Add rule\n6. Remove rule\n7. Reset\n0. Exit\nChoice: ";
}

/// Выполняет выбранный пункт меню. Возвращает false, если выбран выход.
template <typename MachineT>
bool handleChoice(MachineT& machine, int choice, bool logEnabled) {
    switch (static_cast<MenuItem>(choice)) {
        case MenuItem::Exit:
            std::cout << "bye\n";
            return false;
        case MenuItem::ShowState: printMachineState(machine, "state: "); break;
        case MenuItem::ShowProgram: std::cout << "program:\n" << machine.program(); break;
        case MenuItem::Step: stepOnce(machine); break;
        case MenuItem::RunAll: runAll(machine, logEnabled); break;
        case MenuItem::AddRule: addRule(machine); break;
        case MenuItem::RemoveRule: removeRule(machine); break;
        case MenuItem::Reset:
            machine.reset();
            std::cout << "reset\n";
            break;
        default: std::cout << "unknown\n"; break;
    }
    return true;
}

template <typename MachineT>
void runMenu(MachineT& machine, bool logEnabled) {
    bool isRunning = true;
    while (isRunning) {
        printMenu(menuTitle(machine));
        int choice = 0;
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                return;
            }
            clearInput();
            continue;
        }
        isRunning = handleChoice(machine, choice, logEnabled);
    }
}

template <typename MachineT>
int runFromFile(const Options& options) {
    std::ifstream file(options.path);
    if (!file) {
        std::cerr << "Cannot open file: " << options.path << '\n';
        return kFileError;
    }
    MachineT machine;
    if (!(file >> machine)) {
        std::cerr << "Invalid file format\n";
        return kFormatError;
    }
    if (options.menuEnabled) {
        printMachineState(machine, "loaded: ");
        runMenu(machine, options.logEnabled);
        return kOk;
    }
    runAll(machine, options.logEnabled);
    return isSuccess(machine) ? kOk : kRunError;
}
}  // namespace

int main(int argc, char* argv[]) {
    Options options;
    if (!parseArguments(argc, argv, options)) {
        printUsage();
        return kUsageError;
    }
    if (options.kind == MachineKind::Post) {
        return runFromFile<Machine>(options);
    }
    return runFromFile<TuringMachine>(options);
}