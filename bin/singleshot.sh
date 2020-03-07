#!/bin/zsh
F=$1
function die {
	echo "$1" >&2
	exit 1
}
[[ -n $F ]] || die "Usage $0 file"
[[ -r $F ]] || die "cannot read $F"
typ=1

set -x
make && 
	./CompressText "$F" -w $typ -o $F.comp && 
	./DecompressHLZ "$F.comp" -w $typ -o $F.decomp && 
	cmp $F $F.decomp && 
	rm $F.comp $F.decomp
set +x
