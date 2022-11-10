#include "Solution.h"

Solution::Solution(Pb_Data *myData) : myData(myData)
{
	solZ = vector<bool>(myData->nbSamples, true);
	solW = vector<double>(myData->nbFeatures, 0.0);
	solE = vector<double>(myData->nbSamples, 0.0);
	solValue = DBL_MAX;
	solStatus = -1;
	lowerBoundValue = 0;
}

void Solution::copyToMe(Solution *&sol)
{
	this->solW = vector<double>(myData->nbFeatures);
	for (int i = 0; i < myData->nbFeatures; i++)
		this->solW[i] = sol->solW[i];
	this->solB = sol->solB;

	this->solZ = vector<bool>(myData->nbSamples);
	this->solE = vector<double>(myData->nbSamples);

	for (int i = 0; i < myData->nbSamples; i++)
	{
		this->solZ[i] = sol->solZ[i];
		this->solE[i] = sol->solE[i];
	}

	this->solStatus = sol->solStatus;

	this->solValue = sol->solValue;
	this->lowerBoundValue = sol->lowerBoundValue;
	this->isSolComputed = sol->isSolComputed;
}

Solution::~Solution(void) {}

bool Solution::checkClassified(int i, double &distance)
{
	distance = 0.0;
	for (int j = 0; j < myData->nbFeatures; j++)
		distance += solW[j] * myData->samples[i][j];
	distance = myData->labels[i] * (distance + solB) - 1.0;
	bool isClassified = (distance > MY_EPSILON_e7);
	distance = abs(distance);
	return isClassified;
}

double Solution::getDualityGap()
{
	// solution status 102 means that this solution is optimal within some tolerance conditions.
	// Such cases will yields duality gaps strictly greater than 0.
	double UB = this->solValue;
	double LB = this->lowerBoundValue;
	if (UB > 0)
		return 100 * ((UB - LB) / UB);
	else
		return 0;
}

void Solution::displaySolution()
{
	cout << "Solution status: " << solStatus << endl;
	cout << "Solution value: " << solValue << endl;
	if (lowerBoundValue > 0)
	{
		cout << "Lower bound: " << lowerBoundValue << endl;
		cout << "Duality GAP(%): " << this->getDualityGap() << endl;
	}

	int nbMisclassified_FirstClass = 0;
	int nbMisclassified_SecondClass = 0;
	vector<int> misclassifiedSamples, supportVectors;
	for (int i = 0; i < myData->nbSamples; i++)
	{
		if (solZ[i])
		{
			if (myData->labels[i] == -1)
				nbMisclassified_FirstClass++;
			else
				nbMisclassified_SecondClass++;
			misclassifiedSamples.push_back(i);
		}
		else if (solE[i] > 0)
			supportVectors.push_back(i);
	}

	cout << "Nb Misclassified samples: -1:" << nbMisclassified_FirstClass << "; 1:" << nbMisclassified_SecondClass << endl;
	cout << "[";
	int i;
	if (misclassifiedSamples.size() > 0)
	{
		for (i = 0; i < misclassifiedSamples.size() - 1; i++)
			cout << misclassifiedSamples[i] << ", ";
		cout << misclassifiedSamples[i];
	}
	cout << "]" << endl;
	cout << "Nb Support vectors: " << supportVectors.size() << endl;
	cout << "[";
	if (supportVectors.size() > 0)
	{
		for (i = 0; i < supportVectors.size() - 1; i++)
			cout << supportVectors[i] << ", ";
		cout << supportVectors[i];
	}
	cout << "]" << endl;

	for (int j = 0; j < myData->nbFeatures; j++)
		cout << "W[" << j << "]: " << solW[j] << endl;
	cout << "B: " << solB << endl;
}

void Solution::exportSolution()
{
	ofstream solution, samples_classified, samples_misclassified, samples_support_vectors;

	solution.open("solution" + myData->outputIdentifier + ".txt");
	samples_classified.open("samples-classified" + myData->outputIdentifier + ".txt");
	samples_misclassified.open("samples-misclassified" + myData->outputIdentifier + ".txt");
	samples_support_vectors.open("samples-support-vectors" + myData->outputIdentifier + ".txt");

	for (int j = 0; j < myData->nbFeatures; j++)
		solution << solW[j] << " ";
	solution << solB << endl;

	for (int i = 0; i < myData->nbSamples; i++)
	{
		if (solZ[i])
		{
			samples_misclassified << i << " ";
			for (int j = 0; j < myData->nbFeatures; j++)
				samples_misclassified << myData->samples[i][j] << " ";
			samples_misclassified << myData->labels[i] << endl;
		}
		else if (solE[i] > 0.0001)
		{
			samples_support_vectors << i << " ";
			for (int j = 0; j < myData->nbFeatures; j++)
				samples_support_vectors << myData->samples[i][j] << " ";
			samples_support_vectors << myData->labels[i] << endl;
		}
		else
		{
			samples_classified << i << " ";
			for (int j = 0; j < myData->nbFeatures; j++)
				samples_classified << myData->samples[i][j] << " ";
			samples_classified << myData->labels[i] << endl;
		}
	}

	solution.close();
	samples_classified.close();
	samples_misclassified.close();
	samples_support_vectors.close();
	if (myData->nbFeatures == 2)
		cout << "Solution exported, with Gnuplot installed, click on [plotSolution.plt] to print" << endl;
}

bool Solution::predictClass(vector<double> sample_coordinates, int sample_class)
{
	double distance = 0.0;
	for (int j = 0; j < myData->nbFeatures; j++)
		distance += solW[j] * sample_coordinates[j];
	distance = sample_class * (distance + solB) - 1.0;
	bool isClassified = (distance > MY_EPSILON_e7);
	return isClassified;
}


void Solution::displayAccuracy()
{
	if(myData->pathToInstance.find("Brooks_SVM_Data/") != string::npos)
	{
		if(myData->nbFeatures == 2)
			myData->pathToTestInstance = "../Datasets/Brooks_SVM_Data/n1e+05d2test";
		else if(myData->nbFeatures == 5)
			myData->pathToTestInstance = "../Datasets/Brooks_SVM_Data/n1e+05d5test";
		else
			myData->pathToTestInstance = "../Datasets/Brooks_SVM_Data/n1e+05d10test";
	} 
	else if(myData->pathToInstance.find("Brooks_RealWorldData/") != string::npos)
	{
		int pos = myData->pathToInstance.find(".train", 0);
		myData->pathToTestInstance = myData->pathToInstance.substr(0,pos)  + ".test";
	}

	std::cout << "Test instance file: " << myData->pathToTestInstance << std::endl;
	
	std::ifstream testInstance(myData->pathToTestInstance);
	int sample_class;
	
	double sample_coordinate;
	vector<double> sample_coordinates = vector<double>(myData->nbFeatures,0.0);
	int n_predictions = 0, n_correct_predictions = 0;
	while (!testInstance.eof())
	{
		testInstance >> sample_class;
		for (int i = 0; i < myData->nbFeatures ; i++)
		{
			testInstance >> sample_coordinate;
			sample_coordinates[i] = sample_coordinate;
		}
		if(testInstance.eof())
			break;

		if (predictClass(sample_coordinates,sample_class))
			n_correct_predictions++;
		n_predictions++;
		
	}
	
	this->accuracy = 100.0*(n_correct_predictions/(1.0 * n_predictions));
	std::cout << "Accuracy: " << this->accuracy <<  " %" << std::endl;
}