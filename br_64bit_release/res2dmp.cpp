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
// dmp2res.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <math.h>
#include <stdarg.h>

#ifdef WIN32
#define STRICMP						stricmp
#define STRNICMP					strnicmp
#define	KEY							__int64
#else
#define	KEY							long long
#define STRICMP						strcasecmp
#define STRNICMP					strncasecmp
#endif


int X(KEY Key)
{
	return	0xFFF & (Key>>20);
}

int Y(KEY Key)
{
	return	0xFFF & (Key>>8);
}

int Z(KEY Key)
{
	return	0xFF & (Key);
}


KEY MakeKey(int iX, int iY, int iZ)
{
	return	iX<<20 | iY<<8 | iZ;
}

int main(int argc, char* argv[])
{
	FILE*	pInput_File		=	fopen(argv[1],"rt");
	FILE*	pOutput_File	=	fopen(argv[2],"rt");
	FILE*	pDump_File		=	fopen(argv[3],"wb");

	char	cLine[512];
	char	cName[512];

	int	iLowX=0, iLowY=0, iGridX=1, iGridY=1;

	while (fgets(cLine,sizeof(cLine),pInput_File))
	{
		if(cLine[0]=='#')	continue;
		if(strlen(cLine)<2)	continue;

		char*	pToken	=	strtok(cLine," \t\n");
		if(STRICMP(pToken,"grid")==0)
		{

		}
		else if(STRICMP(pToken,"vertical")==0&&strtok(NULL," \t\n"))
		{
		}
		else if(STRICMP(pToken,"horizontal")==0&&strtok(NULL," \t\n"))
		{
		}
		else if(STRICMP(pToken,"minimum")==0)
		{

		}
		else if(STRICMP(pToken,"via")==0)
		{
		}
		else if(STRICMP(pToken,"num")==0)
		{
			pToken			=	strtok(NULL," \t\n");
			if(STRICMP(pToken,"net")==0)
			{
				int	iOffX	=	(int)(iGridX*0.5+iLowX);
				int	iOffY	=	(int)(iGridX*0.5+iLowY);

				int	iNumNet	=	atoi(strtok(NULL," \t\n"));
				printf("[num net=%d]..",iNumNet);

				for(int i=0;i<iNumNet;++i)
				{
					// read output first [4/3/2007 thyeros]
					cLine[0]	=	NULL;
					while(fgets(cLine,sizeof(cLine),pOutput_File))
					{
						if(cLine[0]=='#')	continue;
						if(cLine[0]=='!')	continue;
						if(strlen(cLine)<2)	continue;
						break;
					}

					if(strlen(cLine)<2)	break;
	
					char*	pCurNetName		=	strtok(cLine," \t\n");
					strcpy(cName,pCurNetName);
					int		iCurNetID	 	=	atoi(strtok(NULL," \t\n"));
					int		iCurNumWire		=	atoi(strtok(NULL," \t\n"));

					// read input [4/3/2007 thyeros]
					while(true)
					{
						while(fgets(cLine,sizeof(cLine),pInput_File))
						{
							if(cLine[0]=='#')	continue;
							if(strlen(cLine)<2)	continue;
							break;
						}

						char*	pNetName		=	strtok(cLine," \t\n");
						int		iNetID	 		=	atoi(strtok(NULL," \t\n"));
						int		iNumPin			=	atoi(strtok(NULL," \t\n"));
{
						for(int i=0;i<iNumPin;++i)
						{
							fgets(cLine,sizeof(cLine),pInput_File);
							if(cLine[0]=='#')	--i;
							if(strlen(cLine)<2)	--i;
						}	
}

						if(iNetID==iCurNetID)	break;
						++i;
					}

					KEY	Key		=	iCurNetID;

					fwrite(&Key,1,sizeof(KEY),pDump_File);
					fwrite(&iCurNumWire,1,sizeof(int),pDump_File);

					// read output first [4/3/2007 thyeros]
					while(fgets(cLine,sizeof(cLine),pOutput_File))
					{
						if(cLine[0]=='#')	continue;
						if(cLine[0]=='!')	break;
						if(strlen(cLine)<2)	continue;
						
						
						int	iX1		=	atoi(strtok(cLine,"(,)- \t\n"));
						int	iY1		=	atoi(strtok(NULL,"(,)- \t\n"));
						int	iZ1		=	atoi(strtok(NULL,"(,)- \t\n"));
						int	iX2		=	atoi(strtok(NULL,"(,)- \t\n"));
						int	iY2		=	atoi(strtok(NULL,"(,)- \t\n"));
						int	iZ2		=	atoi(strtok(NULL,"(,)- \t\n"));

						iX1	=	(iX1-iLowX)/iGridX;
						iX2	=	(iX2-iLowX)/iGridX;
						iY1	=	(iY1-iLowY)/iGridY;
						iY2	=	(iY2-iLowY)/iGridY;

						Key	=	MakeKey(iX1,iY1,iZ1);
						fwrite(&Key,1,sizeof(KEY),pDump_File);

						Key	=	MakeKey(iX2,iY2,iZ2);
						fwrite(&Key,1,sizeof(KEY),pDump_File);
					}	
				}

				return	1;
			}
		}
		else
		{
			iLowX	=	atoi(pToken);
			iLowY	=	atoi(strtok(NULL," \t\n"));
			iGridX	=	atoi(strtok(NULL," \t\n"));
			iGridY	=	atoi(strtok(NULL," \t\n"));
		}
	}

	return 0;
}

