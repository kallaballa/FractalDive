#!/bin/bash
function red { echo -ne "\033[1;31m$1\033[0m"; tput sgr0; }

set -e 
#rm -r web/
mkdir -p web/bench
mkdir -p web/bench-mt
mkdir -p web/bench-simd
mkdir -p web/bench-mt-simd
mkdir -p web/dive
mkdir -p web/dive-mt
mkdir -p web/dive-simd
mkdir -p web/dive-mt-simd
mkdir -p web/dive-low
mkdir -p web/dive-mt-low
mkdir -p web/dive-simd-low
mkdir -p web/dive-mt-simd-low
mkdir -p web/dive-high
mkdir -p web/dive-mt-high
mkdir -p web/dive-simd-high
mkdir -p web/dive-mt-simd-high

echo "The content of this directory is generated by the makeweb.sh script. All changes in this directory might get lost!" > web/README
make clean;
BENCHMARK_ONLY_=1 JAVASCRIPT=1  make -j8 hardcore; cp src/bench.html src/dive.js src/dive.wasm src/formula.svg web/bench/
make clean;
BENCHMARK_ONLY_=1 JAVASCRIPT_MT=1  make -j8 hardcore; cp src/bench.html src/dive.js src/dive.worker.js src/dive.wasm src/formula.svg web/bench-mt/
make clean;
BENCHMARK_ONLY_=1 AUTOVECTOR=1 JAVASCRIPT=1  make -j8 hardcore; cp src/bench.html src/dive.js src/dive.wasm src/formula.svg web/bench-simd/
make clean; 
BENCHMARK_ONLY_=1 AUTOVECTOR=1 JAVASCRIPT_MT=1  make -j8 hardcore; cp src/bench.html src/dive.js src/dive.worker.js src/dive.wasm src/formula.svg web/bench-mt-simd/
make clean;
JAVASCRIPT=1                    make -j8 hardcore; cp src/dive.html src/dive.js src/dive.wasm src/formula.svg web/dive/
make clean; 
JAVASCRIPT_MT=1                 make -j8 hardcore; cp src/dive.html src/dive.js src/dive.wasm src/dive.worker.js src/formula.svg web/dive-mt
make clean; 
AUTOVECTOR=1 JAVASCRIPT=1       make -j8 hardcore; cp src/dive.html src/dive.js src/dive.wasm src/formula.svg web/dive-simd/
make clean; 
AUTOVECTOR=1 JAVASCRIPT_MT=1    make -j8 hardcore; cp src/dive.html src/dive.js src/dive.wasm src/dive.worker.js src/formula.svg web/dive-mt-simd

make clean;
LOWRES=1 JAVASCRIPT=1                   make -j8 hardcore; cp src/dive.js src/dive.wasm src/formula.svg web/dive-low; cp src/dive-low.html web/dive-low/dive.html
make clean;
LOWRES=1 JAVASCRIPT_MT=1                make -j8 hardcore; cp src/dive.js src/dive.wasm src/dive.worker.js src/formula.svg web/dive-mt-low; cp src/dive-low.html web/dive-mt-low/dive.html
make clean;
LOWRES=1 AUTOVECTOR=1 JAVASCRIPT=1      make -j8 hardcore; cp src/dive.js src/dive.wasm src/formula.svg web/dive-simd-low; cp src/dive-low.html web/dive-simd-low/dive.html
make clean;
LOWRES=1 AUTOVECTOR=1 JAVASCRIPT_MT=1   make -j8 hardcore; cp src/dive.js src/dive.wasm src/dive.worker.js src/formula.svg web/dive-mt-simd-low; cp src/dive-low.html web/dive-mt-simd-low/dive.html

make clean;
HIGHRES=1 JAVASCRIPT=1                    make -j8 hardcore; cp src/dive.js src/dive.wasm src/formula.svg web/dive-high; cp src/dive-high.html web/dive-high/dive.html
make clean;
HIGHRES=1 JAVASCRIPT_MT=1                 make -j8 hardcore; cp src/dive.js src/dive.wasm src/dive.worker.js src/formula.svg web/dive-mt-high; cp src/dive-high.html web/dive-mt-high/dive.html
make clean;
HIGHRES=1 AUTOVECTOR=1 JAVASCRIPT=1    make -j8 hardcore; cp src/dive.js src/dive.wasm src/formula.svg web/dive-simd-high; cp src/dive-high.html web/dive-simd-high/dive.html
make clean;
HIGHRES=1 AUTOVECTOR=1 JAVASCRIPT_MT=1 make -j8 hardcore; cp src/dive.js src/dive.wasm src/dive.worker.js src/formula.svg web/dive-mt-simd-high; cp src/dive-high.html web/dive-mt-simd-high/dive.html


cp src/wasm-detect.js web/
cp src/index.html web/
rm -f diveweb.zip
cd web/
zip -r ../diveweb.zip *

red "Success\n"

