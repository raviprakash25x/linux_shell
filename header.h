/*
 * header.h
 *
 *  Created on: 26-Sep-2016
 *      Author: Ravi Prakash
 *     Roll No: 20162028
 */

#ifndef HEADER_H_
#define HEADER_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <map>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <signal.h>

#define shellPrompt "My_Shell:"
#define READ_END 0
#define WRITE_END 1

using namespace std;

int initialise(void);
void setBashPrompt(string);
void startShell(void);
string extractCommand(string);
void extractArguments(string, vector<string>&);
int isCommandToImplement(string);
void handleCommandVector(vector <vector <string> >&);
int handleInternalCommand(vector <string>&);
int handleExternalCommand(vector<string>&);
int executeCD(string);
int isValidShellCommand(string);
void getCommandArr(string, vector <string>&);
string trimSpaces(string);
void populateArgs(vector <string> &, char**);
int countOptions(string[]);
void closeDescriptors(int, int[], int[], int);
void printAll(vector <string>&);
void addToHistory(string);
int doesFileExists(string);
void handleHistory(vector <string>&);
string getHistoryPath();
void handleRedirection(vector <string>&);
string checkAndSetEnv(string);
void handleExport(vector <string> &);
int isBackGroundRequested(vector <string>&);
void my_handler(int);
void catchSignal(int);
void setPrompt();
void handleHistoryOptions(map <int, string>&, vector <string> &);
string getFromHistory(map <int, string> &, string);

extern string pwd;

#endif /* HEADER_H_ */
