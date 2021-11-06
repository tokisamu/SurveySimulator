# SurveySimulator
This simulator implements a heuristic k shortest path algorithm, max flow routing algorithm,
Spider and boomerang upon the Spider. It can simulate payments in a PCN using different
algorithm.
# Usage
This is a single file prgram, any machines with C++ compiler can run it directly.
Initially, this program will simulate 200, 400, 800, 1600, 3200, 6400 payments
for each algorithm. If you want to change benchmark settings, you need to modify
the source code directly. In the main function, testSize indicates how many payments
are simulated. testMethod indicates which algorithm is used. totalTime is running time
of simulations. You can change it to change the cnumber of oncurrent payments.
