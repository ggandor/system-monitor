#include "linux_parser.h"

#include <unistd.h>

#include <string>
#include <vector>

// https://en.cppreference.com/w/cpp/preprocessor/include
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

using std::stof;
using std::stoi;
using std::stol;
using std::string;
using std::to_string;

// A helper function for those simple cases when the key and the value can be
// extracted without any complication.
inline string LinuxParser::GetValueForKey(string key_to_find, string path) {
  string line;
  string key, value;
  std::ifstream filestream{path};
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream{line};
      linestream >> key >> value;
      if (key == key_to_find) return value;
    }
  }
  return value;
}

// This nice pattern for splitting a string is from
// https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
inline std::vector<string> LinuxParser::GetWhitespaceSeparatedFields(
    string line) {
  std::istringstream linestream{line};
  std::vector<string> fields(std::istream_iterator<string>{linestream},
                             std::istream_iterator<string>{});
  return fields;
}

string LinuxParser::OperatingSystem() {
  string line;
  string key, value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// The example implementation returned the string 'version' instead of the
// kernel number, this has been fixed.
string LinuxParser::Kernel() {
  string line;
  string os, version, kernel;
  std::ifstream filestream(kProcDirectory + kVersionFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// Updated to use std::filesystem
std::vector<int> LinuxParser::Pids() {
  std::vector<int> pids;
  fs::path procdir{kProcDirectory};
  for (const auto& dirent : fs::directory_iterator(procdir)) {
    if (fs::is_directory(dirent)) {
      string filename = dirent.path().filename();
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

int LinuxParser::TotalProcesses() {
  string value = GetValueForKey("processes", kProcDirectory + kStatFilename);
  return value.empty() ? 0 : stoi(value);
}

int LinuxParser::RunningProcesses() {
  string value =
      GetValueForKey("procs_running", kProcDirectory + kStatFilename);
  return value.empty() ? 0 : stoi(value);
}

float LinuxParser::MemoryUtilization() {
  string memtotal =
      GetValueForKey("MemTotal:", kProcDirectory + kMeminfoFilename);
  string memfree =
      GetValueForKey("MemFree:", kProcDirectory + kMeminfoFilename);
  if (!memtotal.empty() && !memfree.empty())
    return (stof(memtotal) - stof(memfree)) / stof(memtotal);
  return 0.0;
}

long LinuxParser::UpTime() {
  string line;
  string uptime_seconds;
  std::ifstream filestream{kProcDirectory + kUptimeFilename};
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream{line};
    linestream >> uptime_seconds;
    return stol(uptime_seconds);
  }
  return 0;
}

// As we will probably never be interested in the value of active and idle
// jiffies in themselves, there is really no point in creating separate
// functions and doing redundant parsing runs for them. This function returns
// every necessary field instead, and lets the Processor class do the math.
std::vector<string> LinuxParser::CpuUtilization() {
  string line;
  std::ifstream filestream{kProcDirectory + kStatFilename};
  if (filestream.is_open()) {
    // We need the first line (aggregate CPU information) - that will look
    // something like 'cpu  251418 0 158725 1504838 0 7452 0 0 0 0'.
    std::getline(filestream, line);
    auto fields = GetWhitespaceSeparatedFields(line);
    // Remove the key 'cpu'. (It's just a tiny, constant-sized vector, so it
    // does no harm to reallocate stuff once.)
    fields.erase(fields.begin());
    return fields;
  }
  return {};
}

// https://stackoverflow.com/a/16736599
long LinuxParser::ActiveClockTicks(int pid) {
  string line;
  std::ifstream filestream{kProcDirectory + to_string(pid) + kStatFilename};
  if (filestream.is_open()) {
    std::getline(filestream, line);
    auto fields = GetWhitespaceSeparatedFields(line);
    return stol(fields[kUtime_]) + stol(fields[kStime_]) +
           stol(fields[kCutime_]) + stol(fields[kCstime_]);
  }
  return 0;
}

string LinuxParser::Command(int pid) {
  string line;
  std::ifstream filestream{kProcDirectory + to_string(pid) + kCmdlineFilename};
  if (filestream.is_open()) std::getline(filestream, line);
  return line;
}

string LinuxParser::Uid(int pid) {
  return GetValueForKey("Uid:",
                        kProcDirectory + to_string(pid) + kStatusFilename);
}

string LinuxParser::User(int pid) {
  string line;
  string username, x, uid;
  std::ifstream filestream{kPasswordPath};
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      // example line: username:x:1000:1000:...
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream{line};
      linestream >> username >> x >> uid;
      if (uid == Uid(pid)) return username;
    }
  }
  return string{};
}

// Changed signature, Format::Ram handles the rest.
long LinuxParser::Ram(int pid) {
  string value = GetValueForKey(
      "VmSize:", kProcDirectory + to_string(pid) + kStatusFilename);
  return value.empty() ? 0 : stoi(value);
}

// https://stackoverflow.com/a/16736599
long LinuxParser::UpTime(int pid) {
  string line;
  std::ifstream filestream{kProcDirectory + to_string(pid) + kStatFilename};
  if (filestream.is_open()) {
    std::getline(filestream, line);
    auto fields = GetWhitespaceSeparatedFields(line);
    // clock ticks passed from system boot until the start of the process
    long starttime_ticks = stol(fields[kStarttime_]);
    long starttime_seconds = starttime_ticks / sysconf(_SC_CLK_TCK);
    long system_uptime_seconds = UpTime();
    return system_uptime_seconds - starttime_seconds;
  }
  return 0;
}
