// MysqlConnect.cpp: 定义应用程序的入口点。
//

#include "MysqlConnect.h"

using namespace std;
using namespace chrono;

MysqlConnect::MysqlConnect():m_conn(nullptr),m_res(nullptr),m_row(nullptr)
{
	m_conn = mysql_init(nullptr);
	mysql_set_character_set(m_conn, "utf8");
}

MysqlConnect::~MysqlConnect()
{
	freeResult();
	if (m_conn != nullptr) {
		mysql_close(m_conn);
		m_conn = nullptr;
	}
}

bool MysqlConnect::connect(string user, string passwd, string ip, string dbname, unsigned short port)
{
	m_conn = mysql_real_connect(m_conn, ip.c_str(), user.c_str(), passwd.c_str(), dbname.c_str(), port, nullptr, 0);
	if (m_conn == nullptr) {
		cout << "mysql connect error : " << mysql_error(m_conn) << endl;
		return false;
	}
	return true;
}

bool MysqlConnect::update(string sql)
{
	int ret = mysql_query(m_conn, sql.c_str());
	if (ret != 0) {
		cout << "mysql query error : " << mysql_error(m_conn) << endl;
		return false;
	}
	return true;
}

bool MysqlConnect::query(string sql)
{
	int ret = mysql_query(m_conn, sql.c_str());
	if (ret != 0) {
		cout << "mysql query error : " << mysql_error(m_conn) << endl;
		return false;
	}
	m_res = mysql_store_result(m_conn);
	if (m_res == nullptr) {
		cout << "mysql store error : " << mysql_error(m_conn) << endl;
		return false;
	}
	return true;
}

bool MysqlConnect::next()
{
	if ((m_row = mysql_fetch_row(m_res)) != nullptr) 
		return true;

	return false;
}

string MysqlConnect::value(int index)
{
	int col = mysql_num_fields(m_res);
	if (index < 0 || index >= col) return string();
	char* val = m_row[index];
	unsigned long len = mysql_fetch_lengths(m_res)[index];
	return string(val, len);
}

bool MysqlConnect::transaction()
{
	return mysql_autocommit(m_conn, false);
}

bool MysqlConnect::commit()
{
	return mysql_commit(m_conn);
}

bool MysqlConnect::rollback()
{
	return mysql_rollback(m_conn);
}

void MysqlConnect::refreshActiveTime()
{
	m_alivetime = steady_clock::now();
}

long long MysqlConnect::getIdleTime()
{
	nanoseconds res = steady_clock::now() - m_alivetime;
	milliseconds millisec = duration_cast<milliseconds>(res);
	return millisec.count();
}

void MysqlConnect::freeResult()
{
	if (m_res == nullptr) {
		mysql_free_result(m_res);
		m_res = nullptr;
	}
}
