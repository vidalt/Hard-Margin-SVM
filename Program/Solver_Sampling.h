#ifndef SOLVER_SAMPLING_H
#define SOLVER_SAMPLING_H

#include "Solver_Primal.h"

// Meta-Solver based on sampling
class Solver_Sampling : public Solver
{

private:
	// solver for Sampling CB -- Hard IP and Ramp Loss
	void solverSamplingCB();

public:
	Solver_Sampling(Pb_Data *myData, Solution *mySolution, bool isDisplay) : Solver(myData, mySolution, isDisplay)
	{
	}

	~Solver_Sampling()
	{
	}

	int solve();

	int solveProblemWithFixed_Z(void) { return 0; }
};

#endif
