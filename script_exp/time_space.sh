#!/bin/bash

#Runs the command "command_to_run" and prints all the outputs into "output_file"
#Example ./time_space.sh output_file command

output_file=$1
command_to_run=${@:2}

($command_to_run) >& $output_file
#($command_to_run) | tee $output_file
	

