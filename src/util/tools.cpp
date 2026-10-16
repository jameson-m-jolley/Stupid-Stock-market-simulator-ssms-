#include "util/tools.hpp"
#include <cstdio> // Required for printf
#include <string>

void print_json_key_val(char *key, char *value, FILE *out) {
  fprintf(out, "{\"%s\":\"%s\"}", key, value);
}

std::string make_json_string(std::string key, std::string val) {
  return "{\"" + key + "\":" + val + "}";
}