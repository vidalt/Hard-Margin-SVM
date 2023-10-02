#include "Solver_CombinatorialBenders.h"

#define ALLOW_LB_CONSTR_SVM 0
#define ALLOW_QC_SLAVE 0
#define ALLOW_SAMPLING_CUTS_TO_CONSTR_SVM 1


static int CPXPUBLIC callback_constr_svm(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p);

double *master_solution = NULL, *slave_solution = NULL;
double *master_solution_wub = NULL, *master_solution_wlb = NULL;

// variables for global cuts to main problem
int *rmatindAddCut = NULL;
double *rmatvalAddCut = NULL;

// variables for master constraints
int *rmatindMaster = NULL;
double *rmatvalMaster = NULL;

// hyperplane variables for columns in slave model
char **namesWBSlave = NULL;
double *costWBSlave = NULL, *ubWBSlave = NULL, *lbWBSlave = NULL;

// variables for indicator constraints in slave
int *rmatindSlaveHyperplane = NULL;
double *rmatvalSlaveHyperplane = NULL;

// conflict variables for slave
int *rowIndConflict = NULL, *rowBdStatConflict = NULL;

// Quadratic restriction for the Upper bound
int *quadrowSlave;
int *quadcolSlave;
double *quadvalSlave;

vector<double> newUB_IDR, newLB_IDR;

void alloc_global_CB_vars(Pb_Data *myData, Solution *solutionHL)
{
    master_solution = new double[myData->nbSamples];
    slave_solution = new double[2 * myData->nbFeatures + 1];

    master_solution_wlb = new double[myData->nbFeatures];
    master_solution_wub = new double[myData->nbFeatures];

    rmatindAddCut = new int[myData->nbSamples];
    rmatvalAddCut = new double[myData->nbSamples];

    rmatindMaster = new int[myData->nbSamples];
    rmatvalMaster = new double[myData->nbSamples];

    rmatindSlaveHyperplane = new int[myData->nbFeatures + 1];
    rmatvalSlaveHyperplane = new double[myData->nbFeatures + 1];

    namesWBSlave = new char *[myData->nbFeatures + 1];
    for (int i = 0; i < myData->nbFeatures; i++)
        namesWBSlave[i] = new char[100];
    costWBSlave = new double[myData->nbFeatures + 1];
    ubWBSlave = new double[myData->nbFeatures + 1];
    lbWBSlave = new double[myData->nbFeatures + 1];

    // Create columns related for w
    for (int i = 0; i < myData->nbFeatures; i++)
    {
        costWBSlave[i] = 0;
        sprintf(namesWBSlave[i], "_W(%d)", i);
        ubWBSlave[i] = (newUB_IDR[i] != CPX_INFBOUND) ? newUB_IDR[i] : CPX_INFBOUND;
        lbWBSlave[i] = (newLB_IDR[i] != -CPX_INFBOUND) ? newLB_IDR[i] : -CPX_INFBOUND;
    }
    namesWBSlave[myData->nbFeatures] = new char[100];
    sprintf(namesWBSlave[myData->nbFeatures], "_B");

    ubWBSlave[myData->nbFeatures] = CPX_INFBOUND;
    lbWBSlave[myData->nbFeatures] = -CPX_INFBOUND;
    costWBSlave[myData->nbFeatures] = 0;

    rowIndConflict = new int[myData->nbSamples];
    rowBdStatConflict = new int[myData->nbSamples];

    quadrowSlave = new int[myData->nbFeatures];
    quadcolSlave = new int[myData->nbFeatures];
    quadvalSlave = new double[myData->nbFeatures];
    for (int i = 0; i < myData->nbFeatures; i++)
    {
        quadrowSlave[i] = i;
        quadcolSlave[i] = quadrowSlave[i];
        quadvalSlave[i] = 1;
    }
}

void free_global_CB_vars(Pb_Data *myData)
{
    delete[] master_solution;
    delete[] slave_solution;

    delete[] master_solution_wub;
    delete[] master_solution_wlb;

    delete[] rmatindAddCut;
    delete[] rmatvalAddCut;

    delete[] rmatindMaster;
    delete[] rmatvalMaster;

    delete[] rmatindSlaveHyperplane;
    delete[] rmatvalSlaveHyperplane;

    for (int i = 0; i < myData->nbFeatures + 1; i++)
        delete[] namesWBSlave[i];
    delete[] namesWBSlave;
    delete[] lbWBSlave;
    delete[] ubWBSlave;
    delete[] costWBSlave;

    delete[] rowIndConflict;
    delete[] rowBdStatConflict;

    delete[] quadrowSlave;
    delete[] quadcolSlave;
    delete[] quadvalSlave;
}

