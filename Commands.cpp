#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"


/*-----------------CommandParser-------------------*/


CommandParser::CommandParser(string input) : raw_command(input), first_command(""), second_command(""), redirection(NONE), timeout(0)
{
    int first_whitespace_index = input.find_first_not_of(WHITESPACE);
    int last_whitespace_index = input.find_last_not_of(WHITESPACE);
    this->is_background = input.back() == '&';
    this->is_complex = (input.find_first_of(COMPLEX_CHAR) != string::npos);

    if (this->is_background && last_whitespace_index != string::npos)
    {
        input = input.substr(first_whitespace_index, last_whitespace_index - first_whitespace_index);
    }

    this->stripped_flagless_command = input;
    
    int start_index = 0;
    int end_index = 0;
    int counter = 0;
    input.push_back(' '); //for better automation

    for (int i = 0; i < input.length(); ++i)
    {
        if (input[i] == ' ')
        {
            end_index = i;
            if (i != 0)
            {
                string word = input.substr(start_index, end_index - start_index);
                this->stripped_words[counter] = word;
                counter++;
            }

            //run over all whitespaces
            while (input[i] == ' ')
            {
                i++;
                start_index = i;
            }
        }
    }

    this->arg_count = counter;

    // strips the timeout + dur part to act as a normal command
    if (this->stripped_words[0] == "timeout" && counter >= 2)
    {
        timeout = stoi(this->stripped_words[1]);
        arg_count -= 2;  
        int new_index = this->stripped_flagless_command.find(this->stripped_words[2]);
        this->stripped_flagless_command = this->stripped_flagless_command.substr(new_index);
    }

    int redirection_index = input.find(">>");
    if (redirection_index != string::npos)
    {
        int new_index = redirection_index;
        this->first_command = cleanBackgroundCommand(input.substr(0, new_index));
        this->second_command = input.substr(new_index + 2);
        auto second_command_startpoint = this->second_command.find_first_not_of(WHITESPACE);
        if (second_command_startpoint == string::npos)
        {
            this->redirection = this->REDIRECTION_FAIL;
        }
        else
        {
            this->second_command = this->second_command.substr(second_command_startpoint);
            auto second_command_endpoint = this->second_command.find_first_of(WHITESPACE);
            if (second_command_endpoint != string::npos)
            {
                this->second_command = this->second_command.substr(0, second_command_endpoint);
            }
            this->redirection = this->APPEND;
        }
    }
    else if ((redirection_index = input.find(">")) != string::npos)
    {
        int new_index = redirection_index;
        this->first_command = cleanBackgroundCommand(input.substr(0, new_index));
        this->second_command = input.substr(new_index + 1);

        auto second_command_startpoint = this->second_command.find_first_not_of(WHITESPACE);
        if (second_command_startpoint == string::npos)
        {
            this->redirection = this->REDIRECTION_FAIL;
        }
        else
        {
            this->second_command = this->second_command.substr(second_command_startpoint);
            auto second_command_endpoint = this->second_command.find_first_of(WHITESPACE);
            if (second_command_endpoint != string::npos)
            {
                this->second_command = this->second_command.substr(0, second_command_endpoint);
            }
            this->redirection = this->OVERRIDE;
        }
    }
    else if ((redirection_index = input.find("|&")) != string::npos)
    {
        int new_index = redirection_index;
        this->first_command = cleanBackgroundCommand(input.substr(0, new_index));
        this->second_command = input.substr(new_index + 2);
        this->redirection = this->ERROR_PIPE;
    }
    else if ((redirection_index = input.find("|")) != string::npos)
    {
        int new_index = redirection_index;
        this->first_command = cleanBackgroundCommand(input.substr(0, new_index));
        this->second_command = input.substr(new_index + 1);
        this->redirection = this->PIPE;
    }

    if (redirection != this->NONE)
    {
        this->is_background = false;
    }
}


string CommandParser::getRawCommanad()
{


}


string CommandParser::getthis->first_command();
string CommandParser::getthis->second_command();
string CommandParser::getCleanCommand();

bool CommandParser::getis_background();
bool CommandParser::getIsComplex();
int CommandParser::getArgCount();
int CommandParser::getTimeout();
CommandParser::redirectionType CommandParser::getRedirection();

static string CommandParser::cleanBackgroundCommand(string input);

string& CommandParser::operator[](int index);




SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
/*
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}
