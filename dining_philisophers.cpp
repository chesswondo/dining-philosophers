// Color output in Windows
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
HANDLE cons = GetStdHandle(STD_OUTPUT_HANDLE);
#define  COL(x) SetConsoleTextAttribute(cons,x)
#else
#define  COL(x)
#endif


#include <vector>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <thread>
#include <mutex>

using namespace std;

// This flag is used to prove that the
// method works for any fork placement;
// all philosophers use the same algorithm
// for choosing the order of fork acquisition

bool FORKS_SHUFFLE = false;

// This flag allows to show deadlock (if we're lucky)
bool NO_ORDER      = false;

class Philosopher;
int Count = 5;          // Philosophers count
int Timer = 2500;       // Time for thinking/eating

vector<Philosopher*> table;
vector<mutex*>       forks;

// This vector is used for forks numbers to demonstrate
// that all philosophers are used the same algorithm
vector<int> forksNo;

mutex table_state;  // Mutex for output

// State of philosopher
enum State {
    THINK = 0,   // thinking/talking
    WAIT  = 1,   // wait for forks
    EAT   = 2,   // eating
    STOP  = 3 }; // put forks on table

// Random engine and distribution for timers
default_random_engine reng(random_device{}());
uniform_int_distribution<> distr;


class Philosopher           // Don't save philosoper number!
{
    int fork, Fork;         // Numbers of two forks - smaller and greater
    State state = THINK;
public:
    Philosopher(int N):fork(forksNo[N]),Fork(forksNo[(N+1)%Count])
    {
        // Sort forks
        if (!NO_ORDER && fork > Fork) ::swap(fork,Fork);
        // wait for fork first, then for Fork
    }

    void run() // Philosopher behaviour
    {
        for(;;)
        {
            think();     // thinks, talks - not eating
            get_forks(); // hungry...
            eat();       // eating
            put_forks(); // release forks
        }
    }
    State get_state() { return state; }  // auxiliary function for report
private:

    void think()     { report(THINK); this_thread::sleep_for(distr(reng)*1ms);    }
    void eat()       { report(EAT);   this_thread::sleep_for(distr(reng)*1ms);    }
    void get_forks() { report(WAIT);  forks[fork]->lock();   forks[Fork]->lock();   }
    void put_forks() { report(STOP);  forks[Fork]->unlock(); forks[fork]->unlock(); }

    // Function to report about state changes
    void report(State s)
    {
        lock_guard<mutex> lock(table_state);       // table_state.lock()
        state = s;

        int eats = 0;
        for(auto p: table)
        {
            //COL("\x4C\x6E\x2A\x6E"[p->get_state()]);
            COL("\x19\x6E\x2A\x6E"[p->get_state()]);
            cout << "TWES"[p->get_state()] << " ";
            COL(0x08);
            if (p->get_state() == EAT) eats++;
        }
        cout << " Total eaters: " << eats << "\n";
                                                   // table_state.unlock()
    }
};

int main(int argc, char * argv[])
{
    if (argc > 1) Count = atoi(argv[1]);
    if (argc > 2) Timer = atoi(argv[2]);
    if (argc > 3)
    {
        for(int i = 3; i < argc; ++i)
        {
            if (strcmp(argv[i],"shuffle") == 0) FORKS_SHUFFLE = true;
            else if (strcmp(argv[i],"noorder" ) == 0) NO_ORDER = true;
        }
    }

    distr = uniform_int_distribution<>(Timer/5,Timer);

    for(int i = 0; i < Count; ++i)
            forksNo.push_back(i);

    if (FORKS_SHUFFLE)
    {
        shuffle(forksNo.begin(),forksNo.end(),reng);
    }

    for(int i = 0; i < Count; ++i)
    {
        table.push_back(new Philosopher(i));
        forks.push_back(new mutex);
    }

    for(int i = 0; i < Count; ++i)
    {
        thread t(&Philosopher::run,table[i]);
        t.detach();
    }

    getchar();
}

