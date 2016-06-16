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
// Object.h: interface for the CObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBJECT_H__0C7C8E91_33B6_4849_80B0_4F47CA03D786__INCLUDED_)
#define AFX_OBJECT_H__0C7C8E91_33B6_4849_80B0_4F47CA03D786__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BoxRouter.h"

class CDesign;

class CObject  
{
public:
	CObject();
	virtual ~CObject();

	virtual	void		Print(FILE* pFile,int iMode=PRINT_MODE_TEXT);
	void				PrintFile(char* pFileName, int iMode=PRINT_MODE_TEXT);
	static void			PrintFile2(char* pFileName, char *pFMT, ...);
	void				PrintLog(int iMode=PRINT_MODE_TEXT);
	static void			PrintLog2(char *pFMT, ...);
	void				PrintMsg(int iMode);
	static void			PrintMsg2(char *pFMT, ...);
	static void			Display(int iMode, char *pFMT, ...);
	KEY					GetKey();
	int					GetState();
	int					GetProp();
	static CDesign*		GetDesign();
	static int			IsDesignLoaded();

protected:
	static FILE*		m_pLog;
	static CDesign*		m_pDesign;
	KEY					m_Key;
	int					m_iProp;
	int					m_iState;
	CObject*			m_pParent;
};

inline CDesign*	CObject::GetDesign()
{
	return	m_pDesign;
}

inline KEY CObject::GetKey()
{
	return	m_Key;
}

inline int CObject::GetProp()
{
	return	m_iProp;
}

inline int CObject::GetState()
{
	return	m_iState;
}

#endif // !defined(AFX_OBJECT_H__0C7C8E91_33B6_4849_80B0_4F47CA03D786__INCLUDED_)

