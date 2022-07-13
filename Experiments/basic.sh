#!/bin/bash

# Example for MacOS with CPLEX_Studio201
# cd ../Program && make clean && make myOS=MacOSX CPLEXVERSION=CPLEX_Studio201
# IF Unix/Linux-based systems in  CPLEX_Studio129
# cd ../Program/ && make clean && make


# Hard margin loss
arrC=(10000 1000 100 10 1)
timeBudget=600
arrinstances=()
arrinstances+=("Brooks_SVM_Data/n60d2ATj0" "Brooks_SVM_Data/n60d2ATj1" "Brooks_SVM_Data/n60d2ATj2" "Brooks_SVM_Data/n60d2ATj3" "Brooks_SVM_Data/n60d2ATj4" "Brooks_SVM_Data/n60d5ATj0" "Brooks_SVM_Data/n60d5ATj1" "Brooks_SVM_Data/n60d5ATj2" "Brooks_SVM_Data/n60d5ATj3" "Brooks_SVM_Data/n60d5ATj4" "Brooks_SVM_Data/n60d10ATj0" "Brooks_SVM_Data/n60d10ATj1" "Brooks_SVM_Data/n60d10ATj2" "Brooks_SVM_Data/n60d10ATj3" "Brooks_SVM_Data/n60d10ATj4")
arrinstances+=("Brooks_SVM_Data/n100d2ATj0" "Brooks_SVM_Data/n100d2ATj1" "Brooks_SVM_Data/n100d2ATj2" "Brooks_SVM_Data/n100d2ATj3" "Brooks_SVM_Data/n100d2ATj4" "Brooks_SVM_Data/n100d5ATj0" "Brooks_SVM_Data/n100d5ATj1" "Brooks_SVM_Data/n100d5ATj2" "Brooks_SVM_Data/n100d5ATj3" "Brooks_SVM_Data/n100d5ATj4" "Brooks_SVM_Data/n100d10ATj0" "Brooks_SVM_Data/n100d10ATj1" "Brooks_SVM_Data/n100d10ATj2" "Brooks_SVM_Data/n100d10ATj3" "Brooks_SVM_Data/n100d10ATj4")
arrinstances+=("Brooks_SVM_Data/n200d2ATj0" "Brooks_SVM_Data/n200d2ATj1" "Brooks_SVM_Data/n200d2ATj2" "Brooks_SVM_Data/n200d2ATj3" "Brooks_SVM_Data/n200d2ATj4" "Brooks_SVM_Data/n200d5ATj0" "Brooks_SVM_Data/n200d5ATj1" "Brooks_SVM_Data/n200d5ATj2" "Brooks_SVM_Data/n200d5ATj3" "Brooks_SVM_Data/n200d5ATj4" "Brooks_SVM_Data/n200d10ATj0" "Brooks_SVM_Data/n200d10ATj1" "Brooks_SVM_Data/n200d10ATj2" "Brooks_SVM_Data/n200d10ATj3" "Brooks_SVM_Data/n200d10ATj4")
arrinstances+=("Brooks_SVM_Data/n500d2ATj0" "Brooks_SVM_Data/n500d2ATj1" "Brooks_SVM_Data/n500d2ATj2" "Brooks_SVM_Data/n500d2ATj3" "Brooks_SVM_Data/n500d2ATj4" "Brooks_SVM_Data/n500d5ATj0" "Brooks_SVM_Data/n500d5ATj1" "Brooks_SVM_Data/n500d5ATj2" "Brooks_SVM_Data/n500d5ATj3" "Brooks_SVM_Data/n500d5ATj4" "Brooks_SVM_Data/n500d10ATj0" "Brooks_SVM_Data/n500d10ATj1" "Brooks_SVM_Data/n500d10ATj2" "Brooks_SVM_Data/n500d10ATj3" "Brooks_SVM_Data/n500d10ATj4")
arrinstances+=("Brooks_SVM_Data/n60d2BTj0" "Brooks_SVM_Data/n60d2BTj1" "Brooks_SVM_Data/n60d2BTj2" "Brooks_SVM_Data/n60d2BTj3" "Brooks_SVM_Data/n60d2BTj4" "Brooks_SVM_Data/n60d5BTj0" "Brooks_SVM_Data/n60d5BTj1" "Brooks_SVM_Data/n60d5BTj2" "Brooks_SVM_Data/n60d5BTj3" "Brooks_SVM_Data/n60d5BTj4" "Brooks_SVM_Data/n60d10BTj0" "Brooks_SVM_Data/n60d10BTj1" "Brooks_SVM_Data/n60d10BTj2" "Brooks_SVM_Data/n60d10BTj3" "Brooks_SVM_Data/n60d10BTj4")
arrinstances+=("Brooks_SVM_Data/n100d2BTj0" "Brooks_SVM_Data/n100d2BTj1" "Brooks_SVM_Data/n100d2BTj2" "Brooks_SVM_Data/n100d2BTj3" "Brooks_SVM_Data/n100d2BTj4" "Brooks_SVM_Data/n100d5BTj0" "Brooks_SVM_Data/n100d5BTj1" "Brooks_SVM_Data/n100d5BTj2" "Brooks_SVM_Data/n100d5BTj3" "Brooks_SVM_Data/n100d5BTj4" "Brooks_SVM_Data/n100d10BTj0" "Brooks_SVM_Data/n100d10BTj1" "Brooks_SVM_Data/n100d10BTj2" "Brooks_SVM_Data/n100d10BTj3" "Brooks_SVM_Data/n100d10BTj4")
arrinstances+=("Brooks_SVM_Data/n200d2BTj0" "Brooks_SVM_Data/n200d2BTj1" "Brooks_SVM_Data/n200d2BTj2" "Brooks_SVM_Data/n200d2BTj3" "Brooks_SVM_Data/n200d2BTj4" "Brooks_SVM_Data/n200d5BTj0" "Brooks_SVM_Data/n200d5BTj1" "Brooks_SVM_Data/n200d5BTj2" "Brooks_SVM_Data/n200d5BTj3" "Brooks_SVM_Data/n200d5BTj4" "Brooks_SVM_Data/n200d10BTj0" "Brooks_SVM_Data/n200d10BTj1" "Brooks_SVM_Data/n200d10BTj2" "Brooks_SVM_Data/n200d10BTj3" "Brooks_SVM_Data/n200d10BTj4")
arrinstances+=("Brooks_SVM_Data/n500d2BTj0" "Brooks_SVM_Data/n500d2BTj1" "Brooks_SVM_Data/n500d2BTj2" "Brooks_SVM_Data/n500d2BTj3" "Brooks_SVM_Data/n500d2BTj4" "Brooks_SVM_Data/n500d5BTj0" "Brooks_SVM_Data/n500d5BTj1" "Brooks_SVM_Data/n500d5BTj2" "Brooks_SVM_Data/n500d5BTj3" "Brooks_SVM_Data/n500d5BTj4" "Brooks_SVM_Data/n500d10BTj0" "Brooks_SVM_Data/n500d10BTj1" "Brooks_SVM_Data/n500d10BTj2" "Brooks_SVM_Data/n500d10BTj3" "Brooks_SVM_Data/n500d10BTj4")
arrinstances+=("Brooks_RealWorldData/adult5f1i1.train" "Brooks_RealWorldData/adult5f2i1.train" "Brooks_RealWorldData/adult5f3i1.train" "Brooks_RealWorldData/adult5f4i1.train" "Brooks_RealWorldData/adult5f5i1.train")
arrinstances+=("Brooks_RealWorldData/australian5f1i1.train" "Brooks_RealWorldData/australian5f2i1.train" "Brooks_RealWorldData/australian5f3i1.train" "Brooks_RealWorldData/australian5f4i1.train" "Brooks_RealWorldData/australian5f5i1.train")
arrinstances+=("Brooks_RealWorldData/breast5f1i1.train" "Brooks_RealWorldData/breast5f2i1.train" "Brooks_RealWorldData/breast5f3i1.train" "Brooks_RealWorldData/breast5f4i1.train" "Brooks_RealWorldData/breast5f5i1.train")
arrinstances+=("Brooks_RealWorldData/bupa5f1i1.train" "Brooks_RealWorldData/bupa5f2i1.train" "Brooks_RealWorldData/bupa5f3i1.train" "Brooks_RealWorldData/bupa5f4i1.train" "Brooks_RealWorldData/bupa5f5i1.train")
arrinstances+=("Brooks_RealWorldData/german5f1i1.train" "Brooks_RealWorldData/german5f2i1.train" "Brooks_RealWorldData/german5f3i1.train" "Brooks_RealWorldData/german5f4i1.train" "Brooks_RealWorldData/german5f5i1.train")
arrinstances+=("Brooks_RealWorldData/heart5f1i1.train" "Brooks_RealWorldData/heart5f2i1.train" "Brooks_RealWorldData/heart5f3i1.train" "Brooks_RealWorldData/heart5f4i1.train" "Brooks_RealWorldData/heart5f5i1.train")
arrinstances+=("Brooks_RealWorldData/ionosphere5f1i1.train" "Brooks_RealWorldData/ionosphere5f2i1.train" "Brooks_RealWorldData/ionosphere5f3i1.train" "Brooks_RealWorldData/ionosphere5f4i1.train" "Brooks_RealWorldData/ionosphere5f5i1.train")
arrinstances+=("Brooks_RealWorldData/pima5f1i1.train" "Brooks_RealWorldData/pima5f2i1.train" "Brooks_RealWorldData/pima5f3i1.train" "Brooks_RealWorldData/pima5f4i1.train" "Brooks_RealWorldData/pima5f5i1.train")
arrinstances+=("Brooks_RealWorldData/sonar5f1i1.train" "Brooks_RealWorldData/sonar5f2i1.train" "Brooks_RealWorldData/sonar5f3i1.train" "Brooks_RealWorldData/sonar5f4i1.train" "Brooks_RealWorldData/sonar5f5i1.train")
arrinstances+=("Brooks_RealWorldData/wdbc5f1i1.train" "Brooks_RealWorldData/wdbc5f2i1.train" "Brooks_RealWorldData/wdbc5f3i1.train" "Brooks_RealWorldData/wdbc5f4i1.train" "Brooks_RealWorldData/wdbc5f5i1.train")
arrinstances+=("Brooks_RealWorldData/wpbc5f1i1.train" "Brooks_RealWorldData/wpbc5f2i1.train" "Brooks_RealWorldData/wpbc5f3i1.train" "Brooks_RealWorldData/wpbc5f4i1.train" "Brooks_RealWorldData/wpbc5f5i1.train")
for penalizationC in "${arrC[@]}"
do
    echo "${penalizationC}"
    for instance in "${arrinstances[@]}"
    do
        instance_basename="${instance##*/}"
        echo $instance_basename
        
        seed=1
        
        problemType=SAMPLING_CB_HARD_IP
        timeProportionCBCuts=0.2
        timeProportionSampling=0.5
        maxCBCuts=-1
        ../Program/svm-nonconvex ../Datasets/$instance -instanceFormat 2 -problem $problemType -pen $penalizationC -timeBudget $timeBudget -seed $seed -timeProportionCBCuts ${timeProportionCBCuts} -timeProportionSampling ${timeProportionSampling} -maxCBCuts ${maxCBCuts}
        
        problemType=HARD_IP
        ../Program/svm-nonconvex ../Datasets/$instance -instanceFormat 2 -problem $problemType -pen $penalizationC -timeBudget $timeBudget -seed $seed
    done
done
