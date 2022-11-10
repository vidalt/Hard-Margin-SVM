#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <iostream>
#include <cstdlib>
#include <string>
#include "Pb_Data.h"
using namespace std;

class commandline
{
private:
	// is the commandline valid ?
	bool command_ok;

	// path of the dataset
	string instance_name;

	// type of problem
	ProblemType problemType;

	// penalty factor C
	double penaltyC;

	// number of threads for CPLEX
	int nbThreads;

	// time limit for CPLEX
	int timeBudget;

	//read if normalize data is given as input
	int normalizeData;

	// instanceFormat value (1 for our format (default) and 2 for Brooks)
	int instanceFormat;

	// samplingSize
	int samplingSize;

	// nb sampling iterations
	int nbSamplingIterations;

	// Max number of cb cuts when the solution is fractional
	int maxCBCuts;

	// seed for generating pseudo-random numbers (default 1)
	int seed;

	// timeProportionCBCuts
	double timeProportionCBCuts;

	// timeProportionIDR
	double timeProportionIDR;

	// Parameter value for Local Bounds Implied
	int locallyValidImpliedBounds;

	// timeProportionSampling
	double timeProportionSampling;

	// path to test samples
	string pathToTestInstance;

	// set the solver type
	int set_problem_type(string to_parse);

	// display the name of the problem
	void display_problem_name(string to_parse);

public:
	// constructor
	commandline(int argc, char *argv[]);

	// destructor
	~commandline();

	// gets the path to the instance
	string get_path_to_instance();

	// gets the solver type
	ProblemType get_problem_type();

	// get the load penalty
	double get_penaltyC();

	// is the commandline valid ?
	bool is_valid();

	// get number of threads
	int get_nbThreads();

	// get CPLEX time limit
	int get_timeBudget();

	// get normalizeData variable
	int get_normalizeData();

	// get instanceFormat value
	int get_instanceFormat();

	// get seed
	int get_seed();

	//getter for samplingSize
	int get_samplingSize();

	//getter for nbSamplingIterations
	int get_nbSamplingIterations();

	// getter for maxCBCuts
	int get_maxCBCuts();

	// getter timeProportionCBCuts
	double get_timeProportionCBCuts();

	// getter timeProportionIDR
	double get_timeProportionIDR();

	// getter for locallyValidImpliedBounds
	int get_locallyValidImpliedBounds();

	// getter for locallyValidImpliedBounds
	double get_timeProportionSampling();

};

#endif
