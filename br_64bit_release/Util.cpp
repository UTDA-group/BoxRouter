/*----------------------------------------------------------------------------
 *
 *  Released Software:  BoxRouter 2.0
 *
 *  Copyright (c) 2007 Regents of Univerisity of Texas, Austin 
 *  (developed by UT Design Automation lab)
 *
 *  Authors:         
 *                   Minsik Cho -- thyeros@mail.cerc.utexas.edu
 *
 *                   Kun Yuan -- kyuan@cerc.utexas.edu
 *
 *                   Katrian Lu -- yiotse@cerc.utexas.edu
 *                   
 *                   David Z.Pan -- dpan@ece.utexas.edu
 *
 *                         All Rights Reserved
 *
 *  The Released Software is sent to the recipient upon request.
 *  The recipient agrees the following conditions for receiving and using
 *  the Released Software: 
 *  1. Redistributions of any code, with or without modification (the "Code"),
 *     must be accompanied by any documentation and, each time the resulting 
 *     executable program or a program dependent thereon is launched, a 
 *     prominent display (e.g., splash screen or banner text, of the Author's 
 *     attribution information, which includes:
 *     (a) Minsik Cho, Kun Yuan, Katrina Lu and Dr. David Z. Pan ("AUTHOR"),
 *     (b) The University of Texas at Austin ("PROFESSIONAL IDENTIFICATION"), and
 *     (c) http://www.cerc.utexas.edu/utda/ ("URL").
 *  2. Use is limited to academic research groups only. Users who are 
 *     interested in industry or commercial purposes must notify Author and 
 *     request separate license agreement.
 *  3. Neither the name nor any trademark of the Author may be used to endorse 
 *     or promote products derived from this software without specific prior
 *     written permission.
 *  4. Users are entirely responsible, to the exclusion of the Author and any 
 *     other persons,for compliance with 
 *     (1) regulations set by owners or administrators of employed equipment, 
 *     (2) licensing terms of any other software, and 
 *     (3) local, national, and international regulations regarding use, 
 *     including those regarding import, export, and use of encryption software.
 *  If the recipient is interested in using the software as the part of (or 
 *  as the basis of) any commercial software system or for designing and 
 *  developing commercial products, please contact  Prof. David Z.Pan at the 
 *  following address:
 *
 *  Prof. David Z.Pan
 *  University of Texas,Austin
 *  ACES 5.434
 *  Austin, TX 78741
 *  Tel: 512-471-1436
 *  Fax: 512-471-8967
 *  E.mail:  dpan@ece.utexas.edu
 *
 *
 *  The authors of the Released Software disclaim all warranties with regard
 *  to this software.  In no event shall we be liable for any special,
 *  indirect, or consequential damages or any damages whatsoever resulting
 *  from loss of use, data, or profits.
 *
 *----------------------------------------------------------------------------*/
#include "Util.h"

int	KBHit()
{
#ifdef WIN32
	if(kbhit())	return	getch();
#else
	static struct termios initial_settings, new_settings;

	tcgetattr(0,&initial_settings);
	new_settings = initial_settings;
	new_settings.c_lflag &= ~ICANON;
	new_settings.c_lflag &= ~ECHO;
	new_settings.c_lflag &= ~ISIG;
	new_settings.c_cc[VMIN] = 1;
	new_settings.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &new_settings);

	unsigned char ch;

	new_settings.c_cc[VMIN]=0;
	tcsetattr(0, TCSANOW, &new_settings);
	int nread = read(0,&ch,1);
	new_settings.c_cc[VMIN]=1;
	tcsetattr(0, TCSANOW, &initial_settings);
	if(nread == 1)	return	ch;
#endif
	return 0;
}


