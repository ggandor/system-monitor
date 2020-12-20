#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include <curses.h>

#include "process.h"
#include "system.h"

namespace NCursesDisplay {
void Display(System& system, int n = 10);
void DisplaySystem(System& system, WINDOW* window);
void DisplayProcesses(System& system, WINDOW* window, int n);
std::string ProgressBar(float percent);

// These have been moved here from ncurses_display.cpp, in order to be able to
// expose them to formatter functions.
const int pid_column{2};
const int user_column{9};
const int cur_cpu_column{18};
const int avg_cpu_column{26};
const int ram_column{34};
const int time_column{43};
const int command_column{53};
};  // namespace NCursesDisplay

#endif
