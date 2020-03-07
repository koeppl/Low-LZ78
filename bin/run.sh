#!/bin/zsh

indextypes=(1 3 5 7 8 9)
##! if factor = x, then 1/\alpha = 1.x where \alpha is the maximum load factor of the used Bonsai table
factors=(5 10 20 40 60) 

infiles=(/scratch/data/78/dblp.xml /scratch/data/78/english.1024MB /scratch/data/78/est.fa  /scratch/data/78/proteins)

sed -i 's@^CMAKE_BUILD_TYPE:STRING=.*@CMAKE_BUILD_TYPE:STRING=Release@' CMakeCache.txt

# one shot
# infiles=(Makefile)
# factors=(5)
# indextypes=(1)
# factor=5
# indextype=1

for tabletype in 0 1 2; do
	sed -i "s@^#define BONSAI_HASH_TABLE 0@#define BONSAI_HASH_TABLE ${tabletype}@" ../include/SLZ78/defs.h
	cat ../include/SLZ78/defs.h
	make
	for indextype in $indextypes; do 
		for infile in $infiles; do
			for factor in $factors; do 
				filename=$(basename $infile)
				comppressedFile=$(mktemp  -p /scratch/tmp --suffix .${filename}.comp) 
				uncompressedFile=$(mktemp -p /scratch/tmp --suffix .${filename}.dec )
				logCompFile=$(mktemp -p /scratch/tmp --suffix .${filename}.log.comp )
				logDecFile=$(mktemp -p /scratch/tmp --suffix .${filename}.log.dec )
				stats="file=${filename} indextype=${indextype} factor=${factor}"
				set -x
				/usr/bin/time --format='Wall Time: %e' ./CompressText ${infile} -w ${indextype} -f ${factor} -o ${comppressedFile} > "$logCompFile" 2>&1
				set +x
				echo -n "RESULT action=compression compressedsize=$(stat --format="%s" $comppressedFile) $stats "
				./readlog.sh "$logCompFile"

				set -x
				/usr/bin/time --format='Wall Time: %e' ./DecompressHLZ $comppressedFile -w $indextype -o ${uncompressedFile} > "$logDecFile"  2>&1
				set +x
				cmp --silent $uncompressedFile $infile; checkDecomp="$?"
				echo -n "RESULT action=decompression check=${checkDecomp} $stats "
				./readlog.sh "$logDecFile"
				if [[ $checkDecomp -ne 0 ]]; then
					echo "files $infile and $uncompressedFile are different. Compressed file: $comppressedFile"
				else
					rm $comppressedFile $uncompressedFile $logCompFile $logDecFile
				fi
			done #factor
		done #infile
	done #indextype
done #tabletype
