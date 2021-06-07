#include "sqlconnection.h"

using namespace std;
using namespace json_rules;

void OTLErrorMsg(otl_exception& p)
{
    // intercept OTL exceptions
    cerr << p.msg << "\n";      // print out error message
    cerr << p.stm_text << "\n"; // print out SQL that caused the error
    cerr << p.sqlstate << "\n"; // print out SQLSTATE message
    cerr << p.var_info << "\n"; // print out the variable that caused the error
}

SqlConnection::SqlConnection() { otl_connect::otl_initialize(); }

void SqlConnection::Connect(const char* conn_str)
{
    try
    {
        // auto commit：第二个参数设为 1
        db_.rlogon(conn_str, 1);
    }
    catch (otl_exception& p)
    {
        OTLErrorMsg(p);
    }
}

bool SqlConnection::Process()
{
    // 此处可以添加对客户端不同要求的不同处理，目前仅单一需求
    // ...

    return FetchRules();
}

// 从数据库读取预设规则
bool SqlConnection::FetchRules()
{
    Rules r;
    RulesItem ri;

    char pg_name[32];
    char pg_path[255];
    string request = "select * from rules";

    otl_stream sel(1, request.c_str(), db_);
    while (!sel.eof())
    {
        memset(pg_name, '\0', sizeof(pg_name));
        memset(pg_path, '\0', sizeof(pg_path));
        sel >> pg_name >> pg_path;

        ri.pg_name = pg_name;
        ri.pg_path = pg_path;
        r.rules.push_back(ri);
    }

    if (r.rules.empty()) return false;

    size_t len = r.rules.size();
    cout << "[request] " << request << "\n[result] count: " << len << "\n";
    // for (int i = 0; i != len; ++i)
    // {
    //     cout << r.rules.at(i).pg_name << "\n" << r.rules.at(i).pg_path << "\n";
    // }

    // 将自定义 struct 转换成 json 结构
    json j;
    j    = r;
    res_ = j.dump() + "\n";

    return true;
}

string SqlConnection::GetResult() { return res_; }

SqlConnection::~SqlConnection() { db_.logoff(); }
