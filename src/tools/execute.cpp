#include <boost/program_options.hpp>
#include <iostream>
#include <csignal>
#include <cstdlib>
#include "../api/rauschen.h"
#include "logger.hpp"

#include <boost/algorithm/string.hpp>

namespace po = boost::program_options;

static bool running = true;

int main(int ac, char* av[]) {
  std::string mtype;
  std::string exec;

  po::options_description desc("Usage");
  desc.add_options()
      ("help", "produce help message")
      ("type,t", po::value<std::string>(&mtype)->required(), "set message type")
      ("execute,e", po::value<std::string>(&exec)->required(), "program to execute when message of type arrives\n"
          "%message%, %type%, and %sender% will be replaced with actual values from the received message")
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

  std::signal(SIGINT, [] (int){ running = false; });
  std::signal(SIGTERM, [] (int){ running = false;});

  auto handl = rauschen_register_message_handler(mtype.c_str());
  while( running ) {
    auto message = rauschen_get_next_message(handl, false);
    if(message) {
      std::string call = boost::replace_all_copy(exec, "%type%", message->type);
      boost::replace_all(call, "%sender%", message->sender);
      boost::replace_all(call, "%message%", message->text);

      Logger::info("Executing :"+call);
      std::system(call.c_str());

      rauschen_free_message(message);
    }
  }
  rauschen_unregister_message_handler(handl);
}

