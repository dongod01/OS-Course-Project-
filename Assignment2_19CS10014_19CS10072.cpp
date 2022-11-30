#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <vector>
#include <cstdlib>
#include <signal.h>
#include <termios.h> 
#include <sys/types.h>
#include <dirent.h>
#include <deque>
#include <fstream>
#include <iterator>
#include <algorithm>

#define COMMAND_MAX_LEN 1000
#define DIR_MAX_LEN 200
#define MAX 100
#define MAX_HISTORY_SIZE 10000
#define SHOW_HISTORY_SIZE 1000
using namespace std;
pid_t main_p=-1;

void multiwatch();

bool prefix_checker( const string &s, const string &t )
{
    return s.size() >= t.size() && equal( t.begin(), t.end(), s.begin() );
}

string commonPrefix(string str1, string str2)
{
    string result;
    int n1 = str1.length(), n2 = str2.length();
 
    for (int i=0, j=0; i<=n1-1&&j<=n2-1; i++,j++)
    {
        if (str1[i] != str2[j])
            break;
        result.push_back(str1[i]);
    }
 
    return (result);
}

int autocomplete(string prefix,char* complete_name,vector<string> &possible,string &lcp){

    //cout<<"\nRecd prefix "<<prefix<<endl;

    char pwd[1000]="";
    vector<string> files_in_curr_dir;
    char *ptr = getcwd(pwd, sizeof(pwd));

    
    struct dirent *entry;
    DIR *dir = opendir(ptr);
    
    if (dir == NULL) {
        return -1;
    }
    while ((entry = readdir(dir)) != NULL) {
        //cout << entry->d_name << endl;
        files_in_curr_dir.push_back((string)entry->d_name);
    }
    closedir(dir);
    
    for(auto el: files_in_curr_dir){
        //cout<<"1. "<<el<<endl;
        if (prefix_checker(el,prefix)){
            possible.push_back(el);
        }
    }
    lcp = possible[0];
    for(int j=1;j< possible.size();j++){
        lcp = commonPrefix(lcp,possible[j]);
    }
    if(possible.size()==1) lcp = prefix;
    if(prefix.size()==lcp.size()){
        int j=0;
        //complete_name = "";
        for(int chi=prefix.size();chi<possible[0].size();chi++){
            //cout<<"\nElement "<<possible[0]<<"\n";
            complete_name[j] = possible[0][chi];
            j++;
        }
        return possible[0].size() - prefix.size();
    }
    else{
        int j=0;
        //complete_name = "";
        for(int chi=prefix.size();chi<lcp.size();chi++){
            //cout<<"\nElement "<<possible[0]<<"\n";
            complete_name[j] = lcp[chi];
            j++;
        }
        return lcp.size() - prefix.size();
    }
    
    return -1;
}

int tokenizer(char **list,char *str,const char *delim){
	int i=0;
	list[i]=strtok(str,delim);
	while(list[i]!=NULL){
		i++;
		list[i]=strtok(NULL,delim);
	}
	return i;
}

void executeExternal(char *input){

	char *args[MAX];

	if(!strcmp(input,"multiwatch")){		
		multiwatch();
		exit(0);
	}
	tokenizer(args,input," \n");

	execvp(args[0],args);
	printf("Error in executing the file! \n");
	kill(getpid(),SIGTERM);

}

void multiwatch(){
	return;
}

int find_char_index(char *input,char c){
	char *temp = input;
	int loc=0;
	while(*temp!='\0'){
		if(*temp==c) return loc;
		temp++;
		loc++;
	}
	return -1;
}

