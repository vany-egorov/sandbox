#include <iostream>
#include "yaml-cpp/yaml.h"
#include "boost/numeric/ublas/vector.hpp"

using namespace boost::numeric::ublas;

int main() {
  unsigned long long int i = 193213;
  YAML::Node config = YAML::LoadFile("./config.yml");

  std::cout << "hello!" << std::endl;
  if (config["lastLogin"]) {
    std::cout << "Last logged in: " << config["lastLogin"].as<std::string>() << std::endl;
    std::cout << "Last logged in: " << config["foo"].as<int>() << std::endl;
    std::cout << "Last logged in: " << i << std::endl;
  } else {
    std::cout << "FAILED!" << std::endl;
  }
  std::cout << "hello!" << std::endl;
  std::cout << config << std::endl;

  vector<double> x (2);
  x(0) = 100; x(1) = 500;
  std::cout << x(1) << std::endl;
 
  return 0;
}
