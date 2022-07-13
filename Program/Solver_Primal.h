#ifndef SOLVER_PRIMAL_H
#define SOLVER_PRIMAL_H

#include "Solver.h"

// SVM Solver with Hard Margin
// Solution of the Primal Formulation
class Solver_Primal : public Solver
{
public:
	Solver_Primal(Pb_Data *myData, Solution *mySolution, bool isDisplay) : Solver(myData, mySolution, isDisplay)
	{
		this->isSampleConsidered = vector<bool>(myData->nbSamples, true);
	}

	Solver_Primal(Pb_Data *myData, Solution *mySolution, bool isDisplay, vector<bool> &isSampleConsidered) : Solver(myData, mySolution, isDisplay)
	{
		this->isSampleConsidered = isSampleConsidered;
	}

	// Returns 0 if solved properly
	int solve();

	int solve_RampLoss_New_Model();

	int solveLibSVM();

	int solveLinearSeparator();

	int solveProblemWithFixed_Z();
};

#endif
