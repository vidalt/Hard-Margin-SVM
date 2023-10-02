#include "Pb_Data.h"

Pb_Data::Pb_Data(string pathToInstance, ProblemType problemType, double penaltyC, int nbThreads, int timeBudget, int normalizeData, int instanceFormat, int seed, int samplingSize, int nbSamplingIterations, int maxCBCuts, double timeProportionIDR, double timeProportionCBCuts, double timeProportionSampling, int locallyValidImpliedBounds) : pathToInstance(pathToInstance), problemType(problemType), penaltyC(penaltyC), nbThreads(nbThreads), timeBudget(timeBudget), normalizeData(normalizeData), instanceFormat(instanceFormat), samplingSize(samplingSize), nbSamplingIterations(nbSamplingIterations), maxCBCuts(maxCBCuts), timeProportionIDR(timeProportionIDR), timeProportionCBCuts(timeProportionCBCuts), timeProportionSampling(timeProportionSampling), locallyValidImpliedBounds(locallyValidImpliedBounds)
{
	this->time_totalMaster = 0;
	this->time_totalSlave = 0;
	this->time_totalSampling = 0;
	this->userInputTime = timeBudget;
	ifstream fichier;
	ofstream outFile;
	string contenu;
	string uselessStr;

	/* parsing the datasets */
	fichier.open(pathToInstance.c_str());
	if (fichier.is_open())
	{
		// Reader for Brook's instances
		if (instanceFormat == 2)
		{
			cout << "Instance type: Brooks" << endl;
			fichier >> nbSamples;
			fichier >> nbFeatures;

			cout << "nbSamples: " << nbSamples << endl;
			cout << "nbFeatures: " << nbFeatures << endl;
			int current_label;

			// Initializing data structures
			labels = vector<int>(nbSamples, 0);
			string buff;
			//No need to normalize

			if (normalizeData)
			{
				// Reading dataset
				vector<vector<double>> samplesTemp = vector<vector<double>>(nbSamples, vector<double>(nbFeatures, 0.0));
				for (int n = 0; n < nbSamples; n++)
				{
					fichier >> current_label;
					labels[n] = (current_label <= 0) ? -1 : 1;
					for (int d = 0; d < nbFeatures; d++)
						fichier >> samplesTemp[n][d];
				}
				// Normalizing the dataset
				computeNormalizedSamples(samplesTemp);
			}
			else
			{
				samples = vector<vector<double>>(nbSamples, vector<double>(nbFeatures, 0.0));
				for (int n = 0; n < nbSamples; n++)
				{
					fichier >> current_label;
					labels[n] = (current_label <= 0) ? -1 : 1;
					for (int d = 0; d < nbFeatures; d++)
						fichier >> samples[n][d];
				}
			}
		}
		else
		{
			fichier >> uselessStr;
			fichier >> nbSamples;
			fichier >> uselessStr;
			fichier >> nbFeatures;
			fichier >> uselessStr;
			getline(fichier, contenu);

			// Initializing data structures
			labels = vector<int>(nbSamples, 0);

			if (normalizeData)
			{
				vector<vector<double>> samplesTemp = vector<vector<double>>(nbSamples, vector<double>(nbFeatures, 0.0));
				// Reading dataset
				for (int n = 0; n < nbSamples; n++)
					for (int d = 0; d < nbFeatures; d++)
						fichier >> samplesTemp[n][d];

				// Normalizing the dataset
				computeNormalizedSamples(samplesTemp);
			}
			else
			{
				samples = vector<vector<double>>(nbSamples, vector<double>(nbFeatures, 0.0));
				// Reading dataset
				for (int n = 0; n < nbSamples; n++)
					for (int d = 0; d < nbFeatures; d++)
						fichier >> samples[n][d];
			}

			getline(fichier, contenu);
			getline(fichier, contenu);

			for (int n = 0; n < nbSamples; n++)
				fichier >> labels[n];

			// little debugging test to verify that we are at the end of the file
			getline(fichier, contenu);
			fichier >> uselessStr;
			if (!(uselessStr == "EOF"))
			{
				cout << "ERROR when reading instance, not finding EOF where it should be" << endl;
				throw string("ERROR when reading instance, not finding EOF where it should be");
			}
		}
		fichier.close();

		if (problemType == HARD)
			strProblemType = "HARD";
		else if (problemType == HARD_IP)
			strProblemType = "HARD_IP";
		else if (problemType == SOFT_HL)
			strProblemType = "SOFT_HL";
		else if (problemType == SAMPLING_CB_HARD_IP)
			strProblemType = "SAMPLING_CB_HARD_IP";
		else if (problemType == FIRST_CB_THEN_HARD_IP)
			strProblemType = "FIRST_CB_THEN_HARD_IP";
		else
			strProblemType = "NO_PROBLEM_STR";

		char *path = new char[pathToInstance.length() + 1];
		strcpy(path, pathToInstance.c_str());
		char *bname = basename(path);
		instance_name.assign(bname);
		delete[] path;

		this->outputIdentifier = "_" + instance_name;
		this->outputIdentifier += "_problemType" + strProblemType;
		this->outputIdentifier += "_CB" + to_string(timeProportionCBCuts).substr(0,4);
		this->outputIdentifier += "_Sampling" + to_string(timeProportionSampling).substr(0,4);
		this->outputIdentifier += "_timeBudget" + to_string(timeBudget);
		this->outputIdentifier += "_seed" + to_string(seed);
		this->outputIdentifier += "_C" + to_string((int)penaltyC);
		cout << "this->outputIdentifier: " << this->outputIdentifier << endl;
	}
	else
	{
		cout << "ERROR : Impossible to find instance file : " << pathToInstance << endl;
		throw string("ERROR : Impossible to find instance file : " + pathToInstance);
	}
	srand(seed);
}

