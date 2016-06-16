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
// Design.h: interface for the CDesign class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DESIGN_H__E7845521_5E53_493A_8CC5_F1DF684C22CB__INCLUDED_)
#define AFX_DESIGN_H__E7845521_5E53_493A_8CC5_F1DF684C22CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Object.h"
#include "Net.h"
#include "Layer.h"
#include "BBox.h"
#include "Param.h"

#define WIRE_WIDTH_SPACE	(2)

class CLayer;

class CDesign : public CObject, public CBBox
{
	friend	class	CObject;
	friend	class	CBoundary;
public:
CDesign();
	virtual ~CDesign();
	
	CParam				m_Param;

	int					m_iNumBCap;
	int					m_iBestOF;	
	int					m_iBestWL;
	int					m_iBestVia;
	int					m_iBestMOF;
	int					m_iBestLoop;

	int					GetWireInDirection(vector<CNet*>* pNetList, vector<CWire*>* pWireList, int iPos, int iDirection, int iMode=GET_MODE_PROP, int iValue=PROP_WIRE_ANY);
	int					GetWireInDirection(vector<CWire*>* pWireList, int iPos, int iDirection, int iMode=GET_MODE_PROP, int iValue=PROP_WIRE_ANY);

	void				Report(char* pName);
	CBBox				GetCongestedBox();
//	double				GetMetric();
	void				UpdateSteinerTree();
	virtual int			Initialize(char* pName);
	void				CreateNet(int iNumNet);
	void				CreateNet(vector<CNet*>* pNetList);
	void				CreateLayer();
	int					GetOverFlowBoundary(vector<CBoundary*>* pBoundaryList);
	int					GetNetInOverFlowBoundary(hash_map<ADDRESS,int>* pNetList);
	int					GetWireInOverFlowBoundary(hash_map<ADDRESS,int>* pWireList);
	void				WriteDump();
	void				WriteRel();
	int				ReadDump(char* pName);
	void				ReadParam(char* pName);
	
	int					m_iLowX;
	int					m_iLowY;
	int					m_iGridX;
	int					m_iGridY;
#ifdef _DEBUG
	int					m_iDebugNet;
#endif
	int					m_iLoop;
	void				DelNets();
	void				SetProp(int iProp);
	void				SetState(int iState);
	void				Print(FILE* pFile, int iMode);
	void				PrintResult();
	int					GetOverFlow(int iMode);
	int					GetRoutableLayer(int iMode);
	void				SetHighestRoutableLayer(int iLayer);
	void				SetLowestRoutableLayer(int iLayer);
	//void				SetTargetCongestion(double dTargetCongestion);
	CNet*				GetNet(int iIndex);
	CLayer*				GetLayer(int iLayer);	//thyeros- layer starts from ONE (M1->1), ZERO means not assigned yet or Device Layer [6/16/2006]
	CBBox				GetStartBox(int* pNumEdge=NULL);
	int					GetNetInBBox(vector<CNet*>* pNetList, CBBox* pBBox/*, int iMode=GET_MODE_STATE, int iValue=STATE_NET_UNREROUTED*/);
	int					GetWireInBBox(vector<CWire*>* pWireList, CBBox* pBBox, int iMode=GET_MODE_STATE, int iValue=STATE_WIRE_UNROUTED);
	int					GetNumNet(int iMode=GET_MODE_STATE, int iValue=STATE_NET_ANY);
	int					GetNumWire(int iMode=GET_MODE_STATE, int iValue=STATE_WIRE_ANY);
	int					GetLength(int iMode, int iValue);
	int					GetNumVia();

	int					AddPenalty(int iPenalty);
	int					GetCapacity(int iMode);
	double				GetCongestion(int iMode);
	int					m_iNumPreRoutedNet;
	int					m_iPreRoutedWL;
protected:	
	int					m_iHighestRoutableLayer;	//highest routable layer, by default, it'll be the maximum layer
	//double				m_dTargetCongestion;		//design target congestion
	int					m_iLowestRoutableLayer;		//lowest routable layer, by default, it'll be the minimum layer
	int					m_iSizeZ;					
	int					m_iSizeY;
	int					m_iSizeX;
	int					m_iNumNet;					//total # of real nets in the design
	int					m_iNumLocalNet;

	CLayer*				m_pLayer;					
	CNet**				m_ppNet;
	int					m_iMaxCapacity;				//maximum routing capaciity among all the layers
//	int					m_iTargetCapacity;			//target routing capacity among all the layers
};		
		
#endif // !defined(AFX_DESIGN_H__E7845521_5E53_493A_8CC5_F1DF684C22CB__INCLUDED_)

