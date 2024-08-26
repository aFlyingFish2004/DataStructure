//本代码运行环境为Visiual Studio 2022
#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<stack>
#include<cmath>
#include<ctime>
#include <thread>
using namespace std;

struct ArcNode
{
	int dist;			//相邻两站的距离
	double time;		//相邻两站的时间（算上了停靠的一分钟）
	string adjs;			//相邻站点名
	ArcNode* next;		//指向下一条弧
	ArcNode() { next = NULL; } //初始化next指针
};

struct station
{
	string name;		//站点名称
	int line_num;		//存储该站属于多少条线(若为中转站则line_num>1)
	vector<string> line;		//站点所属的地铁线		
	ArcNode* firstarc;			//指向第一条弧
	station() { line_num = 0; firstarc = NULL; }    //初始化站点信息
};

class Graph
{
private:
	int hour, minute;    //存储当前时间
	vector<station> sta_list;			//存储每个站点的信息
	int total_num;						//站点的总数
	bool S[500];	//Dijkstra算法中记录节点是否已被添加		
	int Disk[500];		//Dijkstra算法中记录源节点到各个顶点的距离
	double Time[500];   //Dijkstra算法中记录源节点到各个顶点的时间
	int Path[500];		//Dijkstra算法中记录路径便于回溯输出路径
	bool judge_line(string s, vector<string> transfer);     //判断字符串s在不在transfer内，若在则返回true，否则返回false
	int finddist(int start, int end);			//Dijkstra算法中寻找start和end对应节点之间的距离
	double findtime(int start, int end);        //Dijkstra算法中寻找start和end对应节点之间的时间
	int findmindisk();			 //Dijkstra算法中寻找与当前节点距离最近的节点的索引
	int findmintime();			//Dijkstra算法中寻找到当前节点所需时间最短的节点的索引
	void calculate(vector<int> path, int& distance, int& price, int& time);		//计算路径的总距离、价格、时间
	bool Transfer(vector<int> path, int index, string& s, string& e);			//判断当前站点是否为中转站，若是则将换乘信息保存
	bool Transfer(vector<int> path, int index);									//判断当前站点是否为中转站，与上一个函数形成重载
	string return_sameline(vector<string>a, vector<string>b);					//寻找a与b中相同的线路名，找到则返回该名，未找到则返回空串
	string return_startline(vector<int>a);							//寻找起点站所属线路
public:
	Graph(ifstream& load);		//构造函数
	void Print();				//输出保存的全部站点信息
	int find(string s);			//找到名为s的站点在sta_list中的下标
	bool judge_name(vector<string> a, string b);			//在a中查找b是否存在
	void ShortestTransfer(string start, string end);	//寻找最少换乘线路
	void ShortestDistance(string start, string end);	//寻找最短路径线路
	void PrintShortest(string start ,string end);		//输出函数
	void ShortestTime(string start, string end);		//寻找最短时间线路
	void AddStation(string _line_name, string station_name[], string station_name2[], int _dist[], int _station_num, int speed);		//添加线路
	void DeleteStation(string _line_name);		//删除线路
	void SetTime(int h, int m)					//设置时间
	{
		hour = h; minute = m;
	}		
};

