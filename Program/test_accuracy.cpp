#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <map>
#include <cfloat>
#include <libgen.h>
#include <iomanip>
#include <set>
#include <ctime>
#include <random>
#include <sys/times.h>
#include <unistd.h>
#include <limits.h>
#define MY_EPSILON_e7 1e-3

using namespace std;

ifstream file_testFile;
vector<double> solW;
double solB = 0.0;
double computational_time = 0.0;
int nbFeatures;

bool predictClass(vector<double> sample_coordinates, int sample_class)
{
	double distance = 0.0;
	int j = 0;
	for (; j < nbFeatures; j++)
		distance += solW[j] * sample_coordinates[j];
	distance = sample_class * (distance + solB) - 1.0;
	bool isClassified = (distance >= -MY_EPSILON_e7);
	return isClassified;
}

double get_accuracy()
{

	file_testFile.clear();
	file_testFile.seekg(0, ios::beg);
	int sample_class;
	double sample_coordinate;
	vector<double> sample_coordinates = vector<double>(nbFeatures, 0.0);
	int n_predictions = 0, n_correct_predictions = 0;
	
	while (file_testFile >> sample_class)
	{
		sample_coordinate = 0.0;
		
		if (sample_class)
		{
			for (int i = 0; i < nbFeatures; i++)
			{
				file_testFile >> sample_coordinate;
				sample_coordinates[i] = sample_coordinate;
			}
		}
		if (file_testFile.eof())
			break;

		if (predictClass(sample_coordinates, sample_class))
			n_correct_predictions++;
		n_predictions++;
	}
	
	return ((double)100.0 * (n_correct_predictions / (1.0 * n_predictions)));
}

int main(int argc, char *argv[])
{


	string mode = string(argv[1]);	
	string first_argument = string(argv[2]);
	string testFile = string(argv[3]);
	file_testFile.open(testFile.c_str());
	ifstream file_hyperplane(first_argument.c_str());

	

	if(mode == "1")
	{
		string first_part;
		double second_part;
		while (!file_hyperplane.eof())
		{
			file_hyperplane >> first_part;
			file_hyperplane >> second_part;
			if (first_part[0] == 'W')
			{
				solW.push_back(second_part);
			}
			else
			{
				solB = second_part;
			}
		}
		nbFeatures = solW.size();

		double accuracy = get_accuracy();
		cout << first_argument << "," << testFile << "," << accuracy << endl;
	}
	else
	{
		
		string line;
		getline (file_hyperplane,line);
		int count_spaces = 0;
		for (int i = 0 ; i < line.length() ; i++)
		{
			if(line[i] == ' ')
				count_spaces++;
		}
		file_hyperplane.clear();
		file_hyperplane.seekg(0, ios::beg);

		double buffer = 0.0;
		while (file_hyperplane >> buffer)
		{
			solW.clear();
			solW.push_back(buffer);
			for (int i = 0; i < count_spaces - 2; i++)
			{
				file_hyperplane >> buffer;
				
				solW.push_back(buffer);
			}
			file_hyperplane >> solB;

			file_hyperplane >> computational_time;

			nbFeatures = solW.size();

			double accuracy = get_accuracy();
			if(accuracy > MY_EPSILON_e7)
				cout << first_argument << "," << testFile << "," << accuracy << "," << computational_time << endl;

			if (file_hyperplane.eof())
				break;

		}
	}
	file_testFile.close();
	file_hyperplane.close();
}