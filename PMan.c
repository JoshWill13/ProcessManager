/*
Josh Williams
CSC 360, Assignment 1
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <sys/wait.h>  
#include <sys/types.h>  

//		NODE STRUCTURE
typedef struct node_n{
	pid_t PID;
	char path[50];
	struct node_n* next;
	struct node_n* prev;
} node_n;

//		GLOBAL VARIABLES	//
node_n* Head = NULL;


//							LINKED_LIST_FUNCTIONS							 //
//---------------------------------------------------------------------------//

//Prints entire linked-list
void print_list(){
	if(Head == NULL){
		printf("No list to print!\n");
	}else{
		int count = 0;
		node_n* n = Head;
		while(n!= NULL){
			printf("%d:  %s\n", n->PID, n->path);
			n = n->next;
			count++;
		}
		printf("Total background jobs:  %d\n", count);
	}
}

//Add's bg process to linked-list structure
void add_to_list(pid_t pd, char str[50]){
	node_n* new  = (node_n*)malloc(sizeof(node_n));
	new->PID = pd;
	strncpy(new->path, str, 49);
	new->next = NULL;
	new->prev = NULL;
	if(Head == NULL){	
		Head = new;
	}else{
		node_n* temp = Head;
		while(temp->next != NULL){
				temp = temp->next;
		}
		temp->next = new;
		new->prev = temp;
	}
}

//Remove's bg process to linked-list structure
int remove_from_list(int pid){
		node_n* temp = Head;
		while(temp != NULL){
			int tempPID = (int) temp->PID;
			if(tempPID == pid){
				if(temp!= Head){
					temp->prev->next = temp->next;
					if(temp->next != NULL){
						temp->next->prev = temp->prev;
					}
				}else{
					if(temp->next != NULL){
						Head = temp->next;
						temp->next->prev = NULL;
					}else{
						Head = NULL;
					}
				}
				free(temp);
				return 1;
			}
			temp = temp->next;
		}
		return 0;
}
//---------------------------------------------------------------------------//

//Forks the main program to a create a child that runs a specified program
//using the execvp command.
int CreateBackgroundProcess(char bgp[]){
	pid_t childpid;
	childpid = fork();
	if(childpid>=0){
		
		if(childpid == 0){		//Child process code
			char *argv_execvp[4];
			char RunFile[20] = "./";
			strcat(RunFile, bgp);
			if(strcmp(bgp, "inf")==0){
				argv_execvp[0] = bgp;
				argv_execvp[1] = "*";
				argv_execvp[2] = "5";
				argv_execvp[3] = NULL;
			}else{
				argv_execvp[0] = bgp;
				argv_execvp[1] = NULL;
				argv_execvp[2] = NULL;
				argv_execvp[3] = NULL;
			}
			if (execvp(RunFile, argv_execvp) < 0){
				perror("Error on execvp");
				exit(1);
			}
		}else{					//Parent process code
			char* path = realpath(bgp, NULL);
			if(realpath(bgp, NULL) == NULL){//returns the full path of the file
				printf("Not a valid program input.\n");
				return 0;
			}
			char pt[50] = "";
			strncpy(pt, path, 49);
			add_to_list(childpid ,pt);
		}
		
	}
	return 0;
}

//Checks to see if a specified process is legitimate
//Returns 1 if true, 0 if false
int isProcess(int pd){
	if(Head == NULL){
		return 0;
	}else{
		node_n* n = Head;
		while(n!= NULL){
			int tempPID = (int) n->PID;
			if(tempPID == pd){
				return 1;
			}
			n = n->next;
		}
	}
	return 0;
}

//List of accepted commands
int isCommand(char *input){
	//Check if we're exiting
	if((strcmp(input, "exit")==0)){
		printf("Exiting\n");
		exit(0);
	}else if((strcmp(input, "bg")==0)){
		return 1;
	}else if((strcmp(input, "mypid")==0)){
		return 2;
	}else if((strcmp(input, "bglist")==0)){
		return 3;
	}else if((strcmp(input, "bgkill")==0)){
		return 4;
	}else if((strcmp(input, "bgstop")==0)){
		return 5;
	}else if((strcmp(input, "bgstart")==0)){
		return 6;
	}else if((strcmp(input, "pstat")==0)){
		return 7;
	}else if((strcmp(input, "head")==0)){
		return 8;
	}
	return 0;
}

//Preforms one of the three signal commands
//bgkill, bgstart, bgkill
void bgOperations(int op, char pid[20]){
	int pd = atoi(pid);
	if(pd == 0){
		printf("Incorrect process ID input\n");
		return;
	}
	if(isProcess(pd)==0){
		printf("Error: Process %d does not exist\n", pd);
		return;
	}
	
	if(op == 1){
		int killCommand = kill(pd, SIGTERM);
		if(killCommand<0){
			printf("Kill command was unsucessful\n");
		}else{
			printf("Process: %d has been killed\n", pd);
			remove_from_list(pd);
		}
	}else if(op == 2){
		int killCommand = kill(pd, SIGSTOP);
		if(killCommand<0){
			printf("Stop command was unsucessful\n");
		}else{
			printf("Process: %d has been stopped\n", pd);
		}
		
	}else if(op == 3){
		int killCommand = kill(pd, SIGCONT);
		if(killCommand<0){
			printf("Continue command was unsucessful\n");
		}else{
			printf("Process: %d has been continued\n", pd);
		}
		
	}
	
	
}

//Prints stats about a process based on it's proc file
void pstat(char pid[20]){
	int pd = atoi(pid);
	if(pd == 0){
		printf("Incorrect process ID input\n");
		return;
	}
	if(isProcess(pd)==0){
		printf("Error:  Process %d does not exist\n", pd);
		return;
	}
	
	FILE* f;
	FILE* f2;
	char statPt[500];
	char statusPt[500];
	char *comm = ""; 
	char *state = ""; 
	char *rss = "";
	char vcs[50];
	char nvcs[50];
	float utime, stime;
	sprintf(statPt, "/proc/%d/stat", pd);
	sprintf(statusPt, "/proc/%d/status", pd);
	
	//Go Through proc//status file
	f = fopen(statusPt, "r");
	char line[500] = "";
	char *tok;
	int num;
	while(fgets(line, sizeof(line), f)){
		if(strstr(line, "nonvoluntary_ctxt_switches:")!=NULL){
			tok = strtok(line, " :\n\t");
			tok = strtok(NULL, " :\n\t");
			sscanf(line, "%d", &num);
			strcpy(nvcs, tok);
		}else if(strstr(line, "voluntary_ctxt_switches:")!=NULL){
			tok = strtok(line, " :\n\t");
			tok = strtok(NULL, " :\n\t");
			sscanf(line, "%d", &num);
			strcpy(vcs, tok);
		}
	}
	fclose(f);
	
	//Go Through proc//stat file
	f2 = fopen(statPt, "r");
	char line2[500] = "";
	tok = "";
	fgets(line2, sizeof(line2), f);
	int i = 1;
	tok = strtok(line2, " ");
	while(tok != NULL){
		if(i==2){
			comm = tok;
		}else if(i==3){
			state = tok;
		}else if(i==14){
			utime = atof(tok) / (float) sysconf(_SC_CLK_TCK);
		}else if(i==15){
			stime = atof(tok) / (float) sysconf(_SC_CLK_TCK);
		}else if(i==24){
			rss = tok;
		}
		tok = strtok(NULL, " ");
		i++;
	}
	fclose(f2);
	printf("\nPSTAT DATA\n");
	printf("Comm: %s\n", comm);
	printf("State: %s\n", state);
	printf("Utime: %f\n", utime);
	printf("Stime: %f\n", stime);
	printf("RSS: %s\n", rss);
	printf("VCS: %s\n", vcs);
	printf("NVCS: %s\n", nvcs);
}

//Check to see if a program has been killed or exited
void checkProcesses(){
	int status;
	pid_t checkPID = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
	
	if(checkPID>0){
		if(WIFSIGNALED(status)){
			printf("PID:%d has been terminated by a kill signal\n", checkPID);
			remove_from_list((int) checkPID);
		}else if(WIFEXITED(status)){
			printf("PID:%d has been terminated normally\n", checkPID);
			remove_from_list((int) checkPID);
		} 
	}
}


//Grab commands from the user and pass commands to the correct functions.
//runs forever unless "exit" command is specified
int main(int argc, char* argv[]){
	while(1){
		char *input = NULL ;
		char *prompt = "PMan:> ";
		input = readline(prompt);
		checkProcesses();
		char *words;
		words = strtok(input, " ,.-");
		char* second_word;
		char second_command[20] = "";
		int i = 0;
		while(words != NULL){
			if(i == 0){
				//
			}else if(i == 1){
				second_word = words;
				stpcpy(second_command, second_word);
			}
			i++;
			words = strtok(NULL, " ,.-");
		}
		
		int command = isCommand(input);
		if(command == 1){			//bg
			CreateBackgroundProcess(second_command);
		}else if(command == 2){		//mypid
			printf("My PID is %d \n", getpid());
		}else if(command == 3){		//bglist
			print_list();
		}else if(command == 4){ 	//bgkill 	operation 1
			bgOperations(1, second_command);
		}else if(command == 5){		//bgstop 	operation 2
			bgOperations(2, second_command);
		}else if(command == 6){		//bgstart 	operation 3
			bgOperations(3, second_command);
		}else if(command == 7){		//pstat
			pstat(second_command);
		}else if(command == 8){		//check if head is empty
			if(Head == NULL){
				printf("the head is empty\n");
			}
		}else{
			printf("PMan:> %s:  command not found\n", input);
		}
	checkProcesses();	
	}
	
	return 1;
}

