#ifndef PROCESS_H
#define PROCESS_H

#include <string>

#include "linux_parser.h"

class Process {
 public:
  Process(int id) : id_{id} {}
  int Pid() const;
  std::string User();
  std::string Command();
  long Ram() const;
  long int UpTime();
  bool operator<(Process const& other) const;

 private:
  int id_;
  std::string user_ = LinuxParser::User(id_);
  std::string command_ = LinuxParser::Command(id_);
};

#endif
