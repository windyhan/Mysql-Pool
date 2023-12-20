#include "ConnectPool.h"
#include <iostream>
#include <thread>
#include <chrono>


using namespace std;
using std::chrono::steady_clock;

void test_query()
{
	string sql = "select * from students limit 10";
	auto pool = ConnectPool::getPool();
	auto conn = pool->getConnection();
	if (conn != nullptr)
	{
		if (conn->query(sql))
		{
			while (conn->next())
			{
				for (int i = 0; i < 5; ++i)
					cout << conn->value(i) << "\t";
				cout << "\n";
			}

		}
		else cout << "mysql query failed\n";

	}
	else cout << "get connection from pool failed\n";
}

void clear_table_students()
{
	ConnectPool* pool = ConnectPool::getPool();
	shared_ptr<MysqlConnect> conn = pool->getConnection();
	string sql = "delete from students where 1=1";
	conn->update(sql);
}

void op1(int begin, int end) 
{//不适用连接池插入数据
	for (int i = begin; i < end; i++) 
	{
		MysqlConnect conn;
		conn.connect("han", "123Ert*k23", "192.168.0.154", "testdb");
		char sql[1024] = { 0 };
		sprintf(sql, "INSERT INTO `testdb`.`students` (`id`, `name`, `sex`, `age`, `score`) VALUES ('%d', 'Sarah', 'Female', '18', '89');", i);
		conn.update(sql);
	}
}

void op2(ConnectPool* pool, int begin, int end)
{//使用连接池插入数据
	for (int i = begin; i < end; ++i)
	{
		shared_ptr<MysqlConnect> conn = pool->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "INSERT INTO `testdb`.`students` (`id`, `name`, `sex`, `age`, `score`) VALUES ('%d', 'Sarah', 'Female', '18', '89');", i);
		conn->update(sql);
	}
}
void test_st() {

	clear_table_students();

#if 0
	steady_clock::time_point begin = steady_clock::now();
	op1(0, 5000);
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "非连接池，单线程，用时：" << length.count() << "纳秒，" << length.count() / 1000000 << "毫秒" << endl;
#else
	ConnectPool* pool = ConnectPool::getPool();
	steady_clock::time_point begin = steady_clock::now();
	op2(pool, 0, 5000);
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "连接池，单线程，用时：" << length.count() << "纳秒，" << length.count() / 1000000 << "毫秒" << endl;
#endif
}


void test_mt() {

	clear_table_students();

#if 0
	steady_clock::time_point begin = steady_clock::now();
	thread t1(op1, 0, 1000);
	thread t2(op1, 1000, 2000);
	thread t3(op1, 2000, 3000);
	thread t4(op1, 3000, 4000);
	thread t5(op1, 4000, 5000);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "非连接池，多线程，用时：" << length.count() << "纳秒，" << length.count() / 1000000 << "毫秒" << endl;
#else
	steady_clock::time_point begin = steady_clock::now();
	ConnectPool* pool = ConnectPool::getPool();
	thread t1(op2, pool, 0, 1000);
	thread t2(op2, pool, 1000, 2000);
	thread t3(op2, pool, 2000, 3000);
	thread t4(op2, pool, 3000, 4000);
	thread t5(op2, pool, 4000, 5000);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "连接池，多线程，用时：" << length.count() << "纳秒，" << length.count() / 1000000 << "毫秒" << endl;
#endif 
}
//连接池，单线程，用时：3489763300纳秒，3489毫秒
//非连接池，单线程，用时：25283452100纳秒，25283毫秒

//连接池，多线程，用时：871321400纳秒，871毫秒
//非连接池，多线程，用时：7290033400纳秒，7290毫秒

void testMaxSize()
{
	int num = 120;
	auto pool = ConnectPool::getPool();
	{
		vector<shared_ptr<MysqlConnect>> v(num);
		for (int i = 0; i < num; ++i)
			v[i] = pool->getConnection();
		cout << "当前连接池数量 ：" << pool->getCurSize() << endl;//输出设定的maxSize = 100
	}
	//让连接池闲置，测试recycler线程销毁闲置的连接
	this_thread::sleep_for(chrono::milliseconds(5000));
	cout << "当前连接池数量 ：" << pool->getCurSize() << endl;//输出初始设定minSize = 10
}

int main()
{
	//test_query();
	testMaxSize();
	return 0;
}