void Pb_Data::computeNormalizedSamples(vector<vector<double>> &samplesTemp)
{
	samples = vector<vector<double>>(nbSamples, vector<double>(nbFeatures, 0.0));
	vector<double> samplesAverage = vector<double>(nbFeatures, 0.0);
	vector<double> samplesSTD = vector<double>(nbFeatures, 0.0);

	if (normalizeData == 1)
	{
		cout << "Normalizing data using (value - avg)/std" << endl;
		// Normalize for each feature separately
		// First calculate average and standard deviation for each feature
		for (int j = 0; j < nbFeatures; j++)
		{
			for (int i = 0; i < nbSamples; i++)
				samplesAverage[j] += samplesTemp[i][j];
			samplesAverage[j] = samplesAverage[j] / (double)nbSamples;

			for (int i = 0; i < nbSamples; i++)
				samplesSTD[j] += (samplesTemp[i][j] - samplesAverage[j]) * (samplesTemp[i][j] - samplesAverage[j]);
			samplesSTD[j] = sqrt(samplesSTD[j] / ((double)nbSamples - 1.0));
		}

		for (int i = 0; i < nbSamples; i++)
			for (int j = 0; j < nbFeatures; j++)
				samples[i][j] = (samplesTemp[i][j] - samplesAverage[j]) / samplesSTD[j];
	}
	else if (normalizeData == 2)
	{
		cout << "Normalizing using max-min function" << endl;
		for (int j = 0; j < nbFeatures; j++)
		{
			double max = samplesTemp[0][j];
			double min = max;
			for (int i = 1; i < nbSamples; i++)
			{
				if (samplesTemp[i][j] > max)
					max = samplesTemp[i][j];
				else if (samplesTemp[i][j] < min)
					min = samplesTemp[i][j];
			}
			for (int i = 0; i < nbSamples; i++)
				samples[i][j] = (samplesTemp[i][j] - min) / (max - min);
		}
	}
}

Pb_Data::~Pb_Data(void)
{
}