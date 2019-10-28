#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <algorithm>
#define MAXN 300
#define TESTFILE "1.nfa"
using namespace std;

struct Dstate			//新DFA的状态名称，是否为终结状态
{
	char name;
	bool isEnd;			//是否为终止状态
	string transRow;    //转换表
	set<int> element;	//新状态对应的原始状态集合
	int color;			//划分状态编号
};

int N;								//打印语句的长度
int state_num, end_state_num;		//状态的数量，终止状态集的状态个数
string letters;						//合法字符集
string letters_null;				//合法字符集加上一个空串
set<int> end_state;					//终止状态集合
bool NFA[MAXN][MAXN][MAXN];			//NFA

vector<Dstate> DFA_States;		//DFA状态集
vector<Dstate> DFA_MIN_States;	//最小DFA状态集

char getAlpha(size_t i)
{
	return char(i) + 'A';
}

void readFile()
{
	ifstream in(TESTFILE);
	int start, end, state;
	char trans;

	in >> state_num;
	cout << "最大状态编号：" << state_num << endl;
	state_num++;

	in >> letters;
	cout << "字符集：" << letters << endl;
	letters_null = letters + '*';

	in >> end_state_num;
	cout << "终止状态集的集合数量：" << end_state_num << endl;
	cout << "请输入终止状态集合中的所有状态：";
	for (int i = 0; i < end_state_num; i++)
	{
		in >> state;
		end_state.insert(state);
		cout << state << " ";
	}
	cout << "\n语言的长度大小：";
	in >> N;
	cout << N << endl << "NFA的边集：" << endl;
	while (in >> start >> trans >> end)
	{
		cout << start << " " << trans << " " << end << endl;
		NFA[start][trans][end] = true;
	}
	cout << endl;
	in.close();
}

set<int> Closure(set<int> T)   //epsilon闭包算法
{
	set<int> closure = T;
	stack<int> stack;
	for (int i : T) stack.push(i);

	while (!stack.empty())   //栈每次弹出首位，并以首位进行扩展
	{
		int cur = stack.top();
		stack.pop();

		for (int i = 0; i < state_num; i++)
			if (NFA[cur]['*'][i] && closure.find(i) == closure.end())
			{
				closure.insert(i);
				stack.push(i);
			}
	}
	return closure;
}

set<int> Move(set<int> T, char trans)		//扩展出的新集合
{
	set<int> extension;
	for (int i : T)
	{
		for (int j = 0; j <= state_num; ++j)
		{
			if (NFA[i][trans][j] == 1)
				extension.insert(j);
		}
	}
	return extension;
}

void Expand(size_t state)   //根据初始状态和字符集，构造新的子集
{
	vector<int> intersection;
	intersection.resize(min(end_state.size(), DFA_States[state].element.size()));
	auto it = set_intersection(end_state.begin(), end_state.end(), DFA_States[state].element.begin(), DFA_States[state].element.end(), intersection.begin());
	DFA_States[state].isEnd = (it != intersection.begin());

	for (size_t x = 0; x < letters.length(); ++x)
	{
		set<int> u = Closure(Move(DFA_States[state].element, letters[x]));
		if (u.empty())
		{
			DFA_States[state].transRow.push_back('?');
			continue;
		}

		bool flag = false;
		for (size_t i = 0; i < DFA_States.size(); ++i)
		{
			if (DFA_States[i].element == u)
			{
				flag = true;
				DFA_States[state].transRow.push_back(getAlpha(i));
				break;
			}
		}
		if (!flag)
		{
			DFA_States.push_back(Dstate{ getAlpha(DFA_States.size()), false, "", u });
			DFA_States[state].transRow.push_back(getAlpha(DFA_States.size() - 1));
		}
	}
}

void printDFA()   //打印子集构造信息和DFA状态转换表
{
	cout << "构造了" << DFA_States.size() << "个子集，与NFA状态对应关系如下：" << endl;
	for (size_t i = 0; i < DFA_States.size(); ++i)
	{
		cout << getAlpha(i) << " = <";
		for (int state : DFA_States[i].element)
			cout << setw(2) << state << " ";
		cout << ">";

		if (DFA_States[i].isEnd) cout << "\t[接受状态]";
		cout << endl;
	}

	cout << endl << "构造的DFA状态转换表如下：" << endl;   //打印状态转换表
	for (char letter : letters)
		cout << "\t" << letter;
	cout << endl;

	for (size_t i = 0; i < DFA_States.size(); ++i)
	{
		cout << DFA_States[i].name << "\t";
		for (char c : DFA_States[i].transRow)
			cout << c << '\t';
		cout << endl;
	}
}

