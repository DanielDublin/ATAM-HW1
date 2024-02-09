 
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <iomanip>
#include "Commands.h"




/*------------------Command--------------------*/

Command::Command(CommandParser parsed_command) : pid(-1), parsed_command(parsed_command)
{}


int Command::getPid()
{
    return this->pid;
}

CommandParser Command::getParsedCommand()
{
    return this->parsed_command;
}


/*-----------------CommandParser-------------------*/


CommandParser::CommandParser(string input) : raw_command(input), first_command(""), second_command(""), redirection(NONE), timeout(0)
{
    size_t first_whitespace_index = input.find_first_not_of(WHITESPACE);
    size_t last_whitespace_index = input.find_last_not_of(WHITESPACE);
    this->is_background = input[last_whitespace_index] == '&';
    this->is_complex = ((size_t)input.find_first_of(COMPLEX_CHAR) != string::npos);

    if (this->is_background && last_whitespace_index != string::npos)
    {
        input = input.substr(first_whitespace_index, last_whitespace_index - first_whitespace_index);
    }

    this->stripped_flagless_command = input;
    
    int start_index = 0;
    int end_index = 0;
    int counter = 0;
    input.push_back(' '); //for better automation

    for (size_t i = 0; i < input.length(); ++i)
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

    this->word_count = counter;

    // strips the timeout + dur part to act as a normal command
    if (this->stripped_words[0] == "timeout" && counter >= 2)
    {
        timeout = stoi(this->stripped_words[1]);
        word_count -= 2;  
        int new_index = this->stripped_flagless_command.find(this->stripped_words[2]);
        this->stripped_flagless_command = this->stripped_flagless_command.substr(new_index);
    }


    // handling command based cases for stripping and inits
    size_t redirection_index = input.find(">>"); // Append
    if (redirection_index != string::npos)
    {
        this->first_command = cleanBackgroundCommand(input.substr(0, redirection_index));
        this->second_command = input.substr(redirection_index + 2);
        auto second_command_startpoint = this->second_command.find_first_not_of(WHITESPACE);

        if (second_command_startpoint != string::npos)
        {
            this->second_command = this->second_command.substr(second_command_startpoint);
            auto second_command_endpoint = this->second_command.find_first_of(WHITESPACE);
            if (second_command_endpoint != string::npos)
            {
                this->second_command = this->second_command.substr(0, second_command_endpoint);
            }
            this->redirection = this->APPEND;
            
        }
        else
        {
            this->redirection = this->REDIRECTION_FAIL;
        }
    }

    redirection_index = input.find(">");  // override
    if (redirection_index != string::npos && input.find(">>") == string::npos)
    {
        this->first_command = cleanBackgroundCommand(input.substr(0, redirection_index));
        this->second_command = input.substr(redirection_index + 1);

        auto second_command_startpoint = this->second_command.find_first_not_of(WHITESPACE);
        if (second_command_startpoint != string::npos)
        {
            this->second_command = this->second_command.substr(second_command_startpoint);
            auto second_command_endpoint = this->second_command.find_first_of(WHITESPACE);
            if (second_command_endpoint != string::npos)
            {
                this->second_command = this->second_command.substr(0, second_command_endpoint);
            }
            this->redirection = this->OVERRIDE;
            
        }
        else
        {
            this->redirection = this->REDIRECTION_FAIL;
        }
    }

    redirection_index = input.find("|"); // pipe
    if (redirection_index != string::npos && input.find("|&") == string::npos)
    {
        this->first_command = cleanBackgroundCommand(input.substr(0, redirection_index));
        this->second_command = input.substr(redirection_index + 1);
        this->redirection = this->PIPE;
    }
    
    redirection_index = input.find("|&");  //err pipe
    if (redirection_index != string::npos)
    {
        this->first_command = cleanBackgroundCommand(input.substr(0, redirection_index));
        this->second_command = input.substr(redirection_index + 2);
        this->redirection = this->ERROR_PIPE;
    }
    

    if (this->redirection != this->NONE) // cant have a redirection with a background process
    {
        this->is_background = false;
    }
}


string CommandParser::getRawCommanad()
{
    return this->raw_command;
}


string CommandParser::getFirstCommand()
{
    return this->first_command;
}


string CommandParser::getSecondCommand()
{
    return this->second_command;
}

string CommandParser::getCleanCommand()
{
    return this->stripped_flagless_command;
}

bool CommandParser::getIsBackground()
{
    return this->is_background;
}


bool CommandParser::getIsComplex()
{
    return this->is_complex;
}

int CommandParser::getWordCount()
{
    return this->word_count;
}

int CommandParser::getTimeout()
{
    return this->timeout;
}

CommandParser::redirectionType CommandParser::getRedirection()
{
    return this->redirection;
}

string CommandParser::cleanBackgroundCommand(string input)
{
    int index = input.find_last_not_of(WHITESPACE);
    bool is_background = (input[index] == '&');
    int start_index = input.find_first_not_of(WHITESPACE);
    int end_index = input.find_last_not_of(WHITESPACE);
   
    if (end_index != (int)std::string::npos && is_background)
    {
        input = input.substr(start_index, end_index - start_index);
    }
    return input;
}

string& CommandParser::operator[](int index)
{
    if (index >= 0 && index < this->word_count)
    {
        if (timeout > 0)
        {
            index += this->TIMEOUT_ARG_COUNT;
        }
        return this->stripped_words[index];
    }
    throw std::logic_error("invalid index");
}


