
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <sys/stat.h>

#define DEF_SAMP_RATE     20000000
#define DEF_SAMP_SIZE     1
#define DEF_THIN_INTERVAL 1000
#define DEF_NUM_SAMPS     8192
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
  uint64_t     ullNInterval;
  
  int          iArgIdx;
  uint64_t     ullSampleRate;         //Sampling rate used to produce infile (in Hz)
  uint64_t     ullSampleSize;         //Sample size (in bytes)
  uint64_t     ullThinInterval;       //Periodic interval at which to grab a given number of samples
  uint64_t     ullNumSamps;           //Number of samples to grab at each periodic interval
  uint8_t      bVerbose;

  uint64_t     ullInFilePos;
  uint64_t     ullSampsRead;
  uint64_t     ullTotSampsRead;
  uint64_t     ullSampsWritten;
  
  //Initialize vars
  ullSampleRate   = DEF_SAMP_RATE;
  ullSampleSize   = DEF_SAMP_SIZE;
  ullThinInterval = DEF_THIN_INTERVAL;
  ullNumSamps     = DEF_NUM_SAMPS;
  bVerbose        = DEF_VERBOSE;
  
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
	      sscanf(argv[iArgIdx],"%" PRIu64 ,&ullSampleSize);
	      break;

	    case 'S' :                   /* Sample rate */
	      iArgIdx++;
	      if(iArgIdx >= argc)
		{
		  vUsage();
		  return EXIT_FAILURE;
		}
	      sscanf(argv[iArgIdx],"%" PRIu64 ,&ullSampleRate);
	      break;

	    case 't' :                   /* Thinning Interval */
	      iArgIdx++;
	      if(iArgIdx >= argc)
		{
		  vUsage();
		  return EXIT_FAILURE;
		}
	      sscanf(argv[iArgIdx],"%" PRIu64 ,&ullThinInterval);
	      break;

	    case 'n' :                   /* # Samples */
	      iArgIdx++;
	      if(iArgIdx >= argc)
		{
		  vUsage();
		  return EXIT_FAILURE;
		}
	      sscanf(argv[iArgIdx],"%" PRIu64 ,&ullNumSamps);
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
  printf("Sample size       :\t%" PRIu64 " bytes\n",ullSampleSize);
  printf("Sample rate       :\t%" PRIu64 " S/s\n",ullSampleRate);
  printf("Thinning interval :\t%" PRIu64 " ms\n",ullThinInterval);
  printf("Number of samples :\t%" PRIu64 " \n",ullNumSamps);
  
  //Calculate size of buffer that we need
  pauBuff = calloc( ullNumSamps, ullSampleSize );

  //Based on sample rate (S/s) and thinning interval (ms),
  //calculate number of samples between each interval
  ullNInterval = ullSampleRate * ullThinInterval / 1000;

  printf("# Samples/Interval:\t%" PRIu64 " \n",ullNInterval);
  printf("\n");

  //Can't allow more samples to be grabbed than are in an interval
  if (ullNInterval < ullNumSamps )
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
  ullInFilePos = fseek(psuInFile, 0, SEEK_SET);
  ullSampsRead = 0;
  ullTotSampsRead = 0; 
  ullSampsWritten = 0;
  
  printf("\nThinning %s...\n", szInFile);
 
  while( ( ullInFilePos = ftell(psuInFile) ) < suInFileStat.st_size  )
    {
      //Get samples from infile
      ullSampsRead = fread(pauBuff, ullSampleSize, ullNumSamps, psuInFile);
      if (ullSampsRead != ullSampleSize * ullNumSamps )
	{
	  fprintf(stderr,"Only read %" PRIu64 " bytes of %" PRIu64 " requested!\nEOF?\n",ullSampsRead,ullSampleSize*ullNumSamps);
	  break;
	}

      //Write samples to outfile

      ullSampsWritten = fwrite(pauBuff, ullSampleSize, ullNumSamps, psuOutFile);
      if ( ullSampsWritten != ullSampsRead )
	{
	  fprintf(stderr,"Only wrote %" PRIu64 " bytes of %" PRIu64 " requested!\nWrite error?\n",ullSampsWritten,ullSampsRead);
	  break;
	}
      
      if (bVerbose)
	printf("Read %" PRIu64 " bytes\n",ullInFilePos);

      ullTotSampsRead += ullSampsRead;

      fseek(psuInFile, ullNInterval , SEEK_CUR );

    }
  

  //Summary
  printf("Wrote %" PRIu64 " samples out of %" PRIu64 " samples total\n", ullTotSampsRead, suInFileStat.st_size/ullSampleSize);
  printf("Output file is %.9f%% smaller than input file\n", (float)((float)( ullTotSampsRead * ullSampleSize )/(float)(suInFileStat.st_size)));

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
    printf("                                                     \n");
    printf("   <filename>   Input/output file names              \n");
    printf("                                                     \n");
    printf("   INPUT FILE PARAMS                                 \n");
    printf("   -s SIZE      Size of samples   (in bytes) [%i]    \n",DEF_SAMP_SIZE);
    printf("   -S RATE      Sample Rate       (in S/s)   [%i]    \n",DEF_SAMP_RATE);
    printf("                                                     \n");
    printf("   OUTPUT FILE PARAMS                                \n");
    printf("   -t INTERVAL  Thinning interval (in ms)    [%i]    \n",DEF_THIN_INTERVAL);
    printf("   -n NUM       # samples to grab            [%i]    \n",DEF_NUM_SAMPS);
    printf("                                                     \n");
    printf("   -v           Verbose                      [%i]    \n",DEF_VERBOSE);
    printf("                                                     \n");
}