Graph::Graph(ifstream& load)				
{
	load.open("my_source//subway.txt", ios::in);
	if (!load.is_open())
	{
		cout << "文件打开失败";
	}
	else
	{
		total_num = 0;
		string word;
		load >> word;					//读入地铁站总线路数
		int line_num = atoi(word.c_str());		
		for (int i = 0; i < line_num; i++)
		{
			load >> word;						//读入地铁线名称
			string line_name = word;			
			load >> word;						//读入地铁线站点数
			int station_num = atoi(word.c_str());
			//读取每条线的信息
			int speed;
			load >> word;			//读入地铁线的速度
			speed = atoi(word.c_str());
			for (int i = total_num; i < total_num + station_num-1; i++)
			{
				//每次读入一行的两个地铁站，在下面的注释中，先读入的节点简称前节点，后读入的简称后节点
				string name1, name2;
				int distance;
				load >> name1;				//读入前节点名称
				load >> name2;				//读入后节点名称
				load >> word;				//读入两个节点间距离
				distance = atoi(word.c_str());
				double t = static_cast<double>(distance) / speed + 1;  //计算时间（算上停靠的一分钟）
				if (find(name1) == -1 && find(name2) == -1)		//当 前后节点均未读入过时
				{
					//初始化前节点
					station station1;
					station1.line.push_back(line_name);
					station1.name = name1;
					station1.line_num = station1.line.size();
					ArcNode* s1 = new ArcNode;
					s1->adjs = name2;
					s1->dist = distance;
					s1->time = t;
					s1->next = station1.firstarc;
					station1.firstarc = s1;
					sta_list.push_back(station1);
					//初始化后节点
					station station2;
					station2.line.push_back(line_name);;
					station2.name = name2;
					station2.line_num = station2.line.size();
					ArcNode* s2 = new ArcNode;
					s2->adjs = name1;
					s2->dist = distance;
					s2->time = t;
					s2->next = station2.firstarc;
					station2.firstarc = s2;
					sta_list.push_back(station2);
				}
				else if (find(name1) != -1 && find(name2) == -1)  //当 前节点读入过，后节点未读入过时
				{
					//初始化后节点
					station station2;
					station2.line.push_back(line_name);
					station2.name = name2;
					station2.line_num = station2.line.size();
					ArcNode* s2 = new ArcNode;
					s2->adjs = name1;
					s2->dist = distance;
					s2->time = t;
					s2->next = station2.firstarc;
					station2.firstarc = s2;
					sta_list.push_back(station2);
					//初始化前节点
					ArcNode* s1 = new ArcNode;
					s1->adjs = name2;
					s1->dist = distance;
					s1->time = t;
					s1->next = sta_list[find(name1)].firstarc;
					sta_list[find(name1)].firstarc = s1;
					if (judge_name(sta_list[find(name1)].line,line_name))		//判断读入过的站点先前读入时与本次读入是否在同一条地铁线（由于环线的起点和终点相同会再次读入该点）
					{
						sta_list[find(name1)].line.push_back(line_name);		//不相同时
						sta_list[find(name1)].line_num = sta_list[find(name1)].line.size();
					}
				}
				else if (find(name1) == -1 && find(name2) != -1)	//当前节点未读入过，后节点读入过时
				{
					//初始化前节点
					station station1;
					station1.line.push_back(line_name);
					station1.name = name1;
					station1.line_num = station1.line.size();
					ArcNode* s1 = new ArcNode;
					s1->adjs = name2;
					s1->dist = distance;
					s1->time = t;
					s1->next = station1.firstarc;
					station1.firstarc = s1;
					sta_list.push_back(station1);
					//初始化后节点
					if (judge_name(sta_list[find(name2)].line, line_name))		//同上
					{
						sta_list[find(name2)].line.push_back(line_name);
						sta_list[find(name2)].line_num = sta_list[find(name2)].line.size();
					}
					ArcNode* s2 = new ArcNode;
					s2->adjs = name1;
					s2->dist = distance;
					s2->time = t;
					s2->next = sta_list[find(name2)].firstarc;
					sta_list[find(name2)].firstarc = s2;
				}
				else							//当前后节点均读入过时
				{
					ArcNode* s1 = new ArcNode;
					s1->adjs = name2;
					s1->dist = distance;
					s1->time = t;
					s1->next = sta_list[find(name1)].firstarc;
					sta_list[find(name1)].firstarc = s1;
					ArcNode* s2 = new ArcNode;
					s2->adjs = name1;
					s2->dist = distance;
					s2->time = t;
					s2->next = sta_list[find(name2)].firstarc;
					sta_list[find(name2)].firstarc = s2;
					if (judge_name(sta_list[find(name1)].line, line_name))	//同上
					{
						sta_list[find(name1)].line.push_back(line_name);
						sta_list[find(name1)].line_num = sta_list[find(name1)].line.size();
					}
					if (judge_name(sta_list[find(name2)].line, line_name))	//同上
					{
						sta_list[find(name2)].line.push_back(line_name);
						sta_list[find(name2)].line_num = sta_list[find(name2)].line.size();
					}
				}
			}
			total_num = sta_list.size();
		}
		load.close();
	}
}

