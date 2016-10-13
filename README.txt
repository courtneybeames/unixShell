README:

I have a makefile in the directory
type 'make' to compile
type ./PMan to run

#define inputArraySize 512
#define MAXPROC 35

largest input file can take is 512 

largest amount of arguments are 300 (just made up a random number) 
largest current working directory array is 1024

compile program with makefile provided

run with ./PMan

if type bg: 
begins background process. 
next input is file to be ran and arguments are typed after.

if type bglist:
will list all running background processes
lists pid and then path of process

if type bgkill:
will kill all processes with pid (pid) -- will take out of bglist

if type bgstop pid:
will stop the process with pid (pid) -- will take out of bglist

if type bgstart pid:
will start process with pid (pid) -- will put back in bglist

if type pstat pid:
will display all statistics necessary

if type exit:
exits program

if type command that is not recognized:
will say command not found

///Error handling:
bgstop, bgstart, and bgkill all check to see if there is a PID given and if it is valid
pstat checks to see if there is a valid PID given
bglist checks to see if there is only 1 argument (itself)

