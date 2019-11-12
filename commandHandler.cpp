/*
 * commandHandler.cpp
 *
 *  Created on: 26-Sep-2016
 *  Author    : Ravi Prakash
 *  Roll No   : 20162028
 */

#include "header.h"

/**
 * Handles commands which are not be
 * executed by execVp. These are executed
 * by system calls of C
 */
int handleInternalCommand(vector <string> & commandVector){
	string cmd = commandVector[0];
	if(cmd.compare("cd") == 0){
		string path = commandVector[1];
		executeCD(path);
	}
	else if(cmd.compare("pwd") == 0){
		printf("%s\n",pwd.c_str());
	}
	else if(cmd.compare("echo") == 0){
		printAll(commandVector);
	}
	else if(cmd.compare("history") == 0 ||
			cmd.compare("!!") == 0      ||
	        (cmd[0] == '!' && cmd[1] == '-')){
		handleHistory(commandVector);
	}
	else if(cmd.compare("export") == 0){
		handleExport(commandVector);
	}
	fflush(stdout);
	return 0;
}

/**
 * Handles external commands. Receives a vector of commands.
 * Execute each of them after fork and duping input/output
 * of each command accordingly
 */
void handleCommandVector(vector <vector <string> > & commandVector){
	int cmdCount = commandVector.size();
	int i;
	int isBG = isBackGroundRequested(commandVector[cmdCount-1]);
	pid_t firstChild = fork();

	if(firstChild == 0){
		int fd1[2];
		int fd2[2];

		for(i = 0; i<cmdCount; i++){

			i%2 == 0? pipe(fd2) : pipe(fd1);
			pid_t grandChild = fork();

			if(grandChild == 0){
				vector<string> currentCmd = commandVector[i];
				//cout<<currentCmd[0]<<endl;

				if(i == 0 && i != cmdCount - 1){
					dup2(fd2[WRITE_END], STDOUT_FILENO);
					close(fd2[WRITE_END]);
				}
				else if (i == cmdCount - 1 && cmdCount % 2 != 0){
					dup2(fd1[READ_END],STDIN_FILENO);
					close(fd1[READ_END]);
				}
				else if(i == cmdCount - 1 && cmdCount % 2 == 0){
					dup2(fd2[READ_END],STDIN_FILENO);
					close(fd2[READ_END]);

				}
				else if(i%2 != 0){
					dup2(fd2[READ_END], STDIN_FILENO);
					close(fd2[READ_END]);

					dup2(fd1[WRITE_END], STDOUT_FILENO);
					close(fd1[WRITE_END]);
				}
				else{
					dup2(fd1[READ_END], STDIN_FILENO);
					close(fd1[READ_END]);

					dup2(fd2[WRITE_END], STDOUT_FILENO);
					close(fd2[WRITE_END]);
				}

				handleRedirection(currentCmd);
				char* arg[currentCmd.size()+1];
				bzero(arg,sizeof(arg));
				populateArgs(currentCmd, arg);

				if(isCommandToImplement(arg[0])){
					handleInternalCommand(currentCmd);
				}
				else{
					execvp(arg[0], arg);
					perror("Error in grandchild");
				}
				_exit(0);
			}

			closeDescriptors(i, fd1, fd2, cmdCount);
			int childStatus;
			waitpid(grandChild, &childStatus, 0);
		}
		_exit(0);
	}

	int childStatus;

	if(!isBG){
		waitpid(firstChild, &childStatus, 0);
	}
	fflush(stderr);
	fflush(stdout);
}

/**
 * Executes the change directory command
 */
int executeCD(string path){
	char buff[200];
	int status = chdir(path.c_str());
	if(!status){
		pwd = getcwd(buff,sizeof(buff));
	}
	else{
		printf("cd: %s No such file or directory\n",path.c_str());
		return 1;
	}
	return 0;
}

/**
 * Populates the command arguments inside a
 * character array to be passed to execVp
 * system call
 */
void populateArgs(vector <string> & argVector, char* arg[]){
	int i, len = argVector.size();

	for(i=0; i<len; i++){
		arg[i] = (char*)argVector[i].c_str();
	}
	arg[i] = NULL;
}

/**
 * Closes the file descriptors of the pipes
 * based on the current value of i
 */
void closeDescriptors(int i, int fd1[], int fd2[], int cmdCount){
	if (i == 0){
		close(fd2[WRITE_END]);
	}
	else if (i == cmdCount - 1){
		if (cmdCount % 2 != 0){
			close(fd1[READ_END]);
		}else{
			close(fd2[READ_END]);
		}
	}else{
		if (i % 2 != 0){
			close(fd2[READ_END]);
			close(fd1[WRITE_END]);
		}else{
			close(fd1[READ_END]);
			close(fd2[WRITE_END]);
		}
	}
}

/**
 * Checks if a command is external or
 * internal command
 */
int isCommandToImplement(string cmd){
	return (!cmd.compare("export") ||
			!cmd.compare("echo")   ||
			!cmd.compare("history")||
			!cmd.compare("cd")     ||
			!cmd.compare("pwd"))   ||
			!cmd.compare("!!")     ||
			(cmd.length()>2 && cmd[0] == '!' && cmd[1] == '-');
}

/**
 * Prints everything for the echo
 * command
 */
void printAll(vector <string> & toPrint){
	int i, size = toPrint.size();

	for(i=1; i<size; i++){
		string temp = toPrint[i];
		temp = checkAndSetEnv(temp);
		cout<<" ";
	}
	cout<<endl;
}

