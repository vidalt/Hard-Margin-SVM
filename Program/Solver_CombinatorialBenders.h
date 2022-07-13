#ifndef SOLVER_COMBINATORIALBENDERS_H
#define SOLVER_COMBINATORIALBENDERS_H

#include "Solver_Primal.h"

// SVM Solver with Hard Margin
// Solution of the Primal Formulation
class Solver_CombinatorialBenders : public Solver
{
public:
	Solver_CombinatorialBenders(Pb_Data *myData, Solution *mySolution, bool isDisplay) : Solver(myData, mySolution, isDisplay)
	{
		this->isSampleConsidered = vector<bool>(myData->nbSamples, true);
	}

	Solver_CombinatorialBenders(Pb_Data *myData, Solution *mySolution, bool isDisplay, vector<bool> &isSampleConsidered) : Solver(myData, mySolution, isDisplay)
	{
		this->isSampleConsidered = isSampleConsidered;
		int nbConsideredSamples = 0;
		for (int i = 0; i < isSampleConsidered.size(); i++)
		{
			if (isSampleConsidered[i])
				nbConsideredSamples++;
		}
	}

	// Returns 0 if solved properly
	int solve();

	int solveProblemWithFixed_Z(void) { return 0; }

	void solveHingeLossToInputProblemType();

