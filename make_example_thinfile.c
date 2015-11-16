#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#define DEF_SAMP_SIZE             4
#define DEF_N_MINOR_PER_MAJOR     65536 //16-bit
#define DEF_MAJOR                 100
#define DEF_VERBOSE               0 
#define DEF_HBYTES                0
#define DEF_FBYTES                0
//function declarations
void vUsage(void);

int main(int argc, char * argv[])
{
  //output file stuff

  char         szOutFile[512];
  FILE       * psuOutFile;
  int          iArgIdx;

  int64_t      llSampleSize;         //Sample size (in bytes)

  uint64_t     ullhBytes;
  uint64_t     ullfBytes;
  uint64_t     ullhVal;
  uint64_t     ullfVal;

  uint64_t     ullMinorCount;
  uint64_t     ullMajorCount;
  uint64_t     ullNMinorPerMajor;
  uint64_t     ullNMajor;

  uint64_t     ullBytesWritten;
  uint64_t     ullTotBytesWritten;
  uint64_t     ullTotSampleCount;

  uint8_t      bVerbose;

  
  llSampleSize           = DEF_SAMP_SIZE;

  ullhBytes              = 0;
  ullfBytes              = 0;
  ullhVal                = 0x48;
  ullfVal                = 0x46;

  ullMinorCount          = 0;
  ullMajorCount          = 0;
  ullNMinorPerMajor      = DEF_N_MINOR_PER_MAJOR;
  ullNMajor              = DEF_MAJOR;
  bVerbose               = DEF_VERBOSE;
		         
  ullBytesWritten        = 0;
  ullTotBytesWritten     = 0;
  ullTotSampleCount      = 0;

  szOutFile[0]  = '\0';

  //read command line params
  if (argc < 2) 
    {
      vUsage();
      return EXIT_SUCCESS;
    }

  for (iArgIdx=1; iArgIdx<argc; iArgIdx++) 
  {

    switch (argv[iArgIdx][0]) 
    {
    case '-' :
      switch (argv[iArgIdx][1]) 
      {
        case 's' :                   /* Sample size */
	  iArgIdx++;
	  if(iArgIdx >= argc)
	  {
	    vUsage();
	    return EXIT_FAILURE;
	  }
	  sscanf(argv[iArgIdx],"%" PRIi64 ,&llSampleSize);
	  break;

        case 'M' :                   /* N minor per major */
	  iArgIdx++;
	  if(iArgIdx >= argc)
	  {
	    vUsage();
	    return EXIT_FAILURE;
	  }
	  sscanf(argv[iArgIdx],"%" PRIi64 ,&ullNMinorPerMajor);
	  break;

        case 'N' :                   /* N major */
	  iArgIdx++;
	  if(iArgIdx >= argc)
	  {
	    vUsage();
	    return EXIT_FAILURE;
	  }
	  sscanf(argv[iArgIdx],"%" PRIi64 ,&ullNMajor);
	  break;

        case 'H' :                   /* Header bytes to skip */
	  iArgIdx++;
	  if(iArgIdx >= argc)
	  {
	    vUsage();
	    return EXIT_FAILURE;
	  }
	  sscanf(argv[iArgIdx],"%" PRIi64 ,&ullhBytes);
	  break;

        case 'F' :                   /* Footer bytes to skip */
	  iArgIdx++;
	  if(iArgIdx >= argc)
	  {
	    vUsage();
	    return EXIT_FAILURE;
	  }
	  sscanf(argv[iArgIdx],"%" PRIi64 ,&ullfBytes);
	  break;
	  
        case 'v' :                  /* Verbosities */
	  bVerbose = 1;
	  break;
	      
        case 'h' :                  /* Verbosities */
	  vUsage();
	  return EXIT_SUCCESS;
	  break;

        default :
	  break;
      } /* end flag switch */
      break;

      default :
	if (szOutFile[0] == '\0') strcpy(szOutFile, argv[iArgIdx]);
	break;

    } /* end command line arg switch */
  }

  //Make sure command-line params are sane
  if (ullhBytes > 65536) {
    printf("Header bytes can't exceed 65536!\n");
    printf("Try running with a lower value of header bytes\n");
    return EXIT_FAILURE;
  } else {
    if (ullhBytes > 0 ) printf ("N header bytes        : %" PRIu64 "\n",ullhBytes);
  }
  
  if (ullfBytes > 65536) {
    printf("Footer bytes can't exceed 65536!\n");
    printf("Try running with a lower value of footer bytes\n");
    return EXIT_FAILURE;
  } else {
    if (ullfBytes > 0 ) printf ("N footer bytes        : %" PRIu64 "\n",ullfBytes);
  }

  if ( llSampleSize > 8 || llSampleSize <= 0 ) {
    printf("Invalid sample size: %" PRIi64 "\n",llSampleSize);
    printf("Sample size must be between 1 and 8!\n");
    return EXIT_FAILURE;
  } else {
    printf("Sample size           : %" PRIi64 " bytes\n",llSampleSize);
  }

  //Calc sample size
  int64_t idx;
  //  ullNMinorPerMajor = 1;
  //  for ( idx = 0; idx < llSampleSize; idx++ ) ullNMinorPerMajor *= 2;
  if (bVerbose) {
    printf("N samples per major count: %" PRIu64 "\n",ullNMinorPerMajor);
    printf("%" PRIi64 " bits\n",llSampleSize);
  }

  printf("N minor per major     : %" PRIu64 "\n",ullNMinorPerMajor);
  printf("N major               : %" PRIu64 "\n",ullNMajor);

  if ( bVerbose) printf("Opening %s...\n",szOutFile);
  psuOutFile             = fopen(szOutFile,"wb");
  if (psuOutFile == NULL) 
  {
    fprintf(stderr, "Error opening output file\n");
    return EXIT_FAILURE;
  }  
  printf("Output file           : %s\n", szOutFile);

  
  //write header bytes
  if (ullhBytes > 0) {
    ullBytesWritten = 0;
    for ( idx = 0; idx < ullhBytes; idx++ ) {
      ullBytesWritten += fwrite(&ullhVal, 1, 1, psuOutFile);
    }
    ullTotBytesWritten += ullBytesWritten;
    if ( bVerbose ) printf("Wrote %" PRIu64 " header bytes with value %" PRIu64 "\n",ullBytesWritten,ullhVal);
  }
    
  do {
    
    ullBytesWritten = fwrite(&ullMajorCount, 1, llSampleSize, psuOutFile);
    for ( ullMinorCount = 1; ullMinorCount < ullNMinorPerMajor; ullMinorCount++ ) {
      ullBytesWritten += fwrite(&ullMinorCount, 1, llSampleSize, psuOutFile);
    }

    ullMajorCount++;
    ullTotBytesWritten    += ullBytesWritten;
    ullTotSampleCount     += ullNMinorPerMajor;
    
  } while ( ullMajorCount < ullNMajor );
  printf("Wrote %" PRIu64 " samples (%" PRIu64 " bytes)\n", \
	 ullTotSampleCount,ullTotSampleCount*llSampleSize);
  
  //write footer bytes
  if (ullfBytes > 0) {
    ullBytesWritten = 0;
    for ( idx = 0; idx < ullfBytes; idx++ ) {
      ullBytesWritten += fwrite(&ullfVal, 1, 1, psuOutFile);
    }
    ullTotBytesWritten += ullBytesWritten;
    if ( bVerbose ) printf("Wrote %" PRIu64 " footer bytes with value %" PRIu64 "\n",ullBytesWritten,ullfVal);
  }

  printf("******************************\n");
  printf("Wrote %" PRIu64 " bytes in total to %s.\n", \
	 ullTotSampleCount*llSampleSize+ullhBytes+ullfBytes,szOutFile);

  fclose(psuOutFile);

  return EXIT_SUCCESS;
}


