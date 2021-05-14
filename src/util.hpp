#include <algorithm>
#include <string>

namespace driver {
  void RemoveSpaces(std::string& s) {
    s.erase(std::remove_if(s.begin(), s.end(), isspace), s.end());
  }

  void ToLower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  }
}