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
//// Net.h: interface for the CNet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NET_H__ADE7C9A3_045E_4689_B565_EFB29577C237__INCLUDED_)
#define AFX_NET_H__ADE7C9A3_045E_4689_B565_EFB29577C237__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Object.h"
#include "Pin.h"
#include "Wire.h"
#include "BBox.h"

class CNet : public CObject  
{
public:
	int	GetWireInDirection(vector<CWire*>* pWireList, int iPos, int iDirection,int iMode=GET_MODE_PROP, int iValue=PROP_WIRE_ANY);
	int IsPreRouted();
//	int					m_iMinVia;
	int					m_iMinWL;
	//int					GetWidth();
	//char				m_cWidth;
	CPin*				GetPin(int iX,int iY);
	CPin*				GetPin(CPoint* pPoint, int* pIndex = NULL);
	int					IsFlat();
	void				GetViaBucket(multimap<KEY, int>* pViaBucket);
	void				Check();

	void				UpdateBBox();
	void				MergeWire();
	CWire*				MergeWire(CWire* pWire);
	void				SplitWire();
		int					IsShortest(int iMargin=0);
	int					IsOptimal(int iMargin=0);
	int					IsOptimal2(int iMargin=0);
	//int					IsRerouted();
	int					IsConnected(CPoint* pPoint1, CPoint* pPoint2);
	void				Refine();
	void				DelWires();
	int					GetNumVia();
	virtual void		Print(FILE* pFile, int iMode);
	CWire*				IsFound(CWire* pWire);
	int					GetWires3D(vector<CWire*>* pWires, CPoint* pPoint, int iMode, int iValue);
	int					GetWires2D(vector<CWire*>* pWires, CPoint* pPoint, int iMode, int iValue);
	int					GetPoints(vector<CPoint*>* pPoints, CPoint* pPoint, int iMode=GET_MODE_STATE, int iValue=STATE_WIRE_ROUTED);
	int					GetWireInBBox(vector<CWire*>* pWireList, CBBox* pBBox, int iMode=GET_MODE_STATE, int iState=STATE_WIRE_ANY);
	int					IsLocal();
//	static int			CompareByLength(const void* A, const void* B);
	int					IsRouted();
	int					DelWire(CWire* pWire, int iMode=DELWIRE_MODE_FRMAP);
	void				SetState(int iState);
	int					GetNumWire(int iMode=GET_MODE_STATE, int iValue=STATE_WIRE_ANY);
	int					AddWire(CWire* pWire);
	int					GetLength(int iMode, int iValue);
	CPin*				GetPin(int iIndex);

	vector<CWire*>			m_Wire;
	CBBox				m_BBox;
		
	void				AddPin(vector<CPin*>* pPinList);
	int					GetNumPin();
	short				m_iNumPin;
	CPin**				m_ppPin;
	CDesign*			GetParent();
	virtual void		Initialize(int iIndex, /*char cWidth,*/ CDesign* pParent);

	CNet();
	virtual ~CNet();
};

class CNetOpArea {
public:
	bool operator () (const CNet* pL, const CNet* pR) const
	{ 
		int	iAL		=	((CNet*)pL)->m_BBox.L();
		int	iAR		=	((CNet*)pR)->m_BBox.L();

		if(iAL==iAR)	return	((CNet*)pL)->GetKey()>((CNet*)pR)->GetKey();

		return	iAL>iAR;
	}
};

class CNetOpAreaR {
public:
	bool operator () (const CNet* pL, const CNet* pR) const
	{ 
		int	iAL		=	((CNet*)pL)->m_BBox.L();
		int	iAR		=	((CNet*)pR)->m_BBox.L();

		if(iAL==iAR)	return	((CNet*)pL)->GetKey()<((CNet*)pR)->GetKey();

		return	iAL<iAR;
	}
};


#endif // !defined(AFX_NET_H__ADE7C9A3_045E_4689_B565_EFB29577C237__INCLUDED_)
