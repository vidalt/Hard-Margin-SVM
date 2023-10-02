#include "Solver_Primal.h"
#include "libsvm-3.24/svm.h"

#define Malloc(type, n) (type *)malloc((n) * sizeof(type))

struct svm_parameter param; // set by parse_command_line
struct svm_problem prob;	// set by read_problem
struct svm_model *model;
struct svm_node *x_space;

int Solver_Primal::solveLibSVM()
{
	int nbSamplesInLibSVM = 0;
	for (int i = 0; i < myData->nbSamples; i++)
	{
		if (isSampleConsidered[i])
			nbSamplesInLibSVM++;
	}
	prob.l = nbSamplesInLibSVM;
	cout << "LibSVM: problem size: " << prob.l << endl;

	prob.y = Malloc(double, prob.l); //space for prob.l doubles

	prob.x = Malloc(struct svm_node *, prob.l); //space for prob.l pointers to struct svm_node

	x_space = Malloc(struct svm_node, (myData->nbFeatures + 1) * prob.l); //memory for pairs of index/value

	int addedElements = 0;
	int j = 0;

	for (int i = 0; i < myData->nbSamples; i++)
	{
		if (isSampleConsidered[i])
		{
			prob.y[addedElements] = myData->labels[i];
			prob.x[addedElements] = &x_space[j];
			for (int k = 0; k < myData->nbFeatures; ++k, ++j)
			{
				x_space[j].index = k + 1;				  //index of value
				x_space[j].value = myData->samples[i][k]; //value
			}
			x_space[j].index = -1; //state the end of data vector
			x_space[j].value = 0;
			j++;
			addedElements++;
		}
	}

	//set all default parameters for param struct
	param.svm_type = C_SVC;
	param.kernel_type = LINEAR;
	param.degree = 1;			// Default is OK
	param.gamma = 1/myData->nbFeatures;			// 1/num_features - Default is OK
	param.coef0 = 0;			// Default is OK
	param.nu = 0.5;				// Default is OK
	param.cache_size = 100;		// Default is OK
	param.C = myData->penaltyC; // Input by our program
	param.eps = 1e-3;			// Default is 1e-3
	param.p = 0.1;				// Default is OK
	param.shrinking = 0;		// Default is 1.
	param.probability = 0;		// Default is OK
	param.nr_weight = 0;		// Default is OK
	param.weight_label = NULL;	// Default is OK
	param.weight = NULL;		// Default is OK
	cout << "LibSVM: param.eps: " << param.eps << endl;

	model = svm_train(&prob, &param);

	int nr_class = model->nr_class;
	int l = model->l;

	//it's only binary classes; So, there's only model->rho[0]
	//Also, -model->rho is b as shown in the paper "LIBSVM -- A Library for Support Vector Machines"

	for (int i = 0; i < nr_class * (nr_class - 1) / 2; i++)
		mySolution->solB = -model->rho[i];

	for (int i = 0; i < myData->nbFeatures; i++)
		mySolution->solW[i] = 0.0;

	const double *const *sv_coef = model->sv_coef;
	const svm_node *const *SV = model->SV;

	double alpha;
	for (int i = 0; i < l; i++)
	{
		alpha = 0.0;
		for (int j = 0; j < nr_class - 1; j++)
			alpha = sv_coef[j][i];
		const svm_node *p = SV[i];
		while (p->index != -1)
		{
			mySolution->solW[p->index - 1] += alpha * p->value;
			p++;
		}
	}
	svm_free_and_destroy_model(&model);
	svm_destroy_param(&param);
	free(prob.y);
	free(prob.x);
	free(x_space);
	return 0;
}

