compile:
	clang -o jamcc src/jamcc.c src/scan.c src/parsing/expression.c src/generate_llvm.c src/lib/logging.c
