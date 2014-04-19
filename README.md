opto
====

First three assignments of coursera [discrete optimisation course](https://www.coursera.org/course/optimization) 
(after which I got bored and moved on).

The most interesting solution is assignment 3, the Euclidean Travelling Salemen Problem (TSP), which I solved with 
a genetic algorithm. At some point I would like to play with it more.

The solutions are written in C and launched with python using the python CFFI library. The graphing is done with
[graph-tool](http://http://graph-tool.skewed.de/).

Example usage:

```
cd tsp
make
python solver.py -g -v -s data/tsp_100_1
```

All test data etc must be downloaded from the coursera website.
