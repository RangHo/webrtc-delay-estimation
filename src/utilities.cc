#include "utilities.h"

#include <fstream>

bool exists(const std::string& filename) {
  std::ifstream file_to_test(filename);
  return file_to_test.good();
}
