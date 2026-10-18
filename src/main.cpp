#include <algorithm>
#include <cstdio>
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
  std::unordered_map<
      std::string,
      std::unordered_map<std::string,
                         std::vector<std::pair<std::string, std::string>>>>
      state_types;
  std::unordered_map<std::string,
                     std::vector<std::pair<std::string, std::string>>>
      structs_with_states;
  std::unordered_map<std::string,
                     std::vector<std::pair<std::string, std::string>>>
      vars_with_states;
  std::string code;
  std::string element;
  while (file >> element) {
    if (element == "#include") {
      file >> element;
      code += "#include " + element + "\n";
    } else if (element == "state") {
      std::string state_type_name;
      file >> state_type_name;
      if (state_types.contains(state_type_name)) {
        std::cerr << "There is state " << state_type_name << " twice. Aborted\n"
                  << '\n';
        return 3;
      }
      std::unordered_map<std::string,
                         std::vector<std::pair<std::string, std::string>>>
          state_type;
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
        std::vector<std::pair<std::string, std::string>> functions;
        std::string function_name;
        while (file >> function_name) {
          if (function_name == "}")
            break;
          if (function_name.find('(') != std::string::npos) {
            function_name.erase(function_name.find('('));
            while (file >> element) {
              if (element == "->") {
                file >> element;
                if (element.back() != ';') {
                  std::cerr << "There is an error in state " << state_type_name
                            << ": " << cur_state
                            << ". Maybe you missed ';'. Aborted\n";
                  return 6;
                }
                element.erase(element.find(';'));
                break;
              }
            }
            functions.push_back({function_name, element});
          } else {
            std::cerr << "There is an error in state " << state_type_name
                      << ": " << cur_state << ". " << function_name
                      << " is not function. Aborted\n";
            return 7;
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
        return 8;
        // Это сотая строчка. Если Вы до сих пор пытаетесь понять этот говнокод,
        // мне Вас искренне жаль. Код изначально проектировался с мыслью, чтобы
        // быть максимально нечитабельным. Однако он работает, так что вопросы
        // только к Вам: зачем Вы это до сих пор читаете?
      }
      std::vector<std::pair<std::string, std::string>> states;
      file >> element;
      if (element != "{") {
        std::cerr << "There is an error in struct " << struct_name
                  << ". Use 'struct StructName { struct_body };'. Aborted\n";
        return 9;
      }
      code += "typedef struct " + struct_name + " { ";
      std::string cur_field_type;
      while (file >> cur_field_type) {
        if (cur_field_type == "};")
          break;
        if (state_types.contains(cur_field_type)) {
          file >> element;
          if (element.back() != ';') {
            std::cerr << "There is an error in struct " << struct_name
                      << ", state " << cur_field_type
                      << ". Maybe you missed ';'. Aborted\n";
            return 10;
          }
          element.erase(element.find(";"));
          states.push_back({cur_field_type, element});
        } else {
          code += cur_field_type + " ";
          while (file >> element) {
            code += element + " ";
            if (element.back() == ';')
              break;
          }
        }
      }
      code += "} " + struct_name + "; ";
      structs_with_states.insert({struct_name, states});
      // TODO main()
    } else if (structs_with_states.contains(element)) {
      const std::string struct_name = element;
      std::string var_name;
      file >> var_name;
      if (var_name.back() != ';') {
        std::cerr << "There is var name without ';'. Aborted\n";
        return 11;
      }
      var_name.erase(var_name.find(';'));
      if (vars_with_states.contains(var_name)) {
        std::cerr << "There is var " << var_name << " twice. Aborted\n";
        return 12;
      }
      if (!structs_with_states[struct_name].empty())
        vars_with_states.insert({var_name, structs_with_states[struct_name]});
      code += struct_name + " " + var_name + "; ";
    } else {
      if (element.find('.') == std::string::npos) {
        code += element + " ";
        continue;
      }
      std::string var = element.substr(0, element.find('.'));
      if (vars_with_states.contains(var)) {
        std::string function = element.substr(element.find('.') + 1);
        if (function.find('(') == std::string::npos) {
          code += element + " ";
          continue;
        }
        function.erase(function.find('('));
        auto tmp = state_types[vars_with_states[var][0].first]
                              [vars_with_states[var][0].second];
        auto it =
            std::find_if(tmp.begin(), tmp.end(), [function](const auto &pair) {
              return (pair.first == function);
            });
        if (it == tmp.end()) {
          std::cerr << "You use " << var << "." << function
                    << "() with wrong state. Aborted\n";
          return 13;
        }
        vars_with_states[var][0].second = it->second;
        const std::string args = element.substr(element.find('(') + 1);
        code += function + "(&" + var;
        if (args.empty() || args[0] != ')')
          code += ", ";
        code += args + " ";
      } else {
        code += element + " ";
      }
    }
  }

  FILE *compiler = popen("mkdir -p build && gcc -x c - -o build/program", "w");
  if (compiler == nullptr) {
    std::cerr << "Cannot start GCC. Aborted\n";
    return 14;
  }
  const size_t written = std::fwrite(code.data(), 1, code.size(), compiler);
  const int status = pclose(compiler);
  if (written != code.size() || status != 0) {
    std::cerr << "C compilation failed. Aborted\n";
    return 15;
  }
}