string Graph::return_sameline(vector<string>a, vector<string>b)			//判断a与b中是否存在相同的字符串，若存在则返回该字符串，否则返回空
{
	for (int i = 0; i < a.size(); i++)
	{
		for (int j = 0; j < b.size(); j++)
		{
			if (a[i] == b[j])
			{
				return a[i];
			}
		}
	}
	return "";
}

bool Graph::Transfer(vector<int> path, int index, string& s, string& e)
{
	if (sta_list[path[index]].line_num == 1)		//当前站点不是中转站时
		return false;
	if (index-1 >= 0&& index+1 < path.size())  //当前站有前驱和后继时
	{
		if (return_sameline(sta_list[path[index - 1]].line, sta_list[path[index + 1]].line).empty())			//换乘了
		{
			s = return_sameline(sta_list[path[index - 1]].line, sta_list[path[index]].line);		//换乘了前两个站点共有的线路一定是换乘前的线路，后两个站点共有的线路一定是换乘后的线路
			e = return_sameline(sta_list[path[index]].line, sta_list[path[index + 1]].line);
			return true;
		}
		else
			return false;
	}
	else if (index - 1 < 0)	//无前驱，则当前站为起点站，必定不换乘
	{
		return false;
	}
	else if (index + 1 >= path.size())			//无后继，则当前站为终点站，必定不换乘
	{
		return false;
	}
}

bool Graph::Transfer(vector<int> path, int index)
{
	if (sta_list[path[index]].line_num == 1)
		return false;
	if (index - 1 >= 0 && index + 1 < path.size())  //当前站有前驱和后继时
	{
		if (return_sameline(sta_list[path[index - 1]].line, sta_list[path[index + 1]].line).empty())			//换乘了
			return true;	
		else
			return false;
	}
	else if (index - 1 < 0)	//无前驱，则当前站为起点站，必定不换乘
	{
		return false;
	}
	else if (index + 1 >= path.size())			//无后继，则当前站为终点站，必定不换乘
	{
		return false;
	}
}

void Graph::Print()
{
	cout << "站点总数是：" << total_num << endl;
	for (int i = 0; i < total_num; i++)
	{
		if (sta_list[i].name == "")
		{
			cout << "编号：" << i << '\t' << "站点为空" << endl << endl;
		}
		else
		{
			cout << "编号：" << i << '\t' << "站点名称：" << sta_list[i].name << '\t';
			if (sta_list[i].line_num == 1)
			{
				cout << "所属线路：" << sta_list[i].line[0] << endl;
			}
			else
			{
				cout << "该站为中转站,共有" << sta_list[i].line_num << "条线路经过" << endl;
			}
			if (sta_list[i].firstarc)
			{
				cout << "相邻站点：" << endl;
				ArcNode* p = sta_list[i].firstarc;
				while (p)
				{
					cout << p->adjs << ':' << p->dist << '\t' << p->time << '\t';
					this_thread::sleep_for(chrono::nanoseconds(500));
					p = p->next;
				}
				cout << endl << endl;
			}
		}
	}
}

int Graph::find(string s)		//返回站点s的下标
{
	for (int i = 0; i < sta_list.size(); i++)
	{
		if (sta_list[i].name == s)
			return i;
	}
	return -1;
}

bool Graph::judge_name(vector<string> a, string b)	//当字符串b存在于a中时返回false，不存在返回true
{
	for (int i = 0; i < a.size(); i++)
	{
		if (a[i] == b)
			return false;
	}
	return true;
}

bool Graph::judge_line(string s, vector<string> transfer)		//判断字符串s在不在transfer内，若在则返回true，否则返回false
{
	if (transfer.empty())
		return false;
	for (int i = 0; i < transfer.size(); i++)
	{
		if (s == transfer[i])
			return true;
	}
	return false;
}

