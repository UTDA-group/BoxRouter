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
// Point.h: interface for the CPoint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POINT_H__B41B235A_77C1_4664_8252_91A2B6D879EC__INCLUDED_)
#define AFX_POINT_H__B41B235A_77C1_4664_8252_91A2B6D879EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Object.h"
//#include "RealPoint.h"	// Added by ClassView

class CWire;

class CPoint : public CObject  
{
public:
	CPoint();
	virtual ~CPoint();

	virtual void		Initialize(int iX, int iY, int iZ, CWire* pParent);
	int					IsVertical(CPoint *pPoint);
	int					IsHorizontal(CPoint *pPoint);
	int					IsFlat(CPoint* pPoint);
	int					IsSame3D(CPoint *pPoint);
	int					IsSame3D(int iX, int iY, int iZ);
	int					IsSame2D(CPoint* pPoint);
	int					IsSame2D(int iX, int iY);
	int					IsPin();
	int					IsPerpendicular(CPoint *pPoint);
	
	int					GetDirection(CPoint* pPoint);
	int					GetMDistance(CPoint *pPoint);
	int					GetMDistance2D(CPoint *pPoint);
	void				CreateKey();
	CWire*				GetParent();
	void				Print(FILE* pFile,int iMode/* =PRINT_MODE_TEXT */);
	
	int					X();
	int					Z();
	int					Y();
	void				SetX(int iX);
	void				SetY(int iY);
	void				SetZ(int iZ);
	void				SetXYZ(int iX, int iY, int iZ);
	static KEY			MakeKey(int iX, int iY, int iZ);
	static int			X(KEY Key);
	static int			Y(KEY Key);
	static int			Z(KEY Key);
	
protected:
	short				m_iY;
	short				m_iX;
	char				m_cZ;
};

inline int CPoint::X()
{
	return	m_iX;
}

inline int CPoint::Y()
{
	return	m_iY;
}

inline int CPoint::Z()
{
	return	m_cZ;
}

#endif // !defined(AFX_POINT_H__B41B235A_77C1_4664_8252_91A2B6D879EC__INCLUDED_)

