#!/bin/bash

file_to_check=$1

#ctimes=$(grep '^[0-9]' $file_to_check)

#echo $ctimes
grep '^[0-9]' $file_to_check

#ghlzS2 ghlz mhlz
#grep 'FINAL RAM' $file_to_check | awk '{print $4}'

#bhlz 
grep 'FINAL: Compression' $file_to_check | awk '{print $6}'

grep 'FINAL: Decompression' $file_to_check | awk '{print $6}'


#hlz 
#grep 'FINAL' $file_to_check | awk '{print $6}'


grep 'rcanovas' $file_to_check | awk '{print $5}' 