void Graph::ShortestDistance(string start, string end)  //使用Dijkstra算法
{
	//考虑情况前面的数组
	int start_index = find(start);
	int end_index = find(end);
	for (int i = 0; i < total_num; i++)		//初始化辅助数组
	{
		Disk[i] = 99999;
		Path[i] = -1;
		S[i] = false;
	}
	for (int i = 0; i < total_num; i++)
	{
		Disk[i] = finddist(start_index, i);
		if (Disk[i] != 99999)
			Path[i] = start_index;
	}
	S[start_index] = true;		//初始化起点
	for (int i = 0; i < total_num; i++)
	{
		int v = findmindisk();
		if (v == -1)
			return;		
		S[v] = true;			//v加入到s中
		for (int j = 0; j < total_num; j++)		//更新辅助数组
		{
			if (!S[j] && (Disk[j] > finddist(v, j) + Disk[v]))
			{
				Disk[j] = finddist(v, j) + Disk[v];
				Path[j] = v;
			}
		}
	}
}

void Graph::ShortestTransfer(string start, string end)
{
	//考虑情况前面的数组
	int start_index = find(start);
	int end_index = find(end);
	for (int i = 0; i < total_num; i++)		//初始化辅助数组
	{
		Time[i] = 99999;
		Path[i] = -1;
		S[i] = false;
	}
	for (int i = 0; i < total_num; i++)
	{
		Time[i] = findtime(start_index, i);
		if (Time[i] != 99999)
			Path[i] = start_index;
	}
	S[start_index] = true;		//初始化起点
	for (int i = 0; i < total_num; i++)
	{
		int v = findmintime();			//v为当前节点
		if (v == -1)
			return;
		S[v] = true;
		for (int j = 0; j < total_num; j++)		//j为v的下一个节点，Path[v]为v的前一个节点
		{
			if (!S[j])		//该站点未进入S
			{
				if (return_sameline(sta_list[Path[v]].line, sta_list[j].line).empty())		//若换乘了
				{
					//此处把换乘时间设为一个足够大的时间（此处设为100），这样Dijkstra可以找到换乘次数最少的方案，并且换乘次数相同时优先找到时间最少的方案
					if (Time[j] > findtime(v, j) + Time[v] + 100)
					{
						Time[j] = findtime(v, j) + Time[v] + 100;		
						Path[j] = v;
					}
				}
				else		//若未换乘
				{
					if (Time[j] > findtime(v, j) + Time[v])
					{
						Time[j] = findtime(v, j) + Time[v];
						Path[j] = v;
					}
				}
			}
		}
	}
}

void Graph::ShortestTime(string start, string end)
{
	//考虑情况前面的数组
	int start_index = find(start);
	int end_index = find(end);
	for (int i = 0; i < total_num; i++)		//初始化辅助数组
	{
		Time[i] = 99999;
		Path[i] = -1;
		S[i] = false;
	}
	for (int i = 0; i < total_num; i++)
	{
		Time[i] = findtime(start_index, i);
		if (Time[i] != 99999)
			Path[i] = start_index;
	}
	S[start_index] = true;		//初始化起点
	for (int i = 0; i < total_num; i++)
	{
		int v = findmintime();			//v为当前节点
		if (v == -1)
			return;
		S[v] = true;
		for (int j = 0; j < total_num; j++)		//j为v的下一个节点，Path[v]为v的前一个节点
		{
			if (!S[j])		//该站点未进入S
			{
				if (return_sameline(sta_list[Path[v]].line, sta_list[j].line).empty())		//若换乘了，更新辅助数组时要加入换乘等待时间
				{
					if (Time[j] > findtime(v, j) + Time[v] + 8)
					{
						Time[j] = findtime(v, j) + Time[v] + 8;		//换乘时假设等待9分钟（因为弧节点本来就包含等待的1分钟，此处再加8，共9分钟）
						Path[j] = v;
					}
				}
				else		//若未换乘，则更新辅助数组时无需加入换乘等待时间
				{
					if (Time[j] > findtime(v, j) + Time[v])
					{
						Time[j] = findtime(v, j) + Time[v];
						Path[j] = v;
					}
				}
			}
		}
	}
}

double Graph::findtime(int start, int end)
{
	if (start == end)
		return 0;			//起点和终点一样，时间为0
	ArcNode* p = sta_list[start].firstarc;
	while (p)
	{
		if (find(p->adjs) == end)					//找到后返回距离
			return p->time;
		p = p->next;
	}
	return 99999;		//未找到，则返回大数表示此路不通
}

