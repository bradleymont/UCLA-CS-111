#!/bin/bash

# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

# add .csv file
rm -rf lab2b_list.csv
touch lab2b_list.csv

#lab2b_1.png and lab2b_2.png
for t in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --iterations=1000 --threads=$t --sync=m >> lab2b_list.csv
    ./lab2_list --iterations=1000 --threads=$t --sync=s >> lab2b_list.csv
done

#lab2b_3.png
for t in 1, 4, 8, 12, 16
do
    #no synchronization
    for i in 1, 2, 4, 8, 16
    do
        ./lab2_list --iterations=$i --threads=$t --lists=4 --yield=id >> lab2b_list.csv
    done
    #with synchronization
    for i in 10, 20, 40, 80
    do
        ./lab2_list --iterations=$i --threads=$t --lists=4 --yield=id --sync=m >> lab2b_list.csv
        ./lab2_list --iterations=$i --threads=$t --lists=4 --yield=id --sync=s >> lab2b_list.csv
    done
done

#lab2b_4.png and lab2b_5.png
for l in 4, 8, 16
do
    for t in 1, 2, 4, 8, 12
    do
        ./lab2_list --iterations=1000 --threads=$t --lists=$l --sync=m >> lab2b_list.csv
        ./lab2_list --iterations=1000 --threads=$t --lists=$l --sync=s >> lab2b_list.csv
    done
done