/*----------------Built-in commands--------------------*/


ShowPidCommand::ShowPidCommand(CommandParser parsed_command) : Command(parsed_command) {}

void ShowPidCommand::execute()
{
    std::cout << "smash pid is " << SmallShell::getInstance().get_Smash_Pid() << std::endl;
}



KillCommand::KillCommand(CommandParser parsed_command, JobsList* jobs) : Command(parsed_command), jobs(jobs) {}

void KillCommand::execute()
{

    int sigal_number = -1, job_id = -1;

    if (parsed_command.getWordCount() != 3)  // NEED TO CHECK IF THE ORDER OF ERRORS IS CORRECT - THE PDF IS BS -----------------------------------------------
    {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }

    std::string signal_arg = parsed_command[1];
    std::string job_id_arg = parsed_command[2];

    if (signal_arg[0] != '-')
    {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }

    try
    {
        sigal_number = std::stoi(signal_arg.substr(1, signal_arg.length() - 1)); // starting from after -
        job_id = std::stoi(job_id_arg);
    }
    catch (std::invalid_argument const& ex)
    {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }


    Job* job = jobs->getJobById(job_id);

    if (job == nullptr)
    {
        std::cerr << "smash error: kill: job-id " << job_id << " does not exist" << std::endl;
        return;
    }

    if (kill(job->pid, 0) != 0)
    {
        return;
    }
    else if (kill(job->processId, sigal_number) == -1)
    {
        perror("smash error: kill failed");
    }
    else
    {
        std::cout << "signal number " << sigal_number << " was sent to pid " << job->processId << std::endl;
        if (sigal_number == SIGSTOP || sigal_number == SIGTSTP)
        {
            job->isStopped = true;
        }
        else if (sigal_number == SIGCONT)
        {
            job->isStopped = false;
        }
        if (sigal_number == SIGKILL)
        {
            int temp = waitpid(job->processId, NULL, 0);
            if (temp == job->processId)
            {
                jobs->removeJobById(job->job_id);
            }
            else if (temp == -1)
            {
                perror("smash error: wait failed");
            }
        }
    }
}


//--------------------------Job----------------------------------//
Job::Job(int pid) : pid(pid), jobID(-1), currentStatus(status::FINISHED) {}
void Job::setJobID(int id) {jobID = id;}
int Job::getJobID() {return jobID;}
Job::status Job::getCurrentStatus() {return currentStatus;}
void Job::setCurrentStatus(Job::status status) {currentStatus = status;}
void Job::setCommand(Command *c) {command = c;}
Command* Job::getCommand() {return command;}
void Job::setPid(int id) { this->pid = pid; }
int Job::getPid() { return this->pid; }


//--------------------------JobsList----------------------------//
JobsList::~JobsList()
{
  delete &list;
}
void JobsList::deleteFinishedJobs()
  {
    for(unsigned int i = 0; i < list.size(); i++)
      if (list[i]->getCurrentStatus() == Job::status::FINISHED)
        list.erase(list.begin() + i);
    maxJobID = 0;
    for(unsigned int i = 0; i < list.size(); i++)
      if (list[i]->getJobID() > maxJobID)
        maxJobID = list[i]->getJobID();
  }
void JobsList::addJobToList(Job* j)
  {
    JobsList::deleteFinishedJobs();
    maxJobID++;
    j->setJobID(maxJobID);
    list.push_back(j);

  }
Job* JobsList::getJobById(int job_id)
  {
    JobsList::deleteFinishedJobs();
    Job *j = NULL;
    for(unsigned int i = 0; i < list.size(); i++)
      if (list[i]->getJobID() == job_id)
        j = list[i];
    return j;
  }
void JobsList::printJobsList()
  {
    JobsList::deleteFinishedJobs();
    for(unsigned int i = 0; i < list.size(); i++)
      cout << "[" << i << "]   ID:" << list[i]->getJobID() << "  STATUS:" << list[i]->getCurrentStatus() << endl;
  }

//--------------------------SmallShell------------------------//
SmallShell::SmallShell() : prompt("smash"), last_dir("") {
    this->smash_pid = getpid();
    this->jobs_list = new JobsList();
    //this->time_out_jobs_list = new TimeoutList();
}


SmallShell::~SmallShell() {
  delete jobs_list;
  delete time_out_jobs_list;
}
int SmallShell::get_max_num_of_processes() {return MAX_NUM_OF_PROCESSES;}
int SmallShell::get_args_max() {return ARGS_MAX;}
int SmallShell::get_command_size_max() {return COMMAND_SIZE_MAX;}
int SmallShell::get_process_name_max() {return PROCESS_NAME_MAX;}
int SmallShell::get_Smash_Pid() {return this->smash_pid;}


Command* SmallShell::CreateCommand(string command_line)
{
    CommandParser processed_command(command_line);

    if (processed_command.getWordCount() == 0)
    {
        return nullptr;
    }

    std::string command_name = processed_command[0];


    if (command_name.compare("showpid") == 0) {
        return new ShowPidCommand(processed_command);
    }
    return nullptr;
}


void SmallShell::executeCommand(string command_line)
{
    
    this->jobs_list->deleteFinishedJobs();

    Command* command = CreateCommand(command_line);
    if (command != nullptr)
    {
        command->execute();
    }
  
}