int Graph::finddist(int start, int end)
{
	if (start == end)
		return 0;			//起点和终点一样，距离为0
	ArcNode* p = sta_list[start].firstarc;
	while (p)
	{
		if (find(p->adjs) == end)					//找到后返回距离
			return p->dist;
		p = p->next;
	}
	return 99999;		//未找到，则返回大数表示此路不通
}

int Graph::findmintime()
{
	int temp = 0;
	double min = 99999;
	for (int i = 0; i < total_num; i++)
	{
		if (!S[i] && min > Time[i])		//节点没进入过S才更新
		{
			min = Time[i];
			temp = i;
		}
	}
	if (min == 99999)
		return -1;
	return temp;
}

int Graph::findmindisk()
{
	int temp = 0, min = 99999;
	for (int i = 0; i < total_num; i++)
	{
		if (!S[i] && min > Disk[i])
		{
			min = Disk[i];
			temp = i;
		}
	}
	if (min == 99999)
		return -1;
	return temp;
}

void Graph::PrintShortest(string start, string end)
{
	int start_index = find(start);
	int end_index = find(end);
	for (int i = 0; i < total_num; i++)
	{
		if (end_index == i)				//Dijkstra算法会找到源节点到所有节点的距离，实际应用中只需要输出到目标节点的距离
		{
			stack<int> s;					//Dijkstra算法中用Path[]记录回溯路径，若直接输出则是从终点到起点，为了正序输出利用栈的特性转换顺序
			if (Path[i] != -1 && i != start_index)
			{
				s.push(i);
				int pre = Path[i];
				while (pre != start_index)		//沿Path[]记录的路径回溯，直到回溯到起点位置
				{
					s.push(pre);
					pre = Path[pre];
				}
				s.push(start_index);
				vector<int> v;
				while (!s.empty())			//将栈中存放的数据依次存入向量中，方便后续分析时间，金钱以及路程等信息
				{
					v.push_back(s.top());
					s.pop();
				}
				int distance, price, time;
				calculate(v, distance, price, time);		//计算时间，金钱以及路程等信息
				int h = (hour += time / 60 + 1) % 24;
				int m = (minute += time % 60) % 60;
				cout << "————————————————————————————————————————————————————————" << endl;
				cout << "本线路总距离为" << distance << "米，花费的总费用为" << price << "元,总时间为" << time << "分钟" << endl;
				cout << "********************************************************" << endl;
				cout << "                  预计";
				if (h < 10)
					cout << '0';
				cout << h << ':';
				if (m < 10)
					cout << '0';
				cout << m << "到达" << endl;
				cout << "********************************************************" << endl;
				cout << "         请到" << sta_list[v[0]].name << "站上车，乘坐地铁" << return_startline(v) << endl;
				cout << "————————————————————————————————————————————————————————" << endl;
				for (int i = 0; i < v.size(); i++)
				{
					string s, e;
					if (Transfer(v, i, s, e))
					{
						cout << "           （即将在" << sta_list[v[i]].name << "站换乘地铁" << e << "）" << endl;
						cout << "                   ***" << sta_list[v[i]].name << "***";
					}
					else
						cout << "                      " << sta_list[v[i]].name << "  ";
					cout << endl;
				}
				cout << "————————————————————————————————————————————————————————" << endl;
				cout << endl << endl;
			}
			break;
		}	
	}
}

