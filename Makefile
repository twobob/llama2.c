# choose your compiler, e.g. gcc/clang
# example override to clang: make run CC=clang
CC = gcc

# the most basic way of building that is most likely to work on most systems
.PHONY: run
run: run.c
	$(CC) -O3 -o run run.c -lm

# useful for a debug build, can then e.g. analyze with valgrind, example:
# $ valgrind --leak-check=full ./run out/model.bin 1.0 3
rundebug: run.c
	$(CC) -g -o run run.c -lm

# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
# https://simonbyrne.github.io/notes/fastmath/
# -Ofast enables all -O3 optimizations. 
# Disregards strict standards compliance.
# It also enables optimizations that are not valid for all standard-compliant programs. 
# It turns on -ffast-math, -fallow-store-data-races and the Fortran-specific 
# -fstack-arrays, unless -fmax-stack-var-size is specified, and -fno-protect-parens. 
# It turns off -fsemantic-interposition.
# In our specific application this is *probably* okay to use
.PHONY: runfast
runfast: run.c
	$(CC) -Ofast -o run run.c -lm

# additionally compiles with OpenMP, allowing multithreaded runs
# make sure to also enable multiple threads when running, e.g.:
# OMP_NUM_THREADS=4 ./run out/model.bin
.PHONY: runomp
runomp: run.c
	$(CC) -Ofast -fopenmp -march=native run.c  -lm  -o run

.PHONY: win64
win64: 
	x86_64-w64-mingw32-$(CC) -Ofast -DLLAMAC_AVX2 -fno-math-errno -finline-small-functions -fexpensive-optimizations -ffinite-loops  -funroll-all-loops  -fopenmp -DCOMPILER=\"MINGW\"  -D_WIN32 -o runmingw.exe -I. -xc run.c win.c base64_utils.c save_to_file.c

.PHONY: winclang
winclang: 
	clang -march=native -DLLAMAC_AVX2 -ffinite-loops -fopenmp -DCOMPILER=\"CLANG\" -D_CRT_SECURE_NO_WARNINGS   -Ofast -fno-math-errno -D_WIN32 -o run.exe -I. -xc run.c win.c base64_utils.c save_to_file.c

.PHONY: wingcc
wingcc: 
	$(CC) -march=native -mavx2 -DLLAMAC_AVX2 -finline-small-functions -fexpensive-optimizations  -ffinite-loops -funroll-all-loops -fopenmp -DCOMPILER=\"GCC\"  -Ofast -fno-math-errno -D_WIN32 -o rungcc.exe -I. -xc run.c win.c base64_utils.c save_to_file.c


.PHONY: win64pgoInstrument
win64pgoInstrument:
	clang -march=native -ffinite-loops -fopenmp -DCOMPILER=\"CLANGpgo\" -D_CRT_SECURE_NO_WARNINGS -Ofast -fno-math-errno -D_WIN32 -fprofile-instr-generate -o run_instrumented.exe -I. run.c win_clang.c base64_utils.c save_to_file.c
	powershell -ExecutionPolicy Bypass -Command { $path = (Get-Location).Path; Write-Host 'Executing in path:' $path; Set-Location -Path $path; $profileFile = Join-Path $path 'default.profraw'; $env:LLVM_PROFILE_FILE = $profileFile; Write-Host 'Using profile file:' $profileFile; .\sample_15.ps1 1 'run_instrumented'; }




.PHONY: win64pgoBuild
win64pgoBuild:
	llvm-profdata merge -sparse default.profraw -o .\default.profdata
	
	clang -march=native -ffinite-loops -fopenmp -DCOMPILER=\"CLANGpgo\" -D_CRT_SECURE_NO_WARNINGS -Ofast -fno-math-errno -D_WIN32 -fprofile-instr-use=.\default.profdata -o runpgo.exe -I. run.c win.c base64_utils.c save_to_file.c

.PHONY: win64pgo
win64pgo:
	make win64pgoInstrument
	make win64pgoBuild


# compiles with gnu99 standard flags for amazon linux, coreos, etc. compatibility
.PHONY: rungnu
rungnu:
	$(CC) -Ofast -std=gnu11 -o run run.c -lm

.PHONY: runompgnu
runompgnu:
	$(CC) -Ofast -fopenmp -std=gnu11 run.c  -lm  -o run

.PHONY: clean
clean:
	rm -f run
	
.PHONY: winall
winall:
	make allwin

.PHONY: allwin
allwin:
	make wingcc
	make winclang
	make win64
	build_msvc.bat
