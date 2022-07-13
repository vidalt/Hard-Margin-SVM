#ifndef DATA_H
#define DATA_H

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

using namespace std;

#define MY_EPSILON_e7 1e-7
#define MY_EPSILON_e3 1e-3

enum ProblemType
{
    HARD,
    HARD_IP,
    SOFT_HL,
    SOFT_LIBSVM,
    SAMPLING_CB_HARD_IP,
    FIRST_CB_THEN_HARD_IP
};

class Pb_Data
{
public:
    /* PROBLEM DATA */

    // address of the problem instance, including instance name
    string pathToInstance;

    // it's the basename of the pathToInstance
    string instance_name;

    // This is a string containing all relevant execution data to serve as identifier.
    // It avoid rewriting filenames
    string outputIdentifier;

    // vector of sample features, dimension n x d where n is the number of samples and d the number of features
    vector<vector<double>> samples;

    // vector of labels, dimension n where n is the number of samples
    // can take value in -1 or 1
    vector<int> labels;

    // number of samples
    int nbSamples;

    // number of features
    int nbFeatures;

    // type of problem solved
    ProblemType problemType;

    // penalty coefficient
    double penaltyC;

    // number of threads for CPLEX solvers
    int nbThreads;

    // variable to control the remaining time
    double timeBudget;

    // user input time
    double userInputTime;

    // Starting time of the optimization
    clock_t time_StartComput;

    // End time of the optimization
    clock_t time_EndComput;

    // Total time for Master (Combinatorial Benders's cuts)
    clock_t time_totalMaster;

    // Total time for Slave (Combinatorial Benders's cuts)
    clock_t time_totalSlave;

    // we don't know the nature of the input data.
    // Sometimes it's already scaled. So that we have to block it in order to avoid very small numerical computations
    int normalizeData;

    // instanceFormat value (1 for our format (default), 2 and 3 for Brooks)
    int instanceFormat;

    // sampling size for sampling
    int samplingSize;

    // nb sampling iterations
    int nbSamplingIterations;

    // conflict algorithm for Combinatorial Benders cut's
    const int conflictAlgorithm = 4;

    // Max number of cb cuts when the solution is fractional
    int maxCBCuts;

    // timeProportionIDR: 0.5 = Half of the total time for IDR
    double timeProportionIDR;

    // timeProportionCBCuts: 0.2 = One third of the total for CB
    double timeProportionCBCuts;

    // timeProportionSampling: 0.5 is half of the given CBCuts time to sampling phase;
    double timeProportionSampling;

    // time spent in sampling phase
    double time_totalSampling;

    // Parameter value for Local Bounds Implied
    int locallyValidImpliedBounds;

    // Problem type in string format
    string strProblemType;

    // LowerBound Cuts
    vector<set<int>> LB_CUTS;

    // LowerBound Cuts Right hand side
    vector<double> LB_CUTS_rhs;

    // All generated combinatorial cuts will be here
    std::map<set<int>, bool> map_CB_CUTS;

    // All generated combinatorial in sampling phase cuts will be here
    std::map<set<int>, bool> map_sampling_CB_CUTS;

    // Normalizing the dataset by substracting the mean and dividing by the standard deviation
    void computeNormalizedSamples(vector<vector<double>> &samplesTemp);

    // Constructor
    Pb_Data(string pathToInstance, ProblemType problemType, double penaltyC, int nbThreads, int timeBudget, int normalizeData, int instanceFormat, int seed, int samplingSize, int nbSamplingIterations, int maxCBCuts, double timeProportionIDR, double timeProportionCBCuts, double timeProportionSampling, int locallyValidImpliedBounds);

    ~Pb_Data(void);
};

#endif