/**
 * Handles history and shebang ! command
 */
void handleHistory(vector <string>& command){
	string path = getHistoryPath();
	if(!doesFileExists(path)){
		cout<<"History is empty\n";
	}
	else{
		char* line = NULL;
		FILE* fw;
		size_t len = 0;
		int bytesRead;
		int count = 1;
		fw = fopen(path.c_str(), "r");
		map <int, string> history;

		while ((bytesRead = getline(&line, &len, fw)) != -1) {
			history[count] = line;
			count++;
		}

		handleHistoryOptions(history,command);

		free(line);
		fclose(fw);
	}
}

/**
 * Handles options of the history command such
 * as !-2, !! and history
 */
void handleHistoryOptions(map <int, string>& history, vector <string> & command){
	string option;
	string cmdToken = command[0];

	if(command.size() > 1){
		option = command[1];
	}

	if(option.length() == 0 && cmdToken.compare("history") == 0){
		for(unsigned int i=1; i<=history.size(); i++){
			cout<<i<<"  "<<history[i];
		}
		printf("\n");
	}

	else if(cmdToken[0] == '!'){
		int len = history.size();
		string query;

		if(cmdToken[1] == '!'){
			query = history[len];
		}
		else{
			query = getFromHistory(history, cmdToken);
			query = query.substr(0,query.length()-1);
		}
		cout<<query<<endl;
		vector <string> cmdArr;
		vector <vector <string> > arguments;
		getCommandArr(query, cmdArr);
		int cmdCount = cmdArr.size();
		string cmd;

		for(int i =0; i<cmdCount; i++){
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
	}
}

/**
 * Handles redirection of each command by
 * setting the file descriptor to the given file
 */
void handleRedirection(vector <string>& currentCmd){
	int i, len = currentCmd.size();

	for(i=0; i<len; i++){
		string token = currentCmd[i];

		if(token[0] == '<'){
			string file;
			if(token.length() == 1){
				file = currentCmd[i+1];
				currentCmd.erase(currentCmd.begin()+i);
				currentCmd.erase(currentCmd.begin()+i+1);
			}
			else{
				token.erase(0,1);
				file = token;
				currentCmd.erase(currentCmd.begin()+i);
			}
			int fr = open(file.c_str(),'r');
			dup2(fr,STDIN_FILENO);
		}

		if(token[0] == '>'){
			string outFile;
			if(token.length() == 1){
				outFile = currentCmd[i+1];
				currentCmd.erase(currentCmd.begin()+i);
				currentCmd.erase(currentCmd.begin()+i+1);
			}
			else{
				token.erase(0,1);
				outFile = token;
				currentCmd.erase(currentCmd.begin()+i);
			}
			int fw = open(outFile.c_str(),O_CREAT | O_WRONLY,0777);
			dup2(fw, STDOUT_FILENO);
		}
	}
}

/**
 * checks the variable and set the environment
 * for the current session
 */
string checkAndSetEnv(string token){
	string modifiedToken;
	int pid = getpid();
	stringstream ss;
	int len = token.length();

	for(int i=0; i<len; i++){
		char ch = token[i];

		if(ch == '$' && i<len-1 && token[i+1] == '$'){
			//ss.put((int)pid);
			cout<<pid;
			i++;
		}
		else if(ch == '$' && i<len-1){
			stringstream ss2;
			int j;

			for(j=i+1; j<len; j++){
				char ch2 = token[j];
				if(ch2 != ' '){
					ss2.put(ch2);
				}
				else{
					break;
				}
			}

			string env = ss2.str();

			if(getenv(env.c_str()) != NULL){
				string envValue = getenv(env.c_str());
				cout<<envValue;
				i = j;
			}
			else{
				i= len-1;
				break;
			}
		}
		else{
			cout<<ch;
		}
	}
	modifiedToken = ss.str();
	return modifiedToken;
}

/**
 * Set environment variable for the export
 * command using setenv
 */
void handleExport(vector <string>& commandVector){
	int len = commandVector.size();

	for(int i=1; i<len; i++){
		string expr = commandVector[i];
		stringstream ss;
		ss.str(expr);
		string var, value;
		getline(ss, var, '=');
		getline(ss, value, '=');

		if(setenv(var.c_str(),value.c_str(),1)){
			perror("Unable to set environment variable");
		}
	}
}

/**
 * checks if the command is supposed to run in background
 * by checking presence of '&' character at the end
 */
int isBackGroundRequested(vector <string>& lastCommandVector){
	int size = lastCommandVector.size();
	string lastCommand = lastCommandVector[size-1];
	int len = lastCommand.length();

	if(lastCommand.length() == 1 && lastCommand[0] == '&'){
		lastCommandVector.pop_back();
		return 1;
	}
	else if(lastCommand.length() >= 2 &&
			lastCommand[len-1] == '&' &&
			lastCommand[len-2] != '\\'){

		lastCommand = lastCommand.substr(0, len-1);
		lastCommandVector.pop_back();
		lastCommandVector.push_back(lastCommand);
		return 1;
	}
	return 0;
}

/**
 * Gets query from a particular index of history
 * data structure
 */
string getFromHistory(map <int, string> & history, string query){
     unsigned int Size = history.size();
     int len = query.length();
     string strNum = query.substr(2,len-1);
     unsigned int num = atoi(strNum.c_str());

     if(num <= history.size()){
    	 return history[Size-num];
     }

     return "";
}
