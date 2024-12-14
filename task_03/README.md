# task_03
Write a runsim program that controls the number of concurrently running Unix applications that it can run.

The program reads the name of a Unix command from standard input and runs it. The number of concurrently running commands must not exceed N, where N is the command line parameter when running runsim.

If you attempt to run more than N applications, give an error message and continue waiting for commands to be entered for execution. The runsim program must terminate when the end-of-file sign occurs on standard input.
## Files:
- `runsim.c` - a program source code
- `procs.cpp` - a program imitating a long-lasting process