/*
 *  @fileName       :   main.cpp
 *
 *  @author         :   tanmay-mk
 *
 *  @date           :   26 Oct 2022
 *                                           
 */

/*LIBRARY FILES*/
#include <iostream> //for cout and other generic io operations
#include <fstream>  //for file operations
#include <cstdint>  //for uint32_t
#include <unistd.h> //for getopt_long()
#include <getopt.h> //for struct of long options in getopt_long()
#include <cstdbool> //for boolean values
#include <string>   //for strings
#include <cstdlib>  //for exit()

#include "counter.h"
#include "locks.h"
#include "time.h"

/*PRIVATE DEFINES*/
#define DEFAULT_NUMTHREADS      (4)

using namespace std; 

/*--------------------------------------------------------------------*/
/*TYPEDEFS & ENUMS*/
/*enum to represent return value of --name & --alg command*/
enum 
{
    usr_name = 0,
    barriers, 
    locks
};

/*--------------------------------------------------------------------*/
/*GLOBAL VARIABLES*/
/*array of structure of long command options*/
/*structure definition is in <getopt.h>*/
struct option long_cmd_options[] = 
{
    {"name", no_argument, 0, usr_name},
    {"bar", required_argument, 0, barriers},
    {"lock", required_argument, 0, locks}
};

/*short commands*/
const char *short_cmd_options = "i:t:o:";

/*--------------------------------------------------------------------*/
/*
 * @brief       :   writes the sorted list to a file
 * 
 * @parameters  :   string
 *                      outputFile  :   path to the file   
 *                  int 
 *                      count       :   integer to be written to file
 *
 * @returns     :   none, exits with EXIT_FAILURE if file opening fails
 */
static void writeToFile (string outputFile, int count)
{
    //create instance of ofstream to write to file
    ofstream fout;
    //open file, and check if opening succeeded
    fout.open(outputFile);
    if(!fout)
    {
        //if file opening fails, exit
        cout << "Failed to open file, exiting ..." << endl;
        exit(EXIT_FAILURE);
    }

    //write count to file
    fout << count << endl;

    //close the file
    fout.close();
}

/*--------------------------------------------------------------------*/
/*
 * @brief       :   determines which lock and barrier to be used
 * 
 * @parameters  :   string
 *                      locktype  :   type of lock to be used
 *                      barriertype :    type of barrier to be used
 *                  lock_algs_t 
 *                      *alg    :   pointer to variable which stores the integer
 *                                  value of locking algorithm
 *                  barrier_types_t
 *                      *bar    :   pointer to variable which stores the integer
 *                                  value of barrier algorithm
 *
 * @returns     :   none
 */
static void determine_lock_and_barrier(string locktype, string barriertype, lock_algs_t *alg, barrier_types_t *bar)
{
    if (locktype == "tas")
    {
        *alg = TAS_algorithm;
    }
    else if (locktype == "ttas")
    {
        *alg = TTAS_algorithm;
    }
    else if (locktype == "ticket")
    {
        *alg = TICKETLOCK_algorithm;
    }
    else if (locktype == "mcs")
    {
        *alg = MCS_algorithm;
    }
    else if (locktype == "petersonseq")
    {
        *alg = PETERSON_SEQ_algorithm;
    }
    else if (locktype == "petersonrel")
    {
        *alg = PETERSON_REL_algorithm;
    }
    else    /*default algorithm is pthread*/
    {
        *alg = PTHREAD_algorithm;
    }
    
    /*-------------------------------------------------------------------------*/
    /*now determine the barrier type to be sent to bucketsort*/
    if (barriertype == "sense")
    {
        *bar = SENSE_REV_type;
    }
    else
    {
        *bar = PTHREAD_type;
    }
}

/*--------------------------------------------------------------------*/
/*
 * @brief       :   determines number of threads to be used
 * 
 * @parameters  :   string
 *                      locktype  :   type of lock to be used
 *                  int 
 *                      numThreads  :   number of threads input by user
 *
 * @returns     :   int
 *                      number of threads to be used for application
 */
static int determine_numThreads(string lockType, int num_threads)
{
    if (lockType == "petersonseq" || lockType == "petersonrel")
    {
        return 2;
    }
    return num_threads; 
}

/*--------------------------------------------------------------------*/
/*
 * @brief       :   application entry point
 */
int main(int argc, char* argv[])
{
    //we need at least one command line argument to proceed
    //if command line arguments is less than two, exit 
    if (argc < 2)
    {
        cout << "Please provide at least one argument" << endl;
        cout << "Exiting ... " << endl;
        exit(EXIT_FAILURE);
    }

    //this stores the command line argument received from user
    int opt;

    //output file name, to which sorted list is to be written
    string op_filename;

    string lockType, barrierType; 

    //number of threads
    int num_threads=0;
    int num_iterations=0;

    bool nameflag = false;
    bool barrierFlag = false;

    //get command line arguments that start with '-' or '--'
    while((opt = getopt_long(argc, argv, short_cmd_options, long_cmd_options, NULL)) != -1)
    {
        switch (opt)
        {
            //user entered -o output_filename
            case 'o':
                //getopt_long() automatically stores the argument passed in 'optarg'
                //store the output file name into a variable for future use
                op_filename = optarg;
            break;

            //user entered --name 
            case usr_name:
                //print name
                cout << "Tanmay Mahendra Kothale" << endl;
                nameflag = true;
            break;

            case locks:
                //getopt_long() automatically stores the argument passed in 'optarg'
                //store the locking method into a variable for future use
                lockType = optarg; 
            break;

            case barriers:
                //getopt_long() automatically stores the argument passed in 'optarg'
                //store the barrier type into a variable for future use
                barrierType = optarg; 
                barrierFlag = true;
            break;

            case 't':
                //number of threads
                num_threads = atoi(optarg);
            break;
                
            case 'i':
                //number of iterations
                num_iterations = atoi(optarg);
            break;

            //for any other command
            //should never come here, ideally
            default:
                //user entered invalid argument
                cout << "Invalid argument." << endl;
            break;
        }

    }

    if (!nameflag)
    {
        lock_algs_t alg; barrier_types_t bar; 
        determine_lock_and_barrier(lockType, barrierType, &alg, &bar);
        int numThreads = determine_numThreads(lockType, num_threads);
        int count = counter(alg, bar, barrierFlag, num_iterations, numThreads);
        //write sorted list to file 
        writeToFile(op_filename, count);
        printTimeDifference();
    }

    return 0;
}