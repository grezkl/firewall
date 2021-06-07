#ifndef SQLCONNECTION_H__
#define SQLCONNECTION_H__

#define OTL_ODBC // Compile OTL 4.0/ODBC
#include <otlv4.h>

#include "data.h"
#include <map>
#include <string>

class SqlConnection
{
public:
    SqlConnection();
    ~SqlConnection();

    // connect database -- <username>/<password>@<odbc_name>
    void Connect(const char* conn_str = "fw/scaufw@odbc_fw");
    bool Process();
    std::string GetResult();

private:
    bool FetchRules();

private:
    otl_connect db_;
    nlohmann::json obj_;
    std::string res_;
};

#endif /* SQLCONNECTION_H__ */
