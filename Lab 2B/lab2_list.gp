#! /usr/bin/gnuplot
#
# NAME: Bradley Mont
# EMAIL: bradleymont@gmail.com
# ID: 804993030
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2b_1.png ... throughput vs. number of threads for mutex and spin-lock synchronized list operations.
#	lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations.
#	lab2b_3.png ... successful iterations vs. threads for each synchronization method.
#	lab2b_4.png ... throughput vs. number of threads for mutex synchronized partitioned lists.
#	lab2b_5.png ... throughput vs. number of threads for spin-lock-synchronized partitioned lists.
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# lab2b_1.png: throughput vs. number of threads for mutex and spin-lock synchronized list operations.
set title "Scalability-1: Throughput of Synchronized Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:32]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list ins/lookup/delete w/mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'list ins/lookup/delete w/spin' with linespoints lc rgb 'green'


#lab2b_2.png: mean time per mutex wait and mean time per operation for mutex-synchronized list operations.
set title "Scalability-2: Per-operation Times for Mutex-Protected List Operations"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:24]
set ylabel "mean time/operation (ns)"
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'completion time' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'wait for lock' with linespoints lc rgb 'red'






     
set title "List-3: Protected Iterations that run without failure"
unset logscale x
set xrange [0:5]
set xlabel "Yields"
set xtics("" 0, "yield=i" 1, "yield=d" 2, "yield=il" 3, "yield=dl" 4, "" 5)
set ylabel "successful iterations"
set logscale y 10
set output 'lab2_list-3.png'
plot \
    "< grep 'list-i-none,12,' lab2_list.csv" using (1):($3) \
	with points lc rgb "red" title "unprotected, T=12", \
    "< grep 'list-d-none,12,' lab2_list.csv" using (2):($3) \
	with points lc rgb "red" title "", \
    "< grep 'list-il-none,12,' lab2_list.csv" using (3):($3) \
	with points lc rgb "red" title "", \
    "< grep 'list-dl-none,12,' lab2_list.csv" using (4):($3) \
	with points lc rgb "red" title "", \
    "< grep 'list-i-m,12,' lab2_list.csv" using (1):($3) \
	with points lc rgb "green" title "Mutex, T=12", \
    "< grep 'list-d-m,12,' lab2_list.csv" using (2):($3) \
	with points lc rgb "green" title "", \
    "< grep 'list-il-m,12,' lab2_list.csv" using (3):($3) \
	with points lc rgb "green" title "", \
    "< grep 'list-dl-m,12,' lab2_list.csv" using (4):($3) \
	with points lc rgb "green" title "", \
    "< grep 'list-i-s,12,' lab2_list.csv" using (1):($3) \
	with points lc rgb "blue" title "Spin-Lock, T=12", \
    "< grep 'list-d-s,12,' lab2_list.csv" using (2):($3) \
	with points lc rgb "blue" title "", \
    "< grep 'list-il-s,12,' lab2_list.csv" using (3):($3) \
	with points lc rgb "blue" title "", \
    "< grep 'list-dl-s,12,' lab2_list.csv" using (4):($3) \
	with points lc rgb "blue" title ""
#
# "no valid points" is possible if even a single iteration can't run
#

# unset the kinky x axis
unset xtics
set xtics

set title "List-4: Scalability of synchronization mechanisms"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Length-adjusted cost per operation(ns)"
set logscale y
set output 'lab2_list-4.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,' lab2_list.csv" using ($2):($7)/(($3)/4) \
	title '(adjusted) list w/mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,' lab2_list.csv" using ($2):($7)/(($3)/4) \
	title '(adjusted) list w/spin-lock' with linespoints lc rgb 'green'
