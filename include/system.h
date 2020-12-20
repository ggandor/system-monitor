#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <unordered_map>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

class System {
 public:
  Processor& Cpu();
  const std::string& Kernel();
  const std::string& OperatingSystem();
  float MemoryUtilization();
  long UpTime();
  int TotalProcesses();
  int RunningProcesses();
  std::vector<Process>& Processes();
  float CurrentCpuUtilization(int pid);
  float UptimeCpuUtilization(int pid);

 private:
  std::pair<float, float> UptimeCpuUtilizationSnapshot(int pid);
  Processor cpu_ = {};
  const std::string os_ = LinuxParser::OperatingSystem();
  const std::string kernel_ = LinuxParser::Kernel();
  std::vector<Process> processes_ = {};
  std::unordered_map<int, std::pair<float, float>>
      cachedUptimeCpuUtilizationOfProcess_ = {};
  std::unordered_map<int, float> cachedCpuUtilizationOfProcess_ = {};
};

#endif
