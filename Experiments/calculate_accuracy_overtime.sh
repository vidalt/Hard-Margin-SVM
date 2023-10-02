#!/bin/bash

clang++ ../Program/test_accuracy.cpp -o ../Program/test_accuracy

algorithms=(HARD_IP SAMPLING_CB_HARD_IP SOFT_HL)


for algorithm in "${algorithms[@]}"
do
    echo ${algorithm}
    instances_base=(
    "n60d2ATj0" "n60d2ATj1" "n60d2ATj2" "n60d2ATj3" "n60d2ATj4"
    "n100d2ATj0" "n100d2ATj1" "n100d2ATj2" "n100d2ATj3" "n100d2ATj4" 
    "n200d2ATj0" "n200d2ATj1" "n200d2ATj2" "n200d2ATj3" "n200d2ATj4"
    "n500d2ATj0" "n500d2ATj1" "n500d2ATj2" "n500d2ATj3" "n500d2ATj4"
    "n60d2BTj0" "n60d2BTj1" "n60d2BTj2" "n60d2BTj3" "n60d2BTj4"
    "n100d2BTj0" "n100d2BTj1" "n100d2BTj2" "n100d2BTj3" "n100d2BTj4" 
    "n200d2BTj0" "n200d2BTj1" "n200d2BTj2" "n200d2BTj3" "n200d2BTj4" 
    "n500d2BTj0" "n500d2BTj1" "n500d2BTj2" "n500d2BTj3" "n500d2BTj4")
    touch ${algorithm}_Brooks_SVM_Data_d2.csv
    for instance_base in "${instances_base[@]}"
    do
        array_instances=($(ls -d ../Experiments/overtime-solutions_${instance_base}_problemType${algorithm}*))
        for instance in "${array_instances[@]}"
        do
            ../Program/test_accuracy 2 ${instance} ../Datasets/Brooks_SVM_Data/n1e+05d2test >> ${algorithm}_Brooks_SVM_Data_d2.csv
        done
    done
    echo "1 - done"
    instances_base=(
    "n60d5ATj0" "n60d5ATj1" "n60d5ATj2" "n60d5ATj3" "n60d5ATj4"
    "n100d5ATj0" "n100d5ATj1" "n100d5ATj2" "n100d5ATj3" "n100d5ATj4" 
    "n200d5ATj0" "n200d5ATj1" "n200d5ATj2" "n200d5ATj3" "n200d5ATj4"
    "n500d5ATj0" "n500d5ATj1" "n500d5ATj2" "n500d5ATj3" "n500d5ATj4"
    "n60d5BTj0" "n60d5BTj1" "n60d5BTj2" "n60d5BTj3" "n60d5BTj4"
    "n100d5BTj0" "n100d5BTj1" "n100d5BTj2" "n100d5BTj3" "n100d5BTj4" 
    "n200d5BTj0" "n200d5BTj1" "n200d5BTj2" "n200d5BTj3" "n200d5BTj4" 
    "n500d5BTj0" "n500d5BTj1" "n500d5BTj2" "n500d5BTj3" "n500d5BTj4")
    touch ${algorithm}_Brooks_SVM_Data_d5.csv
    for instance_base in "${instances_base[@]}"
    do
        array_instances=($(ls -d ../Experiments/overtime-solutions_${instance_base}_problemType${algorithm}*))
        for instance in "${array_instances[@]}"
        do  
            instance=
            ../Program/test_accuracy 2 ${instance} ../Datasets/Brooks_SVM_Data/n1e+05d5test >> ${algorithm}_Brooks_SVM_Data_d5.csv
        done
    done
    echo "2 - done"
    instances_base=(
    "n60d10ATj0" "n60d10ATj1" "n60d10ATj2" "n60d10ATj3" "n60d10ATj4"
    "n100d10ATj0" "n100d10ATj1" "n100d10ATj2" "n100d10ATj3" "n100d10ATj4" 
    "n200d10ATj0" "n200d10ATj1" "n200d10ATj2" "n200d10ATj3" "n200d10ATj4"
    "n500d10ATj0" "n500d10ATj1" "n500d10ATj2" "n500d10ATj3" "n500d10ATj4"
    "n60d10BTj0" "n60d10BTj1" "n60d10BTj2" "n60d10BTj3" "n60d10BTj4"
    "n100d10BTj0" "n100d10BTj1" "n100d10BTj2" "n100d10BTj3" "n100d10BTj4" 
    "n200d10BTj0" "n200d10BTj1" "n200d10BTj2" "n200d10BTj3" "n200d10BTj4" 
    "n500d10BTj0" "n500d10BTj1" "n500d10BTj2" "n500d10BTj3" "n500d10BTj4")
    touch ${algorithm}_Brooks_SVM_Data_d10.csv
    for instance_base in "${instances_base[@]}"
    do
        array_instances=($(ls -d ../Experiments/overtime-solutions_${instance_base}_problemType${algorithm}*))
        for instance in "${array_instances[@]}"
        do
            ../Program/test_accuracy 2 ${instance} ../Datasets/Brooks_SVM_Data/n1e+05d10test >> ${algorithm}_Brooks_SVM_Data_d10.csv
        done
    done
    echo "3 - done"


    instances_base=()
    instances_base+=("adult5f1i1" "adult5f2i1" "adult5f3i1" "adult5f4i1" "adult5f5i1")
    instances_base+=("australian5f1i1" "australian5f2i1" "australian5f3i1" "australian5f4i1" "australian5f5i1")
    instances_base+=("breast5f1i1" "breast5f2i1" "breast5f3i1" "breast5f4i1" "breast5f5i1")
    instances_base+=("bupa5f1i1" "bupa5f2i1" "bupa5f3i1" "bupa5f4i1" "bupa5f5i1")
    instances_base+=("german5f1i1" "german5f2i1" "german5f3i1" "german5f4i1" "german5f5i1")
    instances_base+=("heart5f1i1" "heart5f2i1" "heart5f3i1" "heart5f4i1" "heart5f5i1")
    instances_base+=("ionosphere5f1i1" "ionosphere5f2i1" "ionosphere5f3i1" "ionosphere5f4i1" "ionosphere5f5i1")
    instances_base+=("pima5f1i1" "pima5f2i1" "pima5f3i1" "pima5f4i1" "pima5f5i1")
    instances_base+=("sonar5f1i1" "sonar5f2i1" "sonar5f3i1" "sonar5f4i1" "sonar5f5i1")
    instances_base+=("wdbc5f1i1" "wdbc5f2i1" "wdbc5f3i1" "wdbc5f4i1" "wdbc5f5i1")
    instances_base+=("wpbc5f1i1" "wpbc5f2i1" "wpbc5f3i1" "wpbc5f4i1" "wpbc5f5i1")
    touch ${algorithm}_Brooks_RealWorldData.csv
    for instance_base in "${instances_base[@]}"
    do
        array_instances=($(ls -d ../Experiments/results_extraction/${instance_base}.train_${algorithm}_*))        
        for instance in "${array_instances[@]}"
        do
            ../Program/test_accuracy 2 ${instance} ../Datasets/Brooks_RealWorldData/${instance_base}.test >> ${algorithm}_Brooks_RealWorldData.csv
        done
    done
    echo "4 - done"
done


