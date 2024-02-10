#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;


void ctrlCHandler(int sig_num)
{
    Command* fgCommand = SmallShell::getInstance().getForegroundCommand();
    cout << "smash: got ctrl-C" << endl;

    if (fgCommand == nullptr || kill(fgCommand->getPid(), 0) != 0)  // check if the process is able to receive signals
    {
        return;
    }


    if (kill(fgCommand->getPid(), SIGKILL) != 0)
    {
        perror("smash error: kill failed");
    }


    cout << "smash: process " << fgCommand->getPid() << " was killed" << endl;
    SmallShell::getInstance().getForegroundCommand();
}

void alarmHandler(int sig_num)
{
    cout << "smash: got an alarm\n";
    //SmallShell::getInstance().getTimeoutList()->removeFinishedTimeouts();
   // SmallShell::getInstance().getTimeoutList()->addAlarm();
}
