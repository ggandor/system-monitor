#include "process.h"

#include <string>

int Process::Pid() const { return id_; }
std::string Process::User() { return user_; }
std::string Process::Command() { return command_; }
long Process::Ram() const { return LinuxParser::Ram(id_); }
long int Process::UpTime() { return LinuxParser::UpTime(id_); }

// IMHO, here we should only compare attributes that are part of the processes'
// state. Any more complex sorting logic belongs to the System class, especially
// if later on we would want to implement some toggle in the interface that
// switches between different attributes to sort on.
bool Process::operator<(Process const& other) const {
  return Pid() < other.Pid();
}
