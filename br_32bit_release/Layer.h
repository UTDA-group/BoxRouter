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
// Layer.h: interface for the CLayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAYER_H__A6C1DF41_F5B0_446B_8A8D_331D245331DA__INCLUDED_)
#define AFX_LAYER_H__A6C1DF41_F5B0_446B_8A8D_331D245331DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Object.h"
#include "Grid.h"
#include "Boundary.h"
#include "BBox.h"

class CDesign;
class CBoundary;
class CGrid;

class CLayer : public CObject  
{
	friend	class	CDesign; 
public:
	int							GetOverFlowBoundary(vector<CBoundary*>* pBoundaryList);
	void						GetStartBox(vector<int>* pX,vector<int>* pY);
	int							AddPenalty(int iPenalty);
	int							GetNumTrack(int iMode);
	int							GetWireInOverFlowBoundary(hash_map<ADDRESS,int>* pWireList);
	int							GetNetInOverFlowBoundary(hash_map<ADDRESS,int>* pNetList);
	void						GetCongestedBox(CBBox* pBBox);

	int							GetLength();
	void						GetBoundary(CGrid* pGrid, CBoundary** ppBoundary1, CBoundary** ppBoundary2);
	int							GetNumBoundary();
	int							GetCapacity(int iMode);

	int							GetNumGrid();
	void						GetAdjGrid();
	double						GetCongestion(int iMode);

//	int							GetDesignRule(int iMode);
	int							GetOverFlow(int iMode);
	int							GetDirection();
	CDesign*					GetParent();
	CGrid*						GetGrid(int iX,int iY);
	CBoundary*					GetMaxCBoundary(/*int iMode, int iValue*/);
	CBoundary*					GetMinCBoundary();
	CBoundary*					GetBoundary(CPoint *pPoint1, int iX, int iY);
	CBoundary*					GetBoundary(CPoint* pPoint1, CPoint* pPoint2);
	CBoundary*					GetBoundary(int iX1, int iY1, int iX2, int iY2);
	CBoundary*					GetBoundary(int iX, int iY);

	void						TrackCongestion(CBoundary* pBoundary, int iOldC, int iNewC);
	void						TrackWireLength(int iLength);
	void						ResetMaze();

	void						Configure(int iIndex, int iMaxCapacity, int iDirection, int iMinWireWidth, int iMinWireSpacing, int iMinViaSpacing, CDesign *pParent);
	virtual void				Initialize();
	virtual	void				Print(FILE* pFile, int iMode);
	int							IsVertical();
	int							IsHorizontal();
	int							Z();
	void						CreateBoundary();
	void						CreateGrid();

	CLayer();
	virtual ~CLayer();

	multimap<int, CBoundary*>	m_CBoundaryBucket;
	hash_map<ADDRESS, multimap<int,CBoundary* >::iterator>	m_CBoundaryBucketItr;

	int			m_iMaxCapacity;
	int			m_iMaxBY;
	int			m_iMaxBX;

protected:	
	int			m_iMinWireWidth;
	int			m_iMinWireSpacing;
	int			m_iMinViaSpacing;
	int			m_iDirection;
//	int			m_iTargetCapacity;
//	int			m_iNumWL;
	int			m_iSumWL;
//	int			m_iSumWL2;
//	int			m_iSumC;
//double		m_dSumC2;
	//double		m_dTargetCongestion;	
	//int*		m_pMazeCost;
	//CGrid**		m_ppMazeGrid;
	unsigned char*		m_pMaze;
	CGrid**		m_ppGrid;

	CBoundary** m_ppBoundary;
};

inline CBoundary* CLayer::GetBoundary(CPoint *pPoint1, CPoint *pPoint2)
{
	if(pPoint1->Z()!=pPoint2->Z())	return	NULL;
	
	return	GetBoundary(pPoint1->X(),pPoint1->Y(),pPoint2->X(),pPoint2->Y());
}

inline CBoundary* CLayer::GetBoundary(int iX1, int iY1, int iX2, int iY2)
{
	if(iX1==iX2&&iY1==iY2)	return	NULL;

	if(IsHorizontal()&&iY1==iY2)
	{
		assert(abs(iX1-iX2)==1);
		return	GetBoundary(MIN(iX1,iX2),iY1);
	}
	else if(IsVertical()&&iX1==iX2)
	{
		assert(abs(iY1-iY2)==1);
		return	GetBoundary(iX1,MIN(iY1,iY2));
	}

	return	NULL;
}

#endif // !defined(AFX_LAYER_H__A6C1DF41_F5B0_446B_8A8D_331D245331DA__INCLUDED_)

