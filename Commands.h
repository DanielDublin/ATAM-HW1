#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <iostream>
#include <string>


#define COMMAND_ARGS_MAX_LENGTH (80)
#define COMMAND_MAX_ARGS (20)

using namespace std;
const string WHITESPACE = " \n\r\t\f\v";
const string COMPLEX_CHAR = "?*";




/*---------------------CommandParser--------------------------*/


class CommandParser
{
    public:

        enum redirectionType
        {
            NONE,
            APPEND, // >>
            OVERRIDE, // >
            PIPE,  // |
            ERROR_PIPE, // |&
            REDIRECTION_FAIL = -1
        };

        CommandParser() = delete;
        CommandParser(string input);
        ~CommandParser() = default;

        string getRawCommanad();
        string getFirstCommand();
        string getSecondCommand();
        string getCleanCommand();

        bool getIsBackground();
        bool getIsComplex();
        int getWordCount();
        int getTimeout();
        redirectionType getRedirection();


        string& operator[](int index);

    private:      

        const static int TIMEOUT_ARG_COUNT = 2;
        static const int MAX_WORD_COUNT = 21;  // Command + 20 args

        string raw_command;
        string first_command;
        string second_command;
        string stripped_flagless_command;
        string stripped_words[MAX_WORD_COUNT];
        
        redirectionType redirection;
        bool is_background;
        bool is_complex;
        int word_count;
        int timeout;

        static string cleanBackgroundCommand(string input);
};


/*---------------------Command--------------------------*/
class Command {

public:

    Command(CommandParser parsed_command);
    int getPid();
    CommandParser getParsedCommand();
    void setParsedCommand(CommandParser parsed_command);

    virtual ~Command() = default;
    virtual void execute() = 0;
   

protected:
    int pid;
    CommandParser parsed_command;
    
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
  int jobID = 1;
  int pid;
  bool is_stopped = false;
  CommandParser parsed_command;
  status currentStatus = status::FINISHED;

  public:
  Job(int jobID, int pid, CommandParser parsed_command, bool is_stopped);
  ~Job() = default;
  void setJobID(int id);
  int getJobID();
  int getPID();
  void setPID(int pid);
  bool getIsStopped();
  void setIsStopped(bool is_stopped);
  Job::status getCurrentStatus();
  void setCurrentStatus(Job::status status);
  CommandParser getParsedCommand();

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
  void addJobToList(Job* target_job);
  void removeJob(int job_id);
  Job* getJobById(int jobId);
  Job* getLastJob(int* last_job_id);
  Job* getLastJob();
  void printJobsList();
  int getListSize();
  void killAllJobs();
};



/*--------------------Built-in commands-----------------------------*/


class ChromptCommand : public Command {
public:
    ChromptCommand(CommandParser parsed_command);
    virtual ~ChromptCommand() = default;
    void execute() override;
};


class ShowPidCommand : public Command {
public:
    ShowPidCommand(CommandParser parsed_command);
    virtual ~ShowPidCommand() = default;
    void execute() override;
};


class PWDCommand : public Command {
public:
    PWDCommand(CommandParser parsed_command);
    virtual ~PWDCommand() = default;
    void execute() override;
};


class CDCommand : public Command {

public:
    string& last_dir;
    CDCommand(CommandParser parsed_command, string& last_dir);
    virtual ~CDCommand() = default;
    void execute() override;
};


class JobsCommand : public Command {

public:
    JobsList* jobs;
    JobsCommand(CommandParser parsed_command, JobsList* jobs);
    virtual ~JobsCommand() = default;
    void execute() override;
};


class FGCommand : public Command {
   
public:
    JobsList* jobs_list;

    FGCommand(CommandParser parsed_command, JobsList* jobs_list);
    virtual ~FGCommand() = default;
    void execute() override;
};


class QuitCommand : public Command {

public:
    JobsList* jobs;
    QuitCommand(CommandParser parsed_command, JobsList* jobs);
    virtual ~QuitCommand() = default;
    void execute() override;
};


class KillCommand : public Command {
  
public:
    JobsList* jobs;
    KillCommand(CommandParser parsed_command, JobsList* jobs);
    virtual ~KillCommand() = default;
    void execute() override;
};




/*---------------------------External commands--------------------------------*/




/*-------------------------Special commands-------------------*/

class RedirectionCommand : public Command {
    string file_path;
public:
    explicit RedirectionCommand(CommandParser parsed_command);
    virtual ~RedirectionCommand() = default;
    void execute() override;
};

class PipeLineCommand : public Command {
public:
    PipeLineCommand(CommandParser parsed_command);
    virtual ~PipeLineCommand() = default;
    void execute() override;
};


class ChmodCommand : public Command {
    JobsList* jobs;
public:
    ChmodCommand(CommandParser parsed_command, JobsList* jobs);
    virtual ~ChmodCommand() = default;
    void execute() override;
};

class TimeoutCommand : public Command {
    JobsList* jobs;
public:
    TimeoutCommand(CommandParser parsed_command, JobsList* jobs);
    virtual ~TimeoutCommand() = default;
    void execute() override;
}; 



//--------------------------SmallShell------------------------//
class SmallShell {
  private:
  SmallShell();
  public:
  Command *CreateCommand(string command_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(string command_line);

  private:
  int smash_pid;
  string prompt = "smash";
  string last_dir = "";
  Job* foregroundCommandJob;
  JobsList* jobs_list;
  JobsList* time_out_jobs_list;
  const int MAX_NUM_OF_PROCESSES = 100;
  const int ARGS_MAX = 21;
  const int COMMAND_SIZE_MAX = 80;
  const int PROCESS_NAME_MAX = 50;

  public:
  int get_max_num_of_processes();
  int get_args_max();
  int get_command_size_max();
  int get_process_name_max();
  int get_smash_pid();
  string getPrompt();
  void setPrompt(string new_prompt);
  string getPWD();
  Job* getForegroundCommandJob();
  void setForegroundCommandJob(Job* job = nullptr);
  int get_job_list_size();
  void printJobsList();
  void killAllJobs();
  JobsList* getJobsList();
  //int get_smash_pid();
  string getCurrentDir();

};




#endif //SMASH_COMMAND_H_
