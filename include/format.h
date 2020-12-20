#ifndef FORMAT_H
#define FORMAT_H

#include <string>

#include "ncurses_display.h"

namespace Format {
std::string ElapsedTime(long times);
std::string Ram(long kB);
const int ram_column_usable_width =
    NCursesDisplay::time_column - NCursesDisplay::ram_column - 1;
};  // namespace Format

#endif