void vUsage(void)
{
  printf("\nmake_example_thinfile                                                    \n");
  printf("Create an example binary file to be chopped up by thinfile.                \n");
  printf("Usage: make_example_thinfile <output file> [flags]                         \n");
  printf("                                                                           \n");
  printf("   <output file>   output filename                                         \n");
  printf("                                                                           \n");
  printf("   INPUT PARAMETERS                                                        \n");
  printf("   -s SIZE                Size of samples                  (in bytes) [%i] \n",DEF_SAMP_SIZE);
  printf("   -M N_MINOR_PER_MAJOR   Number of minor counts per major            [%i] \n",DEF_N_MINOR_PER_MAJOR);
  printf("   -N N_MAJOR             Number of major to write                    [%i] \n",DEF_MAJOR);
  //  printf("   -S RATE      Sample Rate                        (in S/s)   [%i]    \n",DEF_SAMP_RATE);
  //  printf("                                                                      \n");
  //  printf("   OUTPUT FILE PARAMETERS                                             \n");
  //  printf("   -t INTERVAL  Thinning interval                  (in ms)    [%i]    \n",DEF_THIN_INTERVAL);
  //  printf("   -n NUM       # samples to grab                             [%i]    \n",DEF_NUM_SAMPS);
  printf("                                                                           \n");
  printf("   OPTIONAL PARAMETERS                                                     \n");
  printf("   -H HBYTES    Size of header to write                    (in bytes) [%i] \n",DEF_HBYTES);
  printf("   -F FBYTES    Size of footer to write                    (in bytes) [%i] \n",DEF_FBYTES);
  //  printf("   -i ITER      Total number of thinning intervals            [%i]    \n",DEF_NUM_ITER);
  printf("   -v           Verbose                                               [%i] \n",DEF_VERBOSE);
  printf("   -h           Help                                                       \n");
  printf("                                                                           \n");
  printf("The structure of the file produced is as follows:                          \n");
  printf("[SAMPLE TYPE]                                :  [SAMPLE VALUE]             \n");
  printf("Header 0                                     :  [0x48, ASCII 'H']          \n");
  printf("...                                          :  ...                        \n");
  printf("Header HBYTES-1                              :  [0x48]                     \n");
  printf("Sample 0                                     :  [Major_count]              \n");
  printf("Sample 1                                     :  [1]                        \n");
  printf("Sample 2                                     :  [2]                        \n");
  printf("...                                          :  ...                        \n");
  printf("Sample (N MOD N_MINOR_PER_MAJOR)             :  [Major_count]              \n");
  printf("Sample (N + 1 MOD N_MINOR_PER_MAJOR)         :  [1]                        \n");
  printf("Sample (N + 2 MOD N_MINOR_PER_MAJOR)         :  [2]                        \n");
  printf("Footer 0                                     :  [0x46, ASCII 'F']          \n");
  printf("...                                          :  ...                        \n");
  printf("Header FBYTES-1                              :  [0x46]                     \n");
  printf("                                                                           \n");
  printf("Total file size is HBYTES+(N_MINOR_PER_MAJOR*N_MAJOR)*SAMPLE_SIZE+FBYTES   \n");
}
