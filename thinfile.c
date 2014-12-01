
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <sys/stat.h>

#define DEF_SAMP_RATE     20000000
#define DEF_SAMP_SIZE     1
#define DEF_THIN_INTERVAL 1000
#define DEF_NUM_SAMPS     8192

#define DEF_SKIP          0
#define DEF_NUM_ITER      0

#define DEF_VERBOSE       0

//function declarations
void vUsage(void);

int main( int argc, char * argv[] )
{
  //input/output file stuff
  char         szInFile[512];
  char         szOutFile[512];

  FILE       * psuInFile;
  struct stat  suInFileStat;

  FILE       * psuOutFile;
  char       * pauBuff;
  uint64_t     ullInterval;
  
  int          iArgIdx;
  int64_t      llSampleRate;         //Sampling rate used to produce infile (in Hz)
  int64_t      llSampleSize;         //Sample size (in bytes)
  int64_t      llThinInterval;       //Periodic interval at which to grab a given number of samples
  int64_t      llNumSamps;           //Number of samples to grab at each periodic interval

  int64_t      llHeaderSkipBytes;
  int64_t      llFooterSkipBytes;
  int64_t      llNumIterations;

  uint8_t      bVerbose;

  uint64_t     ullInFilePos;
  uint64_t     ullBytesRead;
  uint64_t     ullSampsRead;
  uint64_t     ullTotBytesRead;
  uint64_t     ullBytesWritten;
  
  //Initialize vars
  llSampleRate      = DEF_SAMP_RATE;
  llSampleSize      = DEF_SAMP_SIZE;
  llThinInterval    = DEF_THIN_INTERVAL;
  llNumSamps        = DEF_NUM_SAMPS;
  
  llHeaderSkipBytes = DEF_SKIP;
  llFooterSkipBytes = DEF_SKIP;
  llNumIterations   = DEF_NUM_ITER;

  bVerbose          = DEF_VERBOSE;

  if (argc < 2) 
    {
      vUsage();
      return EXIT_SUCCESS;
    }

  szInFile[0]  = '\0';
  strcpy(szOutFile,"");                // Default is stdout

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

	    case 'S' :                   /* Sample rate */
	      iArgIdx++;
	      if(iArgIdx >= argc)
		{
		  vUsage();
		  return EXIT_FAILURE;
		}
	      sscanf(argv[iArgIdx],"%" PRIi64 ,&llSampleRate);
	      break;

	    case 't' :                   /* Thinning Interval */
	      iArgIdx++;
	      if(iArgIdx >= argc)
		{
		  vUsage();
		  return EXIT_FAILURE;
		}
	      sscanf(argv[iArgIdx],"%" PRIi64 ,&llThinInterval);
	      break;

	    case 'n' :                   /* # Samples */
	      iArgIdx++;
	      if(iArgIdx >= argc)
		{
		  vUsage();
		  return EXIT_FAILURE;
		}
	      sscanf(argv[iArgIdx],"%" PRIi64 ,&llNumSamps);
	      break;

	    case 'h' :                   /* Header bytes to skip */
	      iArgIdx++;
	      if(iArgIdx >= argc)
		{
		  vUsage();
		  return EXIT_FAILURE;
		}
	      sscanf(argv[iArgIdx],"%" PRIi64 ,&llHeaderSkipBytes);
	      break;

	    case 'f' :                   /* Footer bytes to skip */
	      iArgIdx++;
	      if(iArgIdx >= argc)
		{
		  vUsage();
		  return EXIT_FAILURE;
		}
	      sscanf(argv[iArgIdx],"%" PRIi64 ,&llFooterSkipBytes);
	      break;

	    case 'i' :                   /* # Iterations */
	      iArgIdx++;
	      if(iArgIdx >= argc)
		{
		  vUsage();
		  return EXIT_FAILURE;
		}
	      sscanf(argv[iArgIdx],"%" PRIi64 ,&llNumIterations);
	      break;

	    case 'v' :                  /* Verbosities */
	      bVerbose = 1;
	      break;
	      
	    default :
	      break;
	    } /* end flag switch */
	  break;

	default :
	  if (szInFile[0] == '\0') strcpy(szInFile, argv[iArgIdx]);
	  else                     strcpy(szOutFile,argv[iArgIdx]);
	  break;

        } /* end command line arg switch */
    } /* end for all arguments */

  printf("\n");
  printf("Sample size            :\t%" PRIi64 " bytes\n",llSampleSize);
  printf("Sample rate            :\t%" PRIi64 " S/s\n",llSampleRate);
  printf("Thinning interval      :\t%" PRIi64 " ms\n",llThinInterval);
  printf("Number of samples      :\t%" PRIi64 " \n",llNumSamps);
  
  if ( llHeaderSkipBytes || llFooterSkipBytes )
  {
    printf("\n");
    printf("Header bytes to skip :\t%" PRIi64 " bytes\n",llHeaderSkipBytes);
    printf("Footer bytes to skip :\t%" PRIi64 " bytes\n",llFooterSkipBytes);    
  }

  if ( llNumIterations != 0 )
  {
    printf("Number of iterations :\t%" PRIi64 " bytes\n",llNumIterations);    
  }

  if ( llNumIterations < 0 || llSampleSize <= 0 || llThinInterval <= 0 || llNumSamps <= 0 || llHeaderSkipBytes < 0 || llFooterSkipBytes < 0 )
  {
    fprintf(stderr,"Invalid argument provided! Exiting...\n");
    return EXIT_FAILURE;
  }

  //Calculate size of buffer that we need
  pauBuff = calloc( llNumSamps, llSampleSize );

  //Based on sample rate (S/s) and thinning interval (ms),
  //calculate number of samples between each interval
  ullInterval = llSampleRate * llThinInterval / 1000;

  printf("# Samples/Interval:\t%" PRIu64 " \n",ullInterval);
  printf("\n");

  //Can't allow more samples to be grabbed than are in an interval
  if (ullInterval < llNumSamps )
    {
      fprintf(stderr,"Number of samples requested is greater than samples per interval!\nQuitting...\n");
      return EXIT_FAILURE;
    }
  
  //Open input file
  if (strlen(szInFile)==0) 
    {
      vUsage();
      return EXIT_FAILURE;
    }

  psuInFile = fopen(szInFile,"rb");
  if (psuInFile == NULL) 
    {
      fprintf(stderr, "Error opening input file\n");
      return EXIT_FAILURE;
    }  
  printf("Input file: %s\n", szInFile);

  //Get input file stats
  stat(szInFile, &suInFileStat);

  // If output file specified then open it    
  if (strlen(szOutFile) != 0)
    {
      psuOutFile = fopen(szOutFile,"w");
      if (psuOutFile == NULL) 
	{
	  fprintf(stderr, "Error opening output file\n");
	  return EXIT_FAILURE;
	}
        
      printf("Output file: %s\n", szInFile);
    }
  else  // No output file name so use stdout
    {
      psuOutFile = stdout;
    }

  //To the beginning!
  ullInFilePos = fseek(psuInFile, llHeaderSkipBytes, SEEK_SET);   //If header bytes given, skip those; otherwise, go to zero
  ullBytesRead = 0;
  ullSampsRead = 0;
  ullTotBytesRead = 0; 
  ullBytesWritten = 0;


  if ( llFooterSkipBytes )
  {
    suInFileStat.st_size -= llFooterSkipBytes;
  }
  
  
  printf("\nThinning %s...\n", szInFile);
 
  while( ( ullInFilePos = ftell(psuInFile) ) < suInFileStat.st_size  )
    {
      //Get samples from infile
      ullBytesRead = fread(pauBuff, llSampleSize, llNumSamps, psuInFile);
      if (ullBytesRead != llSampleSize * llNumSamps )
	{
	  fprintf(stderr,"Only read %" PRIu64 " bytes of %" PRIu64 " requested!\nEOF?\n",ullBytesRead,llSampleSize*llNumSamps);
	  break;
	}

      //Write samples to outfile
      ullBytesWritten = fwrite(pauBuff, llSampleSize, llNumSamps, psuOutFile);
      if ( ullBytesWritten != ullBytesRead )
	{
	  fprintf(stderr,"Only wrote %" PRIu64 " bytes of %" PRIu64 " requested!\nWrite error?\n",ullBytesWritten,ullBytesRead);
	  break;
	}
      
      if (bVerbose)
	printf("Read %" PRIu64 " bytes (Sample #%" PRIu64 ")\n",ullInFilePos,ullSampsRead);

      ullTotBytesRead += ullBytesRead;
      ullSampsRead++;

      fseek(psuInFile, ullInterval , SEEK_CUR );

      if( (llNumIterations != 0 ) && ( ullSampsRead >= llNumIterations ) )
      {
	printf("Read %" PRIu64 " samples total! Breaking out of loop...\n", ullSampsRead);
	break;
      }
    }
  

  //Summary
  printf("Wrote %" PRIu64 " bytes out of %" PRIu64 " bytes total\n", ullTotBytesRead, (suInFileStat.st_size - llHeaderSkipBytes));
  printf("Output file is %.9f%% smaller than input file\n", (float)((float)( ullTotBytesRead * llSampleSize )/(float)(suInFileStat.st_size-llHeaderSkipBytes)));

  //close files
  fclose(psuInFile);
  fclose(psuOutFile);
  
  return EXIT_SUCCESS;
}
  
