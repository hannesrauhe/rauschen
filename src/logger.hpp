#pragma once

#include <iostream>

class Logger {
protected:
  static void log(const std::string& level, const std::string& msg) {
    std::cerr<<"["<<level<<"] "<<msg<<std::endl;
  }

public:
  static void debug(const std::string& msg) {
    log("DEBUG", msg);
  }
  static void info(const std::string& msg) {
    log("INFO", msg);
  }
  static void warn(const std::string& msg) {
    log("WARNING", msg);
  }
  static void error(const std::string& msg) {
    log("ERROR", msg);
  }
  static void critical(const std::string& msg) {
    log("CRITICAL", msg);
  }
};