int colors[MAXN][MAXN][MAXN];
void Minimize()
{
	DFA_MIN_States = DFA_States;
	
	//初始划分为<非接受状态：编号1><接受状态：编号2>
	for (size_t i = 0; i < DFA_MIN_States.size(); ++i)
		DFA_MIN_States[i].color = (DFA_MIN_States[i].isEnd ? 2 : 1);

	int maxColor = 2;
	for (size_t n = 0; n < DFA_MIN_States.size(); ++n)				//无脑划分"状态个数"次
	{
		for (size_t i = 0; i < letters.length(); ++i)				//对每个输入符号
		{
			for (size_t j = 0; j < DFA_MIN_States.size(); ++j)		//对每个状态
			{
				int curNumber = DFA_MIN_States[j].color;
				char des = DFA_MIN_States[j].transRow[i];

				int desNumber = (des == '?') ? 100 : DFA_MIN_States[des - 'A'].color;
				if (curNumber != desNumber)
				{
					if (colors[curNumber][i][desNumber] == 0)
					{
						++maxColor;
						DFA_MIN_States[j].color = maxColor;
						colors[curNumber][i][desNumber] = maxColor;
					}
					else DFA_MIN_States[j].color = colors[curNumber][i][desNumber];
				}
			}
		}
	}

	for (auto it0 = DFA_MIN_States.begin(); it0 != DFA_MIN_States.end(); ++it0)
	{
		for (auto it1 = (it0 + 1); it1 != DFA_MIN_States.end(); ++it1)
		{
			if (it0->color != it1->color) continue;

			char tmp1 = it0->name;
			char tmp2 = it1->name;
			cout << "等价状态：" << tmp1 << " " << tmp2 << endl;

			for (size_t x = 0; x < DFA_MIN_States.size(); ++x)
				replace(DFA_MIN_States[x].transRow.begin(), DFA_MIN_States[x].transRow.end(), tmp2, tmp1);

			it1 = DFA_MIN_States.erase(it1);
			--it1;
		}
	}
}

void printMIN()   //打印最小化的DFA
{
	cout << "\n最小化后的DFA为：" << endl << '\t';
	for (char c : letters)
		cout << c << '\t';
	cout << endl;

	for (Dstate state : DFA_MIN_States)
	{
		cout << state.name << '\t';
		for (char c : state.transRow)
			cout << c << '\t';
		cout << endl;
	}
	cout << endl;
}

string str;
set<string> resultNFA;
void searchNFA(int cur_state, int n, int depth)   //对NFA，搜索所有小于等于N长度的字符串
{
	if (depth > n) return;
	if (end_state.find(cur_state) != end_state.end()) resultNFA.insert(str);

	for (size_t i = 0; i < letters_null.length(); ++i)
	{
		for (int j = 0; j < state_num; ++j)
		{
			if (NFA[cur_state][letters_null[i]][j])
			{
				if (letters_null[i] == '*') searchNFA(j, n, depth);
				else
				{
					str += letters_null[i];
					searchNFA(j, n, depth + 1);
					str.pop_back();
				}
			}
		}
	}
}

set<string> resultDFA;
void searchDFA(char cur_state, int n, int depth, vector<Dstate>& States)   //对DFA，搜索所有小于等于N长度的字符串
{
	if (depth > n) return;
	if (DFA_States[cur_state - 'A'].isEnd) resultDFA.insert(str);
	for (size_t i = 0; i < States.size(); i++)
	{
		if (States[i].name == cur_state)
			for (size_t j = 0; j < letters.length(); j++)
			{
				if (States[i].transRow[j] == '?') continue;
				str += letters[j];
				searchDFA(States[i].transRow[j], n, depth + 1, States);
				str.pop_back();
			}
	}
}

void printResult(string constant, set<string> result)   //打印小于等于N的搜索结果
{
	cout << "符合" << constant << "的语言集合如下：" << endl;
	int count = 1;
	for (string s : result)
	{
		cout << count << ".\t";
		cout << (s.empty() ? "epsilon" : s) << endl;
		count++;
	}
	cout << endl;
}

void NFA_to_DFA()
{
	set<int> s0 = { 0 };
	s0 = Closure(s0);
	DFA_States.push_back(Dstate{ getAlpha(0), false, "", s0 });

	size_t done = 0;
	while (done < DFA_States.size())
	{
		Expand(done++);
	}
}

int main()
{
	readFile();

	NFA_to_DFA();
	printDFA();

	Minimize();
	printMIN();

	searchNFA(0, N, 0);
	printResult("NFA", resultNFA);

	searchDFA('A', N, 0, DFA_States);
	printResult("DFA", resultDFA);

	resultDFA.clear();
	searchDFA('A', N, 0, DFA_MIN_States);
	printResult("MIN_DFA", resultDFA);

	system("pause");
	return 0;
}