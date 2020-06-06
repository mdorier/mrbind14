#include <mrbind14/mrbind14.hpp>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>

using namespace std::string_literals;

std::string read_file(const char* filename) {
  std::ifstream f(filename);
  std::string str;
  if(f) {
    std::ostringstream ss;
    ss << f.rdbuf(); // reading data
    return ss.str();
  } else {
    throw std::runtime_error("Could not open file"s + filename);
  }
}

int main(int argc, char** argv) {
  if(argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <script.rb>\n";
    exit(-1);
  }

  std::string script;
  try {
    script = read_file(argv[1]);
  } catch(const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    exit(-1);
  }

  mrbind14::interpreter mruby;
  auto obj = mruby.execute(script.c_str());

  if(obj.convertible_to<int>()) {
    std::cout << "Int value : " << obj.as<int>() << std::endl;
  }
  if(obj.convertible_to<float>()) {
    std::cout << "Float value : " << obj.as<float>() << std::endl;
  }
  if(obj.convertible_to<bool>()) {
    std::cout << "Bool value : " << obj.as<bool>() << std::endl;
  }
  if(obj.convertible_to<std::string>()) {
    std::cout << "String value : " << obj.as<std::string>() << std::endl;
  }
}