#include "Commands.h"

using namespace std;
const std::string WHITESPACE = " \n\r\t\f\v";



//--------------------------Job----------------------------------//
void Job::setJobID(int id) {jobID = id;}
int Job::getJobID() {return jobID;}
Job::status Job::getCurrentStatus() {return currentStatus;}
void Job::setCurrentStatus(Job::status status) {currentStatus = status;}
void Job::setCommand(Command *c) {command = c;}
Command* Job::getCommand() {return command;}


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
Job* JobsList::getJobById(int jobId)
  {
    JobsList::deleteFinishedJobs();
    Job *j = NULL;
    for(unsigned int i = 0; i < list.size(); i++)
      if (list[i]->getJobID() == jobId)
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
SmallShell::SmallShell() {}
SmallShell::~SmallShell() {
  delete jobs_list;
  delete time_out_jobs_list;
}
int SmallShell::get_max_num_of_processes() {return MAX_NUM_OF_PROCESSES;}
int SmallShell::get_args_max() {return ARGS_MAX;}
int SmallShell::get_command_size_max() {return COMMAND_SIZE_MAX;}
int SmallShell::get_process_name_max() {return PROCESS_NAME_MAX;}


Command* SmallShell::CreateCommand(string cmd_line)
{
  return NULL;
}
void SmallShell::executeCommand(string cmd_line)
{
  CreateCommand(cmd_line)->execute();
}
