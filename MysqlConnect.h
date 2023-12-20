// MysqlConnect.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <iostream>
#include <mysql.h>
#include <string>
#include <chrono>

using std::string;

class MysqlConnect {
public:
	MysqlConnect();
	~MysqlConnect();
	bool connect(string user, string passwd, string ip, string dbname, unsigned short port = 3306);
	bool update(string sql);
	bool query(string sql);
	bool next();
	string value(int index);
	bool transaction();
	bool commit();
	bool rollback();

	void refreshActiveTime();
	long long getIdleTime();
private:
	void freeResult();
	MYSQL* m_conn;
	MYSQL_RES* m_res;
	MYSQL_ROW m_row;
	std::chrono::steady_clock::time_point m_alivetime;
};
// TODO: 在此处引用程序需要的其他标头。
