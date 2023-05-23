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

#include "bucketsort.h"
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
const char *short_cmd_options = "o:t:";

/*--------------------------------------------------------------------*/
/*
 * @brief       :   Reads all numbers from a file
 * 
 * @parameters  :   string
 *                      inputFile   :   path to the file   
 *                  vector <int>&
 *                      list        :   array in which numbers are to be stored
 *
 * @returns     :   none, exits with EXIT_FAILURE if file opening fails
 */
void readFromFile (string inputFile, vector <int>& list)
{
    //create instance of ifstream to read file
    ifstream fin;
    //local variable to store value read from file
    int val; 

    //open the file and check if file opening succeeded 
    fin.open(inputFile);
    if(!fin)
    {
        //if file opening failed, exit
        cout << "Failed to open file, exiting ..." << endl;
        exit(EXIT_FAILURE);
    }

    //read input values from file
    while(true)
    {
        //store the value read from file in a local variable
        fin >> val;

        //check for end of file
        if (fin.eof())
        {
            break;
        }
        //push the value in vector list
        list.push_back(val);
    }
    //close file
    fin.close();
}
/*--------------------------------------------------------------------*/
/*
 * @brief       :   writes the sorted list to a file
 * 
 * @parameters  :   string
 *                      outputFile   :   path to the file   
 *                  vector <int>&
 *                      list         :   sorted list to be written
 *
 * @returns     :   none, exits with EXIT_FAILURE if file opening fails
 */
void writeToFile (string outputFile, vector <int>& list)
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
    //create an iterator to go through all elements in the vector list
    vector<int>::iterator itr = list.begin();
    //iterate through all elements 
    for(itr = list.begin(); itr != list.end(); itr++)
    {
        //write to file, and add a new line at the end
        fout << *itr << endl;
    }
    //close the file
    fout.close();
}
/*--------------------------------------------------------------------*/
/*
 * @brief       :   sort the input list
 * 
 * @parameters  :   string
 *                      locktype       :  type of locking algorithm to be used
 *                      barriertype    :  type of barrier to be used 
 *                  vector <int>&
 *                      num_list     :   list of numbers to be sorted
 *                  int         
 *                      list_size    :   number of elements in the list
 *                      num_threads  :   number of threads to be used in the application
 *
 * @returns     :   none
 */
void sort_list(string lockingType, string barrierType, vector<int>& num_list, int list_size, int num_threads)
{
    //local variable for number of threads
    int numThreads = 0;

    /*-------------------------------------------------------------------------*/
    /*first determine number of threads to use*/
    //if num_threads are not provided by user, continue with 4 threads
    if (num_threads == 0)
    {
        numThreads = DEFAULT_NUMTHREADS;
    }
    //if num_threads are greater than list size, just use half of the threads
    else if (num_threads >= (list_size/2))
    {
        numThreads = (num_threads/2);
        //if half = 0, go ahead with single thread
        if (numThreads == 0)
        {
            numThreads++;
        }
    }
    else if (lockingType == "petersonseq" || lockingType == "petersonrel")
    {
        //peterson's algorithm requires only 2 threads
        numThreads = 2;
    }
    //otherwise, just use number of threads provided by user
    else
    {
        numThreads = num_threads;
    }
    /*-------------------------------------------------------------------------*/
    /*now determine the locking algorithm to be sent to bucketsort*/
    lock_algs_t alg = PTHREAD_algorithm;     //default algorithm is pthread 

    if (lockingType == "tas")
    {
        alg = TAS_algorithm;
    }
    else if (lockingType == "ttas")
    {
        alg = TTAS_algorithm;
    }
    else if (lockingType == "ticket")
    {
        alg = TICKETLOCK_algorithm;
    }
    else if (lockingType == "mcs")
    {
        alg = MCS_algorithm;
    }
    else if (lockingType == "petersonseq")
    {
        alg = PETERSON_SEQ_algorithm;
    }
    else if (lockingType == "petersonrel")
    {
        alg = PETERSON_REL_algorithm;
    }
    else    /*default algorithm is pthread*/
    {
        alg = PTHREAD_algorithm;
    }
    
    /*-------------------------------------------------------------------------*/
    /*now determine the barrier type to be sent to bucketsort*/
    barrier_types_t bar = PTHREAD_type;     //default type is pthread 
    if (barrierType == "sense")
    {
        bar = SENSE_REV_type;
    }
    else
    {
        bar = PTHREAD_type;
    }
    /*-------------------------------------------------------------------------*/
    //now, send everything to bucketsort
    bucketsort(alg, bar, num_list, list_size, numThreads);
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

    //input file name, from which list is to be read
    string ip_filename = argv[1];

    //output file name, to which sorted list is to be written
    string op_filename;

    string lockType, barrierType; 

    //number of threads
    int num_threads=0;

    bool nameflag = false;

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
            break;

            case 't':
                num_threads = atoi(optarg);
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
        //declare a vector list to hold input numbers
        vector <int> num_list;

        //get numbers from the input file
        readFromFile(ip_filename, num_list);

        //determine list size
        int list_size = num_list.size();

        //sort the list based on sorting method selected
        sort_list(lockType, barrierType, num_list, list_size, num_threads);

        //write sorted list to file 
        writeToFile(op_filename, num_list);

        printTimeDifference();
    }

    return 0;
}
/*--------------------------------------------------------------------*/
/*EOF*/