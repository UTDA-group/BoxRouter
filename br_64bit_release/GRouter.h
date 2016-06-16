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
// GRouter.h: interface for the CGRouter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GROUTER_H__D649F435_754A_45CA_A69B_987FB2033510__INCLUDED_)
#define AFX_GROUTER_H__D649F435_754A_45CA_A69B_987FB2033510__INCLUDED_

#include "Design.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef int				FP_MAZECOST(CDesign* pDesign, CGrid* pGrid, CGrid* pNGrid,/* CNet* pNet*/ CBBox* pBBox);

class CGRouter  
{
public:

#define	MAX_MAZECOST	4096
	static void			Layering(CDesign* pDesign);
	static void			Layering(CDesign* pDesign, vector<CNet*>* pNetList);
	static int			Layering(CDesign* pDesign, vector<CWire*>* pWireList, int iPos, int iDirection);
	static int			m_iViaCost;
	static KEY			m_iMazeCost[MAX_MAZECOST];
	static void			Routing(CDesign* pDesign);
	static void			PreRouting(CDesign* pDesign);

	static void			SetBoundary(CDesign* pDesign, CBBox* pBBox);
	static void			BoxRouting(CDesign* pDesign, CBBox* pBBoxS, CBBox* pBBoxE);
	static int			ConcurrentRouting(CDesign* pDesign, vector<CWire*>* pWireList, int iMode);
	static int			SequentialRouting(CDesign* pDesign, vector<CWire*>* pWireList, CBBox* pBBox, int iBound);

	static void			ReRoutingForOF(CDesign* pDesign, hash_map<ADDRESS,int>* pObjectList);
	static int			ReRoutingForWL(CDesign *pDesign, vector<CNet*>* pNetList);				// rerouting for WL minimization [2/14/2007 thyeros]
	static int			ReRoutingForOF(CDesign* pDesign, vector<CNet*>* pNetList, int iBound);	// rerouting for OF minimization [2/14/2007 thyeros]
	static int			ReRoutingForOF(CDesign *pDesign, vector<CWire*>* pWireList, int iBound);

	static int			FilterRouting(CDesign* pDesign, CWire* pWire);
//	static int			PatternRouting(FP_MAZECOST* pMazeCost,CDesign* pDesign, CWire* pWire);

	static int			MazeRouting(FP_MAZECOST* pMazeCost,CDesign* pDesign, CWire* pWire, CBBox* pBBox, int iBound=0);
	static int			MazeCost_Box(CDesign *pDesign, CGrid *pGrid, CGrid* pNGrid, CBBox *pBBox);
	static int			MazeCost_BoxBound(CDesign *pDesign, CGrid *pGrid, CGrid* pNGrid, CBBox *pBBox);
	static int			MazeCost_Penalty(CDesign *pDesign, CGrid *pGrid, CGrid* pNGrid, CBBox *pBBox);
	static int			MazeCost_PenaltyBound(CDesign *pDesign, CGrid *pGrid, CGrid* pNGrid, CBBox *pBBox);
	static void			TraceBack(CWire* pWire, vector<CGrid*>* pTrace, vector<CSegment*>* pSegmentList);

	CGRouter();
	virtual ~CGRouter();
};

#endif // !defined(AFX_GROUTER_H__D649F435_754A_45CA_A69B_987FB2033510__INCLUDED_)

