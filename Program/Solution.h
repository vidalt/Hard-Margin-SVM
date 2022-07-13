#ifndef SOLUTION_H
#define SOLUTION_H

#include "Pb_Data.h"

class Solution
{

private:
    // data for which this solution was built
    Pb_Data *myData;

public:
    // solution status (from CPLEX)
    int solStatus;

    // solution value (from CPLEX)
    double solValue;

    // lower bound value (from CPLEX)
    double lowerBoundValue;

    // is solution computed ?
    bool isSolComputed;

    vector<double> solW; // Vector of size nbFeatures, defines the coordinates of the classifier
    double solB;         // Defines the intercept of the classifier
    vector<bool> solZ;   // Vector of size nbSamples, valued to true if sample i is misclassified
    vector<double> solE; // Continuous loss variables

    // Checks whether a sample is correctly classified (TRUE or FALSE), and return the distance
    bool checkClassified(int i, double &distance);

    /* METHODS TO TEST AND EXPORT THE FINAL SOLUTION */

    // Method to display a solution
    void displaySolution();

    // Method to test a solution
    void exportSolution();

    // Calculate and return duality gap
    double getDualityGap();

    // Constructor
    Solution(Pb_Data *myData);

    // Method to copy (This can be improved making more general by static function)
    void copyToMe(Solution *&sol);

    // Destructor
    ~Solution(void);
};

#endif
