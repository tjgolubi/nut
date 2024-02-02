// Copyright 2024 Terry Golubiewski, all rights reserved.
#ifndef PROGRESS_H
#define PROGRESS_H
#pragma once

#include <chrono>
#include <iostream>
#include <iomanip>

class ProgressMonitor {
  const long long total;
  std::ostream& os;
  std::chrono::steady_clock::time_point t;
public:
  explicit ProgressMonitor(long long total_=0, std::ostream& os_ = std::cerr)
    : total{total_}, os{os_}, t{std::chrono::steady_clock::now()}
  { if (total) os << "\n  0% complete"; else os << "\n "; }
  ~ProgressMonitor() { os << "\r100% complete\n"; }
  void operator()(long long val) {
    if (std::chrono::steady_clock::now() < t)
      return;
    t += std::chrono::seconds(1);
    if (total != 0) {
      auto percent = (100 * val) / total;
      os << '\r' << std::setw(3) << percent << std::flush;
      return;
    }
    static const char twirler[] = "-\\|/";
    using namespace std::chrono;
    auto s = duration_cast<seconds>(t.time_since_epoch());
    os << '\r' << twirler[s.count() % 4] << std::flush;
  }
}; // ProgressMonitor

#endif
