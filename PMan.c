#include <string.h>
#include <unistd.h>     // fork(), execvp()
#include <stdio.h>      // printf(), scanf(), setbuf(), perror()
#include <stdlib.h>     // malloc()
#include <sys/types.h>  // pid_t 
#include <sys/wait.h>   // waitpid()
#include <signal.h>     // kill(), SIGTERM, SIGKILL, SIGSTOP, SIGCONT
#include <errno.h>      // errno
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>


#define inputArraySize 512
#define MAXPROC 35


char input[inputArraySize];

//PROCESS LIST--- STORES PID, CMD, AND WHETHER OR NOT PROCESS IS STOPPED
typedef struct proc{
	pid_t pid;
	char *cmdProc;
	int isStop;
}proc;

proc proc_list[MAXPROC];

//updates background process, checks to see if child process is terminated
void update_bg_process() {
	int status;
	int id;
	while(1){
		id= waitpid(-1, &status, WNOHANG); 
		if (id==0){
			break;
		}
		if (id>0){
			int processes=0;
			//if id>0, child terminated, goes through process list and sets proc to = 0
			while(processes<MAXPROC){
				if(proc_list[processes].pid == id){
					proc_list[processes].pid=0;
					proc_list[processes].cmdProc=0;
					proc_list[processes].isStop=0;
				}
				processes++;
			}
			printf("Process %d has been terminated.\n", id);
		}
		if (id<0){
			break;
		}
	}
}

//checks to see if PID exists in process list
int checkFlag(int pid2){
	int processes=0;
	while(processes<MAXPROC){
		if(proc_list[processes].pid == pid2){
			return 0;
		}
		processes++;
	}
	return 1;
} 

