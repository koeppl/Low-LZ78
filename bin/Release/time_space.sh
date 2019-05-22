#!/bin/bash

#Runs the command "command_to_run" 10 times and prints all the outputs into "output_file"

output_file=$1
command_to_run=${@:2}

($command_to_run) >& $output_file
	

