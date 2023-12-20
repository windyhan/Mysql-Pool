// ConnectPool.cpp: 定义应用程序的入口点。
//

#include "ConnectPool.h"
#include <fstream>


using namespace std;


ConnectPool* ConnectPool::getPool()
{
	static ConnectPool pool;
	return &pool;
}

std::shared_ptr<MysqlConnect> ConnectPool::getConnection()
{
	shared_ptr<MysqlConnect> connptr;

	{//控制临界区
		unique_lock<mutex> locker(m_mtxQ);
		while (m_connQ.empty())
		{
			if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout)))
			{
				if (m_connQ.empty()) return nullptr;
			}
		}
		connptr.reset(m_connQ.front(), \
			[this](MysqlConnect* conn) {
				lock_guard<mutex> locker(m_mtxQ);
				conn->refreshActiveTime();
				m_connQ.push(conn);
			});

		m_connQ.pop();
	}

	m_cond.notify_all();
	return connptr;

}

int ConnectPool::getCurSize()
{
	return m_curSize;
}

void ConnectPool::stop()
{
	m_running = false;
}

ConnectPool::~ConnectPool()
{
	while (!m_connQ.empty())
	{
		auto conn = m_connQ.front();
		m_connQ.pop();
		delete conn;
	}
}

ConnectPool::ConnectPool():m_running(false),m_curSize(0)
{
	if (!this->parseJsonFile()) return;

	for (int i = 0; i < m_minSize; ++i)
		addConnection();

	thread producer(&ConnectPool::produceConnection, this);
	thread recycler(&ConnectPool::recycleConnection, this);

	m_running = true;

	producer.detach();
	recycler.detach();
}

bool ConnectPool::parseJsonFile()
{
	ifstream file("config.json");
	if (!file.is_open())
	{
		cout << "config json file open failed\n";
		return false;
	}

	string json_string = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
	string err;
	json11::Json json_data = json11::Json::parse(json_string, err);
	if (!err.empty())
	{
		cout << "json parse failed : " << err << "\n";
		return false;
	}

	m_user = json_data["user"].string_value();
	m_passwd = json_data["passwd"].string_value();
	m_ip = json_data["ip"].string_value();
	m_dbname = json_data["dbname"].string_value();
	m_port = json_data["port"].int_value();
	m_minSize = json_data["minSize"].int_value();
	m_maxSize = json_data["maxSize"].int_value();
	m_timeout = json_data["timeout"].int_value();
	m_maxIdleTime = json_data["maxIdleTime"].int_value();

	return true;
}

void ConnectPool::addConnection()
{
	MysqlConnect* conn = new(nothrow) MysqlConnect();
	if (conn == nullptr) return;

	int ret = conn->connect(m_user, m_passwd, m_ip, m_dbname, m_port);
	if (ret)
	{
		conn->refreshActiveTime();
		m_connQ.push(conn);
		++m_curSize;
	}
}

void ConnectPool::produceConnection()
{
	while (m_running)
	{
		{
			unique_lock<mutex> locker(m_mtxQ);
			while (m_connQ.size() >= m_minSize || m_curSize >= m_maxSize)
				m_cond.wait(locker);

			addConnection();
		}

		m_cond.notify_all();
	}
}

void ConnectPool::recycleConnection()
{
	while (m_running)
	{
		this_thread::sleep_for(chrono::milliseconds(500));
		lock_guard<mutex> locker(m_mtxQ);
		while (m_connQ.size() > m_minSize)
		{
			if (m_connQ.front()->getIdleTime() >= m_maxIdleTime)
			{
				auto conn = m_connQ.front();
				m_connQ.pop();
				delete conn;
				--m_curSize;
			}
		}
	}
}
