// ConnectPool.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <iostream>
#include <mutex>
#include <chrono>
#include <queue>
#include <thread>
#include <string>
#include <condition_variable>

#include "MysqlConnect.h"
#include "json11/json11.hpp"

using std::string;

class ConnectPool {
public:
	static ConnectPool* getPool();
	std::shared_ptr<MysqlConnect> getConnection();
	int getCurSize();
	void stop();
	~ConnectPool();

	ConnectPool(const ConnectPool&) = delete;
	ConnectPool& operator=(const ConnectPool&) = delete;
private:
	ConnectPool();
	bool parseJsonFile();
	void addConnection();
	void produceConnection();
	void recycleConnection();

private:
	string m_user;
	string m_ip;
	string m_dbname;
	string m_passwd;
	unsigned short m_port;
	int m_minSize;
	int m_maxSize;
	int m_curSize;
	int m_timeout;
	int m_maxIdleTime;

	bool m_running;

	std::queue<MysqlConnect*> m_connQ;
	std::mutex m_mtxQ;
	std::condition_variable m_cond;

};

// TODO: 在此处引用程序需要的其他标头。