void Graph::AddStation(string _line_name, string station_name1[], string station_name2[], int _dist[], int _station_num, int speed)
{
	//增加算法的思路与图的构造函数思路基本一致
	string line_name = _line_name;
	int station_num = _station_num;
	for (int i = total_num, j = 0; i < total_num + station_num - 1; i++, j++)
	{
		//每次读入一行的两个地铁站，在下面的注释中，先读入的简称前节点，后读入的简称后节点
		string name1, name2;
		name1 = station_name1[j];				//读入前节点名称
		name2 = station_name2[j];			//读入后节点名称
		int distance = _dist[j];				//读入两个节点间距离
		double t = static_cast<double>(distance) / speed + 1; //计算时间
		if (find(name1) == -1 && find(name2) == -1)		//当前后节点均未读入过时
		{
			//初始化前节点
			station station1;
			station1.line.push_back(line_name);
			station1.name = name1;
			station1.line_num = station1.line.size();
			ArcNode* s1 = new ArcNode;
			s1->adjs = name2;
			s1->dist = distance;
			s1->time = t;
			s1->next = station1.firstarc;
			station1.firstarc = s1;
			sta_list.push_back(station1);
			//初始化后节点
			station station2;
			station2.line.push_back(line_name);;
			station2.name = name2;
			station2.line_num = station2.line.size();
			ArcNode* s2 = new ArcNode;
			s2->adjs = name1;
			s2->dist = s1->dist;
			s2->time = t;
			s2->next = station2.firstarc;
			station2.firstarc = s2;
			sta_list.push_back(station2);
		}
		else if (find(name1) != -1 && find(name2) == -1)  //当 前节点读入过，后节点未读入过时
		{
			//初始化后节点
			station station2;
			station2.line.push_back(line_name);
			station2.name = name2;
			station2.line_num = station2.line.size();
			ArcNode* s2 = new ArcNode;
			s2->adjs = name1;
			s2->dist = distance;
			s2->time = t;
			s2->next = station2.firstarc;
			station2.firstarc = s2;
			sta_list.push_back(station2);
			//初始化前节点
			ArcNode* s1 = new ArcNode;
			s1->adjs = name2;
			s1->dist = s2->dist;
			s1->time = t;
			s1->next = sta_list[find(name1)].firstarc;
			sta_list[find(name1)].firstarc = s1;
			if (judge_name(sta_list[find(name1)].line, line_name))
			{
				sta_list[find(name1)].line.push_back(line_name);
				sta_list[find(name1)].line_num = sta_list[find(name1)].line.size();
			}
		}
		else if (find(name1) == -1 && find(name2) != -1)	//当前节点未读入过，后节点读入过时
		{
			//初始化前节点
			station station1;
			station1.line.push_back(line_name);
			station1.name = name1;
			station1.line_num = station1.line.size();
			ArcNode* s1 = new ArcNode;
			s1->adjs = name2;
			s1->dist = distance;
			s1->time = t;
			s1->next = station1.firstarc;
			station1.firstarc = s1;
			sta_list.push_back(station1);
			//初始化后节点
			if (judge_name(sta_list[find(name2)].line, line_name))
			{
				sta_list[find(name2)].line.push_back(line_name);
				sta_list[find(name2)].line_num = sta_list[find(name2)].line.size();
			}
			ArcNode* s2 = new ArcNode;
			s2->adjs = name1;
			s2->dist = s1->dist;
			s2->time = t;
			s2->next = sta_list[find(name2)].firstarc;
			sta_list[find(name2)].firstarc = s2;
		}
		else							//当前后节点均读入过时
		{
			ArcNode* s1 = new ArcNode;
			s1->adjs = name2;
			s1->dist = distance;
			s1->time = t;
			s1->next = sta_list[find(name1)].firstarc;
			sta_list[find(name1)].firstarc = s1;
			ArcNode* s2 = new ArcNode;
			s2->adjs = name1;
			s2->dist = s1->dist;
			s2->time = t;
			s2->next = sta_list[find(name2)].firstarc;
			sta_list[find(name2)].firstarc = s2;
			if (judge_name(sta_list[find(name1)].line, line_name))
			{
				sta_list[find(name1)].line.push_back(line_name);
				sta_list[find(name1)].line_num = sta_list[find(name1)].line.size();
			}
			if (judge_name(sta_list[find(name2)].line, line_name))
			{
				sta_list[find(name2)].line.push_back(line_name);
				sta_list[find(name2)].line_num = sta_list[find(name2)].line.size();
			}
		}
	}
	total_num = sta_list.size();
}

