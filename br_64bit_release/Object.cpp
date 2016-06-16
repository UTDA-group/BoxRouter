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
// Object.cpp: implementation of the CObject class.
//
//////////////////////////////////////////////////////////////////////

#include "Object.h"
#include "Design.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FILE*		CObject::m_pLog		=	NULL;
CDesign*	CObject::m_pDesign	=	NULL;
CObject::CObject()
{
	m_pParent	=	NULL;
	m_iState	=	STATE_INVALID;
	m_iProp		=	PROP_INVALID;
	m_Key		=	0;
}

CObject::~CObject()
{

}

void CObject::Display(int iMode, char *pFMT, ...)
{
	static	char cMode[DISPLAY_MODE_NONE][5] = {"INFO","ERRO","PARM","WARN","EXEC"};
	static	char cTime[50];

	//GetTime(cTime);

	//fprintf(stdout,"%s [%s]: ",cTime,cMode[iMode]);

	if(iMode!=DISPLAY_MODE_NONE)	fprintf(stdout,"[%s]: ",cMode[iMode]);

	va_list argptr;	
	va_start(argptr, pFMT);
	vfprintf(stdout,pFMT, argptr);
	va_end(argptr);

	fflush(stdout);

	if(iMode==DISPLAY_MODE_ERRO)
	{
		fprintf(stdout,"!Program terminated!\n");
		exit(-100);
	}
}

void CObject::Print(FILE* pFile, int iMode)
{
	//thyeros- doing nothing here [6/16/2006]
	printf("not implemented!\n");
}

void CObject::PrintFile(char *pFileName, int iMode)
{
	FILE*	pFile	=	NULL;
	
	if(strstr(pFileName,">")==pFileName)	pFile	=	fopen(pFileName+1,"ab");
	else									pFile	=	fopen(pFileName,"wb");
	
	assert(pFile);
	
	Print(pFile,iMode);
	
	SAFE_FCLOSE(pFile);
}

void CObject::PrintLog(int iMode)
{
	if(m_pLog)	Print(m_pLog,iMode);
}

void CObject::PrintLog2(char *pFMT, ...)
{
	if(m_pLog)
	{
		va_list argptr;	
		va_start(argptr, pFMT);
		vfprintf(m_pLog,pFMT, argptr);
		va_end(argptr);

		fflush(m_pLog);
	}
}

void CObject::PrintFile2(char *pFileName, char *pFMT, ...)
{
	FILE*	pFile	=	NULL;
	
	if(strstr(pFileName,">")==pFileName)	pFile	=	fopen(pFileName+1,"at");
	else									pFile	=	fopen(pFileName,"wt");
	
	assert(pFile);
	
	if(pFile)
	{
		va_list argptr;
		va_start(argptr, pFMT);
		vfprintf(pFile,pFMT, argptr);
		va_end(argptr);

		fflush(pFile);
	}
	
	SAFE_FCLOSE(pFile);
}

void CObject::PrintMsg2(char *pFMT, ...)
{
	if(GetDesign()->m_Param.GetProp()&PROP_PARAM_DISP_OFF)	return;

	va_list argptr;
	va_start(argptr, pFMT);
	vfprintf(stdout,pFMT,argptr);
	va_end(argptr);
	
	fflush(stdout);
}

void CObject::PrintMsg(int iMode)
{
	if(GetDesign()->m_Param.GetProp()&PROP_PARAM_DISP_OFF)	return;

	Print(stdout,iMode);
}

int	CObject::IsDesignLoaded()
{
	return	m_pDesign!=NULL;
}