#ifdef WIN32
int RunThread(int* pHandle,void (__cdecl * initialcode) (void *),void * argument)
{
	*pHandle	=	_beginthread(*initialcode,0,(void*)argument);
	if(*pHandle>0)
	{
		return	1;
	}

	return	0;
}
#else
//run thread with POSIX api
int RunThread(int* pHandle,void (*initialcode) (void *),void *argument)
{
	if(pthread_create((pthread_t*)pHandle,NULL,(void*(*)(void*))&*initialcode,(void*)argument)==0)
	{
		return  1;
	}

	return  0;
}
#endif


void WaitThread(int iHandle)
{
#ifdef WIN32
	::WaitForSingleObject((HANDLE)iHandle,INFINITE);
#else
	void* pStatus;
	pthread_join(iHandle, &pStatus);
#endif
}


int	GetMemory()
{
	int		iVMem	=	0;	

#ifdef WIN32
	
#else
	char    cBuffer[256];
	sprintf(cBuffer,"/proc/%d/status",getpid());

	FILE*   pFile   =       fopen(cBuffer,"rt");
	while(fgets(cBuffer,sizeof(cBuffer),pFile))
	{
		char*   pToken  =       strtok(cBuffer," \t\n");
		if(strcasecmp(pToken,"VmSize:")==0)
		{
			pToken  =	strtok(NULL," \t\n");
			iVMem	=	atoi(pToken);

			pToken  =	strtok(NULL," \t\n");

			if(strcasecmp(pToken,"KB")==0)				;
			else if(strcasecmp(pToken,"MB")==0)	iVMem*=1024;
			else if(strcasecmp(pToken,"GB")==0)	iVMem*=(1024*1024);
			else if(strcasecmp(pToken,"B")==0)		iVMem/=1024;
			else									iVMem*=-1;
			break;
		}
	}

	fclose(pFile);
#endif
	
	// in KB unit [2/10/2007 thyeros]
	return	iVMem;
}

void GetTime(char* pTime,int iOption)
{
#ifdef WIN32
	HANDLE		hProcess	=	OpenProcess(PROCESS_QUERY_INFORMATION,0,_getpid());
	static	FILETIME	CreationTime;
	static	FILETIME	ExitTime;
	static	FILETIME	KernelTime;
	static	FILETIME	UserTime;
	static	SYSTEMTIME	SystemTime;
#else
	static	rusage _ru;
#endif	
	
	switch(iOption) {
	case TIME_OPTION_CURRENT:
		{
			static	time_t ltime;
			time( &ltime );
			
			sprintf(pTime,"%s",ctime( &ltime ) );
			
			pTime[strlen(pTime)-1]	=	0x00;
		}
		break;
	case TIME_OPTION_KERNEL:
#ifdef WIN32
		if(GetProcessTimes(hProcess,&CreationTime,&ExitTime,&KernelTime,&UserTime))
		{			
			FileTimeToSystemTime(&KernelTime,&SystemTime);
			sprintf(pTime,"%d:%d:%d.%d",SystemTime.wHour,SystemTime.wMinute,SystemTime.wSecond,SystemTime.wMilliseconds);
		}
#else
		if(getrusage(RUSAGE_SELF,&_ru)==0)
		{
			sprintf(pTime,"%f",_ru.ru_stime.tv_sec+1e-6*_ru.ru_stime.tv_usec);	//thyeros: sec [10/30/2005]
		}
#endif
		break;
	case TIME_OPTION_USER:
#ifdef WIN32
		if(GetProcessTimes(hProcess,&CreationTime,&ExitTime,&KernelTime,&UserTime))
		{			
			FileTimeToSystemTime(&UserTime,&SystemTime);
			sprintf(pTime,"%d:%d:%d.%d",SystemTime.wHour,SystemTime.wMinute,SystemTime.wSecond,SystemTime.wMilliseconds);
		}
#else
		if(getrusage(RUSAGE_SELF,&_ru)==0)
		{
			sprintf(pTime,"%f",_ru.ru_utime.tv_sec+1e-6*_ru.ru_utime.tv_usec);	//thyeros: sec [10/30/2005]
		}
#endif
		break;
	default:
		assert(0);
		break;
	}
}

