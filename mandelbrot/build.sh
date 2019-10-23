g++ -g -Wall -Werror -std=c++11 -O3 --fast-math -DFP64 mandelbrot_iterator.cpp -o mdb_double.out
g++ -g -Wall -Werror -std=c++11 -O3 --fast-math -DFP128 mandelbrot_iterator.cpp -o mdb_quad.out -lquadmath
