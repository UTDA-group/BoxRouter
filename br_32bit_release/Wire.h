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
// Wire.h: interface for the CWire class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WIRE_H__36BC78F2_A1E4_4265_8D3D_C0123027D62E__INCLUDED_)
#define AFX_WIRE_H__36BC78F2_A1E4_4265_8D3D_C0123027D62E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Design.h"
#include "Net.h"
#include "Layer.h"
#include "BBox.h"	// Added by ClassView
#include "Segment.h"	// Added by ClassView
#include "Point.h"	// Added by ClassView

class CDesign;
class CNet;

class CWire : public CObject  
{
public:
#ifdef _DEBUG
	static	int m_iNumInstance;
	static	set<CWire*>	m_InstanceList;
	static	int Debug();
#endif
	static	stack<CWire*>	m_InstancePool;

	static CWire*	New();
	void	Delete();

	
	int	IsCompleted();
	int	GetRoutableLength(int iLayer);

	void	AdjustBoundary(CLayer* pLayer, int iBlocakge);
	CWire*	Clone();
	int IsRoutable(int iLayer);
	int GetLength2D();
	void Enumerate(int iMode);
	void				Initialize(int iX1, int iY1, int iZ1, int iX2, int iY2, int iZ2);
	int					MakeRouted(int iLayer);
	void				Pupate();
	int					SplitWire(CPoint* pPoint, CWire** ppWireS=NULL, CWire** ppWireE=NULL);
	CWire*					MergeWire(CPoint* pPoint, CWire* pWire);
	virtual void		Initialize(int iX1, int iY1, int iX2, int iY2, int iZ, CWire* pWire);
	vector<CWire*>		AdjustByBoundary();
	int					IsPerpendicular();
	int					GetNumPinOn();
	int					IsCrossing(CWire* pWire, CPoint* pPoint=NULL);
	int					IsOn3D(CPoint* pPoint);
	int					IsOn2D(CPoint *pPoint);
	int					IsRouted();
	int					IsPoint();
	int					IsVertical();
	int					IsHorizontal();
	int					IsLayerAssigned();
	CNet*				GetParent();
	CLayer*				GetLayer();
	int					GetLength();
	int					GetDirection();
	static double		GetSegmentListCost(vector<CSegment*>* pSegmentList);
	void				GetPoints(vector<CPoint*>* pPoints);
	void				AssignLayer(int iLayer);
	virtual void		Print(FILE* pFile, int Mode);
	bool					CompareLength(CWire* pWire);
#ifdef _DEBUG
	void				Check();
#endif

	vector<CSegment*>*	m_pRoutedSegmentList;
	vector<CSegment*>*	GetSegmentList(int iIndex);
	void				ClearSegmentList();
	vector<CSegment*>	Bend(int iZ);
	vector<CSegment*>	Bend(CPoint* pPoint, int iZ1, int iZ2);
	vector<CSegment*>	Bend(int iX, int iY, int iZ1, int iZ2);
	vector<CSegment*>	Bend(CPoint* pPoint1, CPoint* pPoint2, int iZ1, int iZ2, int iZ3);
	vector<CSegment*>	Bend(int iX1, int iY1, int iX2, int iY2, int iZ1, int iZ2, int iZ3);

	int					GetNumSegmentList();
	void				AddSegmentList(vector<CSegment*> Segment);
	vector<vector<CSegment*> > m_SegmentList;

	void				UpdateBoundary(int iMode);
	void				SetParent(CNet* pParent);

	void				CreateKey();
	int					IsFlat();
	void				SetState(int iState);
	void				Initialize(int iX1, int iY1, int iX2, int iY2, int iZ);
	CPoint*				m_pPointE;
	CPoint*				m_pPointS;
	CWire();
	virtual ~CWire();
};

inline CNet* CWire::GetParent()
{
	return	(CNet*)m_pParent;
}

inline int CWire::IsRouted()
{
	return	GetState()&STATE_WIRE_ROUTED;
}


class CWireOpLen {
public:
	bool operator () (const CWire* pL, const CWire* pR) const
	{ 
		return	((CWire*)pL)->CompareLength((CWire*)pR);
	}
};

#endif // !defined(AFX_WIRE_H__36BC78F2_A1E4_4265_8D3D_C0123027D62E__INCLUDED_)