void vUsage(void)
{
  printf("\nthinfile\n");
  printf("Cherry-pick a subset of samples at a user-specified interval to \"thin\" binary files\n");
  printf("Usage: thinfile <input file> <output file> [flags]   \n");
  printf("                                                                      \n");
  printf("   <filename>   Input/output file names                               \n");
  printf("                                                                      \n");
  printf("   INPUT FILE PARAMETERS                                              \n");
  printf("   -s SIZE      Size of samples                    (in bytes) [%i]    \n",DEF_SAMP_SIZE);
  printf("   -S RATE      Sample Rate                        (in S/s)   [%i]    \n",DEF_SAMP_RATE);
  printf("                                                                      \n");
  printf("   OUTPUT FILE PARAMETERS                                             \n");
  printf("   -t INTERVAL  Thinning interval                  (in ms)    [%i]    \n",DEF_THIN_INTERVAL);
  printf("   -n NUM       # samples to grab                             [%i]    \n",DEF_NUM_SAMPS);
  printf("                                                                      \n");
  printf("   OPTIONAL PARAMETERS                                                \n");
  printf("   -h HBYTES    Size of header                     (in bytes) [%i]    \n",DEF_SKIP);
  printf("   -f HBYTES    Size of footer                     (in bytes) [%i]    \n",DEF_SKIP);
  printf("   -i ITER      Total number of thinning intervals            [%i]    \n",DEF_NUM_ITER);
  printf("   -v           Verbose                                       [%i]    \n",DEF_VERBOSE);
  printf("                                                                      \n");
}