	void iterativeDomainReduction(vector<double> &newUB_IDR, vector<double> &newLB_IDR)
	{
		cout << "Iterative Domain Reduction" << endl;

		clock_t time_start, time_end;
		double elapsedTime = 0.0;
		int i, j;

		double allocatedTime = myData->userInputTime * myData->timeProportionIDR;
		cout << "allocatedTime: " << allocatedTime << endl;

		const int current_CPXPARAM_MIP_Limits_Nodes = 100000;

		int status = 0;
		CPXENVptr envWTightening = CPXopenCPLEX(&status);							 // Open CPLEX environment
																					 // else
		CPXsetintparam(envWTightening, CPXPARAM_ScreenOutput, CPX_OFF);				 // Switching OFF the display
		CPXsetintparam(envWTightening, CPXPARAM_Read_DataCheck, CPX_DATACHECK_WARN); // Print warnings
		CPXsetintparam(envWTightening, CPX_PARAM_THREADS, myData->nbThreads);		 // number of threads
		CPXsetintparam(envWTightening, CPX_PARAM_MIPINTERVAL, 1000);				 // shows the log after every 10000 nodes
		CPXsetintparam(envWTightening, CPXPARAM_MIP_Limits_Nodes, current_CPXPARAM_MIP_Limits_Nodes);
		CPXsetintparam(envWTightening, CPXPARAM_MIP_Cuts_LocalImplied, myData->locallyValidImpliedBounds); // aggressive setting for separating local implied bound cuts [Belotti et al. 2016]

		bool exitLoop = false;

		string str_problem_name = "IterativeDomain_";
		CPXLPptr lpWTightening = CPXcreateprob(envWTightening, &status, str_problem_name.c_str()); // Create LP problem as a container

		// Create columns related to the Z variables
		double *costZ = new double[myData->nbSamples];
		char *xctypeZ = new char[myData->nbSamples];
		char **namesZ = new char *[myData->nbSamples];
		double *ubZ = new double[myData->nbSamples];
		double *lbZ = new double[myData->nbSamples];
		for (int i = 0; i < myData->nbSamples; i++)
		{
			costZ[i] = 0;
			xctypeZ[i] = 'B';
			namesZ[i] = new char[100];
			lbZ[i] = 0.0;
			sprintf(namesZ[i], "_Z(%d)", i);
			ubZ[i] = 1.0;
		}
		status = CPXnewcols(envWTightening, lpWTightening, myData->nbSamples, costZ, lbZ, ubZ, xctypeZ, namesZ);

		char **namesWB = new char *[myData->nbFeatures + 1];
		double *costWB = new double[myData->nbFeatures + 1];
		double *ubWB = new double[myData->nbFeatures + 1];
		double *lbWB = new double[myData->nbFeatures + 1];

		// Create columns related to the w norm_2
		cout << "Initial bounds for w_i: " << endl;
		for (int i = 0; i < myData->nbFeatures; i++)
		{
			costWB[i] = 0;
			namesWB[i] = new char[100];
			sprintf(namesWB[i], "_W(%d)", i);
			ubWB[i] = newUB_IDR[i];
			lbWB[i] = newLB_IDR[i];
			cout << newLB_IDR[i] << " <= w_" << i << " <= " << newUB_IDR[i] << endl;
		}
		cout << endl;
		namesWB[myData->nbFeatures] = new char[100];
		ubWB[myData->nbFeatures] = CPX_INFBOUND;
		lbWB[myData->nbFeatures] = -ubWB[myData->nbFeatures];
		costWB[myData->nbFeatures] = 0;
		sprintf(namesWB[myData->nbFeatures], "_B");
		status = CPXnewcols(envWTightening, lpWTightening, myData->nbFeatures + 1, costWB, lbWB, ubWB, NULL, namesWB);

		// Indicator constraints for z
		int *linind = new int[myData->nbFeatures + 1];
		double *linval = new double[myData->nbFeatures + 1];
		for (i = 0; i < myData->nbSamples; i++)
		{
			for (j = 0; j < myData->nbFeatures + 1; j++)
			{
				linind[j] = myData->nbSamples + j;
				if (j < myData->nbFeatures)
					linval[j] = myData->labels[i] * myData->samples[i][j];
				else
					linval[j] = myData->labels[i];
			}
			status = CPXaddindconstr(envWTightening, lpWTightening, i, 1, myData->nbFeatures + 1, 1.0, 'G', linind, linval, NULL);
		}

		// Quadratic restriction for the Upper bound
		int *quadrow = new int[myData->nbFeatures];
		int *quadcol = new int[myData->nbFeatures];
		double *quadval = new double[myData->nbFeatures];
		int *quadlinind = new int[2 * myData->nbSamples];
		double *quadlinval = new double[2 * myData->nbSamples];
		for (i = 0; i < myData->nbSamples; i++)
		{
			quadlinind[i] = i;
			quadlinval[i] = 2 * myData->penaltyC;
		}
		for (i = 0; i < myData->nbFeatures; i++)
		{
			quadrow[i] = myData->nbSamples + i;
			quadcol[i] = quadrow[i];
			quadval[i] = 1;
		}
		double initialUpperBound = 2 * solutionHL->solValue;
		status = CPXaddqconstr(envWTightening, lpWTightening, myData->nbSamples, myData->nbFeatures, initialUpperBound, 'L', quadlinind, quadlinval, quadrow, quadcol, quadval, NULL);

		if (myData->problemType == FIRST_CB_THEN_HARD_IP)
		{
			// Use all CB cuts as lazy constraints and user cuts
			int rmatbeg[] = {0};
			double rhs[] = {1.0};
			char sense[] = {'G'};
			int *rmatind = new int[myData->nbSamples];
			double *rmatval = new double[myData->nbSamples];
			for (auto const &CB_CUT : myData->map_CB_CUTS)
			{
				for (j = 0; j < myData->nbSamples; j++)
				{
					rmatind[j] = j;
					rmatval[j] = 0;
				}

				for (auto elem : CB_CUT.first)
					rmatval[elem] = 1;

				status = CPXaddusercuts(envWTightening, lpWTightening, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
				status = CPXaddlazyconstraints(envWTightening, lpWTightening, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
			}
			delete[] rmatind;
			delete[] rmatval;
		}

		// Variables for adjusting the bounds during the tightening
		int *integer_indices = new int[myData->nbFeatures + 1];
		for (i = 0; i < myData->nbFeatures + 1; i++)
			integer_indices[i] = myData->nbSamples + i;
		char *lu = new char[myData->nbFeatures + 1];

		int solutionSize = myData->nbSamples + myData->nbFeatures + 1;
		double *solution_values = new double[solutionSize];
		for (int i = 0; i < solutionSize; i++)
			solution_values[i] = -1;

		int it = 0;
		while (true)
		{
			bool lb_improved = false;
			bool ub_improved = false;
			cout << "iteration number: " << it << endl;
			for (int d = 0; d < myData->nbFeatures + 1; d++)
			{
				cout << "Current feature: " << d << endl;

				if (newUB_IDR[d] == newLB_IDR[d])
					continue;

				for (i = 0; i < myData->nbFeatures + 1; i++)
					costWB[i] = 0;

				costWB[d] = 1;

				// Update Bounds
				{
					for (i = 0; i < myData->nbFeatures + 1; i++)
					{
						ubWB[i] = newUB_IDR[i];
						lu[i] = 'U';
					}
					status = CPXtightenbds(envWTightening, lpWTightening, myData->nbFeatures + 1, integer_indices, lu, ubWB);

					for (i = 0; i < myData->nbFeatures + 1; i++)
					{
						lbWB[i] = newLB_IDR[i];
						lu[i] = 'L';
					}
					status = CPXtightenbds(envWTightening, lpWTightening, myData->nbFeatures + 1, integer_indices, lu, lbWB);
				}

				status = CPXchgobj(envWTightening, lpWTightening, myData->nbFeatures + 1, integer_indices, costWB);

				status = CPXchgobjsen(envWTightening, lpWTightening, CPX_MAX);

				// string str_model_export = "tightmodel_max" + std::to_string(d) + ".lp";
				// status = CPXwriteprob(envWTightening, lpWTightening, str_model_export.c_str(), NULL); // Exporting the model
				// if (status)
				// 	cout << "Failed to export ILP status: " << status << endl;

				CPXsetdblparam(envWTightening, CPX_PARAM_TILIM, allocatedTime - elapsedTime);
				cout << "Remaining time: " << (allocatedTime - elapsedTime) << endl;

				time_start = clock();
				status = CPXmipopt(envWTightening, lpWTightening); // Solving the model
				time_end = clock();

				double solValue = -1.0;
				int solStatus = -1;

				status = CPXsolution(envWTightening, lpWTightening, &solStatus, &solValue, solution_values, NULL, NULL, NULL);
				cout << "UB: solValue: " << solValue << "; solStatus: " << solStatus << endl;
				if ((solStatus == 101 || solStatus == 102 || solStatus == 105) && solValue + MY_EPSILON_e3 < newUB_IDR[d])
				{
					cout << status << ":" << solStatus << ":" << d << ":" << solValue << ":" << newUB_IDR[d];
					if ((d < myData->nbFeatures && solutionHL->solW[d] + MY_EPSILON_e3 < solValue) || (d == myData->nbFeatures && solutionHL->solB + MY_EPSILON_e3 < solValue))
					{
						cout << " ==> value updated;" << endl;
						ub_improved = true;
						newUB_IDR[d] = solValue;
					}
					cout << endl;
				}

				elapsedTime += ((double)(time_end - time_start)) / CLOCKS_PER_SEC;
				if (allocatedTime - elapsedTime < 1)
				{
					exitLoop = true;
					break;
				}

				// Update Bounds
				{
					for (i = 0; i < myData->nbFeatures + 1; i++)
					{
						ubWB[i] = newUB_IDR[i];
						lu[i] = 'U';
					}
					status = CPXtightenbds(envWTightening, lpWTightening, myData->nbFeatures + 1, integer_indices, lu, ubWB);

					for (i = 0; i < myData->nbFeatures + 1; i++)
					{
						lbWB[i] = newLB_IDR[i];
						lu[i] = 'L';
					}
					status = CPXtightenbds(envWTightening, lpWTightening, myData->nbFeatures + 1, integer_indices, lu, lbWB);
				}
				status = CPXchgobjsen(envWTightening, lpWTightening, CPX_MIN);

				// str_model_export = "tightmodel_min" + std::to_string(d) + ".lp";
				// status = CPXwriteprob(envWTightening, lpWTightening, str_model_export.c_str(), NULL); // Exporting the model
				// if (status)
				// 	cout << "Failed to export ILP status: " << status << endl;

				CPXsetdblparam(envWTightening, CPX_PARAM_TILIM, allocatedTime - elapsedTime);
				cout << "Remaining time: " << (allocatedTime - elapsedTime) << endl;

				time_start = clock();
				status = CPXmipopt(envWTightening, lpWTightening); // Solving the model
				time_end = clock();

				solValue = -1.0;
				solStatus = -1;
				status = CPXsolution(envWTightening, lpWTightening, &solStatus, &solValue, solution_values, NULL, NULL, NULL);
				cout << "LB: solValue: " << solValue << "; solStatus: " << solStatus << endl;
				if ((solStatus == 101 || solStatus == 102 || solStatus == 105) && solValue - MY_EPSILON_e3 > newLB_IDR[d])
				{
					cout << status << ":" << solStatus << ":" << d << ":" << solValue << ":" << newLB_IDR[d];
					if ((d < myData->nbFeatures && solutionHL->solW[d] - MY_EPSILON_e3 > solValue) || (d == myData->nbFeatures && solutionHL->solB - MY_EPSILON_e3 > solValue))
					{
						cout << " ==> value updated;";
						lb_improved = true;
						newLB_IDR[d] = solValue;
					}
					cout << endl;
				}
				cout << endl;
				elapsedTime += ((double)(time_end - time_start)) / CLOCKS_PER_SEC;
				if (allocatedTime - elapsedTime < 1)
				{
					exitLoop = true;
					break;
				}
			}
			cout << endl;
			cout << "ub_improved: " << ub_improved << "; lb_improved: " << lb_improved << "; exitLoop: " << exitLoop << endl;
			it++;
			if ((!lb_improved && !ub_improved) || exitLoop)
				break;
		}
		status = CPXfreeprob(envWTightening, &lpWTightening);

		myData->timeBudget -= elapsedTime;
		cout << "elapsedTime: " << elapsedTime << endl;
		for (int d = 0; d < myData->nbFeatures + 1; d++)
			cout << d << ": " << newUB_IDR[d] << ": " << newLB_IDR[d] << endl;
		cout << endl
			 << endl;
		status = CPXcloseCPLEX(&envWTightening);

		// Some bounds cannot be improved (equal values in UB and LB), but they are indeed modified in the IDR phase.
		// For each case, we return their values to their original values
		for (int i = 0; i < myData->nbFeatures; i++)
		{
			if (newUB_IDR[i] == newLB_IDR[i])
			{
				newUB_IDR[i] = std::sqrt(2 * this->solutionHL->solValue);
				newLB_IDR[i] = -newUB_IDR[i];
			}
		}
		if (newUB_IDR[myData->nbFeatures] == newLB_IDR[myData->nbFeatures])
		{
			newUB_IDR[myData->nbFeatures] = CPX_INFBOUND;
			newLB_IDR[myData->nbFeatures] = -newUB_IDR[myData->nbFeatures];
		}

		/* Free memory and close CPLEX */
		delete[] solution_values;

		delete[] costZ;
		delete[] xctypeZ;
		for (int i = 0; i < myData->nbSamples; i++)
			delete[] namesZ[i];
		delete[] namesZ;
		delete[] ubZ;
		delete[] lbZ;
		for (int i = 0; i < myData->nbFeatures + 1; i++)
			delete[] namesWB[i];
		delete[] namesWB;
		delete[] lbWB;
		delete[] ubWB;
		delete[] costWB;

		delete[] quadrow;
		delete[] quadcol;
		delete[] quadval;
		delete[] quadlinind;
		delete[] quadlinval;

		delete[] linind;
		delete[] linval;

		delete[] integer_indices;
		delete[] lu;
	}
};
#endif
