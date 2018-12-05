all: kinds solver

kinds:
	python setup-kind.py build_ext --inplace

solver:
	python setup-solver.py build_ext --inplace

clean:
	rm -rf build
	rm -f kinds.cpp
	rm -f kinds.cpython-*
	rm -f cvc4.cpp
	rm -f cvc4.cpython-*
