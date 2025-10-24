#include "GlobalControl.hpp"
#include "Bkk32Info.hpp"

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <csignal>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char **argv) {
  // Define available options
  po::options_description desc("Usage: uzsBkk320NetworkManager [options]");
  desc.add_options()
    ("help,h", "Display help message")
    ("port,p", po::value<unsigned short>()->default_value(30720),"The port to listen on (e.g. 30720)")
    ("log-level,l", po::value<std::string>()->default_value("info"),"The log level (e.g. trace, debug, info, warn, error, critical)")
    ("main-cfg-file,m", po::value<std::string>()->default_value(BKK32_DEFAULT_MAIN_CFG_PATH),"Path to main configuration file");

  // Parse command-line arguments
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      std::cout << desc << '\n';
      return EXIT_SUCCESS;
    }

    po::notify(vm); // Trigger validation of required options
  } catch (const po::error &ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    std::cerr << desc << '\n';
    return EXIT_FAILURE;
  }

  GlobalControl globalControl;
  globalControl.start(vm);
}
