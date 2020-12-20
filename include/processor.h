#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "linux_parser.h"

class Processor {
 public:
  float Utilization();

 private:
  std::pair<float, float> UptimeUtilization();
  std::pair<float, float> cachedUptimeUtilization_;
};

#endif
