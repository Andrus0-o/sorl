#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Please, write 'sorlc path/to/your/code.sr'. Aborted" << '\n';
    return 1;
  }
  std::ifstream file(argv[1]);
  if (!file.is_open()) {
    std::cerr << "There is not file " << argv[1] << ". Aborted" << '\n';
    return 2;
  }
  std::unordered_map<std::string,
                     std::unordered_map<std::string, std::vector<std::string>>>
      state_types;
  std::string element;
  while (file >> element) {
    if (element == "state") {
      std::string state_type_name;
      file >> state_type_name;
      if (state_types.contains(state_type_name)) {
        std::cerr << "There is state " << state_type_name << " twice. Aborted"
                  << '\n';
        return 3;
      }
      // state_types.insert({state_type_name, {}});
      std::unordered_map<std::string, std::vector<std::string>> state_type;
      file >> element;
      if (element != "{") {
        std::cerr << "There is an error in state " << state_type_name
                  << ". Use 'state StateName { state_body }'. Aborted";
        return 4;
      }
      std::string cur_state;
      while (file >> cur_state) {
        if (cur_state == "}")
          break;
        file >> element;
        if (element != "{") {
          std::cerr << "There is an error in state " << state_type_name << ": "
                    << cur_state << ". Aborted";
          return 5;
        }
        std::vector<std::string> functions;
        while (file >> element) {
          if (element == "}")
            break;
          element.erase(element.find('('));
          functions.push_back(element);
        }
        state_type.insert({cur_state, functions});
      }
      state_types.insert({state_type_name, state_type});
    }
  }
}
