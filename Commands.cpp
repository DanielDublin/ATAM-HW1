#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"


/*-----------------CommandParser-------------------*/


CommandParser::CommandParser(std::string input) : raw_command(input), first_command(""), second_command(""), redirection(this->NONE), timeout(0)
{
    this->is_background = input.back() == '&';
    size_t ide = input.find_last_not_of(WHITESPACE);
    size_t ids = input.find_first_not_of(WHITESPACE);

    if (ide != std::string::npos && this->is_background)
    {
        input = input.substr(ids, ide - ids);
    }
    this->stripped_flagless_command = input;

    this->is_complex = (input.find_first_of(COMPLEX_CHAR) != std::string::npos);

    input.push_back(' ');
    size_t startPos = 0;
    size_t endPos = 0;
    int counter = 0;

    for (size_t i = 0; i < input.length(); ++i)
    {
        if (input[i] == ' ')
        {
            endPos = i;
            if (i != 0)
            {
                this->stripped_words[counter++] = input.substr(startPos, endPos - startPos);
            }
            while (input[i] == ' ')
            {
                ++i;
                startPos = i;
            }
        }
    }
    argAmount = counter;


    if (processedInput[0] == "timeout" && counter >= 2)
    {
        timeout = std::stoi(processedInput[1]);
        argAmount -= 2;
        size_t idx = inputNoBG.find(processedInput[2]);
        inputNoBG = inputNoBG.substr(idx);
    }

    auto pos = input.find(">>");
    if (pos != std::string::npos)
    {
        size_t idx = pos;
        firstCommand = removeBackground(input.substr(0, idx));
        secondCommand = input.substr(idx + 2);
        auto secondCommandStart = secondCommand.find_first_not_of(WHITESPACE);
        if (secondCommandStart == std::string::npos)
        {
            redirection = REDIRECTION_FAIL;
        }
        else
        {
            secondCommand = secondCommand.substr(secondCommandStart);
            auto secondCommandEnd = secondCommand.find_first_of(WHITESPACE);
            if (secondCommandEnd != std::string::npos)
            {
                secondCommand = secondCommand.substr(0, secondCommandEnd);
            }
            redirection = APPEND;
        }
    }
    else if ((pos = input.find(">")) != std::string::npos)
    {
        size_t idx = pos;
        firstCommand = removeBackground(input.substr(0, idx));
        secondCommand = input.substr(idx + 1);
        auto secondCommandStart = secondCommand.find_first_not_of(WHITESPACE);
        if (secondCommandStart == std::string::npos)
        {
            redirection = REDIRECTION_FAIL;
        }
        else
        {
            secondCommand = secondCommand.substr(secondCommandStart);
            auto secondCommandEnd = secondCommand.find_first_of(WHITESPACE);
            if (secondCommandEnd != std::string::npos)
            {
                secondCommand = secondCommand.substr(0, secondCommandEnd);
            }
            redirection = OVERRIDE;
        }
    }
    else if ((pos = input.find("|&")) != std::string::npos)
    {
        size_t idx = pos;
        firstCommand = removeBackground(input.substr(0, idx));
        secondCommand = input.substr(idx + 2);
        redirection = ERROR_PIPE;
    }
    else if ((pos = input.find("|")) != std::string::npos)
    {
        size_t idx = pos;
        firstCommand = removeBackground(input.substr(0, idx));
        secondCommand = input.substr(idx + 1);
        redirection = PIPE;
    }

    if (redirection != NONE)
    {
        isBackground = false;
    }
}

std::string CommandParser::getRawCommanad()
{


}


std::string CommandParser::getFirstCommand();
std::string CommandParser::getSecondCommand();
std::string CommandParser::getCleanCommand();

bool CommandParser::getIsBackground();
bool CommandParser::getIsComplex();
int CommandParser::getArgCount();
int CommandParser::getTimeout();
CommandParser::redirectionType CommandParser::getRedirection();

static std::string CommandParser::cleanBackgroundCommand(std::string input);

std::string& CommandParser::operator[](int index);



/

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
