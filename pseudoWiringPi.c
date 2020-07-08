#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>

#include <sched.h>
#include <string.h>

#include "pseudoWiringPi.h"

static int waitForInterruptSTDIN_process = 0;

pthread_t myThread ;
static pthread_mutex_t piMutexes [4] ;

// Misc

static int wiringPiMode = WPI_MODE_UNINITIALISED ;
static pthread_mutex_t pinMutex ;


// Debugging & Return codes

int wiringPiDebug       = FALSE ;
int wiringPiReturnCodes = FALSE ;

// Time for easy calculations

static uint64_t epochMilli ;

// ISR Data

static void (*isrFunctions [64])(void) ;

static int columnaTecladoActiva = -1;
static char pseudoTecladoTL04[4][4] = {
	{'1', '2', '3', 'c'},
	{'4', '5', '6', 'd'},
	{'7', '8', '9', 'e'},
	{'a', '0', 'b', 'f'}
};

static int array_flags_check_columnas_teclado [4] = {0x01, 0x02, 0x04, 0x08};


/*
 * wiringPiFailure:
 *	Fail. Or not.
 *********************************************************************************
 */

int wiringPiFailure (int fatal, const char *message, ...)
{
  va_list argp ;
  char buffer [1024] ;

  if (!fatal && wiringPiReturnCodes)
    return -1 ;

  va_start (argp, message) ;
    vsnprintf (buffer, 1023, message, argp) ;
  va_end (argp) ;

  fprintf (stderr, "%s", buffer) ;
  exit (EXIT_FAILURE) ;

  return 0 ;
}

/*
 * wiringPiSetupGpio:
 *	Must be called once at the start of your program execution.
 *
 * GPIO setup: Initialises the system into GPIO Pin mode and uses the
 *	memory mapped hardware directly.
 *********************************************************************************
 */

int wiringPiSetupGpio (void)
{
  if (wiringPiDebug)
    printf ("wiringPi: wiringPiSetupGpio called\n") ;

  wiringPiMode = WPI_MODE_GPIO ;

  return 0 ;
}

/*
 * pinMode:
 *	Sets the mode of a pin to be input, output or PWM output
 *********************************************************************************
 */

void pinMode (int pin, int mode)
{
	if (wiringPiMode != WPI_MODE_GPIO)	// Sys mode
	{
	  printf("[pseudoWiringPi][ERROR!!!][Modo de configuración incorrecto!!!][Use wiringPiSetupGpio ()]\n");
	  fflush(stdout);
	  return;
	}

	if (mode == INPUT){
	  printf("[pseudoWiringPi][pinMode][pin %d][INPUT]\n", pin);
	  fflush(stdout);
	}
	else if (mode == OUTPUT){
	  printf("[pseudoWiringPi][pinMode][pin %d][OUTPUT]\n", pin);
	  fflush(stdout);
	}
}

/*
 * pullUpDownCtrl:
 *	Control the internal pull-up/down resistors on a GPIO pin.
 *********************************************************************************
 */

void pullUpDnControl (int pin, int pud)
{
  if ((pin & PI_GPIO_MASK) == 0)		// On-Board Pin
  {
	 if (wiringPiMode != WPI_MODE_GPIO)	// Sys mode
	  {
		printf("[pseudoWiringPi][ERROR!!!][Modo de configuración incorrecto!!!][Use wiringPiSetupGpio ()]\n");
		fflush(stdout);
		return;
	  }

		switch (pud)
		{
			case PUD_OFF:
				printf("[pseudoWiringPi][pullUpDnControl][pin %d][PUD_OFF]\n", pin);
				break;
			case PUD_UP:
				printf("[pseudoWiringPi][pullUpDnControl][pin %d][PUD_UP]\n", pin);
				break;
			case PUD_DOWN:
				printf("[pseudoWiringPi][pullUpDnControl][pin %d][PUD_DOWN]\n", pin);
				break;
			default:
				printf("[pseudoWiringPi][ERROR!!!][pullUpDnControl][Modo incorrecto!!!][Use PUD_OFF o PUD_DOWN]\n");
				fflush(stdout);
				return ; /* An illegal value */
		}
  }
}

/*
 * waitForInterrupt:
 *	Pi Specific.
 *	Wait for Interrupt on a GPIO pin.
 *	This is actually done via the /sys/class/gpio interface regardless of
 *	the wiringPi access mode in-use. Maybe sometime it might get a better
 *	way for a bit more efficiency.
 *********************************************************************************
 */

int waitForInterruptSTDIN (int mS)
{
  uint8_t c ;
  int i, flagsColumnsChecked;
  int pinesFilasTeclado[4] = {
		  GPIO_KEYBOARD_ROW_1,
		  GPIO_KEYBOARD_ROW_2,
		  GPIO_KEYBOARD_ROW_3,
		  GPIO_KEYBOARD_ROW_4};

  // Wait for it ...
  while(1) {
		delay(10); // Wiring Pi function that pauses program execution for at least 10 millisecond

		piLock (STD_IO_BUFFER_KEY);
		if(kbhit()) {
			c = kbread();
			piUnlock (STD_IO_BUFFER_KEY);
			break;
		}
		piUnlock (STD_IO_BUFFER_KEY);
  }

	flagsColumnsChecked = 0;
	while(flagsColumnsChecked<15) { // antes de tirar una pulsacion me aseguro de haber comprobado las 4 columnas
		piLock(KEYBOARD_KEY); // columnaTecladoActiva lo modifican los digitalWrite
		for(i=0;i<4;i++){
			if(c == pseudoTecladoTL04[i][columnaTecladoActiva]){
				piUnlock(KEYBOARD_KEY);
				isrFunctions [pinesFilasTeclado[i]] () ;
				return c;
			}
		}

		flagsColumnsChecked |= array_flags_check_columnas_teclado[columnaTecladoActiva];
		piUnlock(KEYBOARD_KEY);
		// delay para permitir el cambio de excitacion
		delay(5);
	}

	return c ;
}

