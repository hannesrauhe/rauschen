/** @file   send_message.cpp
 *  @date   Oct 10, 2015
 *  @Author Hannes Rauhe (hannes.rauhe@sap.com)
 */

#include <boost/program_options.hpp>
#include <iostream>
#include "../api/rauschen.h"

namespace po = boost::program_options;

int main(int ac, char* av[]) {
  std::string mtype;
  std::string message;
  std::string receiver;

  po::options_description desc("Usage");
  desc.add_options()
      ("help", "produce help message")
      ("type,t", po::value<std::string>(&mtype)->default_value("text"), "set message type")
      ("message,m", po::value<std::string>(&message)->required(), "set message text")
      ("receiver,r", po::value<std::string>(&receiver)->default_value(""), "set message receiver (broadcast if empty)")
  ;

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
  } catch ( const std::exception& e) {
    std::cerr << "Error:" << e.what() << std::endl;
    std::cout << desc << "\n";
    return 1;
  }

  if (vm.count("help")) {
      std::cout << desc << "\n";
      return 1;
  }

  if(receiver.empty()) {
    return rauschen_send_message(message.c_str(), mtype.c_str(), nullptr);
  } else {
    return rauschen_send_message(message.c_str(), mtype.c_str(), receiver.c_str());
  }
}