int Solver_Primal::solveProblemWithFixed_Z()
{
	CPXENVptr env = NULL;
	CPXLPptr lp = NULL;
	int i, j;
	int status = 0;
	env = CPXopenCPLEX(&status); // Open CPLEX environment
	if (this->isDisplay)
		CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_ON); // Switching ON the display
	else
		CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_OFF);								// Switching OFF the display
	CPXsetintparam(env, CPXPARAM_Read_DataCheck, CPX_DATACHECK_WARN);						// Print warnings
	CPXsetintparam(env, CPX_PARAM_MIPINTERVAL, 10000);										// shows the log after every 10000 nodes
	CPXsetintparam(env, CPX_PARAM_THREADS, myData->nbThreads);								// number of threads
	CPXsetintparam(env, CPXPARAM_MIP_Cuts_LocalImplied, myData->locallyValidImpliedBounds); // aggressive setting for separating local implied bound cuts [Belotti et al. 2016]

	// sets time limit for the solver.
	if (myData->timeBudget > 0)
		CPXsetdblparam(env, CPX_PARAM_TILIM, myData->timeBudget);

	string str_problem_name = "SVM" + myData->outputIdentifier;
	lp = CPXcreateprob(env, &status, str_problem_name.c_str()); // Create LP problem as a container

	int nbConsideredSamples = 0;
	for (int i = 0; i < myData->nbSamples; i++)
	{
		if (this->isSampleConsidered[i])
			nbConsideredSamples++;
	}
	if (this->isDisplay)
	{
		cout << "Solving Primal Model with: " << endl;
		cout << "\t" << nbConsideredSamples << " samples" << endl
			 << endl;
	}
	// Create columns related to the Z variables
	double *costZ = new double[myData->nbSamples];
	char *xctypeZ = new char[myData->nbSamples];
	char **namesZ = new char *[myData->nbSamples];
	double *ubZ = new double[myData->nbSamples];
	double *lbZ = new double[myData->nbSamples];
	for (int i = 0; i < myData->nbSamples; i++)
	{
		costZ[i] = lbZ[i] = ubZ[i] = 0.0;
		xctypeZ[i] = 'B';
		namesZ[i] = new char[100];
		sprintf(namesZ[i], "_Z(%d)", i);
		if (this->isSampleConsidered[i])
		{
			costZ[i] = myData->penaltyC;
			ubZ[i] = lbZ[i] = mySolution->solZ[i];
		}
	}
	status = CPXnewcols(env, lp, myData->nbSamples, costZ, lbZ, ubZ, xctypeZ, namesZ);

	// Create columns related to the W variables and b
	char **namesWB = new char *[myData->nbFeatures + 1];
	double *lbWB = new double[myData->nbFeatures + 1];
	double *ubWB = new double[myData->nbFeatures + 1];
	for (int i = 0; i < myData->nbFeatures + 1; i++)
	{
		namesWB[i] = new char[100];
		ubWB[i] = CPX_INFBOUND;
		lbWB[i] = -ubWB[i];
		if (i < myData->nbFeatures)
			sprintf(namesWB[i], "_W(%d)", i);
		else
			sprintf(namesWB[i], "_B");
	}
	status = CPXnewcols(env, lp, myData->nbFeatures + 1, NULL, lbWB, ubWB, NULL, namesWB); // By default LB is set to 0, need to manually allow negative values with lbWB

	// Adding the quadratic terms of the objective
	double *qsepvec = new double[myData->nbSamples + myData->nbFeatures + 1];
	for (int i = 0; i < myData->nbSamples + myData->nbFeatures + 1; i++)
	{
		if (i >= myData->nbSamples && i < myData->nbSamples + myData->nbFeatures)
			qsepvec[i] = 1;
		else
			qsepvec[i] = 0;
	}
	status = CPXcopyqpsep(env, lp, qsepvec);

	// Adding rows (constraints) related to the hyperplanes
	for (int i = 0; i < myData->nbSamples; i++)
	{
		if (this->isSampleConsidered[i])
		{
			int *linind = new int[myData->nbFeatures + 1];
			double *linval = new double[myData->nbFeatures + 1];
			for (int j = 0; j < myData->nbFeatures + 1; j++)
			{
				linind[j] = myData->nbSamples + j;
				if (j < myData->nbFeatures)
					linval[j] = myData->labels[i] * myData->samples[i][j];
				else
					linval[j] = myData->labels[i];
			}
			CPXaddindconstr(env, lp, i, 1, myData->nbFeatures + 1, 1.0, 'G', linind, linval, NULL);
			delete[] linind;
			delete[] linval;
		}
	}

	// string str_model_export = "Fixed-Z-SVM" + myData->outputIdentifier + ".lp";
	// status = CPXwriteprob(env, lp, str_model_export.c_str(), NULL); // Exporting the model
	// if (status)
	// 	cout << "Failed to export ILP" << endl;

	int returnStatus = 0;
	status = CPXmipopt(env, lp); // Solving the model
	double *solution = new double[myData->nbSamples + myData->nbFeatures + 1];
	status = CPXsolution(env, lp, &mySolution->solStatus, &mySolution->solValue, solution, NULL, NULL, NULL);
	if (status)
		cout << "Fixed Z. Failed to obtain solution. Status code: " << status << endl;

	if (mySolution->solStatus == CPXMIP_OPTIMAL || mySolution->solStatus == CPXMIP_OPTIMAL_TOL)
		mySolution->lowerBoundValue = mySolution->solValue;
	else
		status = CPXgetbestobjval(env, lp, &mySolution->lowerBoundValue);
	/* Fill solution structure */
	for (i = 0; i < myData->nbSamples; i++)
		mySolution->solZ[i] = (bool)solution[i];
	for (j = 0; j < myData->nbFeatures; j++)
		mySolution->solW[j] = solution[myData->nbSamples + j];
	for (i = 0; i < myData->nbSamples; i++)
		mySolution->solE[i] = 0.0;
	mySolution->solB = solution[myData->nbSamples + myData->nbFeatures];
	mySolution->isSolComputed = true;

	delete[] solution;
	/* Free memory and close CPLEX */
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
	delete[] ubWB;
	delete[] lbWB;

	delete[] qsepvec;

	status = CPXfreeprob(env, &lp);
	status = CPXcloseCPLEX(&env);
	return returnStatus;
}

