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
#ifndef __UTIL_H__
#define	__UTIL_H__
/********************************************************************
	created:	2005/10/28
	created:	28:10:2005   14:51
	file base:	Util
	file ext:	h
	author:		Minsik Cho
	
	purpose:	
*********************************************************************/

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <io.h>
#include <process.h>
#include <Windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <unistd.h>
#include <iostream.h>
#include <fcntl.h>
#include <sys/procfs.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <curses.h>
#endif

#define	STOPWATCH_OPTION_SET		(0)
#define	STOPWATCH_OPTION_RESET		(1)

#define	TIME_OPTION_CURRENT			(1)
#define	TIME_OPTION_KERNEL			(2)
#define	TIME_OPTION_USER			(3)

#ifdef WIN32
int		RunThread(int* pHandle,void (__cdecl * initialcode) (void *),void * argument);
#else
int		RunThread(int* pHandle,void (*initialcode) (void *),void *argument);
#endif

void	WaitThread(int iHandle);
void	CreateAndWait(char* pCommand);
void	GetTime(char* pTime,int iOption=TIME_OPTION_CURRENT);
double	StopWatch(int iOption);
int		GetMemory();
int		KBHit();
#endif


