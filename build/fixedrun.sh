#!/bin/zsh
# run fixed already knowing the number of factors to compute in advance

indextypes=(1)
##! if factor = x, then 1/\alpha = 1.x where \alpha is the maximum load factor of the used Bonsai table
factors=(5 10 20 40 60) 

#/scratch/data/78/english.1MB 2

infiles=(
/scratch/data/78/english.1MB 158958
/scratch/data/78/english.1024MB 96986744
/scratch/data/78/est.fa 227421464
/scratch/data/78/dblp.xml 16205171
/scratch/data/78/proteins 147482019
/scratch/data/78/commoncrawl 679552342
/scratch/data/78/fibonacci 1522286
/scratch/data/78/gutenberg 63426351
/scratch/data/78/wikipedia 24211561
    )


sed -i 's@^CMAKE_BUILD_TYPE:STRING=.*@CMAKE_BUILD_TYPE:STRING=Release@' CMakeCache.txt

# one shot
# infiles=(Makefile)
# factors=(5)
# indextypes=(1)
# factor=5
# indextype=1

function runExperiment {
	filename=$(basename $_infile)
	infile=$(readlink -f "$_infile")
	comppressedFile=$(mktemp  -p /scratch/tmp --suffix .${filename}.comp) 
	uncompressedFile=$(mktemp -p /scratch/tmp --suffix .${filename}.dec )
	logCompFile=$(mktemp -p /scratch/tmp --suffix .${filename}.log.comp )
	logDecFile=$(mktemp -p /scratch/tmp --suffix .${filename}.log.dec )
	prefix=$(stat --format="%s" $infile)
	stats="file=${filename} tabletype=${tabletype} indextype=${indextype} factor=${factor} n=${prefix}"
	set -x
	/usr/bin/time --format='Wall Time: %e' ./CompressText ${infile} -g ${factor_number} -w ${indextype} -f ${factor} -o ${comppressedFile} > "$logCompFile" 2>&1
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
}

((infile_it=1))
while [[ $infile_it -lt $#infiles ]]; do
    _infile=$infiles[$infile_it]
    filename=$(basename $infile)
    factor_number=$infiles[$(expr $infile_it + 1)]
	for tabletype in 2; do
		sed -i "s@^#define BONSAI_HASH_TABLE [0-2]@#define BONSAI_HASH_TABLE ${tabletype}@" ../include/SLZ78/defs.h
		cat ../include/SLZ78/defs.h
		make
		for indextype in $indextypes; do 
			for factor in $factors; do 
				runExperiment
			done #factor
		done #indextype
	done #tabletype
	((infile_it+=2))
done #infile

# the indextypes without nested displacement array
indextypes=(0)


((infile_it=1))
while [[ $infile_it -lt $#infiles ]]; do
    _infile=$infiles[$infile_it]
    filename=$(basename $infile)
    factor_number=$infiles[$(expr $infile_it + 1)]
	tabletype='-1'
	for indextype in $indextypes; do 
		for factor in $factors; do 
			runExperiment
		done #factor
	done #infile
	((infile_it+=2))
done #indextype

# # baseline
# indextypes=(6)
#
# for _infile in $infiles; do
# 	tabletype='-1'
# 	factor=0
# 	for indextype in $indextypes; do 
# 			runExperiment
# 	done #infile
# done #indextype
