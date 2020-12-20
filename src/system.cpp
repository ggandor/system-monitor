#include "system.h"

#include <unistd.h>  // for sysconf

#include <cassert>
#include <string>
#include <vector>

// Constant fields
Processor& System::Cpu() { return cpu_; }
const std::string& System::OperatingSystem() { return os_; }
const std::string& System::Kernel() { return kernel_; }

// Dynamic fields
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }
long int System::UpTime() { return LinuxParser::UpTime(); }

std::vector<Process>& System::Processes() {
  processes_.clear();  // start with a blank slate
  std::vector<int> pids = LinuxParser::Pids();
  for (int pid : pids) processes_.emplace_back(Process{pid});
  std::sort(processes_.rbegin(), processes_.rend(),
            [this](const auto& p1, const auto& p2) {
              // By all means, work with cached values here, because calling
              // CurrentCpuUtilization() would have the unintended side effect
              // of caching a new value, and that could cause us problems (see
              // the comments in said function).
              return cachedCpuUtilizationOfProcess_[p1.Pid()] <
                     cachedCpuUtilizationOfProcess_[p2.Pid()];
            });
  return processes_;
}

// As processes are now ephemeral objects that cannot maintain a CPU utilization
// cache as part of their state, that should be managed by the System class, and
// these calculations had to be moved here in order to avoid an awkward design
// with circular dependencies.

// As I understand, the value of 'active_seconds' might actually be bigger than
// 'uptime_seconds' on a multicore system, since LinuxParser::ActiveJiffies
// returns the sum of ticks for all cores the process has been using, while the
// calculation in LinuxParser::UpTime, based on the 'starttime' field in
// proc/[pid]/stat expresses a "real" time measurement(?)
std::pair<float, float> System::UptimeCpuUtilizationSnapshot(int pid) {
  float active_seconds = LinuxParser::ActiveClockTicks(pid) /
                         static_cast<float>(sysconf(_SC_CLK_TCK));
  float uptime_seconds = LinuxParser::UpTime(pid);
  return (cachedUptimeCpuUtilizationOfProcess_[pid] = {active_seconds,
                                                       uptime_seconds});
}

// This just returns the cache, so inherently coupled with CurrentCpuUtilization
// that triggers the snapshot. (Maybe not the best design, on the other hand, we
// don't want this to trigger caching here.)
float System::UptimeCpuUtilization(int pid) {
  float active, total;
  std::tie(active, total) = cachedUptimeCpuUtilizationOfProcess_[pid];
  return active / total;
}

// Note: an unassigned PID could be reused by a different process after reaching
// the wrap-around value specified in '/proc/sys/kernel/pid_max' - thus, when
// taking the first snapshot for a process, in some very rare cases the program
// might display an incorrect value for a moment because of the leftover value
// in the cache, but we can live with that :)
float System::CurrentCpuUtilization(int pid) {
  float prevActive, prevTotal;
  std::tie(prevActive, prevTotal) = cachedUptimeCpuUtilizationOfProcess_[pid];
  float active, total;
  std::tie(active, total) = UptimeCpuUtilizationSnapshot(pid);
  float dActive = active - prevActive;
  float dTotal = total - prevTotal;
  // Caveat: as the value in /proc/uptime (the system uptime) is only updated in
  // every second, if we take snapshots in intervals < 1 second, then UpTime()
  // will not change, hence UpTime(pid) will not change, so
  // UptimeCpuUtilizationSnapshot(pid) will return the same 'total' value, and
  // we will get either +infinity (dTotal == 0.0) or even NaN (when both dActive
  // and dTotal == 0.0) returned!
  return (cachedCpuUtilizationOfProcess_[pid] = dActive / dTotal);
}
