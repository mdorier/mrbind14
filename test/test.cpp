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

void f() {
  std::cout << "Yey!" << std::endl;
}
double f2(int x, float y) {
  return y*x*x;
}
std::string f3(const std::string& s) {
  return "Hello World "s + s;
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

  try {
  mrbind14::interpreter mruby;
  mruby.def_const("VAL", 42);
  mruby.set_global_variable("$a", 4.5);
  mruby.def_function("myfunction", &f);
  mruby.def_function("f2", &f2);
  mruby.def_function("f3", &f3);
  std::function<double(int,float)> fun = [](int x, float y) -> double {
        return x+y;
      };
  mruby.def_function("mylambda", fun);
  auto obj = mruby.execute(script.c_str());
  } catch(const std::exception& ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
  }
}
