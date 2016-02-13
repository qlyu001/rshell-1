#ifndef SHELL_H
#define SHELL_H

//std included
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <utility>
//syscalls
#include <unistd.h> //execvp fork cd ..
#include <sys/wait.h> //waitpid
#include <errno.h>
//files included
#include "parse.h"
#include "commandnode.h"
using namespace std;

class Shell
{
	private:
		bool status;		//check status of whether shell cont. running or not
  
	public:
		Shell(){};			//default constructor
		
		bool getStatus()	//get current status of shell
		{ return status; }

		void start()		//start shell program
		{ status = true; }

		void finish()		//shell is done
		{ status = false; }

		pair<bool,bool> execute(const CommandNode & cn)
		{
			//initialize variables and values
			bool returnStatus = false;
			bool child = false;
			string exe = cn.getCommand().getExec();

			//cout << "exe: " << exe << endl; 
			//cout << cn << endl;

			//Check to see if command is cd
			if(exe == "cd")
			{
				//cout << "cd might happend:" << endl;
				int argSize = cn.getCommand().getArgList().size();
				
				if(argSize > 0)
				{
					if(cn.getCommand().getArgListStr() == "~")
					{ chdir(getenv("HOME")); }
					else
					{ chdir(cn.getCommand().getArgList_c_str()); }
					//cout << "error:" << errno << endl;
				}
				else
				{
					//cd to home directory
					chdir(getenv("HOME"));
					//cout << "error:" << errno << endl;
				}
				
				if(errno)
				{
					cout << "rshell: " << exe << ": " +  cn.getCommand().getArgListStr() << ": " << strerror(errno) << endl;
					child = false;
					return make_pair(false,child);
				}
				else
				{
					child = false;
					return make_pair(true,child);
				}
			}
			else
			{
				pid_t pid = fork();

				if(pid > 0)
				{
					//I am the parent process
					//add later check for background check
					int status = errno;
					waitpid(pid,&status,0);
					//cout << "Parent end" << endl;
					if(errno)
					{
						returnStatus = false;
						child = false;
					}
					else
					{
						returnStatus = true;
						child = false;
					}
				}
				else if(pid == 0)
				{
					// I am the child process
					//cout << "child start" << endl;
					//execute command
					//What about ~ in command?
					string command = exe;
					//construct arguement list c string array
					//int size = 32;
					char *arguementList[1024];		//standard amount
					memset(arguementList,0,sizeof(arguementList));

					//add command 
					arguementList[0] =  const_cast<char*>(command.c_str());

					for(unsigned int i = 0; i < cn.getCommand().getArgList().size(); i++)
					{
						//add c string to arguement list
						//need to use const cast to remove error: invalid conversion from ‘const char*’ to ‘char*’ [-fpermissive] error
						arguementList[i+1] = const_cast<char*>(cn.getCommand().getArgList().at(i).c_str());
						//cout << arguementList[i] << endl;
						arguementList[i+2] = 0;
					}

					//cout << command.c_str() << " ";
					//for(unsigned int i = 0; i <  cn.getCommand().getArgList().size() && i < 33 ; i++)
					//{ cout << arguementList[i] << " "; }
					//cout << endl;
					//cout << "execvp" << endl;

					int status = 0; 
					status = execvp(command.c_str(),arguementList);

					//cout << "status::"<< status << endl;
					//cout << "execvp end" << endl;
					if(status >= 0)
					{
						returnStatus = true;
						child = true;
					}
					else
					{
						cout << "rshell: " << exe << ": " +  cn.getCommand().getArgListStr() << ": " << strerror(errno) << endl;
						returnStatus = false;
						child = true;
					}
				}
				else
				{
					//an error occurred
					cerr << "Fork failed" << endl;
					returnStatus = false;
					child = true;
				}
			}
			//cout << "child end" << endl;
			//check status
			return make_pair(returnStatus,child);
		}

