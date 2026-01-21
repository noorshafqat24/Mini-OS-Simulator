#include <iostream>
#include <limits>
#include "simulator.hpp"
#include "process.hpp"

static int read_int(const std::string& prompt) {
    int x;
    while (true) {
        std::cout << prompt;
        if (std::cin >> x) return x;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Try again.\n";
    }
}

static std::vector<int> read_vector(int m) {
    std::vector<int> v(m);
    for (int i = 0; i < m; i++) {
        v[i] = read_int("  Resource R" + std::to_string(i + 1) + ": ");
    }
    return v;
}

int main() {
    // Defaults (you can change)
    // available = {3,3,2} means 3 resource types
    Simulator sim(/*buffer*/5, /*producers*/2, /*each*/5, /*available*/{3,3,2});

    while (true) {
        std::cout << "\n===== MINI OS SIM MENU =====\n";
        std::cout << "1) Start Simulation\n";
        std::cout << "2) Add Process\n";
        std::cout << "3) Display State\n";
        std::cout << "4) Exit\n";

        int choice = read_int("Enter choice: ");

        if (choice == 1) {
            sim.start();
        } else if (choice == 2) {
            int pid = read_int("PID: ");
            int at  = read_int("Arrival Time: ");
            int bt  = read_int("Burst Time: ");
            int pr  = read_int("Priority (lower = higher): ");

            std::cout << "Enter max resource claim vector (3 values):\n";
            auto need = read_vector(3);

            sim.add_process(Process(pid, at, bt, pr, need));
        } else if (choice == 3) {
            sim.display_state();
        } else if (choice == 4) {
            std::cout << "Exiting...\n";
            break;
        } else {
            std::cout << "Invalid choice.\n";
        }
    }

    return 0;
}
