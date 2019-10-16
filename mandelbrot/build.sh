g++ -g -Wall -Werror -std=c++11 -O3 -DFP64 mandelbrot_iterator.cpp -o mdb_double.out
g++ -g -Wall -Werror -std=c++11 -O3 -DFP128 mandelbrot_iterator.cpp -o mdb_quad.out -lquadmath