void executeInputOutput(char *input){

	char *args[MAX];
	char *files[MAX];

	int i=0,in,out;

	in=find_char_index(input,'<');
	out=find_char_index(input,'>');
	
	i=tokenizer(args,input,"&<>\n");
	
	pid_t p=0;
	
	if(p==0){
		if(in==-1 && out==-1){
			if(i<2) executeExternal(args[0]);
			else printf("Error encountered! Unexpected additional parameters\n");
		}
		else if(in>0 && out==-1 && i==2){
			//Only input redirection is encountered
			if(i==2){
				tokenizer(files,args[1]," \n");
				cout<<"\nRecieved "<<files[0]<<" "<<args[1]<<endl;
				int inp_fd;
				if((inp_fd= open(files[0], O_RDONLY))<0){
					printf("Error when opening input file.\n");
					return;
				}

				//redirect STDIN to the mentioned input file
				close(0);
				dup(inp_fd);
				close(inp_fd);
			}
			else printf("Error!\n");
		}
		else if(out>0 && in==-1){
			if(i==2){
				tokenizer(files,args[1]," \n");
				
				int out_fd ;
				if((out_fd= open(files[0], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))<0){
					printf("Error when opening output file.\n");
					return;
				}

				//redirect STDOUT to the mentioned output file
				close(1);
				dup(out_fd);
				close(out_fd);
			}
		}
		else if(in>0 && out>0 && i==3){
			int inp_fd,out_fd,f;
			if(in>out){
				char * temp = args[2];
				args[2] = args[1];
				args[1] = temp;
			}
			else if(in<out){
				f=tokenizer(files,args[2]," \n");
				if(f!=1){
					printf("Error in output file\n");
					return;
				}
				out_fd = open(files[0], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

				f=tokenizer(files,args[1]," \n");
				if(f!=1){
					printf("Error in input file\n");
					return;
				}
				inp_fd = open(files[0], O_RDONLY);				
			}

			//redirect STDIN and STDOUT to mentioned files

			close(0);
			dup(inp_fd);
			close(1);
			dup(out_fd);
			close(inp_fd);
			close(out_fd);
		}
		else return;
		executeExternal(args[0]);
	}
	return;
	exit(0);
}

void executePipe(char** args, int N){
	int p[N-1][2];

	for(int i=0;i<N-1;i++){
		//create pipe
		if(pipe(p[i])<0){
			cout<<"Pipe creation failed.";
			return;
		}	
	}

	pid_t pd;
	for(int i=0;i<N;i++) { 
        if((pd=fork()) == 0) { 
        	//child process
        	if(i>0){
				close(p[i-1][1]);
        		dup2(p[i-1][0], 0);
				close(p[i-1][0]); 
        	}
        	if(i<N-1){
				close(p[i][0]);
        		dup2(p[i][1], 1);
				close(p[i][1]);	
        	}
			for(int j=0;j<N-1;j++){	
			// closes all pipes
				close(p[j][0]);
				close(p[j][1]);
			}

        	executeInputOutput(args[i]);	
            exit(0); 
        }
		else {
			//wait for a while
			usleep(10000);

			for(int j=0;j<i;j++){	// closes all pipes
				close(p[j][0]);
				close(p[j][1]);
			}
		}
    }
	while(wait(NULL)>0);
	exit(0);
}

void sighandler(int signum) {
   printf("Terminated\n>>>");
	if(signum == SIGINT and main_p !=-1)
		kill(main_p,SIGKILL);
	if(signum == SIGTSTP and main_p !=-1)
		kill(main_p,SIGCONT);
	main_p=-1;

}

void read_history(deque<string>&commands)
{
	fstream hist_file("history.txt");
	string cmd;
	
	if(!hist_file)
		return;
	
	int count = 0;
	while(getline(hist_file,cmd))
	{
		if(count==MAX_HISTORY_SIZE)
			break;
		else
		{
			commands.push_back(cmd);
			count++;
		}
	}
	hist_file.close();
} 
void add_command_to_history(deque<string>&commands,char*input_cmd)
{
	while(commands.size()>MAX_HISTORY_SIZE)
		commands.pop_front();
	commands.push_back(input_cmd);

}
void print_history(deque<string>commands)
{
	//cout<<"*4";
	for(int i = std::max(0,(int)(commands.size()-SHOW_HISTORY_SIZE)); i < (int)commands.size();i++)
	{
		cout<< i+1 <<" "<<commands[i]<<"\n";
	}

}

void write_history(deque<string> commands){
	ofstream m;
	m.open("history.txt");
	for(int i=0;i<(int)commands.size();i++)
	{
		m<<commands[i]<<"\n";
	}
	m.close();
}

int lcs(int i, int j, int count,string X,string Y)
{
 
    if (i == 0 || j == 0)
        return count;
 
    if (X[i - 1] == Y[j - 1]) {
        count = lcs(i - 1, j - 1, count + 1,X,Y);
    }
    count = std::max(count,
                std::max(lcs(i, j - 1, 0,X,Y),
                    lcs(i - 1, j, 0,X,Y)));
    return count;
}

void search_history(string s,deque<string>commands)
{	
		//cout<<"Reached str hist.\n"<<commands.size();
        int flag=0;
        int max=0;
        for(int i=commands.size()-1;i>=0;i--)
        {
			//cout<<commands[i].size()<<" "<<s.size()<<"\n";
                if(commands[i]==s)
                {
                        cout<<s<<"\n";
                        flag=1;
                        break;
                }
                else
                {
                        int cnt = lcs(s.size(),commands[i].size(),0,s,commands[i]);
                        if(cnt>max)
                                max=cnt;
                }
        }
        if(flag==0&&max>2)
        {
                for(int i=commands.size()-1;i>=0;i--)
                {
                        int cnt = lcs(s.size(),commands[i].size(),0,s,commands[i]);
                        if(cnt==max)
                                cout<<commands[i]<<"\n";
                        
                }

        }
}

void detect_exit(char * input){
	static struct termios oldt;
	tcgetattr( STDIN_FILENO, &oldt);
	static char exitstr[5] ="stop";
	if(strlen(input)==4 && strstr(input,exitstr)==input){
            printf("\n(Exiting from shell)\n");
			tcsetattr( STDIN_FILENO, TCSANOW, &oldt);	
            exit(0);
		}
}

int main(){  
    static struct termios oldt, newt;
	struct sigaction sigh;
	sigh.sa_handler = sighandler;
	sigemptyset(&sigh.sa_mask);
	sigh.sa_flags=0;
	int n  = sigaction(SIGINT,&sigh, NULL);
	int ctrl_z = sigaction(SIGTSTP, &sigh, NULL);
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;

	deque <string> command_history;
	read_history(command_history);

    newt.c_lflag &= ~(ICANON|ECHO);          
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
    

    cout<<"Hi! Welcome to this peronalized shell by Ayush and Pramit.\n\nType 'stop' to exit shell.\n";

    char pwd[DIR_MAX_LEN]="";
    
	static char ampersand[2] = "&";
	char *ptr;
    int tot=10;
	bool background_run=false;

	//signal(SIGINT, sighandler);

    while(1){
		char input[COMMAND_MAX_LEN]="";
		background_run=false;
        cout<<">>>";
		//printf("Hello");
        char *ptr = getcwd(pwd, sizeof(pwd));		

		if(ptr==NULL) {
			perror("Error in getting the current directory");
			continue;
		}

		//print present working directory in blue colour
		cout<<"\033[1;34m"<<pwd<<"\033[0m"<<"$ ";
        //cin.getline(input,COMMAND_MAX_LEN);	
		int char_cnt=0;
		char ch;
		string str,lcp;
		char complete_name[1000]="***";
		vector<string> possible;
		bool ctrlr=false,ctrlcz = false;
		while(char_cnt<1000){
			ch=getc(stdin);
			if((int)(ch)==18) {
				
				str.clear();
				ctrlr = true;
				break;
				
				
			}
			else if((int)(ch)==3 || (int)(ch)==26){
				//cout<<"Ctrl c detected\n";
				ctrlcz = true;
				str.clear();
				break;
			}
			else if (ch == '\b')
			{
				putchar('\\');
				putchar('b');
			}
			else if(ch!='\t') putchar(ch);

			if(ch!=' ' && ch!='\n' && ch!='\t') str.push_back(ch);
			
			if(ch=='\n'){
				//input[char_cnt]='\0';
				str.clear();
				break;
			} 
			if(ch==' ' && str.size()>0) str.clear();
			
			if(ch=='\t'){
				//putc('#',stdout);
				//printf("Tab entered.");
				int len=autocomplete(str,complete_name,possible,lcp);
				if(possible.size()==0){
					//do nothing
					printf("\nDoing nothing\n");
				}
				else if(possible.size()==1){
					for(int lenidx=0;lenidx<len;lenidx++) {
						putchar(complete_name[lenidx]);
						input[char_cnt] = complete_name[lenidx];
						char_cnt++;
					}
					putchar(' ');
								
				}
				else if(possible.size()>1 && str.size()<lcp.size()){
					for(int lenidx=0;lenidx<len;lenidx++) {
						putchar(complete_name[lenidx]);
						input[char_cnt] = complete_name[lenidx];
						char_cnt++;
					}
								
				}
				else if(possible.size()>0 && lcp.size()==str.size()){
					cout<<endl;
					for(int indx=0;indx<possible.size();indx++){
						cout<<indx+1<<". "<<possible[indx]<<" ";
					}
					cout<<"\n<<<"<<"\\033[1;34m"<<pwd<<"\033[0m"<<"$ ";
					printf("%s",input);
				}
				possible.clear();
			}
			if(ch!='\t' && ch!='\b') input[char_cnt]=ch;
			char_cnt++;
		}	

		if(ctrlr){
			char inp1[COMMAND_MAX_LEN]="";
			cout<<"Enter search term: ";
			char_cnt=0;
			while(1){
				ch=getc(stdin);
				if(ch!='\t') putchar(ch);
				
				if(ch=='\n'){
					inp1[char_cnt]='\0';
					break;
				} 
				inp1[char_cnt]=ch;
				char_cnt++;
			}
			string x = inp1;
			//for(int i=0;i<char_cnt;i++) x.push_back(inp1[i]);
			search_history(x,command_history);
			//cout<<"Entered something like "<<inp1<<"\n";
			continue;
		}

		if(ctrlcz){
			continue;
		}

		add_command_to_history(command_history,input);


		//check for exit condition
		static char exitstr[5] ="stop";
		if(strlen(input)==4 && strstr(input,exitstr)==input){
				printf("\n(Exiting from shell)\n");
				tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
				write_history(command_history);		
				exit(0);
			}

		static char histstr[8] ="history";
		if(strlen(input)==7 && strstr(input,histstr)==input){
			print_history(command_history);
			continue;
		}

		static char multiwatchstr[11] = "multiwatch";
		if(strlen(input)>=10 && strstr(input,multiwatchstr)==input){
			multiwatch();
			continue;
		}
		//printf("bYE\n");
		//checking & character followed by any number of spaces
		char *findamp = strstr(input,ampersand);
		if(findamp){
			char *trav = findamp+1;
			while(*trav==' ') trav+=1;
			if(*trav=='\n'){
				background_run=true;
				*(findamp+1)=*trav;
			}

		}

		//for the pipe commands' arguments
		char *args[100];
		int i=0;

		args[i]=strtok(input,"|");
	
		while(args[i]!=NULL){
			i++;
			args[i]=strtok(NULL,"|");
		}
		pid_t pid, wpid;
    	int status;
		pid = fork();

		if(pid < 0){
			printf("\nFailed to create process\n");
		}
		else if(pid==0){
			main_p=getpid();
			executePipe(args, i);
			
		}
		else {
			//waiting for child process to exit if backflag is 0	
				
				//while(waitpid(-1,NULL,0)>0);
				do{
					
                	wpid = waitpid(pid,&status,WUNTRACED);
            	}while(!WIFEXITED(status) && !WIFSIGNALED(status));
			if(!background_run){
				
			}
		}
		printf("%d \n",getpid());
		
	}

	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
	return 0;
}

