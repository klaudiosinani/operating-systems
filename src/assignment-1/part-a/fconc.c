#include <stdio.h>
#include <string.h>

#include <unistd.h>
/*  A header file that defines miscellaneous symbolic constants and types,
 *  and declares miscellaneous functions. */

#include <stdlib.h>
/* A header file of the general purpose standard library of C programming
 * language which includes functions involving memory allocation, process control, conversions and others. */

#include <sys/stat.h>
/* A header file that define the structure of the data returned by the functions
 * fstat(), lstat(), and stat(). */

#include <fcntl.h>
/* A header file that defines file control options */

#include <errno.h>
/* A header file that defines macros for reporting and retrieving error conditions
 * through error codes stored in a static memory location called errno (short for "error number"). */


#define BUF_SIZE 100
#define OPENFLAGS  O_WRONLY | O_CREAT | O_TRUNC
/* O_WRONLY: Open the file for writing only
*     O_CREAT: Create file if it doesn’t already exist
*     O_TRUNC: Truncate existing file to zero length  */

#define FILEPERMS S_IRUSR | S_IWUSR
//The Mode Bits for Access Permission
/* S_IRUSR: Read permission bit for the owner of the file.
*     S_IWUSR: Write permission bit for the owner of the file. */


void check_input (int argc) {
/* This function simply checks if the
* total number of our input arguments
* is 3 or 4. If not, an error message appears
* on the screen */
  if (argc < 3) {
      printf("Usage: ./fconc infile1 infile2 [outfile (default: focnc.out)]\n");
      exit(EXIT_FAILURE); // EXIT_FAILURE is the standard value for returning unsuccessful termination.
  }
  else if (argc > 4) {
      printf("Usage: ./fconc infile1 infile2 [outfile (default: focnc.out)]\n");
      exit(EXIT_FAILURE); // EXIT_FAILURE is the standard value for returning unsuccessful termination.
  }
}

void overwrite_prevent (int argc, char **argv) {
// We wont allow the user to set an input fill as an outfile too
  if (argc == 4) {
    if ((strcmp(argv[1],argv[3]) == 0 ) || (strcmp(argv[2],argv[3]) == 0)) {
      printf("Please do not use any input files as output\n");
      exit(EXIT_FAILURE);
    }
  }
}

int open_input (char *File) {
/* The opening process of an
* input file officially starts
* by using this function */
  int inputFd = open(File, O_RDONLY);
  if (inputFd == -1) {
  // The IF statements is used to check
  // whether or not the input files exist
    perror(File);
    close(inputFd);     // Shutdown the input file
    exit(EXIT_FAILURE);
  }
  else
    return inputFd;
}

int open_output (int argc, char **argv) {
// Open the output file that we want to write into it.
// If an output file was not given as an argument,
// it will be created from scratch,
// due to the O_CREAT flag value
  if (argc == 4) {
  /* Covering the case where the user supplied
   * us with an output file argument
   * This means that we don't have to
   * create a new output file */
    int outputFd = open(argv[3], OPENFLAGS, FILEPERMS);
    /* The output file has an index of 3,
     * since the counting takes place with
     * 0 as the initial value, value which
     * corresponds to the executable file itself */
    if (outputFd == -1) {
      perror(argv[3]);
      close(outputFd);
      exit(EXIT_FAILURE);
    }
    else
      return outputFd;
  }
  else {
  /* Covering the case where the user did not
   * supply us with an output file argument
   * This means that we have to
   * create a new output file named fconc.out */
    int outputFd = open("fconc.out", OPENFLAGS, FILEPERMS);
    if (outputFd == -1) {
      perror("fconc.out");
      exit(EXIT_FAILURE);
    }
    else
      return outputFd;
  }
}

void shutdown_input (int descriptor, char *inputFile) {
// Shutdown the input file, since we got all we wanted
// The manipulation process of the input file officially ends
  if (close(descriptor) == -1) {
    perror(inputFile);
    exit(EXIT_FAILURE);
  }
}

void shutdown_output (int File) {
// Shutdown the final output file. We are done
  if (close(File) == -1) {
    perror("Output File");
    exit(EXIT_FAILURE);
  }
}

void doWrite (int outputFd, char *buf, int leng) {
  ssize_t dataWrite = write(outputFd, buf, leng);
  /* This data type is used to represent the sizes of blocks
   * that can be read or written in a single operation.
   * It is similar to size_t, but must be a signed type. */
  ssize_t dataMonitor = 0;
  /* A medium variable that holds the true
   * amount of data written and is used in order
   * to keep the writting-reading process in check */
  if (dataWrite == -1) {
    perror("Output File");
    close(outputFd);
    exit(EXIT_FAILURE);
  }
  else if (dataWrite != leng) {
  /* Here we cover the crucial case where we
   * could not write the whole buffer.
   * The data-processing monitor value, dataMonitor,
   * prevents any possible undesired outcome  */
    dataMonitor += dataWrite;   // Increment dataMonitor by dataWrite
    while (leng != dataMonitor) {
    dataMonitor += write(outputFd, buf + dataMonitor, leng - dataMonitor);
    // The backup writting process takes place
      if (dataWrite == -1) {
        perror("Output File");
        close(outputFd);
        exit(EXIT_FAILURE);
      }
    }
  }
}

void write_file (int outputFd, char *inputFile) {
  int inputFd = open_input(inputFile);
  char buffer[BUF_SIZE+1];
  ssize_t dataRead = 1;
  while (dataRead != 0) {
    dataRead = read(inputFd, buffer, BUF_SIZE);
    /* Transfer data from the input
    * file to the final output file, until
    * we encounter end of input or an error */
    if (dataRead == -1) {   // Error has occurred
      perror(inputFile);
      close(inputFd);   // Shutdown the input file
      close(outputFd);  // Shutdown the output file
      exit(EXIT_FAILURE);
    }
    else {    // Everythings has gone smoothly well
      buffer[dataRead] = '\0';
      doWrite (outputFd, buffer, dataRead);
    }
  }
  shutdown_input(inputFd, inputFile);
}

int main (int argc, char **argv) {
  int fileDescriptor;
  check_input(argc);   // Check the number of arguments
  overwrite_prevent (argc,argv);
  fileDescriptor = open_output(argc, argv);
  write_file(fileDescriptor, argv[1]);
  write_file(fileDescriptor, argv[2]);
  shutdown_output(fileDescriptor);
  return 0;
}
