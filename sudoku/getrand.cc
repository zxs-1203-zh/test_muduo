#include <cstring>
#include <string>
#include <fstream>
#include <random>

using namespace std;

bool exited[10];

string getExample()
{
	bzero(exited, sizeof(exited));
	string ans;
	ans.resize(81);

	for(int i = 0; i < 81; ++i)
	{
		int num = rand() % 50;

		if(num > 9)
		{
			num = 0;
		}
		ans[i] = static_cast<char>(num + '0');

	}
	ans += "\r\n";
	return ans;
}


int main()
{
	srand(time(NULL));
	ofstream fout("randExample.txt");

	for(int i = 0; i < 100; ++i)
	{
		string example = getExample();
		fout << example;
	}

	return 0;
}
