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
// Boundary.h: interface for the CBoundary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOUNDARY_H__6A1EA5A6_08C5_4B38_B51F_72D78F9C1AC8__INCLUDED_)
#define AFX_BOUNDARY_H__6A1EA5A6_08C5_4B38_B51F_72D78F9C1AC8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Point.h"
#include "Wire.h"
#include "Segment.h"

class CLayer;

class CBoundary : public CPoint  
{
	friend class CLayer;

public:
	int					GetCongestion();
	CBoundary();
	virtual ~CBoundary();
	
	virtual void		Initialize(int iX, int iY, int iZ, CLayer* pParent);

	int					IsFound(CNet* pNet);
	int					IsFound(CWire* pWire);

	int					GetOverFlow();

	int					GetCapacity(int iMode=GET_MODE_ACAP);
	void				AddSegmentList(vector<CSegment*>* pSegment);
	vector<CSegment*>*	GetSegmentList(int iIndex);
	void				ClearSegmentList();
	int					GetNumSegmentList();
	
	int					AddPenalty(int iPenalty);
	int					GetPenalty();
	int					AddWire(CWire* pWire);
	int					AdjustCapacity(int iBlockage);
	int					GetNumWire();
	void				DelWire(CWire* pWire);

	virtual void		Print(FILE* pFile, int iMode);
	CLayer*				GetParent();
	short							m_iNumOCap;
	short							m_iNumACap;
	short							m_iCongestion;

protected:

	vector<CWire*>					m_Wire;
	vector<vector<CSegment*>* >		m_SegmentList;
};

#endif // !defined(AFX_BOUNDARY_H__6A1EA5A6_08C5_4B38_B51F_72D78F9C1AC8__INCLUDED_)

