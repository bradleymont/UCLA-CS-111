#!/bin/bash

# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030

# add .csv files
rm -rf lab2_add.csv
touch lab2_add.csv

rm -rf lab2_list.csv
touch lab2_list.csv

#lab2_add-1.png, lab2_add-2.png, and lab2_add-3.png
for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
do
    for t in 1, 2, 4, 8, 12
    do
        ./lab2_add --iterations=$i --threads=$t >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --yield >> lab2_add.csv
    done
done

#lab2_add-4.png and lab2_add-5.png
for i in 10, 20, 40, 80, 100, 1000, 10000, 100000
do
    for t in 1, 2, 4, 8, 12
    do
        #no synchronization
        ./lab2_add --iterations=$i --threads=$t >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --yield >> lab2_add.csv
        #mutex
        ./lab2_add --iterations=$i --threads=$t --sync=m >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --yield --sync=m >> lab2_add.csv
        #spin
        ./lab2_add --iterations=$i --threads=$t --sync=s >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --yield --sync=s >> lab2_add.csv
        #compare and swap
        ./lab2_add --iterations=$i --threads=$t --sync=c >> lab2_add.csv
        ./lab2_add --iterations=$i --threads=$t --yield --sync=c >> lab2_add.csv
    done
done

#lab2_list-1.png
for i in 10, 100, 1000, 10000, 20000
do
    ./lab2_list --iterations=$i >> lab2_list.csv
done

#lab2_list-2.png
for i in 1, 10, 100, 1000
do
    for t in 2, 4, 8, 12
    do
        #conflicts between inserts
        ./lab2_list --iterations=$i --threads=$t --yield=i >> lab2_list.csv
        #conflicts between deletes
        ./lab2_list --iterations=$i --threads=$t --yield=d >> lab2_list.csv
        #conflicts between inserts and lookups
        ./lab2_list --iterations=$i --threads=$t --yield=il >> lab2_list.csv
        #conflicts between deletes and lookups
        ./lab2_list --iterations=$i --threads=$t --yield=dl >> lab2_list.csv
    done
done

for i in 1, 2, 4, 8, 16, 32
do
    for t in 2, 4, 8, 12
    do
        #conflicts between inserts
        ./lab2_list --iterations=$i --threads=$t --yield=i >> lab2_list.csv
        #conflicts between deletes
        ./lab2_list --iterations=$i --threads=$t --yield=d >> lab2_list.csv
        #conflicts between inserts and lookups
        ./lab2_list --iterations=$i --threads=$t --yield=il >> lab2_list.csv
        #conflicts between deletes and lookups
        ./lab2_list --iterations=$i --threads=$t --yield=dl >> lab2_list.csv
    done
done

#lab2_list-3.png
for i in 1, 2, 4, 8, 16, 32
do
    for t in 2, 4, 8, 12
    do
        #mutex locks
        ./lab2_list --iterations=$i --threads=$t --yield=i --sync=m >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=d --sync=m >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=il --sync=m >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=dl --sync=m >> lab2_list.csv
        #spin locks
        ./lab2_list --iterations=$i --threads=$t --yield=i --sync=s >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=d --sync=s >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=il --sync=s >> lab2_list.csv
        ./lab2_list --iterations=$i --threads=$t --yield=dl --sync=s >> lab2_list.csv
    done
done

#lab2_list-4.png
for t in 1, 2, 4, 8, 12, 16, 24
do
    ./lab2_list --iterations=1000 --threads=$t --sync=m >> lab2_list.csv
    ./lab2_list --iterations=1000 --threads=$t --sync=s >> lab2_list.csv
done

