
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <iostream>
#include <vector>
using namespace std;

//--------------------------Command------------------------------//
class Command {
  //////////////////////
  // copied from skeleton for now, to be completed by daniel
  /////////////////////

  /// TODO: Add your data members
  public:
  Command(string cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed

};

//--------------------------Job----------------------------------//
class Job{
  public:
  enum status {
      RUNNING_FG,
      RUNNING_BG,
      FINISHED
  };

  private:
  int jobID = -1;
  Command* command;
  status currentStatus = status::FINISHED;

  public:
  Job() = default;
  ~Job() = default;
  void setJobID(int id);
  int getJobID();
  Job::status getCurrentStatus();
  void setCurrentStatus(Job::status status);
  void setCommand(Command *c);
  Command* getCommand();
};


//--------------------------JobsList----------------------------//
class JobsList{
  private:
  vector<Job*> list;
  int maxJobID = 0;

  public:
  JobsList() = default;
  ~JobsList();
  void deleteFinishedJobs();
  void addJobToList(Job* j);
  Job *getJobById(int jobId);
  void printJobsList();
};


//--------------------------SmallShell------------------------//
class SmallShell {
  private:
  SmallShell();
  public:
  Command *CreateCommand(string cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(string cmd_line);
  // TODO: add extra methods as needed

  //////////////////////
  // our code
  //////////////////////

  private:
  int pid;
  string prompt = "smash";
  string last_dir = "";
  JobsList *jobs_list;
  JobsList *time_out_jobs_list;
  const int MAX_NUM_OF_PROCESSES = 100;
  const int ARGS_MAX = 21;
  const int COMMAND_SIZE_MAX = 80;
  const int PROCESS_NAME_MAX = 50;

  public:
  int get_max_num_of_processes();
  int get_args_max();
  int get_command_size_max();
  int get_process_name_max();

};


#endif //SMASH_COMMAND_H_




