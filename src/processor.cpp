#include "processor.h"

namespace LP = LinuxParser;

std::pair<float, float> Processor::UptimeUtilization() {
  std::vector<std::string> strfields = LP::CpuUtilization();
  std::vector<float> fields;
  for (auto& s : strfields) fields.emplace_back(std::stof(s));
  // Calculation based on https://stackoverflow.com/a/23376195
  auto idle = fields[LP::kIdle_] + fields[LP::kIOwait_];
  auto active = fields[LP::kUser_] + fields[LP::kNice_] + fields[LP::kSystem_] +
                fields[LP::kIRQ_] + fields[LP::kSoftIRQ_] + fields[LP::kSteal_];
  auto total = idle + active;
  return (cachedUptimeUtilization_ = {active, total});
}

float Processor::Utilization() {
  // The cached pair will be value-initialized to 0.0s, it's OK
  float prevActive, prevTotal;
  std::tie(prevActive, prevTotal) = cachedUptimeUtilization_;
  float active, total;
  std::tie(active, total) = UptimeUtilization();
  // auto [prevActive, prevTotal] = cachedUptimeUtilization_;
  // auto [active, total] = UptimeUtilization();
  auto dActive = active - prevActive;
  auto dTotal = total - prevTotal;
  return dActive / dTotal;
}