/*
 * interruptHandler:
 *	This is a thread and gets started to wait for the interrupt we're
 *	hoping to catch. It will call the user-function when the interrupt
 *	fires.
 *********************************************************************************
 */

static void *interruptHandlerSTDIN (UNU void *arg)
{
  int myPin = -1;

  (void)piHiPri (55) ;	// Only effective if we run as root

  for (;;)
	myPin = waitForInterruptSTDIN (-1);

  return NULL ;
}

/*
 * piHiPri:
 *	Attempt to set a high priority schedulling for the running program
 *********************************************************************************
 */

int piHiPri (const int pri)
{
  struct sched_param sched ;

  memset (&sched, 0, sizeof(sched)) ;

  if (pri > sched_get_priority_max (SCHED_RR))
    sched.sched_priority = sched_get_priority_max (SCHED_RR) ;
  else
    sched.sched_priority = pri ;

  return sched_setscheduler (0, SCHED_RR, &sched) ;
}

/*
 * wiringPiISR:
 *	Pi Specific.
 *	Take the details and create an interrupt handler that will do a call-
 *	back to the user supplied function.
 *********************************************************************************
 */

int wiringPiISR (int pin, int mode, void (*function)(void))
{
  pthread_t threadId ;
  const char *modeS ;

  if ((pin < 0) || (pin > 63))
    return wiringPiFailure (WPI_FATAL, "wiringPiISR: pin must be 0-63 (%d)\n", pin) ;

  else if (wiringPiMode == WPI_MODE_UNINITIALISED)
    return wiringPiFailure (WPI_FATAL, "wiringPiISR: wiringPi has not been initialised. Unable to continue.\n") ;
  else if (wiringPiMode != WPI_MODE_GPIO)	// Sys mode
	  {
		printf("[pseudoWiringPi][ERROR!!!][Modo de configuración incorrecto!!!][Use wiringPiSetupGpio ()]\n");
		fflush(stdout);
		return wiringPiFailure (WPI_FATAL, "wiringPiISR: wiringPi has not been initialised properly. Unable to continue.\n") ;
	  }

  // Now export the pin and set the right edge
  if (mode != INT_EDGE_SETUP)
  {
    /**/ if (mode == INT_EDGE_FALLING)
      modeS = "falling" ;
    else if (mode == INT_EDGE_RISING)
      modeS = "rising" ;
    else
      modeS = "both" ;
  }

  isrFunctions [pin] = function ;

  if(!waitForInterruptSTDIN_process) {
	pthread_mutex_lock (&pinMutex) ;
	pthread_create (&threadId, NULL, interruptHandlerSTDIN, NULL) ;
	pthread_mutex_unlock (&pinMutex) ;
	waitForInterruptSTDIN_process = 1;
  }

  return 0 ;
}


/*
 * digitalWrite:
 *	Set an output bit
 *********************************************************************************
 */

void digitalWrite (int pin, int value)
{
  if ((pin & PI_GPIO_MASK) == 0)		// On-Board Pin
  {
    if (wiringPiMode != WPI_MODE_GPIO)	// Sys mode
    {
    	printf("[pseudoWiringPi][ERROR!!!][Modo de configuración incorrecto!!!][Use wiringPiSetupGpio ()]\n");
    	fflush(stdout);
    	return;
    }

	if (value == HIGH)
		if(pin >= 0 && pin <=3)
			columnaTecladoActiva = pin;
  }
}


/*
 * delay:
 *	Wait for some number of milliseconds
 *********************************************************************************
 */

void delay (unsigned int howLong)
{
  struct timespec sleeper, dummy ;

  sleeper.tv_sec  = (time_t)(howLong / 1000) ;
  sleeper.tv_nsec = (long)(howLong % 1000) * 1000000 ;

  nanosleep (&sleeper, &dummy) ;
}

/*
 * millis:
 *	Return a number of milliseconds as an unsigned int.
 *	Wraps at 49 days.
 *********************************************************************************
 */

unsigned int millis (void)
{
  uint64_t now ;

#ifdef	OLD_WAY
  struct timeval tv ;

  gettimeofday (&tv, NULL) ;
  now  = (uint64_t)tv.tv_sec * (uint64_t)1000 + (uint64_t)(tv.tv_usec / 1000) ;

#else
  struct  timespec ts ;

  clock_gettime (CLOCK_MONOTONIC_RAW, &ts) ;
  now  = (uint64_t)ts.tv_sec * (uint64_t)1000 + (uint64_t)(ts.tv_nsec / 1000000L) ;
#endif

  return (uint32_t)(now - epochMilli) ;
}

/*
 * piThreadCreate:
 *	Create and start a thread
 *********************************************************************************
 */

int piThreadCreate (void *(*fn)(void *))
{
  pthread_t myThread ;

  return pthread_create (&myThread, NULL, fn, NULL) ;
}

/*
 * piLock: piUnlock:
 *	Activate/Deactivate a mutex.
 *	We're keeping things simple here and only tracking 4 mutexes which
 *	is more than enough for out entry-level pthread programming
 *********************************************************************************
 */

void piLock (int key)
{
  pthread_mutex_lock (&piMutexes [key]) ;
}

void piUnlock (int key)
{
  pthread_mutex_unlock (&piMutexes [key]) ;
}
