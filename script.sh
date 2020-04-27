#!/bin/sh
#./main < OS_PJ1_Test/FIFO_1.txt > test.txt

for i in FIFO PSJF RR SJF 
do
	for j in 1 2 3 4 5
	do
	  ./main < OS_PJ1_Test/${i}_${j}.txt > output/${i}_${j}_stdout.txt
	  dmesg | grep Project1 > output/${i}_${j}_dmesg.txt
	  dmesg -c
	done
done