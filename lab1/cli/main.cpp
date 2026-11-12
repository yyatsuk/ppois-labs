#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include "Machine.h"
#include "TuringMachine.h"

struct Options {
    std::string path;      // путь к файлу
    std::string kind;      // "post" или "turing"
    bool log = false;      // флаг -log
    bool menu = false;     // флаг --menu
};

// Разбор аргументов командной строки
bool parseArgs(int argc, char* argv[], Options& o) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-log") {
            o.log = true;
        } else if (a == "--menu") {
            o.menu = true;
        } else if (a == "--machine" && i + 1 < argc) {
            o.kind = argv[++i];
        } else if (o.path.empty()) {
            o.path = a;
        } else {
            return false;
        }
    }
    return !o.path.empty() && !o.kind.empty();
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// ========== МАШИНА ПОСТА ==========

void runPost(const Options& o) {
    std::ifstream file(o.path);
    if (!file) {
        std::cerr << "Cannot open file\n";
        return;
    }
    Machine m;
    if (!(file >> m)) {
        std::cerr << "Invalid format\n";
        return;
    }

    // Автоматический режим без --menu
    if (!o.menu) {
        if (o.log) { std::cout << "start: "; m.printState(std::cout); std::cout << "\n"; }
        while (m.step()) {
            if (o.log) { m.printState(std::cout); std::cout << "\n"; }
        }
        std::cout << "final: "; m.printState(std::cout); std::cout << "\n";
        return;
    }

    // Меню
    int choice = -1;
    while (choice != 0) {
        std::cout << "\nPost menu: 1-state 2-program 3-step 4-run 5-add 6-remove 7-reset 0-exit\n> ";
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) return;
            clearInput();
            continue;
        }
        if (choice == 1) {
            m.printState(std::cout); std::cout << "\n";
        } else if (choice == 2) {
            std::cout << m.program();
        } else if (choice == 3) {
            m.step();
            m.printState(std::cout); std::cout << "\n";
        } else if (choice == 4) {
            while (m.step()) {}
            m.printState(std::cout); std::cout << "\n";
        } else if (choice == 5) {
            int number; Command cmd;
            std::cout << "line and command: ";
            if ((std::cin >> number >> cmd) && m.program().addRule(number, cmd)) {
                m.reset(); std::cout << "added\n";
            } else { clearInput(); std::cout << "cannot add\n"; }
        } else if (choice == 6) {
            int number;
            std::cout << "line: ";
            if ((std::cin >> number) && m.program().removeRule(number)) {
                m.reset(); std::cout << "removed\n";
            } else { clearInput(); std::cout << "not found\n"; }
        } else if (choice == 7) {
            m.reset(); std::cout << "reset\n";
        }
    }
}

// ========== МАШИНА ТЬЮРИНГА ==========

void runTuring(const Options& o) {
    std::ifstream file(o.path);
    if (!file) {
        std::cerr << "Cannot open file\n";
        return;
    }
    TuringMachine m;
    if (!(file >> m)) {
        std::cerr << "Invalid format\n";
        return;
    }

    if (!o.menu) {
        if (o.log) { std::cout << "start: "; m.printState(std::cout); std::cout << "\n"; }
        while (m.step()) {
            if (o.log) { m.printState(std::cout); std::cout << "\n"; }
        }
        std::cout << "final: "; m.printState(std::cout); std::cout << "\n";
        return;
    }

    int choice = -1;
    while (choice != 0) {
        std::cout << "\nTuring menu: 1-state 2-program 3-step 4-run 5-add 6-remove 7-reset 0-exit\n> ";
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) return;
            clearInput();
            continue;
        }
        if (choice == 1) {
            m.printState(std::cout); std::cout << "\n";
        } else if (choice == 2) {
            std::cout << m.program();
        } else if (choice == 3) {
            m.step();
            m.printState(std::cout); std::cout << "\n";
        } else if (choice == 4) {
            while (m.step()) {}
            m.printState(std::cout); std::cout << "\n";
        } else if (choice == 5) {
            TuringRule rule;
            std::cout << "rule (state read write move next): ";
            if ((std::cin >> rule) && m.program().addRule(rule)) {
                m.reset(); std::cout << "added\n";
            } else { clearInput(); std::cout << "cannot add\n"; }
        } else if (choice == 6) {
            std::string state; char symbol = TuringTape::kBlank;
            std::cout << "state and symbol: ";
            if ((std::cin >> state >> symbol) && m.program().removeRule(state, symbol)) {
                m.reset(); std::cout << "removed\n";
            } else { clearInput(); std::cout << "not found\n"; }
        } else if (choice == 7) {
            m.reset(); std::cout << "reset\n";
        }
    }
}

// ========== ТОЧКА ВХОДА ==========

int main(int argc, char* argv[]) {
    Options o;
    if (!parseArgs(argc, argv, o)) {
        std::cerr << "Usage: lab1_cli --machine post|turing <file> [-log] [--menu]\n";
        return 1;
    }
    if (o.kind == "post") {
        runPost(o);
    } else if (o.kind == "turing") {
        runTuring(o);
    } else {
        std::cerr << "Unknown machine: " << o.kind << "\n";
        return 1;
    }
    return 0;
}