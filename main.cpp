//============================================================================
// Name        : Shell.cpp
// Author      : Ravi Prakash
// Roll No     : 20162028
//============================================================================

#include "header.h"

string pwd;

int main() {
	printf("\t\t\t\t\t\t--------------------------------\n");
	printf("\t\t\t\t\t\t|  Welcome to advanced shell ! |\n");
	printf("\t\t\t\t\t\t| =====Copyright licence %c==== |\n",169);
	printf("\t\t\t\t\t\t|       Shell v1.0.5           |\n");
	printf("\t\t\t\t\t\t--------------------------------\n");

	if(!initialise()){
		printf("Problem in init phase\n");
		exit(1);
	}
	signal(SIGINT, catchSignal);
	startShell();

	return 0;
}

/**
 * Initializes PWD to set on prompt
 */
int initialise(){
	char buff[100];
	pwd = getcwd(buff,sizeof(buff));

	if(pwd == ""){
		return 0;
	}

	return 1;
}

/*
 * starts the shell.
 * Also takes input from user and
 * iterates over the for execution.
 */
void startShell(){
	string query;
	int i;

	do{
		vector <string> cmdArr;
		string options[20];
		vector <vector <string> > arguments;
		bzero(options,sizeof(options));

		setPrompt();
		fflush(stdout);
		getline(std::cin, query);
		if(query.compare("exit") == 0)_exit(0);
		addToHistory(query);
		getCommandArr(query, cmdArr);
		int cmdCount = cmdArr.size();
		string cmd;

		for(i =0; i<cmdCount; i++){
			vector <string> currentCmd;
			cmd = extractCommand(cmdArr[i]);
			cmd = trimSpaces(cmd);
			extractArguments(cmdArr[i], currentCmd);
			arguments.push_back(currentCmd);
		}

		if(cmd.compare("cd") == 0 || cmd.compare("export") == 0){
			vector <string> internalCmd = arguments[0];
			handleInternalCommand(internalCmd);
		}
		else if(cmd.length() > 0){
			handleCommandVector(arguments);
		}
		arguments.clear();
	}while(query.compare("exit"));

}

/**
 * extracts the command from a query input
 */
string extractCommand(string query){
	string cmd;
	int firstNonNull = query.find_first_not_of(" ");
	int index = query.find(' ',firstNonNull);

	if(index < 0){
		return query;
	}

	cmd = query.substr(firstNonNull,index);
	return cmd;
}

/**
 * Extracts all the arguments and commands for a
 * given query input from the user.
 * Also tokenizes the input
 */
void extractArguments(string query, vector<string> & args){
	query = trimSpaces(query);
	int i;
	int index = query.find_first_not_of(" ");

	if(index < 0){
		return;
	}

	int quoteActive = 0;
	string current;
	int len = query.length();
	stringstream ss;

	for(i=0; i<len; i++){
		char ch = query[i];
		//string s = ss.str();
		//string temp = (const char*)ch;

		if((ch == '\"' || ch == '\'') && !quoteActive){
			quoteActive = 1;
			//current.append(temp);
			//ss << ch;
		}
		else if(ch == ' ' && quoteActive){
			//current.append(temp);
			//ss << ch;
			ss.put(' ');
		}
		else if(ch == '\"' || ch == '\''){
			if(query[i-1] == '\\'){
				//current.append(temp);
				ss << ch;
			}
			else{
				quoteActive = 0;
				//current.append(temp);
				//ss << ch;
			}
		}
		else if(ch == ' ' && !quoteActive && ss.str().length() != 0){
			current = ss.str();
			//ss >> current;
			args.push_back(current);
			current.erase();
			ss.clear();
			ss.str("");
		}
		else if(ch != ' '){
			//current.append(temp);
			ss << ch;
		}
	}

	if(ss.str().length() != 0){
		current = ss.str();
		//ss >> current;
		args.push_back(current);
	}
}

/**
 * Get command list from a user given input.
 * Separates the input with the pipe delimiter
 */
void getCommandArr(string query, vector <string> & cmdArr){
	int len = query.length();
	int i,quoteActive = 0;
	string current = "";
	stringstream ss;

	for(i=0; i<len; i++){
		char ch = query[i];

		if((ch == '\"' || ch == '\'') && !quoteActive){
			ss << ch;
			quoteActive = 1;
		}
		else if(ch == '|' && !quoteActive){
			current = ss.str();
			cmdArr.push_back(current);
			current.clear();
			ss.clear();
			ss.str("");
		}
		else if((ch == '\"' || ch == '\'') && quoteActive){
			if(query[i-1] != '\\'){
				ss << ch;
				quoteActive = 0;
			}
		}
		else{
			ss.put(ch);
		}
	}
	if(ss.str().length() != 0){
		current = ss.str();
		cmdArr.push_back(current);
	}
}

/**
 * Trims leading and trailing spaces from
 * a given string
 */
string trimSpaces(string str)
{
	size_t first = str.find_first_not_of(' ');
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last-first+1));
}

/**
 * Adds a user given input to the history.txt
 * file.
 */
void addToHistory(string query){
	if(query.length() == 0 || query[0] == '!') return;
	int fw;
	string path = getHistoryPath();
	if(!doesFileExists(path)){
		fw = open(path.c_str(),O_CREAT | O_WRONLY,0777);
	}
	else{
		fw = open(path.c_str(),O_WRONLY | O_APPEND,0777);
		write(fw,"\n",1);
	}
	write(fw,query.c_str(),query.length());
	close(fw);
}

/**
 * Checks whether a given file exists or not
 */
int doesFileExists(string name)
{
	struct stat fileInfo;
	return stat(name.c_str(), &fileInfo) == 0;
}

/**
 * Gets path of the history file
 */
string getHistoryPath(){
	char* homedir = getenv("HOME");
	string file = "/history.txt";
	string path = homedir;
	path.append(file);
	return path;
}

/**
 * catches the ctrl+C signal and redirects
 * it to the prompt
 */
void catchSignal(int sigNum){
	cout<<endl;
	setPrompt();
	fflush(stdout);
}

/**
 * sets the prompt of the shell
 * taking current PWD
 */
void setPrompt(){
	printf("%c[1;32m",27);
	cout<<shellPrompt;
	printf("%c[1;34m", 27);
	cout<<pwd;
	printf("\e[0m");
	cout<<"$ ";
}
