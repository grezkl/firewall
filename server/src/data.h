#ifndef DATA_H__
#define DATA_H__

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

/*
json 结构如下
{
    "rules": [
        {"pg_name": "360安全卫士", "pg_path": "D:\Program Files (x86)\360se6\Application\360se.exe"},
        {"pg_name": "网易有道词典", "pg_path": "D:\有道词典\Dict\YoudaoDict.exe"}
    ]
}
*/

namespace json_rules
{
using nlohmann::json;

struct RulesItem
{
    std::string pg_name;
    std::string pg_path;
};

struct Rules
{
    std::vector<RulesItem> rules;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RulesItem, pg_name, pg_path);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Rules, rules);

}; // namespace json_rules

#endif /* DATA_H__ */