void Graph::DeleteStation(string _line_name)
{
	for (int i = 0; i < total_num; i++)
	{
		if (sta_list[i].line_num == 1)			//待删除的站点非中转站
		{
			if (sta_list[i].line[0] == _line_name)		
			{
				sta_list.erase(sta_list.begin() + i);
				total_num = sta_list.size();
				i--;				//删除后vector会将下标依次前移，此时应i--来保证继续向下检测而不是跳过一个
			}
		}
		else if (sta_list[i].line_num > 1)		//待删除的站点为中转站
		{
			if (!judge_name(sta_list[i].line, _line_name))		//若待删除的站点中包含需要删除的线路
			{
				for (int j = 0; j < sta_list[i].line.size(); j++)		//遍历线路，找到需要删除的线路删除
				{
					if (sta_list[i].line[j] == _line_name)
					{
						sta_list[i].line.erase(sta_list[i].line.begin() + j);
						sta_list[i].line_num = sta_list[i].line.size();
						break;
					}
				}
				//检测中转站相邻站点的弧中有没有需要删除的站点     
				ArcNode* p = sta_list[i].firstarc;
				while (p)
				{
					if (find(p->adjs) == -1)			//此时说明弧对应站点已经被删，则被删的必定不是中转站
					{
						ArcNode* q = sta_list[i].firstarc;
						if (q == p)
						{
							p = p->next;
							delete q;
							sta_list[i].firstarc = p;
						}
						else
						{
							while (q->next != p)
								q = q->next;
							q->next = p->next;
							q = p;
							p = p->next;
							delete q;
						}		
					}
					else if (!judge_name(sta_list[find(p->adjs)].line, _line_name))			//若弧结点连接的站点没有被删，并且站点被待删线路经过
					{
						if (sta_list[find(p->adjs)].line_num == 1)		//该站点不是中转站,则在本次删除中最终会被删掉，不与本站点相邻
						{
							ArcNode* q = sta_list[i].firstarc;
							if (q == p)
							{
								p = p->next;
								delete q;
								sta_list[i].firstarc = p;
							}
							else
							{
								while (q->next != p)
									q = q->next;
								q->next = p->next;
								q = p;
								p = p->next;
								delete q;
							}
						}
						else if (sta_list[find(p->adjs)].line_num > 1)	//p->adjs对应的节点为中转站，则本次删除后该站依然存在,但不再与本站相邻（原因是相邻站点间只有一条线路）
						{
							ArcNode* q = sta_list[i].firstarc;
							if (q == p)
							{
								p = p->next;
								delete q;
								sta_list[i].firstarc = p;
							}
							else
							{
								while (q->next != p)
									q = q->next;
								q->next = p->next;
								q = p;
								p = p->next;
								delete q;
							}
						}
					}
					else		//站点未被删，且并未被待删线路经过
						p=p->next;
				}
			}
		}
	}
}

void Graph::calculate(vector<int> path, int& distance, int& price, int& time)
{
	int dis = 0;
	int pri = 0;
	double t = 0;
	for (int i = 0; i < path.size()-1; i++)
	{
		ArcNode* p;
		p = sta_list[path[i]].firstarc;
		//寻找两个相邻节点间的路程和时间
		while (p)
		{
			if (find(p->adjs) == path[i + 1])
			{
				dis += p->dist;
				if (Transfer(path, i))	//换乘后需要加等待时间
				{
					t += 8;
					t += p->time;
				}
				else
					t += p->time;
				break;
			}
			p = p->next;
		}
	}
	//根据路程计算需要的费用
	if (dis <= 6000)
		pri = 3;
	else if (dis <= 12000)
		pri = 4;
	else if (dis <= 22000)
		pri = 5;
	else if (dis <= 32000)
		pri = 6;
	else
	{
		pri = 6;
		int x = dis - 32000;
		int y = x / 20000;
		pri += y + 1;
	}
	distance = dis;
	price = pri;
	time = round(t);
}

string Graph::return_startline(vector<int>a)
{
	if (sta_list[a[0]].line_num == 1)
		return sta_list[a[0]].line[0];
	else
		return return_sameline(sta_list[a[0]].line, sta_list[a[1]].line);
}

