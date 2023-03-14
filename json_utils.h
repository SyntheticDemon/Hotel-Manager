#ifndef JSON_UTILS
#define JSON_UTILS
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

json read_json_f(string &location);
json search_json(string id, string id_field, string sub_path, json input);
int search_json_array(string id, string id_field, string sub_path, json input);
int search_json_array_int_key(int id, string id_field, string sub_path, json input);
json search_json_reference(string id, string id_field, string sub_path, json input);
json update_json(string id, string id_field, string sub_path, json input, json new_val);

#endif // !JSON_UTILS