double StopWatch(int iOption)
{
#ifdef WIN32
	static	LARGE_INTEGER	PrevTime;
	static	LARGE_INTEGER	Frequency;
	static	int				iFrequncy	=	0;

	switch(iOption) {
	case STOPWATCH_OPTION_SET:
		{
			LARGE_INTEGER	CurrTime;
			::QueryPerformanceCounter(&CurrTime);

			return	1.0*(CurrTime.QuadPart-PrevTime.QuadPart)/Frequency.QuadPart;	//thyeros: sec [10/28/2005]
		}
	case STOPWATCH_OPTION_RESET:
		if(!iFrequncy)
		{
			iFrequncy	=	TRUE;
			QueryPerformanceFrequency(&Frequency);
		}
		::QueryPerformanceCounter(&PrevTime);

		return	0;
	default:
		assert(0);
		break;
	}	
#else
	static	rusage prev_ru;
	switch(iOption) {
	case STOPWATCH_OPTION_SET:
		{
			rusage	curr_ru;
			getrusage(RUSAGE_SELF,&curr_ru);

			return	(curr_ru.ru_utime.tv_sec+1e-6*curr_ru.ru_utime.tv_usec
			-prev_ru.ru_utime.tv_sec+1e-6*prev_ru.ru_utime.tv_usec);	//thyeros: sec [10/30/2005]
		}
	case STOPWATCH_OPTION_RESET:
		getrusage(RUSAGE_SELF,&prev_ru);
		return	0;
	default:
		assert(0);
		break;
	}	
#endif
	return	-1;
}

void CreateAndWait(char* pCommand)
{
#ifdef _WIN32
#ifdef _DEBUG
 //   printf("CMD[%s]\n",pCommand);
#endif
  
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    
    // Start the child process. 
    if( !CreateProcess( 0x00, // No module name (use command line). 
        pCommand, // Command line. 
        0x00,             // Process handle not inheritable. 
        0x00,             // Thread handle not inheritable. 
        0,            // Set handle inheritance to 0. 
        0,                // No creation flags. 
        0x00,             // Use parent's environment block. 
        0x00,             // Use parent's starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
        ) 
    {
		printf("CreateProcess Error[%]\n",GetLastError());
    }

    WaitForSingleObject(pi.hProcess,INFINITE);  

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
#else 
    //thyeros: unix fork and wait here until the forked process is terminated [10/6/2004]

    int iPID    =   fork();

    if(iPID>0)//thyeros - parent process [1/24/2003]
    {
        while(wait((int *) 0) != -1) ;
    }
    else if(iPID<0)//thyeros - error [1/28/2003]
    {
        fprintf(stderr,"ERROR: not enough resource\n");
    }
    else    //thyeros - child process [1/26/2003]
    {   
        char    cWhiteSpace[0x20-0x09]  =   {0x09,0x0A,0x0B,0x0C,0x0D,0x20,0x00,};

#define     MAX_COMMAND     512
        char    cTempCommand[MAX_COMMAND+1];
        strcpy(cTempCommand,pCommand);

        char*   pToken  =   strtok(cTempCommand," ");
        int     iIndex  =   0;      
        for(iIndex=0;pToken;iIndex++)
        {
            pToken=strtok(0x00," ");
        }
        
        //thyeros - allocated within child process [1/30/2003]
        char **args =   (char**)malloc((++iIndex)*sizeof(char*));
        
        iIndex  =   0;
        for(pToken=strtok(pCommand,cWhiteSpace);pToken;pToken=strtok(0x00,cWhiteSpace))
        {            
            args[iIndex++]  =   pToken;         
        }

        args[iIndex]    =   0x00;
        
        execvp(args[0],args);
        
        fprintf(stderr,"ERROR: A command[%s] does not exist or cannot be executed.\n",pCommand);
        _exit(-3);
    }       
#endif
}

extern void trace(char* pFMT, ...)
{
	va_list argptr;	
	va_start(argptr, pFMT);
	vprintf(pFMT, argptr);
	va_end(argptr);	
}

