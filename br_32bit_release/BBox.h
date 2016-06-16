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
// BBox.h: interface for the CBBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BBOX_H__C2EA4446_83B2_4CA5_BA60_E4785EE23989__INCLUDED_)
#define AFX_BBOX_H__C2EA4446_83B2_4CA5_BA60_E4785EE23989__INCLUDED_

#include "Point.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBBox
{
public:
	CBBox();
	virtual ~CBBox();
	int				A();	// area [8/5/2006 thyeros]
	int				X();	// left bottom X [7/14/2006 thyeros]
	int				Y();	// left bottom Y [7/14/2006 thyeros]
	int				Z();	// left bottom Z [7/14/2006 thyeros]
	int				H();	// height [7/14/2006 thyeros]
	int				W();	// width [7/14/2006 thyeros]
	int				T();	// thickness [7/14/2006 thyeros]
	int				L();
//	int				V();	// volume [7/14/2006 thyeros]
	
	void			Initialize(CPoint* pPointS, CPoint* pPointE);
	void			Initialize(int iX1, int iY1, int iZ1, int iX2, int iY2, int iZ2);

	int				IsInside(CPoint* pPoint);
	int				IsInside(int iX, int iY, int iZ);
	int				IsInside(int iX, int iY);
	int				IsInside(CBBox* pBBox);
	int				Distance(CPoint* pPoint);
	int				IsOverlapped(CWire* pWire);
	int				IsOverlapped(CBBox* pBBox);

	void			Expand(int iMaxX, int iMaxY, int iStep = 1);
	void			AddPoint(CPoint* pPoint);
	void			AddPoint(int iX,int iY,int iZ);
	CPoint			GetCenter();
	void			Print(FILE *pFile, int iMode);

protected:	
	short			m_iMinX;
	short			m_iMaxX;
	short			m_iMinY;
	short			m_iMaxY;
	char			m_cMinZ;
	char			m_cMaxZ;
};

inline int CBBox::X()	{	return	m_iMinX;}
inline int CBBox::Y()	{	return	m_iMinY;}
inline int CBBox::Z()	{	return	m_cMinZ;}
inline int CBBox::W()	{	return	m_iMaxX-m_iMinX;}
inline int CBBox::H()	{	return	m_iMaxY-m_iMinY;}
inline int CBBox::T()	{	return	m_cMaxZ-m_cMinZ;}
inline int CBBox::L()	{	return	m_iMaxX-m_iMinX+m_iMaxY-m_iMinY;}
//inline int CBBox::V()	{	return	W()*H()*T();}
inline int CBBox::A()	{	return	W()*H();}
//
//class CBoxOpArea {
//public:
//	bool operator () (const CBBox* pL, const CBBox* pR) const
//	{ 
//		return	((CBBox*)pL)->A()<((CBBox*)pR)->A();
//	}
//};


#endif // !defined(AFX_BBOX_H__C2EA4446_83B2_4CA5_BA60_E4785EE23989__INCLUDED_)

