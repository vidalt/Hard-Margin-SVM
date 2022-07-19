#include "commandline.h"

int commandline::set_problem_type(string to_parse)
{
	if (to_parse == "HARD")
		problemType = HARD;
	else if (to_parse == "HARD_IP")
		problemType = HARD_IP;
	else if (to_parse == "SOFT_HL")
		problemType = SOFT_HL;
	else if (to_parse == "SAMPLING_CB_HARD_IP")
		problemType = SAMPLING_CB_HARD_IP;
	else if (to_parse == "FIRST_CB_THEN_HARD_IP")
		problemType = FIRST_CB_THEN_HARD_IP;
	else
		return -1; // problem

	return 0; // OK
}

void commandline::display_problem_name(string to_parse)
{
	char caractere1 = '/';
	char caractere2 = '\\';

	int position = to_parse.find_last_of(caractere1);
	int position2 = to_parse.find_last_of(caractere2);
	if (position2 > position)
		position = position2;

	if (position != -1)
		cout << "DATASET: " << to_parse.substr(position + 1) << endl;
	else
		cout << "DATASET: " << to_parse << endl;
}

commandline::commandline(int argc, char *argv[])
{
	this->command_ok = false;

	//Default parameter values
	this->normalizeData = 0;
	this->problemType = HARD_IP;
	this->penaltyC = 1.0;
	this->nbThreads = 1;
	this->timeBudget = -1;
	this->instanceFormat = 2;
	this->seed = 1;
	this->samplingSize = -1;
	this->nbSamplingIterations = -1;
	this->maxCBCuts = -1;
	this->timeProportionCBCuts = 0.2;
	this->timeProportionIDR = 0.0;
	this->timeProportionSampling = 0.5;
	this->locallyValidImpliedBounds = 3;

	string strProblemType;

	if (argc % 2 != 0 || argc > 38 || argc < 2)
	{
		cout << "ERROR: invalid command line" << endl;
		cout << "USAGE: ./executable path_to_instance [-problem solver_type] [-pen penaltyC] [-nbThreads threadsCPLEX] [-timeBudget timeBudgetForCPLEX] " << endl;
		cout << "ERROR: invalid command line" << endl;
		return;
	}
	else
	{
		// Default values
		this->instance_name = string(argv[1]);
		if (this->instance_name.size() == 0)
		{
			cout << "ERROR: path_to_instance must be provided" << endl;
			return;
		}
		else
		{
			ifstream fichier(this->instance_name.c_str());
			if (!fichier.is_open())
			{
				cout << "ERROR: file does not exist" << endl;
				return;
			}
			fichier.close();
		}
		display_problem_name(this->instance_name);

		for (int i = 2; i < argc; i += 2)
		{
			if (string(argv[i]) == "-problem")
			{
				if (set_problem_type(string(argv[i + 1])) != 0)
				{
					cout << "ERROR: Unrecognized problem type : " + string(argv[i + 1]) << endl;
					return;
				}
				strProblemType = string(argv[i + 1]);
			}
			else if (string(argv[i]) == "-pen")
				this->penaltyC = atof(argv[i + 1]);
			else if (string(argv[i]) == "-nbThreads")
				this->nbThreads = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-timeBudget")
				this->timeBudget = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-normalizeData")
				this->normalizeData = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-instanceFormat")
				this->instanceFormat = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-seed")
				this->seed = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-samplingSize")
				this->samplingSize = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-nbSamplingIterations")
				this->nbSamplingIterations = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-maxCBCuts")
				this->maxCBCuts = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-timeProportionCBCuts")
				this->timeProportionCBCuts = std::fabs(atof(argv[i + 1]));
			else if (string(argv[i]) == "-timeProportionSampling")
				this->timeProportionSampling = std::fabs(atof(argv[i + 1]));
			else if (string(argv[i]) == "-timeProportionIDR")
				this->timeProportionIDR = std::fabs(atof(argv[i + 1]));
			else if (string(argv[i]) == "-locallyValidImpliedBounds")
				this->locallyValidImpliedBounds = atoi(argv[i + 1]);
			else
			{
				cout << "ERROR: Unrecognized command : " + string(argv[i]) << endl;
				return;
			}
		}

		if (timeProportionIDR + timeProportionCBCuts > 0.9)
		{
			cout << "The sum of time proportions must be at most 0.9;" << endl;
			this->command_ok = false;
			return;
		}

		//Input
		cout << "problemType: " << strProblemType << endl;
		cout << "normalizeData: " << normalizeData << endl;
		cout << "PENALTY C: " << this->penaltyC << endl;
		cout << "nbThreads: " << this->nbThreads << endl;
		cout << "timeBudget: " << this->timeBudget << endl;
		cout << "instanceFormat: " << this->instanceFormat << endl;
		cout << "seed: " << this->seed << endl;
		cout << "samplingSize: " << this->samplingSize << endl;
		cout << "nbSamplingIterations : " << this->nbSamplingIterations << endl;
		cout << "maxCBCuts: " << this->maxCBCuts << endl;
		cout << "timeProportionCBCuts: " << this->timeProportionCBCuts << endl;
		cout << "timeProportionIDR: " << this->timeProportionIDR << endl;
		cout << "timeProportionSampling: " << this->timeProportionSampling << endl;
		cout << "locallyValidImpliedBounds: " << this->locallyValidImpliedBounds << endl;

		cout << endl
			 << endl;
		this->command_ok = true;
	}
}

commandline::~commandline() {}

int commandline::get_normalizeData()
{
	return normalizeData;
}

string commandline::get_path_to_instance()
{
	return instance_name;
}

ProblemType commandline::get_problem_type()
{
	return problemType;
}

double commandline::get_penaltyC()
{
	return penaltyC;
}

bool commandline::is_valid()
{
	return command_ok;
}

int commandline::get_timeBudget()
{
	return timeBudget;
}

int commandline::get_nbThreads()
{
	return nbThreads;
}

int commandline::get_instanceFormat()
{
	return instanceFormat;
}

int commandline::get_seed()
{
	return seed;
}

//getter for samplingSize
int commandline::get_samplingSize()
{
	return samplingSize;
}

//getter for nbSamplingIterations
int commandline::get_nbSamplingIterations()
{
	return nbSamplingIterations;
}

// getter for maxCBCuts
int commandline::get_maxCBCuts()
{
	return maxCBCuts;
}

// getter timeProportionCBCuts
double commandline::get_timeProportionCBCuts()
{
	return timeProportionCBCuts;
}

// getter timeProportionIDR
double commandline::get_timeProportionIDR()
{
	return timeProportionIDR;
}

// getter locallyValidImpliedBounds
int commandline::get_locallyValidImpliedBounds()
{
	return locallyValidImpliedBounds;
}

// getter timeProportionSampling
double commandline::get_timeProportionSampling()
{
	return timeProportionSampling;
}