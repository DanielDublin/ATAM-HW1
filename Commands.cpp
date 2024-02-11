#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include "Commands.h"

using namespace std;

/*------------------Command--------------------*/

Command::Command(CommandParser parsed_command) : pid(-1), parsed_command(parsed_command)
{}


int Command::getPid()
{
    return pid;
}

CommandParser Command::getParsedCommand()
{
    return this->parsed_command;
}

void Command::setParsedCommand(CommandParser parsed_command)
{
    this->parsed_command = parsed_command;
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
   
    if (end_index != (int)string::npos && is_background)
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



/*--------------------------------------------------Built-in commands--------------------------------------------------*/

ChromptCommand::ChromptCommand(CommandParser parsed_command) : Command(parsed_command) {}
ShowPidCommand::ShowPidCommand(CommandParser parsed_command) : Command(parsed_command) {}
CDCommand::CDCommand(CommandParser parsed_command, string& last_dir) : Command(parsed_command), last_dir(last_dir) {}
PWDCommand::PWDCommand(CommandParser parsed_command) : Command(parsed_command) {}
JobsCommand::JobsCommand(CommandParser parsed_command, JobsList* jobs) : Command(parsed_command) {}
FGCommand::FGCommand(CommandParser parsed_command, JobsList* jobs_list) : Command(parsed_command), jobs_list(jobs_list) {}
QuitCommand::QuitCommand(CommandParser parsed_command, JobsList* jobs) : Command(parsed_command) {}
KillCommand::KillCommand(CommandParser parsed_command, JobsList* jobs) : Command(parsed_command), jobs(jobs) {}




void ChromptCommand::execute()
{
    if (this->getParsedCommand().getWordCount() > 1)
    {
        SmallShell::getInstance().setPrompt(this->getParsedCommand()[1]);
    }
    else
    {
        SmallShell::getInstance().setPrompt("smash");
    }
}


/*-----------------------------------------------------------------------------------------------------------------------*/
void ShowPidCommand::execute()
{
    cout << "smash pid is " << this->getPid() << endl;
}

/*-----------------------------------------------------------------------------------------------------------------------*/

void PWDCommand::execute()
{
    cout << SmallShell::getInstance().getPWD() << endl;
}

/*-----------------------------------------------------------------------------------------------------------------------*/

void JobsCommand::execute()
{
    SmallShell::getInstance().printJobsList();
}

/*-----------------------------------------------------------------------------------------------------------------------*/

void FGCommand::execute()
{
    int job_id = -1, job_pid = -1;
    string job_description = "";
    Job* job = nullptr;
    Command* current_command = nullptr;

    // check args
    if (parsed_command.getWordCount() > 2)
    {
        std::cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    else if (parsed_command.getWordCount() == 2) // get job from list
    {
        try
        {
            job_id = std::stoi(parsed_command[1]);
        }
        catch (std::invalid_argument const& ex)
        {
            std::cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }


        job = this->jobs_list->getJobById(job_id);
        if (job == nullptr)
        {
            std::cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
            return;
        }
    }
    else // get the highest job (which is the last one on the list)
    {
        job = this->jobs_list->getLastJob(&job_id);
        if (job == nullptr)
        {
            std::cerr << "smash error: fg: jobs list is empty" << endl;
            return;
        }
    }


    job_description.append(job->getParsedCommand().getRawCommanad());
    job_description.append(" : ").append(std::to_string(job->getPID()));
    cout << job_description << endl;


    if (job->getIsStopped())
    {
        if (kill(job->getPID(), 0) != 0)  // check channel for errors
        {
            return;
        }

        if (kill(job->getPID(), SIGCONT) != 0)
        {
            perror("smash error: kill failed");
            return;
        }
    }

    job_pid = job->getPID();
    job->setIsStopped(false);
    current_command->setParsedCommand(job->getParsedCommand());

    SmallShell::getInstance().setForegroundCommand(current_command); // force the background process to run in foreground

    if (waitpid(job_pid, NULL, WUNTRACED) == -1)
    {
        perror("smash error: wait failed");
        return;
    }

    SmallShell::getInstance().setForegroundCommand(); // reset the current foreground command  - not built-in

    if (!job->getIsStopped())
    {
        this->jobs_list->removeJob(job->getJobID());
    }
    else
    {
        // job->timeCreated = time(NULL);
        //int a = 0;
        ;
    }

}

/*-----------------------------------------------------------------------------------------------------------------------*/

void QuitCommand::execute()
{
    if (parsed_command.getWordCount() > 1 && parsed_command[1].compare("kill") == 0)
    {
        cout << "smash: sending SIGKILL signal to " << SmallShell::getInstance().get_job_list_size() << " jobs:" << endl;
        SmallShell::getInstance().killAllJobs();
    }

    exit(0);
}

/*-----------------------------------------------------------------------------------------------------------------------*/


void CDCommand::execute()
{
    if (parsed_command.getWordCount() < 2)   // no args
    {
        std::cerr << "smash error:> \"" << parsed_command.getRawCommanad() << "\"" << endl;
        return;
    }
    else if (parsed_command.getWordCount() > 2) // too many args
    {
        std::cerr << "smash error: cd: too many arguments\n";
        return;
    }


    string curr_dir = SmallShell::getInstance().getCurrentDir(); //change to a better name later --------------------------------------------------------------------------

    if (parsed_command[1].compare("-") == 0)
    {
        if (this->last_dir.compare("") == 0)
        {
            std::cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        if (chdir(this->last_dir.c_str()) == -1)
        {
            perror("smash error: chdir failed");
            return;
        }
        this->last_dir = curr_dir;
    }
    else if (chdir(parsed_command[1].c_str()) == -1)
    {
        perror("smash error: chdir failed");
        return;
    }
    else
    {
        this->last_dir = curr_dir;
    }
}


/*-----------------------------------------------------------------------------------------------------------------------*/
void KillCommand::execute()
{

    int sigal_number = -1, job_id = -1;
    string signal_arg = "";
    string job_id_arg = "";
    Job* job = nullptr;

    if (parsed_command.getWordCount() != 3)  // NEED TO CHECK IF THE ORDER OF ERRORS IS CORRECT - THE PDF IS BS -----------------------------------------------
    {
        std::cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }

    signal_arg = parsed_command[1];
    job_id_arg = parsed_command[2];

    if (signal_arg[0] != '-')
    {
        std::cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }

    try
    {
        sigal_number = std::stoi(signal_arg.substr(1, signal_arg.length() - 1)); // starting from after -
        job_id = std::stoi(job_id_arg);
    }
    catch (std::invalid_argument const& ex)
    {
        std::cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }


    job = jobs->getJobById(job_id);

    if (job == nullptr) // existance check
    {
        std::cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
        return;
    }

    if (kill(job->getPID(), 0) != 0) // check for errors as preperation
    {
        return;
    }

    if (kill(job->getPID(), sigal_number) == -1) // send sig
    {
        perror("smash error: kill failed");
    }
    else 
    {
        
        cout << "signal number " << sigal_number << " was sent to pid " << job->getPID() << endl;

        if (sigal_number == SIGSTOP || sigal_number == SIGTSTP)
        {
            job->setIsStopped(true);
        }
        else if (sigal_number == SIGCONT)
        {
            job->setIsStopped(false);
        }
        else if (sigal_number == SIGKILL)
        {
            int child_pid = waitpid(job->getPID(), NULL, 0);

            if (child_pid == job->getPID())
            {
                this->jobs->removeJob(job->getJobID());
            }
            else if (child_pid == -1)
            {
                perror("smash error: wait failed");
            }
        }
    }
}


/*---------------------------------Special Command----------------------------------------------*/

RedirectionCommand::RedirectionCommand(CommandParser parsed_command) : Command(parsed_command), file_path(parsed_command.getSecondCommand()) {}

void RedirectionCommand::execute()
{
    CommandParser::redirectionType redirection = parsed_command.getRedirection();
    int file_FD = -1;
    int mode = std::stoul("0777", nullptr, 8);  //  read, write and execute permissions for the owner 
                                                // needs an octal value for file perms

    int forked_pid = fork();
    if (forked_pid == 0) // son
    {
        if (close(STDOUT_FILENO) < 0) 
        {
            perror("smash error: close failed");
            exit(1);
        }

        // check handle type
        if (redirection == CommandParser::OVERRIDE)
        {
            file_FD = open(this->file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, mode);  // read-write, create, override
        }
        else
        {
            file_FD = open(this->file_path.c_str(), O_RDWR | O_CREAT | O_APPEND, mode); // read-write, create, append
        }

        // handle failed
        if (file_FD < 0)
        {
            perror("smash error: open failed");
            exit(1);
        }

        SmallShell::getInstance().executeCommand(parsed_command.getFirstCommand().c_str());
        exit(1);
    }
    else if (forked_pid > 0)  // father
    {
        if (waitpid(forked_pid, NULL, WUNTRACED) == -1)
        {
            perror("smash error: wait failed");
        }
    }
    else   // fork failed
    {
        perror("smash error: fork failed");
    }


}



//-----------------------------------------------------Job-----------------------------------------------//
Job::Job(int jobID, int pid, CommandParser parsed_command, bool is_stopped) :
    jobID(jobID), pid(pid), parsed_command(parsed_command), is_stopped(is_stopped) {}



void Job::setJobID(int id) {jobID = id;}
int Job::getJobID() {return jobID;}
int Job::getPID(){ return this->pid; }
bool Job::getIsStopped() {return this->is_stopped; }
void Job::setIsStopped(bool is_stopped) { this->is_stopped = is_stopped; }
Job::status Job::getCurrentStatus() {return currentStatus;}
void Job::setCurrentStatus(Job::status status) {currentStatus = status;}
void Job::setPID(int pid) { this->pid = pid; }
//void Job::setIsStopped(bool is_stopped) { this->is_stopped = is_stopped; }
//bool Job::getIsStopped() { return this->is_stopped; }
CommandParser Job::getParsedCommand() { return this->parsed_command; }

//--------------------------JobsList----------------------------//
JobsList::~JobsList()
{
  //no need for deletion here, it causes double free
  ;
}
void JobsList::deleteFinishedJobs()
  {
    int status;
    for(unsigned int i = 0; i < list.size(); i++)
      if (waitpid(list[i]->getPID(), &status, WNOHANG) > 0 || kill(list[i]->getPID(), 0) < 0)
        list[i]->setCurrentStatus(Job::status::FINISHED);
    for(unsigned int i = 0; i < list.size(); i++)
      if (list[i]->getCurrentStatus() == Job::status::FINISHED)
        list.erase(list.begin() + i);
    maxJobID = 0;
    for(unsigned int i = 0; i < list.size(); i++)
      if (list[i]->getJobID() > maxJobID)
        maxJobID = list[i]->getJobID();
  }

void JobsList::addJobToList(Job* target_job)
  {
    JobsList::deleteFinishedJobs();
    maxJobID++;
    target_job->setJobID(maxJobID);
    list.push_back(target_job);

  }



void JobsList::removeJob(int job_id)
{
    for (int i = 0; i < (int)this->list.size(); i++)
    {
        if (list[i]->getJobID() == job_id)
        {
            Job* target_job = list[i];
            list.erase(list.begin() + i);

            delete(target_job);
            break;
        }
    }
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


Job* JobsList::getLastJob(int* last_job_id)
{
    this->deleteFinishedJobs();

    if (this->list.empty())
    {
        return nullptr;
    }

    if (last_job_id != nullptr)
    {
        *last_job_id = list.back()->getJobID();
    }

    return list.back();
}




void JobsList::printJobsList()
  {
    JobsList::deleteFinishedJobs();
    for(unsigned int i = 0; i < list.size(); i++)
      cout << "[" << list[i]->getJobID() << "] " << list[i]->getParsedCommand().getRawCommanad() << endl;
  }

  int JobsList::getListSize() {return list.size();}


  void JobsList::killAllJobs()
  {
    JobsList::deleteFinishedJobs();
    for(unsigned int i = 0; i < list.size(); i++)
      {
        cout << list[i]->getPID() << ": " << list[i]->getParsedCommand().getRawCommanad() << endl;
        kill(list[i]->getPID(), SIGKILL);
        list[i]->setCurrentStatus(Job::status::FINISHED);
      }
    JobsList::deleteFinishedJobs();
  }
  


//--------------------------SmallShell------------------------//
SmallShell::SmallShell() : prompt("smash"), last_dir("") {
    this->smash_pid = getpid();
    this->jobs_list = new JobsList();
    //this->time_out_jobs_list = new TimeoutList();
}
SmallShell::~SmallShell() {
  delete jobs_list;
  //delete time_out_jobs_list;
}
int SmallShell::get_max_num_of_processes() {return MAX_NUM_OF_PROCESSES;}
int SmallShell::get_args_max() {return ARGS_MAX;}
int SmallShell::get_command_size_max() {return COMMAND_SIZE_MAX;}
int SmallShell::get_process_name_max() {return PROCESS_NAME_MAX;}
int SmallShell::get_smash_pid() { return this->smash_pid; }
void SmallShell::setPrompt(string new_prompt) { this->prompt = new_prompt; }

string SmallShell::getCurrentDir()
{
    int size = COMMAND_ARGS_MAX_LENGTH;

    try
    {
        char* pathCharArr = new char[size]();
        pathCharArr = getcwd(pathCharArr, size);

        string pathStr(pathCharArr);
        delete[](pathCharArr);

        return pathStr;
    }
    catch (std::bad_alloc& e)  //need to remove
    {
        throw e;
    }

    return nullptr;
}

string SmallShell::getPWD()
{
  char *temp_arr = new char[500];
  temp_arr = getcwd(temp_arr, 500);
  string path(temp_arr);
  delete[] temp_arr;
  return path;
}

void SmallShell::printJobsList()
{
  this->jobs_list->printJobsList();
}

int SmallShell::get_job_list_size()
{
  return this->jobs_list->getListSize();
}

JobsList* SmallShell::getJobsList() { return this->jobs_list; }

void SmallShell::killAllJobs()
{
  jobs_list->killAllJobs();
}

string SmallShell::getPrompt()
{
  return this->prompt;
}

Command* SmallShell::getForegroundCommand()
{
    return this->foregroundCommand;
}

void SmallShell::setForegroundCommand(Command* new_command)
{
    this->foregroundCommand = new_command;
}


/////////////////////////////////////////////////////////  built-in commands /////////////////////////////////////////////////////////
Command* SmallShell::CreateCommand(string command_line)
{
    CommandParser processed_command(command_line);

    if (processed_command.getWordCount() == 0)
    {
        return nullptr;
    }

    string command_name = processed_command[0];

    // redirection
    if (processed_command.getRedirection() == CommandParser::REDIRECTION_FAIL)
    {
        std::cerr << "smash error: redirection: invalid arguments" << std::endl;
        return nullptr;
    }
    else if (processed_command.getRedirection() == CommandParser::OVERRIDE || 
        processed_command.getRedirection() == CommandParser::APPEND)
    {
        return new RedirectionCommand(processed_command);
    }

    //built-in commands
    else if (command_name.compare("showpid") == 0) {
        return new ShowPidCommand(processed_command);
    }
    else if (command_name.compare("cd") == 0) {
        return new CDCommand(processed_command, this->last_dir);
    }
    else if (command_name.compare("chprompt") == 0)
    {
        return new ChromptCommand(processed_command);
    }
    else if (command_name.compare("pwd") == 0)
    {
        return new PWDCommand(processed_command);
    }
    else if (command_name.compare("jobs") == 0)
    {
        return new JobsCommand(processed_command, nullptr);
    }
    else if (command_name.compare("fg") == 0) 
    {
        return new FGCommand(processed_command, this->getJobsList());
    }
    else if (command_name.compare("kill") == 0) {
        return new KillCommand(processed_command, this->getJobsList());
    }
    else if (command_name.compare("quit") == 0)
    {
        return new QuitCommand(processed_command, nullptr);
    }

    /////////////////////////////////////////////////////////  external commands /////////////////////////////////////////////////////////
    else if (! (processed_command.getIsComplex()))
    {
        int status = 0;
        int pid = fork();
        if (pid == 0)
        {
            setpgrp(); 
            char* temp[processed_command.getWordCount() + 1] = {nullptr};
            for(int i = 0 ; i < processed_command.getWordCount(); i++)
            {
                const char* c1 = processed_command[i].c_str();
                char* c2 = const_cast<char*>(c1);
                temp[i] = c2;
            }
            
            execvp(temp[0] ,temp);
        }
        wait( (int *)1 );
        Job *j = new Job(1, pid, processed_command, false);
        j->setCurrentStatus(Job::status::RUNNING_FG);
        SmallShell::getInstance().getJobsList()->addJobToList(j);
        if((processed_command.getIsBackground()))
        {  
           j->setCurrentStatus(Job::status::RUNNING_BG);
           return nullptr;
        }
        while (wait(&status) > 0);
        return nullptr;
	}
    else //is a complex external command
    {   
        int status = 0;
        int pid = fork();
        if (pid == 0)
        {
            setpgrp();
            string s1 = "/bin/bash";
            string s2 = "-c";
            
            const char* c1 = s1.c_str();
            char* c2 = const_cast<char*>(c1);

            const char* c3 = s2.c_str();
            char* c4 = const_cast<char*>(c3);

            const char* c5 = processed_command.getRawCommanad().c_str();
            char* c6 = const_cast<char*>(c5);

            char* temp[4] = {c2, c4, c6 , nullptr};

            execvp(temp[0] ,temp);
        }
        wait( (int *)1 );
        Job *j = new Job(1, pid, processed_command, false);
        j->setCurrentStatus(Job::status::RUNNING_FG);
        SmallShell::getInstance().getJobsList()->addJobToList(j);
        if((processed_command.getIsBackground()))
        {  
           j->setCurrentStatus(Job::status::RUNNING_BG);
           return nullptr;
        }
        while (wait(&status) > 0);
        return nullptr;
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
