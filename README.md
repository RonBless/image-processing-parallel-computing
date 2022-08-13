# image-processing-parallel-computing
Using MPI and Openmp in C in order to find if Images contains smaller images inside them.

How to run: 
1. Clone repository on at least 2 Linux machines. (for parallel-computing in case of a sequential solution 1 machine is enough)
2. Open the terminal and locate it into the debug directory
3. [ Parallel-computing step ] Edit hosts.txt to contain in the first line the ip of the machine you want to run the code from, and in the second line the ip of the 2nd     machine. (find ip using "hostname -I")
4.[ Parallel-computing step ] In the terminal type "mpiexec -np 2 -hostfile hosts ./FinalProjectRon" (you can change the number 2 to up to 4) 
  [ Sequential run step ] In the terminal type "mpiexec -np 2 ./FinalProjectRon" (you can change the number 2 to up to 4) 
