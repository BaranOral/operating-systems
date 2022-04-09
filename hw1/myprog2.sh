#!/bin/bash

#print welcoming message and take first input. 
read -p	"Enter a sequence of numbers followed by 'end'. 
" input

maxNUM=0

if [ "${input,,}" != "end" ] # if first input is not end set manNUM to first input.
then 
	
	if [ ${input} -gt $maxNUM ] 
	then
		maxNUM=$input
	fi
	
	#start taking consecutive input and update maxNUM when greater number is entered. 
	read  innerInput
	while [ "${innerInput,,}" != "end" ]
	do
		if [ $innerInput -gt $maxNUM ]
		then
			maxNUM=$innerInput
		else
			read  innerInput
		fi
	done
	
	echo "Maximum: ${maxNUM}"

fi