		//start shell
		void initialize()	
		{
			start();	
			bool child = false;

			//display current logged in user & hostname then start shell program
			while(status && !child)		
			{
				string input; 

				//Output prompt
				char username[1024] ={0};
				int getUsername_resp = getlogin_r(username,sizeof(username)-1);

				/*
				//printf("%s\n",username);
				char* home = getlogin();
				if(home != NULL)
				printf("%s\n",home);
				*/
				char hostname [1024] = {0};
				int getHostname_resp = gethostname(hostname,sizeof(hostname)-1);
				//cout << getUsername_resp << " " << getHostname_resp<< endl;

				if((getHostname_resp == 0) && (getUsername_resp == 0))
				{
					//string username_str = "";
					//string hostname_str = "";
					//cout << username_str <<"@"<<hostname_str;
					printf("%s@%s", username,hostname);
				}
				cout <<"$ ";	//start getting inputs for shell program

				getline(cin, input);	//get line input

				//Parse input
				Parse p;				//parse object
				input = p.ltrimr(input," ");
				//cout << input << endl;
				
				if(input != "exit")
				{
					//Split inputs on # and ignore everything afterwards
					string commands = p.split(input,"#")[0];
					
					//Split on ';'
					vector<string> commandParts = p.split(commands,";");
					
					//Now go through command parts and create List of commandNode
					string command = "";
					vector<CommandNode> commandList;

					for(unsigned int i = 0; i < commandParts.size(); i++)
					{
						//cout << commandParts[i] << endl;
						//Parse now on || and &&
						command = commandParts[i];
						string tmp = "";
						int orIndex = command.find("||");
						int andIndex = command.find("&&");
						string connector ="";

						while( (int(orIndex) != int(string::npos)) or (int(andIndex) != int(string::npos)) )
						{
							//cout << command << endl;
							int first = 0;

							if( (int(orIndex) != int(string::npos)) && (int(andIndex) != int(string::npos)) )
							{
								//first = (orIndex < andIndex) ? orIndex : andIndex;
								if(orIndex < andIndex)
								{
									first = orIndex;
									connector = "||";
								}
								else if(orIndex > andIndex)
								{
									first = andIndex;
									connector = "&&";
								}
		            		}
							else if( (int(orIndex) != int(string::npos)) && (int(andIndex) == int(string::npos)) )
							{
								first = orIndex;
								connector = "||";
							}
							else if ( (int(orIndex) == int(string::npos)) && (int(andIndex) != int(string::npos)) )
							{
								first = andIndex;
								connector = "&&";
							}
							
							tmp = command.substr(0,first);
							commandList.push_back(CommandNode(tmp,connector));
							command = command.substr(first+connector.length());
							orIndex = command.find("||");
							andIndex = command.find("&&");
						}

						if(command.length() > 0)
						{ commandList.push_back(CommandNode(command,";")); }
					}
					//cout << commandList.size() << endl;
					//Now for through each command in commandList and exectue them if possible
					//https://en.wikipedia.org/wiki/Bash_(Unix_shell)

					bool chainStatus = true;
					Connector preCon;
					bool currentReturnStatus = false;

					for(unsigned int i = 0; i < commandList.size(); i++)
					{
						//Execute command node
						CommandNode currentNode = commandList[i];
						Connector con = currentNode.getConnector();
						bool run = false;

						//skip logic
						if(chainStatus)
						{ run = true; }
						else
						{
							if(con.isOR())
							{
								//cout << "has ||" << endl;
								run = true;
								chainStatus = true;
							}
							else if(con.isAND())
							{
								//cout << "has &&" << endl;
								run = false;
							}
						}
						
						if(con.isSEMI())
						{
							//cout << "has ;" << endl;
							chainStatus = true;
						}

						if(run)
						{
							pair<bool,bool> ret;
							ret = execute(commandList[i]);
							currentReturnStatus = ret.first;
							child = ret.second;

							//assign new status
							chainStatus = currentReturnStatus && chainStatus;
						}
						else
						{ /*cout << "no run" << endl;*/ }

						//assign new status
						//chainStatus = currentReturnStatus && chainStatus;
						preCon = con;

						if(child)
						{ break; }
					}
				}
				else
				{ break; }	//if user input is exit then break loop and exit
			}
			finish();
		}
};

#endif