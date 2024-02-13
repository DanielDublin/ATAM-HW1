#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;


void ctrlCHandler(int sig_num)
{
    Job* fgCommandJob = SmallShell::getInstance().getForegroundCommandJob();
    cout << "smash: got ctrl-C" << endl;

    if (fgCommandJob == nullptr || kill(fgCommandJob->getPID(), 0) != 0)  // check if the process is able to receive signals
    {
        return;
    }


    if (kill(fgCommandJob->getPID(), SIGKILL) != 0)
    {
        perror("smash error: kill failed");
    }


    cout << "smash: process " << fgCommandJob->getPID() << " was killed" << endl;
    SmallShell::getInstance().setForegroundCommandJob(nullptr);
}

void alarmHandler(int sig_num)
{
    cout << "smash: got an alarm\n";
    //SmallShell::getInstance().getTimeoutList()->removeFinishedTimeouts();
   // SmallShell::getInstance().getTimeoutList()->addAlarm();
}
