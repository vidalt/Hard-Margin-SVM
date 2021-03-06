
# Support Vector Machines with the Hard-Margin Loss: Optimal Training via Combinatorial Benders' Cuts

This is the repository for [Support Vector Machines with the Hard-Margin Loss: Optimal Training via Combinatorial Benders' Cuts](https://arxiv.org/abs/2207.07690) by 
Ítalo Santana, Breno Serrano, Maximilian Schiffer and Thibaut Vidal. 

This paper is under peer-review but its preprint is available on https://arxiv.org/abs/2207.07690.

```
@misc{Santana2022,
  doi = {10.48550/ARXIV.2207.07690}, 
  url = {https://arxiv.org/abs/2207.07690},
  author = {Santana, Ítalo and Serrano, Breno and Schiffer, Maximilian and Vidal, Thibaut},
  title = {Support Vector Machines with the Hard-Margin Loss: Optimal Training via Combinatorial Benders' Cuts},
  publisher = {arXiv},
  year = {2022},
  copyright = {arXiv.org perpetual, non-exclusive license}
}
```


## Reported results

The interested reader can check all results of the paper in **Experiments/Reported-results.xlsx**.


## Requirements

The source-code is working on MacOS 11.6 using **Clang++ 11.x.x** and Ubuntu 20.x using **C++ 9.x.x**. 
For both systems, **CPLEX_Studio1210** is used in the experiments and already set in the **Makefile**. 
We believe that recent CPLEX versions should work fine too.

#### Compiling on Ubuntu
Using terminal, you just need to enter **Program/** folder and run:
`make`

To make a simple test in the code, we created a makefile target named 'test'.
Using terminal, run `make test`. The expected result concerns three stages: clean object files, compile the code, and run a test case. If the last line of the output contains the solution-cost and some related values, the generated executable is fine.

#### Compiling on MacOSX
For running on MacOSX, you must include myOS parameter as argument in the makefile, so it can compile the code using the correct CPLEX root folder. Check it below:
`make myOS=MacOSX`

To make a simple test in the code, we created a makefile target named 'test'.
Using terminal, run `make test myOS=MacOSX`. The expected result concerns three stages: clean object files, compile the code, and running an test case. If the last line of the output contains the solution-cost and some other values, the executable is fine.




#### Using different CPLEX versions in the Makefile
In case you have a different version than CPLEX_Studio1210, you may execute
`make CPLEXVERSION=CPLEX_Studio@@@` where **@@@** refers to the suffix of 'CPLEX_Studio' in the folder name. As an example, **CPLEX_Studio201** is the folder name for CPLEX 20.10, requiring `make CPLEXVERSION=CPLEX_Studio201` to correctly compile the code.
Our makefile is a friendly text file, which does not require much effort in adapting it to any CPLEX_Studio version. 

## Running the algorithm

```
# Usage example:
./svm-nonconvex datasetPath -instanceFormat {} -timeProportionCBCuts ${} -timeProportionSampling {} -maxCBCuts {} -problem {} -pen {} -timeBudget {} -seed {}

# Available parameters:

datasetPath
# Instance path

-instanceFormat {}
# Input format to read instances (defaults to 2; file format used in instances of [1]).

-timeProportionCBCuts {}
# Proportion of time used to generate CB cuts (defaults to 0.2; 20%).

-timeProportionSampling {}
# Proportion of time related to timeProportionCBCuts to generate CB cuts in the sampling phase (defaults to 0.5; 50%).

-maxCBCuts {}
# Bounds the number of CB cuts to generate (defaults to -1; limitless).

-problem {}
# Set the problem type for solving. (defaults to HARD_IP; SVM with hard-margin loss);
# Available options: SAMPLING_CB_HARD_IP for SVM with hard-margin loss with CB cuts extraction on whole model with sampling strategies, and HARD for classic SVM.

-pen {} 
# Penalty factor in the objective function, i.e., C value (defaults to 1.0).

-timeBudget {}
# Maximum of running time in seconds (defaults to -1; limitless).

-seed {}
# Input seed for the pseudorandom number generator used in sampling (defaults to  1).

-nbThreads {}
# Number of available threads for CPLEX (defaults to 1).

-samplingSize {}
# Size of each sampling iteration (defaults to min(50,floor(n/2))).

-nbSamplingIterations {}
# Number of iterations for the sampling phase (defaults to -1; limitless).

-locallyValidImpliedBounds {}
# Value for locally Valid Implied Bounds parameter in CPLEX (defaults to 3).

-normalizeData {}
# If 1, all values are normalized using (value - avg)/std. while, if 2, all values are normalized using max-min (defaults to 0; No normalization).
```

All instances used in this work were generated by Brooks [1]. 

## Executing

All commands below concern the command line.

### A simple example:
Within **Experiments/** folder, run one of the following.

Using Ubuntu, run `make test`.

Using MacOSX, run `make test myOS=MacOSX`.

### Basic script:

Check `basic.sh` within **Experiments/** folder for a working example on all instances using SAMPLING_CB_HARD_IP and HARD_IP with 600 seconds (=10 minutes) of time limit.


### Reproducing all experiments of the paper:

Run `paper.sh` within **Experiments/** folder for the script to reproduce all experiments of the paper.

## References

[1]  J.P. Brooks, Support vector machines with the ramp loss and the hard margin loss, Oper. Res. 59 (2011) 467–479. https://doi.org/10.1287/opre.1100.0854.
    