#!/bin/bash
# These versions MUST match the ones mentioned in readme_k2src.txt
k2=k2pdfopt
git clone --branch 4.1.1 --depth 1 https://github.com/tesseract-ocr/tesseract
git clone --branch 1.17.0 --depth 1 https://github.com/ArtifexSoftware/mupdf
git clone --branch 1.79.0 https://github.com/DanBloomberg/leptonica/

for pkg in mupdf tesseract leptonica; do
	# Unwind pre-existing changes
	(cd $pkg && git reset --hard HEAD)

	mod=$k2/${pkg}_mod
	for n in $mod/*.[ch] $mod/*.cpp; do
		if [ ! -e "$n" ]; then
			continue
		fi
		bn=$(basename $n)
		files=( $(find $pkg -name $bn) )
		nf=${#files[@]}
		if [ $nf -gt 1 ]; then
			echo "$n => ${files} ??"
			echo "Ambiguous match, this is fatal"
			exit 1
		fi
		if [ $nf -eq 0 ]; then
			continue
		fi
		cp -vf $n ${files[0]}
	done
	if [ "$pkg" = "tesseract" ]; then
		repl='s|#include <leptonica\.h>|#include "allheaders.h"|'
		repl2='s|#include "tesseract.h"||g'
		echo "#pragma GCC visibility push(default)" >> tesseract/src/api/capi.h
		cat $mod/tesscapi.cpp tcapi.cpp | sed -e "$repl" | sed -e "$repl2" >> tesseract/src/api/capi.cpp
		cat $mod/tesseract.h tcapi.h | sed -e "$repl" | sed -e "$repl2" >> tesseract/src/api/capi.h
		echo "#pragma GCC visibility pop" >> tesseract/src/api/capi.h
	fi

	(cd $pkg && git diff) > patches/${pkg}-k2pdfopt.patch
done