int Solver_Primal::solve()
{
	if (myData->problemType == SOFT_LIBSVM)
	{
		return solveLibSVM();
	}
	else
	{
		CPXENVptr env = NULL;
		CPXLPptr lp = NULL;
		int i, j;
		int status = 0;
		env = CPXopenCPLEX(&status); // Open CPLEX environment
		if (this->isDisplay)
			CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_ON); // Switching ON the display
		else
			CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_OFF);								// Switching OFF the display
		CPXsetintparam(env, CPXPARAM_Read_DataCheck, CPX_DATACHECK_WARN);						// Print warnings
		CPXsetintparam(env, CPX_PARAM_MIPINTERVAL, 10000);										// shows the log after every 10000 nodes
		CPXsetintparam(env, CPX_PARAM_THREADS, myData->nbThreads);								// number of threads
		CPXsetintparam(env, CPXPARAM_MIP_Cuts_LocalImplied, myData->locallyValidImpliedBounds); // aggressive setting for separating local implied bound cuts [Belotti et al. 2016]

		MYCB info;
		info.myData = myData;
		info.startTime = clock();
		
		if (myData->problemType == HARD_IP)
		{
			info.overtimeSolutions.open("overtime-solutions" + myData->outputIdentifier + ".txt");
			status = CPXsetincumbentcallbackfunc(env, callback_check_new_incumbent, &info);
		}

		// sets time limit for the solver.
		if (myData->timeBudget > 0)
			CPXsetdblparam(env, CPX_PARAM_TILIM, myData->timeBudget);

		string str_problem_name = "SVM" + myData->outputIdentifier;
		lp = CPXcreateprob(env, &status, str_problem_name.c_str()); // Create LP problem as a container

		int nbConsideredSamples = 0;
		for (int i = 0; i < myData->nbSamples; i++)
		{
			if (this->isSampleConsidered[i])
				nbConsideredSamples++;
		}
		if (this->isDisplay)
		{
			cout << "Solving Primal Model with: " << endl;
			cout << "\t" << nbConsideredSamples << " samples" << endl
				 << endl;
		}
		// Create columns related to the Z variables
		double *costZ = new double[myData->nbSamples];
		char *xctypeZ = new char[myData->nbSamples];
		char **namesZ = new char *[myData->nbSamples];
		double *ubZ = new double[myData->nbSamples];
		double *lbZ = new double[myData->nbSamples];
		for (int i = 0; i < myData->nbSamples; i++)
		{
			costZ[i] = lbZ[i] = ubZ[i] = 0.0;
			xctypeZ[i] = 'B';
			namesZ[i] = new char[100];
			sprintf(namesZ[i], "_Z(%d)", i);
			if (this->isSampleConsidered[i] && (myData->problemType == HARD_IP || myData->problemType == FIRST_CB_THEN_HARD_IP))
			{
				costZ[i] = myData->penaltyC;
				ubZ[i] = 1.0;
			}
		}
		status = CPXnewcols(env, lp, myData->nbSamples, costZ, lbZ, ubZ, xctypeZ, namesZ);

		// Create columns related to the W variables and b
		char **namesWB = new char *[myData->nbFeatures + 1];
		double *lbWB = new double[myData->nbFeatures + 1];
		for (int i = 0; i < myData->nbFeatures + 1; i++)
		{
			namesWB[i] = new char[100];
			if (i < myData->nbFeatures)
				sprintf(namesWB[i], "_W(%d)", i);
			else
				sprintf(namesWB[i], "_B");
			lbWB[i] = -CPX_INFBOUND;
		}
		status = CPXnewcols(env, lp, myData->nbFeatures + 1, NULL, lbWB, NULL, NULL, namesWB); // By default LB is set to 0, need to manually allow negative values with lbWB

		// Create columns related to the epsilon (E) variables
		char **namesE = new char *[myData->nbSamples];
		double *lbE = new double[myData->nbSamples];
		double *ubE = new double[myData->nbSamples];
		double *costE = new double[myData->nbSamples];
		for (int i = 0; i < myData->nbSamples; i++)
		{
			namesE[i] = new char[100];
			sprintf(namesE[i], "_E(%d)", i);
			costE[i] = lbE[i] = ubE[i] = 0.0;
			if (this->isSampleConsidered[i] && myData->problemType == SOFT_HL)
			{
				ubE[i] = CPX_INFBOUND;
				costE[i] = myData->penaltyC;
			}
		}
		status = CPXnewcols(env, lp, myData->nbSamples, costE, lbE, ubE, NULL, namesE);
		// Adding the quadratic terms of the objective
		double *qsepvec = new double[2 * myData->nbSamples + myData->nbFeatures + 1];
		for (int i = 0; i < 2 * myData->nbSamples + myData->nbFeatures + 1; i++)
		{
			if (i >= myData->nbSamples && i < myData->nbSamples + myData->nbFeatures)
				qsepvec[i] = 1;
			else
				qsepvec[i] = 0;
		}
		status = CPXcopyqpsep(env, lp, qsepvec);

		// Adding rows (constraints) related to the hyperplanes
		for (int i = 0; i < myData->nbSamples; i++)
		{
			if (this->isSampleConsidered[i])
			{
				int *linind = new int[myData->nbFeatures + 2];
				double *linval = new double[myData->nbFeatures + 2];
				for (int j = 0; j < myData->nbFeatures + 1; j++)
				{
					linind[j] = myData->nbSamples + j;
					if (j < myData->nbFeatures)
						linval[j] = myData->labels[i] * myData->samples[i][j];
					else
						linval[j] = myData->labels[i];
				}
				linind[myData->nbFeatures + 1] = myData->nbSamples + myData->nbFeatures + 1 + i;
				if (myData->problemType == SOFT_HL)
					linval[myData->nbFeatures + 1] = 1.0;
				else
					linval[myData->nbFeatures + 1] = 0.0;

				CPXaddindconstr(env, lp, i, 1, myData->nbFeatures + 2, 1.0, 'G', linind, linval, NULL);
				delete[] linind;
				delete[] linval;
			}
		}

		// string str_model_export = "Primal-SVM" + myData->outputIdentifier + ".lp";
		// status = CPXwriteprob(env, lp, str_model_export.c_str(), NULL); // Exporting the model
		// if (status)
		// 	cout << "Failed to export ILP" << endl;

		int returnStatus = 0;
		status = CPXmipopt(env, lp); // Solving the model
		if (status)
			cout << "Primal-SVM. Failed to solve problem. Status code: " << status << endl;

		double *solution = new double[2 * myData->nbSamples + myData->nbFeatures + 1];
		returnStatus = CPXsolution(env, lp, &mySolution->solStatus, &mySolution->solValue, solution, NULL, NULL, NULL);
		if (returnStatus)
			cout << "Primal-SVM. Failed to obtain solution problem. Status code: " << returnStatus << "; mySolution->solStatus: " << mySolution->solStatus << endl;
		if (mySolution->solStatus == CPXMIP_OPTIMAL || mySolution->solStatus == CPXMIP_OPTIMAL_TOL)
			mySolution->lowerBoundValue = mySolution->solValue;
		else
			status = CPXgetbestobjval(env, lp, &mySolution->lowerBoundValue);
		/* Fill solution structure */
		for (i = 0; i < myData->nbSamples; i++)
			mySolution->solZ[i] = (bool)solution[i];
		for (j = 0; j < myData->nbFeatures; j++)
			mySolution->solW[j] = solution[myData->nbSamples + j];
		for (i = 0; i < myData->nbSamples; i++)
			mySolution->solE[i] = solution[myData->nbSamples + myData->nbFeatures + 1 + i];
		mySolution->solB = solution[myData->nbSamples + myData->nbFeatures];
		mySolution->isSolComputed = true;

		delete[] solution;
		/* Free memory and close CPLEX */
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

		delete[] qsepvec;

		status = CPXfreeprob(env, &lp);
		status = CPXcloseCPLEX(&env);
		
		if (myData->problemType == HARD_IP)
			info.overtimeSolutions.close();
		return returnStatus;
	}
}
