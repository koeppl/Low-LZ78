#!/bin/bash

#Compress and Decompress 10 times measuring its real time

in_file=$1
out_file=$1
type_index=$2
sigma=$3
factor=$4
d_bits=$5

case $type_index in 
	0) out_file+=".hlz78" ;;
	1) out_file+=".hlz78_hash" ;;
	2) out_file+=".mhlz78" ;;
	3) out_file+=".mhlz78_hash" ;;
	*) echo "INVALID INDEX TYPE" ;;
esac

#echo $out_file

#out_file must be equal to in_file.<type_index>

command_com="./CompressText $in_file -w $type_index -s $sigma -f $factor -d $d_bits" 

command_dec="./DecompressHLZ $out_file -w $type_index -o ./tmp_file"

#echo $command_com

repeat=1
while [ $repeat -lt 2 ]; do
	echo $repeat
	echo "Compress"
	/usr/bin/time -f "%e" $command_com 
	ls -l  $out_file
	echo "Decompress"
	/usr/bin/time -f "%e" $command_dec
	ls -l tmp_file
	echo " "
	let repeat=repeat+1
done
rm -rf tmp_file
	

