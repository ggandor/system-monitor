#include "format.h"

#include <iomanip>  // for setprecision
#include <string>

// INPUT: long int measuring seconds
// OUTPUT: HH:MM:SS
std::string Format::ElapsedTime(long seconds) {
  // assert (seconds >= 0);
  const int SECONDS_IN_MINUTE = 60;
  const int MINUTES_IN_HOUR = 60;
  const int SECONDS_IN_HOUR = 3600;
  auto hours = seconds / SECONDS_IN_HOUR;
  auto mins = (seconds / SECONDS_IN_MINUTE) % MINUTES_IN_HOUR;
  auto secs = seconds % SECONDS_IN_MINUTE;
  // TODO: maybe use std::setfill instead
  auto format = [](auto n) {
    std::string s = std::to_string(n);
    // assert (s.length() <= 2);
    return s.length() == 1 ? ("0" + s) : s;
  };
  return format(hours) + ":" + format(mins) + ":" + format(secs);
}

std::string Format::Ram(long kB) {
  auto value_in_MB = kB / 1024.0;
  std::stringstream ss;
  // two decimal digits are enough
  ss << std::fixed << std::setprecision(2) << value_in_MB;
  std::string strval = ss.str();
  if (strval.length() >= ram_column_usable_width) {
    strval = strval.substr(0, ram_column_usable_width - 1);
    // don't leave a period in the last column
    if (strval.back() == '.')
      strval = strval.substr(0, ram_column_usable_width - 2);
  }
  return strval;
}
