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
  std::unordered_map<std::string,
                     std::vector<std::pair<std::string, std::string>>>
      structs_with_states;
  std::unordered_map<std::string,
                     std::vector<std::pair<std::string, std::string>>>
      vars_with_states;
  std::string element;
  while (file >> element) {
    if (element == "state") {
      std::string state_type_name;
      file >> state_type_name;
      if (state_types.contains(state_type_name)) {
        std::cerr << "There is state " << state_type_name << " twice. Aborted\n"
                  << '\n';
        return 3;
      }
      std::unordered_map<std::string, std::vector<std::string>> state_type;
      file >> element;
      if (element != "{") {
        std::cerr << "There is an error in state " << state_type_name
                  << ". Use 'state StateName { state_body };'. Aborted\n";
        return 4;
      }
      std::string cur_state;
      while (file >> cur_state) {
        if (cur_state == "};")
          break;
        file >> element;
        if (element != "{") {
          std::cerr << "There is an error in state " << state_type_name << ": "
                    << cur_state << ". Aborted\n";
          return 5;
        }
        std::vector<std::string> functions;
        while (file >> element) {
          if (element == "}")
            break;
          if (element.find('(') != std::string::npos) {
            element.erase(element.find('('));
            functions.push_back(element);
          }
        }
        state_type.insert({cur_state, functions});
      }
      state_types.insert({state_type_name, state_type});
    } else if (element == "struct") {
      std::string struct_name;
      file >> struct_name;
      if (structs_with_states.contains(struct_name)) {
        std::cerr << "There is struct " << struct_name << " twice. Aborted\n";
        return 6;
      }
      std::vector<std::pair<std::string, std::string>> states;
      file >> element;
      if (element != "{") {
        std::cerr << "There is an error in struct " << struct_name
                  << ". Use 'struct StructName { struct_body };'. Aborted\n";
        return 7;
      }
      std::string cur_field_type;
      while (file >> cur_field_type) {
        if (cur_field_type == "};")
          break;
        if (state_types.contains(cur_field_type)) {
          file >> element;
          if (element.back() != ';') {
            std::cerr << "There is an error in struct " << struct_name
                      << ", state " << cur_field_type
                      << ". You missed ';'. Aborted\n";
            return 8;
          }
          element.erase(element.find(";"));
          states.push_back({cur_field_type, element});
        } else {
          while (file >> element) {
            if (element.back() == ';')
              break;
          }
        }
      }
      structs_with_states.insert({struct_name, states});
    } else if (structs_with_states.contains(element)) {
      std::string var_name;
      file >> var_name;
      if (var_name.back() != ';') {
        std::cerr << "There is var name without ';'. Aborted\n";
        return 9;
      }
      var_name.erase(var_name.find(';'));
      if (vars_with_states.contains(var_name)) {
        std::cerr << "There is var " << var_name << " twice. Aborted\n";
        return 10;
      }
      vars_with_states.insert({var_name, structs_with_states[element]});
    }
  }
}