void Solver_CombinatorialBenders::solveHingeLossToInputProblemType()
{
    // Get a solution from Hinge Loss
    // We use this solution to IDR and initial start in the solution algorithm
    double timeForHingeLoss = myData->userInputTime - myData->userInputTime * (myData->timeProportionIDR + myData->timeProportionCBCuts);
    double elapsedTimeHingeLoss = 0.0, elapsedTimeFixedZ = 0.0;
    ProblemType bkpProblemtype = myData->problemType;
    myData->problemType = SOFT_HL;
    //We allocate a bit of time to generate the initial solution
    double auxTimeBudget = myData->timeBudget;
    myData->timeBudget = timeForHingeLoss * 0.1;
    Solver *solverHL = new Solver_Primal(myData, this->solutionHL, false, isSampleConsidered);
    clock_t time_start = clock();
    solverHL->solve();
    clock_t time_end = clock();
    myData->timeBudget = auxTimeBudget;
    myData->problemType = bkpProblemtype;

    elapsedTimeHingeLoss = ((double)(time_end - time_start)) / CLOCKS_PER_SEC;

    myData->timeBudget -= elapsedTimeHingeLoss;
    if (isDisplay)
    {
        cout << "Calculating Hinge loss" << endl;
        cout << "Allocated time for initial solution: " << myData->timeBudget << endl;
        cout << "Solution cost value: " << this->solutionHL->solValue << endl;
        cout << "elapsedTimeHingeLoss: " << elapsedTimeHingeLoss << endl;
        for (int i = 0; i < myData->nbFeatures; i++)
            cout << "w[" << i << "]: " << this->solutionHL->solW[i] << endl;
        cout << "b: " << this->solutionHL->solB << endl;
        cout << "------\n"
             << endl;
    }

    // We convert each \xi_i of Hinge loss to a respective z_i for hard-margin
    for (int i = 0; i < myData->nbSamples; i++)
    {
        this->solutionHL->solZ[i] = (this->solutionHL->solE[i] > MY_EPSILON_e7);
        this->solutionHL->solE[i] = 0;
    }

    Solver *solverHLFixed = new Solver_Primal(myData, this->solutionHL, false, isSampleConsidered);
    time_start = clock();
    solverHLFixed->solveProblemWithFixed_Z();
    time_end = clock();

    elapsedTimeFixedZ = ((double)(time_end - time_start)) / CLOCKS_PER_SEC;
    myData->timeBudget -= elapsedTimeFixedZ;
    if (isDisplay)
    {
        cout << "Solving problem with fixed Z" << endl;
        cout << "Solution cost value: " << this->solutionHL->solValue << endl;
        cout << "elapsedTimeFixedZ: " << elapsedTimeFixedZ << endl;
        for (int i = 0; i < myData->nbFeatures; i++)
            cout << "w[" << i << "]: " << this->solutionHL->solW[i] << endl;
        cout << "b: " << this->solutionHL->solB << endl;
        cout << "------\n"
             << endl;
    }
    delete solverHL;
    delete solverHLFixed;
}

