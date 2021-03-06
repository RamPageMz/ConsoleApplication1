// ConsoleApplication1.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "stdlib.h"
#include <algorithm>
#include <iostream>
#include <ctime>;
#include <cstdio>;
#include <cstring>;

using namespace std;

#define DBlen 6      //数据库中对象长度
#define TRSlen 6	//每个事务中包含的操作个数
#define TRSnum 3	//共几个事务

typedef struct DB {
	int data[DBlen];	//数据库中对象的值
	int lock[DBlen];	//是否加锁，-1为未加锁，整数值0-2，表示谁对它加的锁
};

typedef struct Operation {
	int oper;	//读或者写，0表示读，1表示写
	int index;	//对数据库中第几个对象进行操作
	int value;	//读写的值
};

typedef struct Transaction {
	Operation operlist[TRSlen];
	int current;	//当前事务位置
	int commit[TRSlen / 2];	//表示事务所涉及的三个对象是否被提交，0表示没提交，1表示提交
	int completelock;	//是否完成加锁阶段，0表示未完成，1表示完成
	int locklist[TRSlen / 2];	//对哪些对象加了锁
	int wait;	//0表示未等待，1表示处于等待状态
};


DB initDB() {	//初始化数据库
	DB db;
	int i;
	for (i = 0; i<DBlen; i++)
	{
		db.data[i] = i + 1;
		db.lock[i] = -1;
	}
	return db;
}

bool ifrepeat(int randx, Transaction trs)
{
	int i;
	for (i = 0; i<TRSlen; i = i + 2)
	{
		if (trs.operlist[i].index == randx)
			return 1;
	}
	return 0;
}

Transaction generatetransaction() {	//随机生成事务
	Transaction trs;
	int i;
	trs.current = 0;
	trs.completelock = 0;
	trs.wait = 0;
	for (i = 0; i<3; i++)
	{
		trs.commit[i] = 0;
		trs.locklist[i] = -1;
	}
	i = 0;
	while (i<TRSlen)
	{
		int ranindex = rand() % 6;
		if (ifrepeat(ranindex, trs) == 1) continue;
		trs.operlist[i].oper = 0;
		trs.operlist[i].index = ranindex;
		trs.operlist[i + 1].oper = 1;
		trs.operlist[i + 1].index = ranindex;
		trs.operlist[i + 1].value = rand() % 20;
		i = i + 2;
	}
	return trs;
}

void printtransaction(Transaction trs, int i) {	//显示生成的事务，i表示第几个事务
	int j;
	cout << "T" << i << ": ";
	for (j = 0; j<TRSlen; j++)
	{
		if (trs.operlist[j].oper == 0)
		{
			cout << "r(x" << trs.operlist[j].index + 1 << ")";
		}
		else
		{
			cout << "w(x" << trs.operlist[j].index + 1 << "," << trs.operlist[j].value << ") ";
		}
	}
	cout << endl;
}

void printDB(DB db) {	//显示数据库
	cout << "[";
	int i;
	for (i = 0; i<DBlen; i++)
	{
		cout << " " << db.data[i] << " ";
	}
	cout << "]" << endl;
}

DB executeOperation(DB db, Operation *op) {
	if (op->oper == 0)//读
	{
		op->value = db.data[op->index];
	}
	else//写
	{
		db.data[op->index] = op->value;
	}
	return db;
}

DB executeTransation(DB db, Transaction *trs) {
	int j;
	for (j = 0; j<TRSlen; j++)
	{
		db = executeOperation(db, trs->operlist + j);
		trs->current++;
	}
	return db;
}

void resetTransation(Transaction *trs)
{
	int i, j;
	for (i = 0; i<TRSnum; i++)
	{
		(trs + i)->current = 0;
		(trs + i)->completelock = 0;
		(trs + i)->wait = 0;
		for (j = 0; j<3; j++)
		{
			(trs + i)->commit[j] = 0;
			(trs + i)->locklist[j] = -1;
		}
	}
	return;
}


