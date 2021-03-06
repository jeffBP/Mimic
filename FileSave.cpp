// FileSave.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;


int getDelimit(string str) {
	string delimiter = ", ";
	int pos = str.find(delimiter);
	return pos;

}

int stringToInt(string str) {
	int len = str.length();
	int sum = 0;
	for (int i = 0; i < len; i++) {
		char digit = str[i];
		int num = digit - '0';
		num = num * std::pow(10, len - i-1);
		sum += num;
	}
	return sum;
}

void getAngs(string str) {
	ofstream ang("ang.txt", std::ios::app);

	int pos = getDelimit(str);
	string ang1 = str.substr(0, pos);
	string ang2 = str.substr(pos + 2, str.length());
	int angle1 = stringToInt(ang1);
	int angle2 = stringToInt(ang2);
	
	ang << angle1 << "      " << angle2 << "\n";
	ang.close();
}
int main() {
	string str;
	
	ifstream angles ("angles.txt");

	if (angles.is_open()) {
		while (getline(angles, str)) {
			getAngs(str);
		}
	}
	else {
		cout << "No read";
	}
	
	angles.close();
	
	system("pause");
	
	
}