int main()
{
	ifstream load;
	Graph subway(load);
	// 获取当前系统时间
	chrono::system_clock::time_point now = chrono::system_clock::now();
	time_t currentTime = chrono::system_clock::to_time_t(now);
	tm* timeInfo = localtime(&currentTime);
	int currentHour = timeInfo->tm_hour;
	int currentMinute = timeInfo->tm_min;
	subway.SetTime(currentHour, currentMinute);
	cout << "欢迎使用地铁导航系统!\t当前时间：";
	if (currentHour < 10)
		cout << '0';
	cout << currentHour << ':';
	if (currentMinute < 10)
		cout << '0';
	cout << currentMinute << endl;
	while (true)
	{
		string word;
		cout << "请选择您的身份（1表示乘客模式，2表示管理员模式，输入exit退出）：" << endl;
		cin >> word;
		if (word == "1")
		{
			while (true)
			{
				string start, end, word;
				cout << "您已进入乘客模式，请选择您的乘车方案（1表示最短时间，2表示最少换乘次数，3表示最短距离,输入exit退出乘客模式）" << endl;
				cin >> word;
				if (word == "1")
				{
					cout << "请输入您的起点站和终点站" << endl;
					cin >> start >> end;
					if (subway.find(start) == -1 || subway.find(end) == -1)
						cout << "站点不存在，请重新选择方案！" << endl;
					else
					{
						subway.ShortestTime(start, end);
						subway.PrintShortest(start, end);
						cout << "感谢您的使用，祝您旅途愉快！" << endl;
						break;
					}
				}
				else if (word == "2")
				{
					cout << "请输入您的起点站和终点站" << endl;
					cin >> start >> end;
					if (subway.find(start) == -1 || subway.find(end) == -1)
						cout << "站点不存在，请重新选择方案！" << endl;
					else
					{
						subway.ShortestTransfer(start, end);
						subway.PrintShortest(start, end);
						cout << "感谢您的使用，祝您旅途愉快！" << endl;
						break;
					}
				}
				else if (word == "3")
				{
					cout << "请输入您的起点站和终点站" << endl;
					cin >> start >> end;
					if (subway.find(start) == -1 || subway.find(end) == -1)
						cout << "站点不存在，请重新选择方案！" << endl;
					else
					{
						subway.ShortestDistance(start, end);
						subway.PrintShortest(start, end);
						cout << "感谢您的使用，祝您旅途愉快！" << endl;
						break;
					}
				}
				else if (word == "exit")
					break;
				else
					cout << "输入非法,请重新输入：" << endl;
			}
		}
		else if (word == "2")
		{
			while (true)
			{
				string word;
				cout << "您已进入管理员模式，请选择您要进行的操作（1表示增加线路，2表示删除线路，3表示输出所有线路，输入exit退出管理员模式）：" << endl;
				cin >> word;
				if (word == "1")
				{
					string line_name;
					bool loop;
					int speed, station_num;
					cout << "请输入线路名称：" << endl;
					cin >> line_name;
					cout << "请输入线路是否为环线（1表示是，0表示不是）" << endl;
					cin >> loop;
					cout << "请输入该线路站点总数以及运行的平均速度（单位是米/分）：" << endl;
					cin >> station_num >> speed;
					cout << "请从起点站开始依次输入车站的名称：" << endl;
					string name1[50];
					string name2[50];
					for (int k = 0; k < station_num; k++)
					{
						string c;
						cin >> c;
						if (k < station_num - 1)
							name1[k] = c;
						if (k > 0)
							name2[k - 1] = c;
					}
					if (loop)
					{
						name1[station_num - 1] = name2[station_num - 2];
						name2[station_num - 1] = name1[0];
					}
					cout << "请从起点站到第二个站点开始依次输入各站间距（环线从最后一站到起点站的间距也要输入）：" << endl;
					int dist[50];
					if (loop)
						station_num++;
					for (int k = 0; k < station_num - 1; k++)
						cin >> dist[k];
					subway.AddStation(line_name, name1, name2, dist, station_num, speed);
					cout << line_name << "增加成功！" << endl;
				}
				else if (word == "2")
				{
					string line_name;
					cout << "请输入您要删除的线路名称" << endl;
					cin >> line_name;
					subway.DeleteStation(line_name);
					cout << line_name << "删除成功" << endl;
				}
				else if (word == "3")
					subway.Print();
				else if (word == "exit")
					break;
				else
					cout << "输入非法，请重新输入：" << endl;
			}
		}
		else if(word=="exit")
			break;
		else
			cout << "输入非法，请重新输入" << endl;
	}
	return 0;
}