int main() {
	srand((unsigned)time(NULL));
	DB mydb = initDB();
	Transaction mytrs[TRSnum];
	int i, k, m;
	for (i = 0; i<TRSnum; i++) {
		mytrs[i] = generatetransaction();
	}

	cout << "随机生成事务：" << endl;
	for (i = 0; i<TRSnum; i++) {
		printtransaction(mytrs[i], i);
	}

	/******顺序执行*********/
	cout << "****************顺序执行*******************" << endl;
	for (i = 0; i<TRSnum; i++)
	{
		for (k = 0; k<TRSnum; k++)
		{
			if (i == k) continue;
			for (m = 0; m<TRSnum; m++)
			{
				if (m == k || m == i) continue;
				cout << "(T" << i << ",T" << k << ",T" << m << ")" << endl;

				mydb = executeTransation(mydb, mytrs + i);
				mydb = executeTransation(mydb, mytrs + k);
				mydb = executeTransation(mydb, mytrs + m);

				cout << "Final DB:";
				printDB(mydb);

				resetTransation(mytrs);
				mydb = initDB();   //重置数据库的元素
			}
		}
	}

	/******随机执行*********/
	cout << "****************随机执行*******************" << endl;
	while (mytrs[0].current<6 || mytrs[1].current<6 || mytrs[2].current<6)
	{
		i = rand() % 3;
		if (mytrs[i].current>5)	continue;
		mydb = executeOperation(mydb, (mytrs[i].operlist + mytrs[i].current));
		if ((mytrs[i].operlist + mytrs[i].current)->oper == 0)
		{
			cout << "r" << i << "(x" << (mytrs[i].operlist + mytrs[i].current)->index + 1 << "): " << (mytrs[i].operlist + mytrs[i].current)->value << endl;
		}
		else
		{
			cout << "w" << i << "(x" << (mytrs[i].operlist + mytrs[i].current)->index + 1 << "," << (mytrs[i].operlist + mytrs[i].current)->value << "):      DB=";
			printDB(mydb);
		}
		mytrs[i].current++;
	}

	cout << "Final DB:";
	printDB(mydb);

	resetTransation(mytrs);
	mydb = initDB();   //重置数据库的元素


	/****************2PL执行*************************/
	//判断死锁使用
	cout << "****************2PL执行*******************" << endl;
	int lasti[2] = { -1,-1 };
	while (mytrs[0].current<6 || mytrs[1].current<6 || mytrs[2].current<6)
	{
		i = rand() % 3;
		if (mytrs[i].current>5)	continue;

		//防止随机数重复生成两个正在等待的事务,而不去执行第三个可执行的事务
		if ((lasti[0] != -1 && lasti[1] != -1) && (mytrs[lasti[0]].wait == 1 && mytrs[lasti[1]].wait == 1) && (i == lasti[0] || i == lasti[1]))
		{
			for (int r = 0; r<3; r++)
			{
				if (r != lasti[0] && r != lasti[1])
					i = r;
			}
		}

		lasti[0] = lasti[1];//记录前两次执行的事务序号
		lasti[1] = i;


		if (mydb.lock[(mytrs[i].operlist + mytrs[i].current)->index] == -1)//判断该对象有没有锁
		{
			mytrs[i].wait = 0;

			mydb.lock[(mytrs[i].operlist + mytrs[i].current)->index] = i;//加锁
			cout << "l" << i << "(x" << (mytrs[i].operlist + mytrs[i].current)->index + 1 << ")" << endl;
			mytrs[i].locklist[mytrs[i].current / 2] = (mytrs[i].operlist + mytrs[i].current)->index;	//将当前操作对象加入锁列表

			if (mytrs[i].current == 4)//完成所有加锁
				mytrs[i].completelock = 1;

			//读写操作
			mydb = executeOperation(mydb, (mytrs[i].operlist + mytrs[i].current));
			//打印读写结果
			if ((mytrs[i].operlist + mytrs[i].current)->oper == 0)
			{
				cout << "r" << i << "(x" << (mytrs[i].operlist + mytrs[i].current)->index + 1 << "): " << (mytrs[i].operlist + mytrs[i].current)->value << endl;
			}
			else
			{
				cout << "w" << i << "(x" << (mytrs[i].operlist + mytrs[i].current)->index + 1 << "," << (mytrs[i].operlist + mytrs[i].current)->value << "):      DB=";
				printDB(mydb);
				mytrs[i].commit[(mytrs[i].current - 1) / 2] = 1;
			}

			mytrs[i].current++;//指向该事务的下一个操作

		}
		else
		{
			if (mydb.lock[(mytrs[i].operlist + mytrs[i].current)->index] == i)//判断是否是自己的锁
			{
				mytrs[i].wait = 0;

				//进行读写
				mydb = executeOperation(mydb, (mytrs[i].operlist + mytrs[i].current));
				//打印读写结果
				if ((mytrs[i].operlist + mytrs[i].current)->oper == 0)
				{
					cout << "r" << i << "(x" << (mytrs[i].operlist + mytrs[i].current)->index + 1 << "): " << (mytrs[i].operlist + mytrs[i].current)->value << endl;
				}
				else
				{
					cout << "w" << i << "(x" << (mytrs[i].operlist + mytrs[i].current)->index + 1 << "," << (mytrs[i].operlist + mytrs[i].current)->value << "):      DB=";
					printDB(mydb);
					mytrs[i].commit[(mytrs[i].current - 1) / 2] = 1;
				}

				mytrs[i].current++;//指向该事务的下一个操作

			}
			else
			{
				cout << "l" << i << "(x" << (mytrs[i].operlist + mytrs[i].current)->index + 1 << ")        Refused!Waiting..." << endl;	//拒绝操作，等待
				mytrs[i].wait = 1;
			}
		}

		//解锁阶段
		int j;
		for (j = 0; j<TRSlen / 2; j++)
		{
			if (mytrs[i].commit[j] == 1 && mytrs[i].completelock == 1 && mytrs[i].locklist[j] != -1)
			{
				mydb.lock[mytrs[i].locklist[j]] = -1;   //解锁
				mytrs[i].locklist[j] = -1;
			}
		}

		//判断死锁
		if (mytrs[0].wait + mytrs[1].wait + mytrs[2].wait >= 3)
		{
			cout << "发生死锁！！重置！" << endl;
			resetTransation(mytrs);
			mydb = initDB();   //重置数据库的元素
		}

	}

	cout << "Final DB:";
	printDB(mydb);


	system("pause");

	return 0;

}
