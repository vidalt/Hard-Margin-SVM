#ifndef SOLVER_H
#define SOLVER_H

#include "Solution.h"
#include <iostream>
#define ILOSTLBEGIN
#include <ilcplex/ilocplex.h>

using namespace std;


struct MYCBstr
{
    Pb_Data *myData;
    int count;
    CPXENVptr envCB;
    clock_t startTime;
    vector<bool> isSampleConsidered;
    double solutionHLvalue;
	ofstream overtimeSolutions;
};
typedef struct MYCBstr MYCB, *MYCBptr;


static int CPXPUBLIC callback_check_new_incumbent(CPXCENVptr env,
           void *cbdata,
           int wherefrom,
           void *cbhandle,
           double objval,
           double *x,
           int *isfeas_p,
           int *useraction_p)
{
	*useraction_p = CPX_CALLBACK_DEFAULT;
	MYCBptr mycbinfo = (MYCBptr)cbhandle;

	for (int i = mycbinfo->myData->nbSamples; i < mycbinfo->myData->nbSamples + mycbinfo->myData->nbFeatures + 1; i++)
		mycbinfo->overtimeSolutions << x[i] << " ";

	double elapsedTime = (((double)(clock() - mycbinfo->startTime)) / CLOCKS_PER_SEC);
	mycbinfo->overtimeSolutions << elapsedTime << "\n";
	return 0;
}

class Solver
{

protected:
	// Parameters of the problem
	Pb_Data *myData;

	// Parameters of the problem
	Solution *mySolution;

	// Solution from Hinge Loss problem used as initial bound in IDR and initial solution in SVM variants
	Solution *solutionHL;

	// Switch to display traces (TRUE) or not (FALSE)
	bool isDisplay;
	//  ---------------------------------------------------

	// keeps track of the samples which are currently included or not in the problem
	vector<bool> isSampleConsidered;

public:
	// Constructor common to all Solver solvers
	Solver(Pb_Data *myData, Solution *mySolution, bool isDisplay) : myData(myData), mySolution(mySolution), isDisplay(isDisplay)
	{
		solutionHL = new Solution(myData);
	}

	// Solution methods, implemented in each solver independently
	virtual int solve(void) = 0;

	virtual int solveProblemWithFixed_Z(void) = 0;

	// Virtual destructor (needs to stay virtual otherwise inherited destructors will not be called)
	virtual ~Solver(){};
};

#endif