int Solver_CombinatorialBenders::solve()
{
    int status = 0;
    if (!(myData->problemType == FIRST_CB_THEN_HARD_IP || myData->problemType == SAMPLING_CB_HARD_IP))
        return status;

    clock_t clock_start = 0.0, clock_end = 0.0;
    double elapsedTime = 0.0;

    // Solution obtain a solution for the current problem type by solving hinge loss
    this->solveHingeLossToInputProblemType();

    int numberOfZ_1 = 0, nbSamplesConsidered = 0;
    for (int i = 0; i < myData->nbSamples; i++)
    {
        if (this->solutionHL->solZ[i])
            numberOfZ_1++;
        if (this->isSampleConsidered[i])
            nbSamplesConsidered++;
    }

    //We only apply the initial bound reduction regarding the whole set of instances
    if (nbSamplesConsidered == myData->nbSamples)
    {
        newUB_IDR = vector<double>(myData->nbFeatures + 1, std::sqrt(2 * this->solutionHL->solValue));
        newLB_IDR = vector<double>(myData->nbFeatures + 1, -std::sqrt(2 * this->solutionHL->solValue));
    }
    else
    {
        newUB_IDR = vector<double>(myData->nbFeatures + 1, CPX_INFBOUND);
        newLB_IDR = vector<double>(myData->nbFeatures + 1, -CPX_INFBOUND);
    }
    newUB_IDR[myData->nbFeatures] = CPX_INFBOUND;
    newLB_IDR[myData->nbFeatures] = -CPX_INFBOUND;

    //We only apply the iterative domain reduction regarding the whole set of instances
    if (myData->timeProportionIDR > 0 && nbSamplesConsidered == myData->nbSamples)
        this->iterativeDomainReduction(newUB_IDR, newLB_IDR);

    if (isDisplay)
    {
        cout << "Bounds for w_i: " << endl;
        for (int i = 0; i < newLB_IDR.size(); i++)
            cout << newLB_IDR[i] << " <= w_" << i << " <= " << newUB_IDR[i] << endl;
        cout << endl;
    }

    alloc_global_CB_vars(myData, this->solutionHL);

    int i = 0, j;
    CPXENVptr envCB = NULL;
    envCB = CPXopenCPLEX(&status); // Open CPLEX environment
    if (this->isDisplay)
        CPXsetintparam(envCB, CPXPARAM_ScreenOutput, CPX_ON); // Switching ON the display
    else
        CPXsetintparam(envCB, CPXPARAM_ScreenOutput, CPX_OFF); // Switching ON the display

    CPXsetintparam(envCB, CPXPARAM_Read_DataCheck, CPX_DATACHECK_WARN); // Print warnings
    CPXsetintparam(envCB, CPX_PARAM_MIPINTERVAL, 1000);                 // shows the log after every 10000 nodes
    CPXsetintparam(envCB, CPX_PARAM_THREADS, myData->nbThreads);        // number of threads
    CPXsetintparam(envCB, CPXPARAM_Conflict_Algorithm, myData->conflictAlgorithm);
    CPXsetintparam(envCB, CPXPARAM_MIP_Strategy_CallbackReducedLP, CPX_OFF);
    CPXsetintparam(envCB, CPXPARAM_Preprocessing_Linear, CPX_OFF);
    CPXsetintparam(envCB, CPXPARAM_MIP_Strategy_Search, CPX_MIPSEARCH_TRADITIONAL);
    CPXsetintparam(envCB, CPXPARAM_Preprocessing_Presolve, CPX_OFF);
    CPXsetintparam(envCB, CPXPARAM_MIP_Cuts_LocalImplied, myData->locallyValidImpliedBounds); // aggressive setting for separating local implied bound cuts [Belotti et al. 2016]

    if (myData->maxCBCuts == 0)
    {
        cout << "myData->maxCBCuts is set to 0. Cuts are just generated in root node;" << endl;
        CPXsetintparam(envCB, CPXPARAM_MIP_Limits_Nodes, 0);
    }

    string str_problem_name = "Constrained-SVM";
    CPXLPptr lpMasterProblem = CPXcreateprob(envCB, &status, str_problem_name.c_str()); // Create LP problem as a container

    // Create columns related to the z_i variables
    double *costZ = new double[myData->nbSamples];
    char **namesZ = new char *[myData->nbSamples];
    char *xctypeZ = new char[myData->nbSamples];
    double *ubZ = new double[myData->nbSamples];
    double *lbZ = new double[myData->nbSamples];
    for (i = 0; i < myData->nbSamples; i++)
    {
        costZ[i] = 1;
        xctypeZ[i] = 'B';
        namesZ[i] = new char[100];
        sprintf(namesZ[i], "_Z(%d)", i);
        lbZ[i] = 0.0;
        ubZ[i] = 1.0;
        if (!isSampleConsidered[i])
            ubZ[i] = 0.0;
    }
    status = CPXnewcols(envCB, lpMasterProblem, myData->nbSamples, costZ, lbZ, ubZ, xctypeZ, namesZ);

    // string str_model_export = "CB_HARD_IP_Master_model.lp";
    // status = CPXwriteprob(envCB, lpMasterProblem, str_model_export.c_str(), NULL); // Exporting the model
    // if (status)
    //     cout << "Failed to export ILP" << endl;

    {
        int effortlevel[1] = {CPX_MIPSTART_AUTO};
        int beg[1] = {0};
        int nzcnt = myData->nbSamples;
        int *integer_indices = new int[nzcnt];
        double *values = new double[nzcnt];
        for (i = 0; i < nzcnt; i++)
            integer_indices[i] = i;
        for (i = 0; i < nzcnt; i++)
            values[i] = 1;
        status = CPXaddmipstarts(envCB, lpMasterProblem, 1, nzcnt, beg, integer_indices, values, effortlevel, NULL);
        if (status)
        {
            cout << "status:" << status << endl;
            exit(0);
        }
        delete[] integer_indices;
        delete[] values;
    }

    // cout << "ALLOW_SAMPLING_CUTS_TO_CONSTR_SVM: " << ALLOW_SAMPLING_CUTS_TO_CONSTR_SVM << endl;
    if (ALLOW_SAMPLING_CUTS_TO_CONSTR_SVM)
    {
        int rmatbeg[] = {0};
        double rhs[] = {1.0};
        char sense[] = {'G'};
        int *rmatind = new int[myData->nbSamples];
        double *rmatval = new double[myData->nbSamples];
        // In the sampling phase, these cuts are stored in a different variable
        clock_t start_sampling_cuts = clock();
        for (auto const &CB_CUT : myData->map_sampling_CB_CUTS)
        {
            for (j = 0; j < myData->nbSamples; j++)
            {
                rmatind[j] = j;
                rmatval[j] = 0;
            }
            bool useCut = true;
            for (auto elem : CB_CUT.first)
            {
                if (!isSampleConsidered[elem])
                {
                    useCut = false;
                    break;
                }
                rmatval[elem] = 1;
            }
            if (useCut)
            {
                status = CPXaddusercuts(envCB, lpMasterProblem, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
                status = CPXaddlazyconstraints(envCB, lpMasterProblem, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
            }
        }
        myData->timeBudget -= ((double)(clock() - start_sampling_cuts) / CLOCKS_PER_SEC);
        delete[] rmatind;
        delete[] rmatval;
    }

    MYCB info;
    info.myData = myData;
    info.count = 0;
    info.startTime = clock();
    info.isSampleConsidered = this->isSampleConsidered;
    // cout << "Number of Z_i=1 in solutionHL: " << numberOfZ_1 << endl;
    // cout << "ALLOW_QC_SLAVE: " << ALLOW_QC_SLAVE << endl;
    info.solutionHLvalue = this->solutionHL->solValue;

    status = CPXsetlazyconstraintcallbackfunc(envCB, &callback_constr_svm, &info);
    status = CPXsetusercutcallbackfunc(envCB, &callback_constr_svm, &info);

    clock_start = clock();
    status = CPXmipopt(envCB, lpMasterProblem);
    clock_end = clock();
    if (status)
        cout << "Failed to optimize ILP: MASTER " << status << endl;

    status = CPXsolution(envCB, lpMasterProblem, &mySolution->solStatus, &mySolution->solValue, master_solution, NULL, NULL, NULL);
    if (status)
        cout << "Failed to obtain solution (MASTER). status: " << status << "; mySolution->solStatus: " << mySolution->solStatus << endl;

    if (mySolution->solStatus == CPXMIP_OPTIMAL || mySolution->solStatus == CPXMIP_OPTIMAL_TOL)
        mySolution->lowerBoundValue = mySolution->solValue;
    else
        status = CPXgetbestobjval(envCB, lpMasterProblem, &mySolution->lowerBoundValue);

    elapsedTime = (((double)(clock_end - clock_start)) / CLOCKS_PER_SEC);
    myData->timeBudget -= elapsedTime;

    if (myData->problemType != SAMPLING_CB_HARD_IP)
    {
        cout << "------" << endl;
        cout << "Execution data: " << endl;
        cout << "mySolution->solValue: " << mySolution->solValue << endl;
        cout << "mySolution->lowerBoundValue: " << mySolution->lowerBoundValue << endl;
        cout << "Number of callback calls (info.count): " << info.count << endl;
        cout << "Number of generated cuts (myData->map_CB_CUTS.size()): " << myData->map_CB_CUTS.size() << endl;
        cout << "Elapsed time: " << elapsedTime << endl;
        cout << "myData->timeBudget: " << myData->timeBudget << endl;
        cout << "------\n"
             << endl;
    }

    /* Fill solution structure */
    for (i = 0; i < myData->nbSamples; i++)
        mySolution->solZ[i] = (bool)master_solution[i];

    /* Free memory and close CPLEX */
    delete[] costZ;
    for (i = 0; i < myData->nbSamples; i++)
        delete[] namesZ[i];
    delete[] namesZ;
    delete[] xctypeZ;
    delete[] ubZ;
    delete[] lbZ;
    status = CPXfreeprob(envCB, &lpMasterProblem);

    status = CPXcloseCPLEX(&envCB);

    //We solve the Hard-margin loss of Brooks
    if (myData->problemType == FIRST_CB_THEN_HARD_IP)
    {
        cout << "FIRST_CB_THEN_HARD_IP" << endl;
        CPXENVptr env = NULL;
        CPXLPptr lp = NULL;
        int status = 0;
        env = CPXopenCPLEX(&status); // Open CPLEX environment
        if (this->isDisplay)
            CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_ON); // Switching ON the display
        else
            CPXsetintparam(env, CPXPARAM_ScreenOutput, CPX_OFF);
        CPXsetintparam(env, CPXPARAM_Read_DataCheck, CPX_DATACHECK_WARN);                       // Print warnings
        CPXsetintparam(env, CPX_PARAM_MIPINTERVAL, 10000);                                      // shows the log after every 10000 nodes
        CPXsetintparam(env, CPX_PARAM_THREADS, myData->nbThreads);                              // number of threads
        CPXsetintparam(env, CPXPARAM_MIP_Cuts_LocalImplied, myData->locallyValidImpliedBounds); // aggressive setting for separating local implied bound cuts [Belotti et al. 2016]

        MYCB info;
		info.myData = myData;
		info.startTime = clock();
		info.overtimeSolutions.open("overtime-solutions" + myData->outputIdentifier + ".txt");
		status = CPXsetincumbentcallbackfunc(env, callback_check_new_incumbent, &info);


        string str_problem_name = "SVM" + myData->outputIdentifier;
        lp = CPXcreateprob(env, &status, str_problem_name.c_str()); // Create LP problem as a container

        // Create columns related to the Z variables
        double *costZ = new double[myData->nbSamples];
        char *xctypeZ = new char[myData->nbSamples];
        char **namesZ = new char *[myData->nbSamples];
        double *ubZ = new double[myData->nbSamples];
        double *lbZ = new double[myData->nbSamples];
        for (i = 0; i < myData->nbSamples; i++)
        {
            costZ[i] = myData->penaltyC;
            xctypeZ[i] = 'B';
            namesZ[i] = new char[100];
            lbZ[i] = 0.0;
            sprintf(namesZ[i], "_Z(%d)", i);
            ubZ[i] = 1.0; // Only is the problem is non-convex this variable remains free if it is considered
        }
        status = CPXnewcols(env, lp, myData->nbSamples, costZ, lbZ, ubZ, xctypeZ, namesZ);

        // Create columns related to the W variables and b
        char **namesWB = new char *[myData->nbFeatures + 1];
        double *lbWB = new double[myData->nbFeatures + 1];
        double *ubWB = new double[myData->nbFeatures + 1];
        for (i = 0; i < myData->nbFeatures + 1; i++)
        {
            namesWB[i] = new char[100];
            if (i < myData->nbFeatures)
                sprintf(namesWB[i], "_W(%d)", i);
            else
                sprintf(namesWB[i], "_B");
            if (newUB_IDR[i] == newLB_IDR[i])
            {
                lbWB[i] = -CPX_INFBOUND;
                ubWB[i] = CPX_INFBOUND;
            }
            else
            {
                ubWB[i] = newUB_IDR[i];
                lbWB[i] = newLB_IDR[i];
            }
        }
        status = CPXnewcols(env, lp, myData->nbFeatures + 1, NULL, lbWB, ubWB, NULL, namesWB); // By default LB is set to 0, need to manually allow negative values with lbWB

        // Adding the quadratic terms of the objective
        double *qsepvec = new double[myData->nbSamples + myData->nbFeatures + 1];
        for (i = 0; i < myData->nbSamples + myData->nbFeatures + 1; i++)
        {
            if (i >= myData->nbSamples && i < myData->nbSamples + myData->nbFeatures)
                qsepvec[i] = 1;
            else
                qsepvec[i] = 0;
        }
        status = CPXcopyqpsep(env, lp, qsepvec);

        // Adding rows (constraints) related to the hyperplanes
        for (i = 0; i < myData->nbSamples; i++)
        {
            int *linind = new int[myData->nbFeatures + 1];
            double *linval = new double[myData->nbFeatures + 1];
            for (j = 0; j < myData->nbFeatures + 1; j++)
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

        // Use all CB cuts as lazy constraints and user cuts
        int rmatbeg[] = {0};
        double rhs[] = {1.0};
        char sense[] = {'G'};
        int *rmatind = new int[myData->nbSamples];
        double *rmatval = new double[myData->nbSamples];
        clock_t start_inserting_cuts_phase = clock();
        for (auto const &CB_CUT : myData->map_CB_CUTS)
        {
            for (j = 0; j < myData->nbSamples; j++)
            {
                rmatind[j] = j;
                rmatval[j] = 0;
            }

            for (auto elem : CB_CUT.first)
                rmatval[elem] = 1;

            status = CPXaddusercuts(env, lp, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
            status = CPXaddlazyconstraints(env, lp, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
        }
        // In the sampling phase, these cuts are stored in a different variable
        cout << "CB cuts generated in sampling phase (myData->map_sampling_CB_CUTS.size()): " << myData->map_sampling_CB_CUTS.size() << endl;
        for (auto const &CB_CUT : myData->map_sampling_CB_CUTS)
        {
            for (j = 0; j < myData->nbSamples; j++)
            {
                rmatind[j] = j;
                rmatval[j] = 0;
            }

            for (auto elem : CB_CUT.first)
                rmatval[elem] = 1;

            status = CPXaddusercuts(env, lp, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
            status = CPXaddlazyconstraints(env, lp, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
        }

        cout << "ALLOW_LB_CONSTR_SVM: " << ALLOW_LB_CONSTR_SVM << endl;
        if (ALLOW_LB_CONSTR_SVM)
        {
            // Use the LB of the separate problem as constraint
            if (mySolution->lowerBoundValue > MY_EPSILON_e3)
            {
                for (i = 0; i < myData->nbSamples; i++)
                {
                    rmatind[i] = i;
                    rmatval[i] = 1;
                }
                rhs[0] = mySolution->lowerBoundValue;
                status = CPXaddusercuts(env, lp, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
                status = CPXaddlazyconstraints(env, lp, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
            }
        }

        // Use the constraints of the Sampling approach as lazy constraints and user cuts
        cout << "LB Sampling cuts generated (myData->LB_CUTS.size()): " << myData->LB_CUTS.size() << endl;
        rmatbeg[0] = {0};
        sense[0] = {'G'};
        int LB_cuts_used = 0;
        for (i = 0; i < myData->LB_CUTS.size(); i++)
        {
            if (myData->LB_CUTS_rhs[i] < 2.0)
                continue;

            for (j = 0; j < myData->nbSamples; j++)
            {
                rmatind[j] = j;
                rmatval[j] = 0;
            }
            for (auto elem : myData->LB_CUTS[i])
                rmatval[elem] = 1;

            rhs[0] = {myData->LB_CUTS_rhs[i]};

            status = CPXaddusercuts(env, lp, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
            status = CPXaddlazyconstraints(env, lp, 1, myData->nbSamples, rhs, sense, rmatbeg, rmatind, rmatval, NULL);
            LB_cuts_used++;
        }
        myData->timeBudget -= ((double)(clock() - start_inserting_cuts_phase) / CLOCKS_PER_SEC);
        cout << "Sampling cuts used: " << LB_cuts_used << endl;
        delete[] rmatind;
        delete[] rmatval;

        // Setting initial solution
        {
            int effortlevel[1] = {CPX_MIPSTART_AUTO};
            int beg[1] = {0};
            int nzcnt = myData->nbSamples;
            int *integer_indices = new int[nzcnt];
            double *values = new double[nzcnt];
            for (i = 0; i < nzcnt; i++)
                integer_indices[i] = i;
            for (i = 0; i < myData->nbSamples; i++)
                values[i] = solutionHL->solZ[i] ? 1.0 : 0.0;
            status = CPXaddmipstarts(env, lp, 1, nzcnt, beg, integer_indices, values, effortlevel, NULL);
            if (status)
            {
                cout << "status:" << status << endl;
                exit(0);
            }
            delete[] integer_indices;
            delete[] values;
        }
        // string str_model_export = "SVM-" + myData->outputIdentifier + ".lp";
        // status = CPXwriteprob(env, lp, str_model_export.c_str(), NULL); // Exporting the model
        // if (status)
        // cout << "Failed to export ILP" << endl;
        if (myData->timeBudget > 0)
            CPXsetdblparam(env, CPX_PARAM_TILIM, myData->timeBudget);
        clock_start = clock();
        status = CPXmipopt(env, lp); // Solving the model
        if (status)
            cout << "FIRST_CB_THEN_HARD_IP. Failed to solve the model. Status code: " << status << endl;
        clock_end = clock();
        double *solution = new double[myData->nbSamples + myData->nbFeatures + 1];
        status = CPXsolution(env, lp, &mySolution->solStatus, &mySolution->solValue, solution, NULL, NULL, NULL);

        cout << "mySolution->solValue: " << mySolution->solValue << endl;

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
        for (i = 0; i < myData->nbSamples; i++)
            delete[] namesZ[i];
        delete[] namesZ;
        delete[] ubZ;
        delete[] lbZ;
        for (i = 0; i < myData->nbFeatures + 1; i++)
            delete[] namesWB[i];
        delete[] namesWB;
        delete[] ubWB;
        delete[] lbWB;
        delete[] qsepvec;

        info.overtimeSolutions.close();
        status = CPXfreeprob(env, &lp);
        status = CPXcloseCPLEX(&env);
    }

    free_global_CB_vars(myData);

    return 0;
}

static int CPXPUBLIC
callback_constr_svm(CPXCENVptr envCB,
                    void *cbdata,
                    int wherefrom,
                    void *cbhandle,
                    int *useraction_p)
{
    CPXENVptr envSlave;
    CPXLPptr nodelp, master_lp_relaxation;
    MYCBptr mycbinfo = (MYCBptr)cbhandle;
    int status = 0, i, j, nbIterations = 0, returnStatus = 0;
    bool continueSearch = true, isIntegerSolution = true;
    set<set<int>> CB_CUTS;
    set<int> lastCut;
    double elapsedTime;
    mycbinfo->count++;

    *useraction_p = CPX_CALLBACK_DEFAULT;
    status = CPXgetcallbacknodelp(envCB, cbdata, wherefrom, &master_lp_relaxation);
    nodelp = CPXcloneprob(envCB, master_lp_relaxation, &status);
    status = CPXgetcallbacknodex(envCB, cbdata, wherefrom, master_solution, 0, mycbinfo->myData->nbSamples - 1);

    envSlave = CPXopenCPLEX(&status);
    CPXsetintparam(envSlave, CPXPARAM_ScreenOutput, CPX_OFF); // Switching ON the display
    CPXsetintparam(envSlave, CPXPARAM_Conflict_Algorithm, mycbinfo->myData->conflictAlgorithm);
    CPXsetintparam(envSlave, CPX_PARAM_THREADS, mycbinfo->myData->nbThreads); // number of threads

    int rmatbeg[] = {0};
    double rhs[] = {1.0};
    char sense[] = {'G'};
    clock_t time_start, time_end;

    int cutcallbackadd_purgeable = CPX_USECUT_FORCE;
    double cutcallbackadd_rhs = 1.0;
    int cutcallbackadd_sense = 'G';

    for (i = 0; i < mycbinfo->myData->nbSamples; i++)
    {
        if (master_solution[i] > MY_EPSILON_e7 && master_solution[i] < 1)
        {
            isIntegerSolution = false;
            break;
        }
    }

    double allocatedTime = mycbinfo->myData->userInputTime * mycbinfo->myData->timeProportionCBCuts - mycbinfo->myData->time_totalSampling;
    elapsedTime = ((double)(clock() - mycbinfo->startTime) / CLOCKS_PER_SEC);
    if (elapsedTime > allocatedTime)
    {
        cout << "Time limit reached (before cuts)" << endl;
        returnStatus = 1;
        goto TERMINATE;
    }

    if (mycbinfo->myData->maxCBCuts > 0 && (mycbinfo->myData->map_CB_CUTS.size() + mycbinfo->myData->map_sampling_CB_CUTS.size()) >= mycbinfo->myData->maxCBCuts)
    {
        cout << "Number of cuts reached" << endl;
        returnStatus = 1;
        goto TERMINATE;
    }

    while (continueSearch)
    {
        if (!isIntegerSolution && mycbinfo->myData->maxCBCuts > 0 && (CB_CUTS.size() + mycbinfo->myData->map_CB_CUTS.size() + mycbinfo->myData->map_sampling_CB_CUTS.size()) >= mycbinfo->myData->maxCBCuts)
            break;

        elapsedTime = ((double)(clock() - mycbinfo->startTime) / CLOCKS_PER_SEC);
        if (!isIntegerSolution && elapsedTime > allocatedTime)
        {
            cout << "Time limit reached (generating cuts from fractional solution)" << endl;
            returnStatus = 1;
            break;
        }

        continueSearch = false;
        if (lastCut.size() > 0)
        {
            //Master
            rmatbeg[0] = {0};
            rhs[0] = {1.0};
            sense[0] = {'G'};
            for (j = 0; j < mycbinfo->myData->nbSamples; j++)
            {
                rmatindMaster[j] = j;
                rmatvalMaster[j] = 0;
            }

            for (int elem : lastCut)
                rmatvalMaster[elem] = 1;
            status = CPXaddrows(envCB, nodelp, 0, 1, mycbinfo->myData->nbSamples, rhs, sense, rmatbeg, rmatindMaster, rmatvalMaster, NULL, NULL);
            time_start = clock();
            status = CPXdualopt(envCB, nodelp);
            time_end = clock();
            mycbinfo->myData->time_totalMaster += (time_end - time_start);
            status = CPXgetx(envCB, nodelp, master_solution, 0, mycbinfo->myData->nbSamples - 1);
        }
        {
            // Slave
            string str_problem_name = "CB_Slave_SVM-HARD-MARGIN-Norm2";
            CPXLPptr lpSlaveProblem = CPXcreateprob(envSlave, &status, str_problem_name.c_str()); // Create LP problem as a container

            status = CPXnewcols(envSlave, lpSlaveProblem, mycbinfo->myData->nbFeatures + 1, costWBSlave, lbWBSlave, ubWBSlave, NULL, namesWBSlave); // By default LB is set to 0, need to manually allow negative values with lbWB

            // Adding rows (constraints) related to the hyperplanes ==> y_i(w \cdot x + b) >= 1
            std::vector<int> corresponding_positions;
            double sumZ = 0.0;
            for (i = 0; i < mycbinfo->myData->nbSamples; i++)
            {
                // Constraints to SLAVE only if Master[i] < very tiny positive value
                if (master_solution[i] < MY_EPSILON_e3 && mycbinfo->isSampleConsidered[i])
                {
                    corresponding_positions.push_back(i);
                    rmatbeg[0] = {0};
                    rhs[0] = {1.0};
                    sense[0] = {'G'};

                    for (j = 0; j < mycbinfo->myData->nbFeatures; j++)
                    {
                        rmatindSlaveHyperplane[j] = j;
                        rmatvalSlaveHyperplane[j] = mycbinfo->myData->labels[i] * mycbinfo->myData->samples[i][j];
                    }
                    rmatindSlaveHyperplane[mycbinfo->myData->nbFeatures] = mycbinfo->myData->nbFeatures;
                    rmatvalSlaveHyperplane[mycbinfo->myData->nbFeatures] = mycbinfo->myData->labels[i];
                    status = CPXaddrows(envSlave, lpSlaveProblem, 0, 1, mycbinfo->myData->nbFeatures + 1, rhs, sense, rmatbeg, rmatindSlaveHyperplane, rmatvalSlaveHyperplane, NULL, NULL);
                }
                if (mycbinfo->isSampleConsidered[i])
                    sumZ += master_solution[i];
            }

            // If CPXaddqconstr is used, we should use baropt
            // Otherwise, the problem has only linear constraints, which is solvable by lpopt
            time_start = clock();
            if (ALLOW_QC_SLAVE)
            {
                rhs[0] = {(2 * (mycbinfo->solutionHLvalue - mycbinfo->myData->penaltyC * sumZ))};
                status = CPXaddqconstr(envSlave, lpSlaveProblem, 0, mycbinfo->myData->nbFeatures, rhs[0], 'L', NULL, NULL, quadrowSlave, quadcolSlave, quadvalSlave, NULL);
                status = CPXbaropt(envSlave, lpSlaveProblem); // Solving the Slave model
            }
            else
                status = CPXlpopt(envSlave, lpSlaveProblem); // Solvingthe Slave model
            time_end = clock();
            mycbinfo->myData->time_totalSlave += (time_end - time_start);

            // string str_model_export = "CB_HARD_IP_Slave_model.lp";
            // status = CPXwriteprob(envSlave, lpSlaveProblem, str_model_export.c_str(), NULL); // Exporting the model
            // if (status)
            //     cout << "Failed to export ILP" << endl;
            //http://www-eio.upc.edu/lceio/manuals/cplex-11/html/refcppcplex/html/enumerations/IloCplex_CplexStatus.html
            if (CPXgetstat(envSlave, lpSlaveProblem) == CPX_STAT_INFEASIBLE || CPXgetstat(envSlave, lpSlaveProblem) == CPX_STAT_CONFLICT_MINIMAL)
            {
                int statusConflict = 0, confNumRows = 0;
                status = CPXrefineconflict(envSlave, lpSlaveProblem, &confNumRows, NULL);

                // string cplFile = "CB_HARD_IP_Slave_Conflicts.clp";
                // status = CPXclpwrite(envSlave, lpSlaveProblem, cplFile.c_str());

                if (confNumRows > 0)
                {
                    status = CPXgetconflict(envSlave, lpSlaveProblem, &statusConflict, rowIndConflict, rowBdStatConflict, &confNumRows, NULL, NULL, NULL);
                    if (statusConflict == CPX_STAT_CONFLICT_MINIMAL)
                    {
                        lastCut.clear();
                        for (i = 0; i < confNumRows; i++)
                        {
                            if (rowIndConflict[i] < corresponding_positions.size())
                                lastCut.insert(corresponding_positions[rowIndConflict[i]]);
                        }

                        auto ret = CB_CUTS.insert(lastCut);
                        // if ret.second is true, the element was inserted
                        if (ret.second)
                            continueSearch = true;
                    }
                }
            }
            status = CPXfreeprob(envSlave, &lpSlaveProblem);
        }
        nbIterations++;
    }

    for (auto CB_CUT : CB_CUTS)
    {
        // If the newly generated cut is already present in the pool of cuts, we don't need to add it again
        auto ret = mycbinfo->myData->map_CB_CUTS.insert(std::pair<set<int>, bool>(CB_CUT, true));
        // if ret.second is true, then the CB_CUT was successfully inserted
        if (ret.second)
        {
            for (j = 0; j < mycbinfo->myData->nbSamples; j++)
            {
                rmatindAddCut[j] = j;
                rmatvalAddCut[j] = 0;
            }
            for (int elem : CB_CUT)
                rmatvalAddCut[elem] = 1;
            status = CPXcutcallbackadd(envCB, cbdata, wherefrom, mycbinfo->myData->nbSamples, cutcallbackadd_rhs, cutcallbackadd_sense, rmatindAddCut, rmatvalAddCut, cutcallbackadd_purgeable);
            *useraction_p = CPX_CALLBACK_SET;
        }
    }

    elapsedTime = ((double)(clock() - mycbinfo->startTime) / CLOCKS_PER_SEC);
    if (elapsedTime > allocatedTime)
    {
        cout << "Time limit reached (after cuts)" << endl;
        returnStatus = 1;
        goto TERMINATE;
    }

TERMINATE:
    /* Free memory and close CPLEX */
    status = CPXfreeprob(envCB, &nodelp);
    status = CPXcloseCPLEX(&envSlave);
    return returnStatus;
}
