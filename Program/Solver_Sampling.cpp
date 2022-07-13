#include "Solver_Sampling.h"
#include "Solver_CombinatorialBenders.h"
int Solver_Sampling::solve()
{
	if (myData->problemType == SAMPLING_CB_HARD_IP)
		solverSamplingCB();
	else
		cout << "No valid option" << endl;
	return 0;
}

void Solver_Sampling::solverSamplingCB()
{
	//We test the separability of the instance by running HARD problem.
	//If there's a solution (problemStatus == 0), the problem is linearly separable.
	//Therefore, no CB will be generated
	isSampleConsidered = vector<bool>(myData->nbSamples, true);
	Solution *mySolutionSeparability = new Solution(myData);
	ProblemType bkpProblemType = myData->problemType;
	myData->problemType = HARD;
	Solver *mySolverPrimal = new Solver_Primal(myData, mySolutionSeparability, false, isSampleConsidered);
	int problemStatus = mySolverPrimal->solve();
	myData->problemType = bkpProblemType;

	// If problemStatus == 0, the problem is linearly separable. No need in wasting time attempting to generate cuts
	if (problemStatus == 0)
		cout << "Problem is linearly separable. Avoiding sampling phase..." << endl;
	else
	{
		cout << "** Ignore CPLEX error. The problem is not linearly separable." << endl;
		/** Initial parameters **/
		if (myData->timeProportionSampling <= 1.0 + MY_EPSILON_e7)
			myData->nbSamplingIterations = -1;
		else if (myData->nbSamplingIterations < 0)
		{
			myData->nbSamplingIterations = myData->nbSamples;
		}

		if (myData->samplingSize < 0)
			myData->samplingSize = (myData->nbSamples <= 100) ? myData->nbSamples / 2 : 50;
		cout << "nbSamplingIterations: " << myData->nbSamplingIterations << endl;
		cout << "samplingSize: " << myData->samplingSize << endl;
		cout << "time allocated to sampling phase: " << (myData->userInputTime * myData->timeProportionCBCuts * myData->timeProportionSampling) << endl;
		if (myData->userInputTime * myData->timeProportionCBCuts * myData->timeProportionSampling > MY_EPSILON_e7)
		{
			vector<int> idSamples = vector<int>(myData->nbSamples);
			for (int i = 0; i < myData->nbSamples; i++)
				idSamples[i] = i;

			// accounting proportion in class members
			int nbSamplesPositive = 0, nbSamplesNegative = 0;
			for (int i = 0; i < myData->nbSamples; i++)
			{
				if (myData->labels[i] == 1)
					nbSamplesPositive++;
				else
					nbSamplesNegative++;
			}
			cout << "nbSamplesPositive: " << nbSamplesPositive << " " << nbSamplesNegative << endl;

			int nbMaxPositiveSamplesInSampling = (nbSamplesPositive / (myData->nbSamples * 1.0) * myData->samplingSize) + 1;
			int nbMaxNegativeSamplesInSampling = (nbSamplesNegative / (myData->nbSamples * 1.0) * myData->samplingSize) + 1;
			cout << "nbMaxPositiveSamplesInSampling: " << nbMaxPositiveSamplesInSampling << endl;
			cout << "nbMaxNegativeSamplesInSampling: " << nbMaxNegativeSamplesInSampling << endl;

			Solution *subProblemSolution = new Solution(myData);
			clock_t start_sampling;
			for (int k = 0;; k++)
			{
				start_sampling = clock();
				if (myData->nbSamplingIterations > 0 && k >= myData->nbSamplingIterations)
				{
					cout << "Number of iterations exceeded." << endl;
					break;
				}

				isSampleConsidered = vector<bool>(myData->nbSamples, false);
				std::random_shuffle(idSamples.begin(), idSamples.end());
				int currentNbPositiveSamples = 0, currentNbNegativeSamples = 0;
				int i = 0;
				set<int> selectedSamples;
				while (currentNbPositiveSamples + currentNbNegativeSamples < myData->samplingSize && i < myData->nbSamples)
				{
					if (myData->labels[idSamples[i]] == 1 && currentNbPositiveSamples < nbMaxPositiveSamplesInSampling)
					{
						selectedSamples.insert(idSamples[i]);
						currentNbPositiveSamples++;
						isSampleConsidered[idSamples[i]] = true;
					}
					else if (myData->labels[idSamples[i]] == -1 && currentNbNegativeSamples < nbMaxNegativeSamplesInSampling)
					{
						selectedSamples.insert(idSamples[i]);
						currentNbNegativeSamples++;
						isSampleConsidered[idSamples[i]] = true;
					}
					i++;
				}
				if (k % 250 == 0)
					cout << "Sampling iteration: " << k;
				myData->timeBudget -= (((double)(clock() - start_sampling)) / CLOCKS_PER_SEC);
				subProblemSolution->solStatus = -1;
				Solver *mySolverSubproblem = new Solver_CombinatorialBenders(myData, subProblemSolution, false, isSampleConsidered);
				mySolverSubproblem->solve();

				start_sampling = clock();
				myData->LB_CUTS.push_back(selectedSamples);
				myData->LB_CUTS_rhs.push_back(subProblemSolution->lowerBoundValue);
				for (auto const &CB_CUT : myData->map_CB_CUTS)
					myData->map_sampling_CB_CUTS.insert(CB_CUT);

				if (k % 250 == 0)
					cout << "; nb generated cuts: " << myData->map_sampling_CB_CUTS.size() << "; timeBudget: " << myData->timeBudget << endl;

				myData->map_CB_CUTS.clear();
				delete mySolverSubproblem;
				myData->timeBudget -= (((double)(clock() - start_sampling)) / CLOCKS_PER_SEC);
				if ((myData->userInputTime - myData->timeBudget) >= myData->userInputTime * myData->timeProportionCBCuts * myData->timeProportionSampling)
				{
					cout << "Time limit exceeded" << endl;
					break;
				}

				if (myData->maxCBCuts > 0 && myData->map_sampling_CB_CUTS.size() >= myData->maxCBCuts)
				{
					cout << "Number cuts exceeded" << endl;
					break;
				}
			}
			delete subProblemSolution;
		}
	}
	cout << "nb CB cuts generated in sampling phase: " << myData->map_sampling_CB_CUTS.size() << endl; 
	myData->time_totalSampling = myData->userInputTime - myData->timeBudget;
	cout << "myData->time_totalSampling: " << myData->time_totalSampling << endl
		 << endl;

	myData->problemType = FIRST_CB_THEN_HARD_IP;
	Solver *mySolver = new Solver_CombinatorialBenders(myData, mySolution, isDisplay);
	mySolver->solve();

	delete mySolver;
	delete mySolverPrimal;
	delete mySolutionSeparability;
	return;
}
