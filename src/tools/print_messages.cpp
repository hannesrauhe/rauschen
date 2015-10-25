#include <boost/program_options.hpp>
#include <iostream>
#include <csignal>
#include "../api/rauschen.h"

namespace po = boost::program_options;

static bool running = true;

int main(int ac, char* av[]) {
  std::string mtype;

  po::options_description desc("Usage");
  desc.add_options()
      ("help", "produce help message")
      ("type,t", po::value<std::string>(&mtype)->default_value("text"), "set message type")
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

//  std::signal(SIGINT, [] (int){ running = false; });
//  std::signal(SIGTERM, [] (int){ running = false;});

  auto handl = rauschen_register_message_handler(mtype.c_str());
  while( running ) {
    auto message = rauschen_get_next_message(handl);
    std::cout<<"["<<message->sender<<"|"<<message->type<<"] "<<message->text<<std::endl;
    rauschen_free_message(message);
  }
  rauschen_unregister_message_handler(handl);
}

