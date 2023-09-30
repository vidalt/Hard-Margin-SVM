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

vector<double> solW;
double solB = 0;
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

int main(int argc, char *argv[])
{

	string hyperplane_coordinates = string(argv[1]);
	string testFile = string(argv[2]);

	ifstream file_hyperplane(hyperplane_coordinates.c_str());

	ifstream file_testFile(testFile.c_str());

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

	int sample_class;
	double sample_coordinate;
	vector<double> sample_coordinates = vector<double>(nbFeatures, 0.0);
	int n_predictions = 0, n_correct_predictions = 0;
	while (!file_testFile.eof())
	{
		sample_coordinate = 0.0;
		file_testFile >> sample_class;
		if (sample_class)
			for (int i = 0; i < nbFeatures; i++)
			{
				file_testFile >> sample_coordinate;
				sample_coordinates[i] = sample_coordinate;
			}
		if (file_testFile.eof())
			break;

		if (predictClass(sample_coordinates, sample_class))
			n_correct_predictions++;
		n_predictions++;
	}

	double accuracy = (double)100.0 * (n_correct_predictions / (1.0 * n_predictions));

	cout << hyperplane_coordinates << "," << testFile << "," << accuracy << endl;
	file_testFile.close();
	file_hyperplane.close();
}