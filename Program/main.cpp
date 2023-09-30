
//--------------------------------------------------------
//LIBRARY OF SVM Algorithms
//Author : Thibaut VIDAL
//Date   : August 15th, 2018
//E-mail : vidalt@inf.puc-rio.br
//
//This code is distributed for research purposes.
//All rights reserved.
//--------------------------------------------------------

#include <iostream>
#include "commandline.h"
#include "Solution.h"
#include "Solver_Primal.h"
#include "Solver_Sampling.h"
#include "Solver_CombinatorialBenders.h"

void lastLineCSVFormat(Pb_Data *myData, Solution *mySolution, Solver *mySolver);

int main(int argc, char *argv[])
{
	Pb_Data *myData = NULL;
	Solution *mySolution = NULL;
	Solver *mySolver = NULL;
	bool isDisplay = true;

	commandline c(argc, argv);
	if (c.is_valid())
	{
		// Parsing the problem instance
		myData = new Pb_Data(c.get_path_to_instance(), c.get_problem_type(), c.get_penaltyC(), c.get_nbThreads(), c.get_timeBudget(), c.get_normalizeData(), c.get_instanceFormat(), c.get_seed(), c.get_samplingSize(), c.get_nbSamplingIterations(), c.get_maxCBCuts(), c.get_timeProportionIDR(), c.get_timeProportionCBCuts(), c.get_timeProportionSampling(), c.get_locallyValidImpliedBounds());

		// Begin of clock
		myData->time_StartComput = clock();

		// Create an empty solution structure
		mySolution = new Solution(myData);

		// Create the solver data structures
		if (myData->problemType == HARD_IP || myData->problemType == SOFT_HL || myData->problemType == SOFT_LIBSVM)
			mySolver = new Solver_Primal(myData, mySolution, isDisplay);
		else if (myData->problemType == SAMPLING_CB_HARD_IP)
			mySolver = new Solver_Sampling(myData, mySolution, isDisplay);

		// Solve the problem
		mySolver->solve();

		// End of clock
		myData->time_EndComput = clock();

		// get accuracy
		mySolution->displayAccuracy();

		// Export and print the solution in case of 2D features
		mySolution->displaySolution();

		mySolution->exportSolution() ;

		cout << "END OF ALGORITHM, TIME: " << (((double)(myData->time_EndComput - myData->time_StartComput)) / CLOCKS_PER_SEC) << endl;
		lastLineCSVFormat(myData, mySolution, mySolver);
		delete myData;
		delete mySolution;
		delete mySolver;
	}
	else
		cout << "ERROR: Non-valid commandline, Usage: myProgram instance [-sol solution] [-problem problemType]" << endl;
	return 0;
}

void lastLineCSVFormat(Pb_Data *myData, Solution *mySolution, Solver *mySolver)
{
	string headerCSV = "", dataCSV = "";
	headerCSV += "solution-cost,";
	dataCSV += to_string(mySolution->solValue) + ",";

	headerCSV += "lower-bound,";
	dataCSV += to_string(mySolution->lowerBoundValue) + ",";

	headerCSV += "duality-gap,";
	dataCSV += to_string(mySolution->getDualityGap()) + ",";

	headerCSV += "elapsed-time,";
	double elapsed_time = (((double)(myData->time_EndComput - myData->time_StartComput)) / CLOCKS_PER_SEC);
	dataCSV += to_string(elapsed_time) + ",";

	headerCSV += ",";
	dataCSV += ",";

	headerCSV += "elapsed-time-master,";
	elapsed_time = ((double)myData->time_totalMaster) / CLOCKS_PER_SEC;
	dataCSV += to_string(elapsed_time) + ",";

	headerCSV += "elapsed-time-slave,";
	elapsed_time = ((double)myData->time_totalSlave) / CLOCKS_PER_SEC;
	dataCSV += to_string(elapsed_time) + ",";

	long int nbCombinatorialBendersCuts = myData->map_CB_CUTS.size() + myData->map_sampling_CB_CUTS.size();
	headerCSV += "nb-cb-cuts,";
	dataCSV += to_string(nbCombinatorialBendersCuts) + ",";

	long int sumCutsSize = 0;
	for (auto CB_CUT : myData->map_CB_CUTS)
		sumCutsSize += CB_CUT.first.size();
	for (auto CB_CUT : myData->map_sampling_CB_CUTS)
		sumCutsSize += CB_CUT.first.size();

	headerCSV += "avg-cut-size,";
	if (nbCombinatorialBendersCuts > 0)
		dataCSV += to_string(((double)sumCutsSize) / nbCombinatorialBendersCuts) + ",";
	else
		dataCSV += "0,";

	headerCSV += "elapsed-time-sampling,";
	dataCSV += to_string(myData->time_totalSampling) + ",";

	headerCSV += "accuracy";
	dataCSV += to_string(mySolution->accuracy) + ",";

	cout << "Output statistics: " << endl
		 << headerCSV << endl
		 << dataCSV << endl;
}