//parses all user input
void parseInput(){
	pid_t pidF;
	int i;
	char * token;
	char * cmd;
	char * arg[300];
	token = strtok(input," \n\t\r\a");
	
	//if input is bgkill, kills process and then sets that process in process list to = 0
	if(strcmp(token, "bgkill")==0){
		token = strtok(NULL, " \n\t\r\a");
		if (token == NULL){
			printf("Error: No PID given\n");
		}else if(checkFlag(atoi(token))==0 && atoi(token) != 0){
			int pid_num = atoi(token);
			kill(pid_num, SIGKILL);
			printf("PID number %d killed\n", pid_num);
			int processes=0;
			while(processes< MAXPROC){
				if(proc_list[processes].pid == pid_num){
					proc_list[processes].pid=0;
					proc_list[processes].cmdProc=0;
					proc_list[processes].isStop=0;
				}
				processes++;
			}	
		}else{
			printf("PID does not exist\n");
		}
	}
	//if input is bgstart, finds process (with pid) in process list and sets isStop to =0
	else if(strcmp(input, "bgstart")==0){
		token = strtok(NULL, " \n\t\r\a");
		if (token == NULL){
			printf("Error: No PID given\n");
		}else if(checkFlag(atoi(token))==0 && atoi(token) != 0){
			int pid_num = atoi(token);
			kill(pid_num, SIGCONT);
			printf("PID number %d started\n",pid_num);
			int processes=0;
			while(processes<MAXPROC){
				if(proc_list[processes].pid == pid_num){
					proc_list[processes].isStop=0;
				}
				processes++;
			}
		}else{
			printf("PID does not exist\n");
		}
	}
	//if input is bgstop, finds process (with pid) and sets isStop to equal 1 (means it is stopped)
	else if(strcmp(input, "bgstop")==0){
		token = strtok(NULL, " \n\t\r\a");
		if (token == NULL){
			printf("Error: No PID given\n");
		}else if(checkFlag(atoi(token))==0  && atoi(token) != 0){
			int pid_num = atoi(token);
			kill(pid_num, SIGSTOP);
			printf("PID number %d stopped\n",pid_num);
			int processes=0;
			while(processes<MAXPROC){
				if(proc_list[processes].pid == pid_num){
					proc_list[processes].isStop=1;
				}
				processes++;
			}
		}else{
			printf("PID does not exist\n");
		}
	}
	else if(strcmp(token, "bg")==0){
		int i=0;
		char path[100];
		path[99]='\0';
		char * tester;
		
		token = strtok(NULL, " \n\t\r\a");
		
		//getting path --cut off './' if it exists
		int j=0;
		strncpy(path, token, sizeof(path)-1);
		if(path[0] == '.' && path[1] == '/'){
			while(j<97){
				path[j]=path[j+2];
				j++;
			}
		}
		//get current working directory and put filename on end
		char cwd[1024];
		getcwd(cwd, sizeof(cwd));
		strcat(cwd, "/");
		strcat(cwd, path);
		
		
		cmd=token;
		//making array called arg of all arguments after bg (to pass to execvp)
		while(token != NULL){
			arg[i]=token;
		 	i++;
			token = strtok(NULL, " \n\t\r\a");
		}
		arg[i]=0;

		pidF=fork(); 
		if (pidF < 0){
			printf("Error with fork\n");
			exit(1);
		}if (pidF==0){
			//IN CHILD PROCESS
			if(execvp(cmd, arg)<0){
				exit(1);
			}
		}else{
			//IN PARENT PROCESS
			//goes through process list until finding an open spot (Pid=0) then adds proc to process list
			int procIndex=0;
			while(procIndex<MAXPROC){
				if(proc_list[procIndex].pid==0){
					proc_list[procIndex].pid=pidF;
					proc_list[procIndex].isStop=0; //0=not stopped
					proc_list[procIndex].cmdProc=malloc(strlen(cwd)+1);
					strcpy(proc_list[procIndex].cmdProc, cwd);
					break;
				}
				procIndex++; 
			}

		}

	}
	//reads in 2 different files, stat and status, to get attributes we need
	else if(strcmp(input, "pstat")==0){
		token = strtok(NULL, " \n");
		if (token == NULL){
			printf("Error: No PID given\n");
		}else if(checkFlag(atoi(token))==0  && atoi(token) != 0){
			//FINDING THE PATH INCLUDING PID GIVEN
			char src[10];
			char dest[200];
			strcpy(dest, "/proc/");
			strcpy(src, "/stat");
			strcat(dest, token);
			strcat(dest, src);

			char path[100];
			int counter=1;
			
			//OPENING STAT FILE FOR READING TO GET MOST OF PSTAT ELEMENTS
			FILE *fp= fopen(dest, "r");
			if(fp) {
				while(fgets(path, sizeof(path)-1, fp) != NULL){
					char *str; 
					str = strtok(path, " ");
			
					while (str != NULL){
						if(counter==2){ //in comm
							printf("comm: %s\n", str);
						}
						if(counter==3){ //instate
							printf("state: %s\n", str);
						}
						if(counter==14){ //in utime
							int f=atoi(str);
							int final=f/sysconf(_SC_CLK_TCK);
							printf("utime: %d\n", final);
						}
						if(counter==15){ //in stime
							int f=atoi(str);
							int final=f/sysconf(_SC_CLK_TCK);
							printf("stime: %d\n", final);
						}
						if(counter==24){ //in rss
							printf("rss: %s\n", str);
						}
						str = strtok(NULL, " ");
						counter++;
					}
				
				} 
			} else {
				printf("Error opening stat file\n");
				exit(1);
			}
			fclose(fp);
		
			//FINDING PATH FROM STATUS USING PID GIVEN
			char src2[10];
			char dest2[20];
			strcpy(dest2, "/proc/");
			strcpy(src2, "/status");
			strcat(dest2, token);
			strcat(dest2, src2);
		
			char path2[100];
			int counter2=1;
			
			//READING STATUS FILE FOR VOLUNTARY/INVOLUNTARY CONTEXT SWITCHES
			FILE *fp2= fopen(dest2, "r");
			if(fp2) {
				while(fgets(path2, sizeof(path2)-1, fp2) != NULL){
					char *str2; 
					str2 = strtok(path2, " ");
					while (str2 != NULL){
						if(counter==126 || counter==127){ //in voluntary/involuntary_ctxt_switches
							printf("%s", str2);
						}
						str2 = strtok(NULL, " ");
						counter++;
					}            	            	
				} 
			} else {
				printf("Error opening status file\n");
				exit(1);
			}
			fclose(fp2);
		}else{
			printf("Error PID not found\n");
		}

	
	}
	else if(strcmp(token, "bglist")==0){
		token = strtok(NULL, " \n");
		if (token == NULL){
		int processes=0;
			//goes through process list and lists cmds where pid !=0 and process is not stopped
			while(processes<MAXPROC){
				if(proc_list[processes].pid != 0 && proc_list[processes].isStop != 1){
					printf("%d: %s\n", proc_list[processes].pid, proc_list[processes].cmdProc);
				}
				processes++;
			}
		}else{
			printf("Error: too many arguments.\n");
		}
	}else if(strcmp(input, "exit")==0){
		exit(1);
	}else{
		printf("Command not found\n");
	}

}



int main(int argc, char *argv[]){	
//allocates memory for proc_list array
	memset(proc_list, 0, sizeof(proc)*MAXPROC);
	
	while(1){
		printf("PMan: > ");
		fgets(input,inputArraySize,stdin);
		if(strcmp(input, "\n")!=0){
			parseInput();
			update_bg_process();
		}

	}
}