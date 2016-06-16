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
// Layer.cpp: implementation of the CLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "Layer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLayer::CLayer()
{
	m_ppGrid			=	NULL;
	m_ppBoundary		=	NULL;
	m_iSumWL			=	0;	
	m_iMaxBY			=	0;
	m_iMaxBX			=	0;
	//m_iTargetCapacity	=	-1;
	//m_dTargetCongestion	=	0;
}

CLayer::~CLayer()
{
	for (int i=0;i<GetDesign()->W();++i)	SAFE_DELA(m_ppGrid[i]);
	SAFE_DELA(m_ppGrid);

	//for (int i=0;i<m_iMaxBX;++i)	SAFE_DELA(m_ppBoundary[i]);

	//SAFE_DELA(m_ppBoundary);
	SAFE_DELA(m_pMaze);
}

void CLayer::Configure(int iIndex, int iMaxCapacity, int iDirection, int iMinWireWidth, int iMinWireSpacing, int iMinViaSpacing, CDesign *pParent)
{
	if(iIndex>=0)			m_Key				=	iIndex;
	if(iMaxCapacity>=0)		m_iMaxCapacity		=	iMaxCapacity;
	if(iDirection>=0)		m_iDirection		=	iDirection;
	if(iMinWireWidth>=0)	m_iMinWireWidth		=	iMinWireWidth;
	if(iMinWireSpacing>=0)	m_iMinWireSpacing	=	iMinWireSpacing;
	if(iMinViaSpacing>=0)	m_iMinViaSpacing	=	iMinViaSpacing;

	m_pParent			=	(CObject*)pParent;
}

void CLayer::Initialize()
{
	m_iMinWireWidth		=	MAX(1,m_iMinWireWidth);
	//	m_dSumC				=	0;

	CreateGrid();
	CreateBoundary();
}

void CLayer::ResetMaze()
{
	memset(m_pMaze,0x00,GetNumGrid()*sizeof(char));
}

void CLayer::CreateGrid()
{
	m_pMaze				=	new	unsigned char[GetNumGrid()];
	assert(m_pMaze);

	m_ppGrid			=	new	CGrid*[GetDesign()->W()];
	assert(m_ppGrid);

	int	iIndex	=	0;
	for (int i=0;i<GetDesign()->W();++i)
	{
		m_ppGrid[i]			=	new	CGrid[GetDesign()->H()];
		assert(m_ppGrid[i]);

		for (int j=0;j<GetDesign()->H();++j,++iIndex)
		{
			m_ppGrid[i][j].Initialize(i,j,GetKey(),this);
			m_ppGrid[i][j].m_pMaze		=	&m_pMaze[iIndex];
		}
	}
}

CGrid* CLayer::GetGrid(int iX, int iY)
{
	if(iX>=0&&iY>=0&&iX<GetDesign()->W()&&iY<GetDesign()->H())	return	&m_ppGrid[iX][iY];

	return	NULL;
}

CDesign* CLayer::GetParent()
{	
	return	(CDesign*)m_pParent;
}

double CLayer::GetCongestion(int iMode)
{
	switch(iMode) {
	case GET_MODE_MAX:
		{
			CBoundary*	pBoundary	=	GetMaxCBoundary();
			return	pBoundary? pBoundary->GetCongestion()/100.0:-1;
		}
		break;
	//case GET_MODE_AVG:
	//	{
	//		return	m_iSumC/100.0/(m_iMaxBX*m_iMaxBY);
	//	}
	default:
		assert(FALSE);
		break;
	}

	return	-1;
}

void CLayer::CreateBoundary()
{ 
	assert(IsHorizontal()||IsVertical());

	// different size of boundary array by direction [6/22/2006 thyeros]
	if(IsHorizontal())	
	{
		m_iMaxBX	=	GetDesign()->W()-1;
		m_iMaxBY	=	GetDesign()->H();
	}
	else		
	{
		m_iMaxBX	=	GetDesign()->W();
		m_iMaxBY	=	GetDesign()->H()-1;
	}

	for (int i=0;i<m_iMaxBX;++i)
		for (int j=0;j<m_iMaxBY;++j)
			GetBoundary(i,j)->CBoundary::Initialize(i,j,GetKey(),this);
}

void CLayer::GetBoundary(CGrid* pGrid, CBoundary** ppBoundary1, CBoundary** ppBoundary2)
{
	if(IsHorizontal())
	{
		if(ppBoundary1)	*ppBoundary1	=	GetBoundary(pGrid,pGrid->X()+1,pGrid->Y());
		if(ppBoundary2)	*ppBoundary2	=	GetBoundary(pGrid,pGrid->X()-1,pGrid->Y());
	}
	else
	{
		if(ppBoundary1)	*ppBoundary1	=	GetBoundary(pGrid,pGrid->X(),pGrid->Y()+1);
		if(ppBoundary2)	*ppBoundary2	=	GetBoundary(pGrid,pGrid->X(),pGrid->Y()-1);
	}
}

CBoundary* CLayer::GetBoundary(CPoint *pPoint1, int iX, int iY)
{
	return	GetBoundary(pPoint1->X(),pPoint1->Y(),iX,iY);
}

CBoundary* CLayer::GetBoundary(int iX, int iY)
{
	if(iX>=0&&iY>=0&&iX<m_iMaxBX&&iY<m_iMaxBY)	return	&m_ppGrid[iX][iY];

	//the outer grids don't have boundary [6/18/2006 thyeros]
	return	NULL;
}

int CLayer::GetCapacity(int iMode)
{
	switch(iMode){
	case GET_MODE_ACAP:	
	case GET_MODE_OCAP:	
		//	case GET_MODE_ECAP:	
		{
			int	iCapacity	=	0;

			for (int i=0;i<m_iMaxBX;++i)
				for (int j=0;j<m_iMaxBY;++j)
					iCapacity	+=	GetBoundary(i,j)->GetCapacity(iMode);

			return	iCapacity;
		}
		break;
	case GET_MODE_MAX:		return	m_iMaxCapacity;
		//	case GET_MODE_TARGET:	return	m_iTargetCapacity;
	default:
		assert(FALSE);
		break;
	}

	return	-1;
}

int CLayer::Z()
{
	return	GetKey();
}

int CLayer::IsHorizontal()
{
	assert((m_iDirection==DIR_VERTICAL&&m_iDirection!=DIR_HORIZONTAL)||(m_iDirection!=DIR_VERTICAL&&m_iDirection==DIR_HORIZONTAL));
	return	m_iDirection==DIR_HORIZONTAL;
}

int CLayer::IsVertical()
{
	assert((m_iDirection==DIR_VERTICAL&&m_iDirection!=DIR_HORIZONTAL)||(m_iDirection!=DIR_VERTICAL&&m_iDirection==DIR_HORIZONTAL));
	return	m_iDirection==DIR_VERTICAL;
}

//int CLayer::GetDesignRule(int iMode)
//{
//	switch(iMode){
//	case GET_DR_MIN_VIA_SPACING:	return	m_iMinViaSpacing;
//	case GET_DR_MIN_WIR_SPACING:	return	m_iMinWireSpacing;
//	case GET_DR_MIN_WIR_WIDTH:		return	m_iMinWireWidth;
//	default:
//		assert(FALSE);
//		break;
//	}
//	
//	return	-1;
//}

void CLayer::TrackWireLength(int iLength)
{
	m_iSumWL	+=	iLength;
	assert(m_iSumWL>=0);
}

void CLayer::TrackCongestion(CBoundary* pBoundary, int iOldC, int iNewC)
{
	assert(pBoundary);

	//ZERO(dOldC);
	//ZERO(dNewC);

	assert(iOldC>=0);
	assert(iNewC>=0);

	if(iOldC>100)
	{
		assert(m_CBoundaryBucketItr.find((ADDRESS)pBoundary)!=m_CBoundaryBucketItr.end());
		m_CBoundaryBucket.erase(m_CBoundaryBucketItr[(ADDRESS)pBoundary]);
		m_CBoundaryBucketItr.erase((ADDRESS)pBoundary);
		assert(m_CBoundaryBucketItr.find((ADDRESS)pBoundary)==m_CBoundaryBucketItr.end());
	}

	if(iNewC>100)
	{
		assert(m_CBoundaryBucketItr.find((ADDRESS)pBoundary)==m_CBoundaryBucketItr.end());
		m_CBoundaryBucketItr[(ADDRESS)pBoundary]	=	m_CBoundaryBucket.insert(pair<const int, CBoundary*>(iNewC, pBoundary));
		assert(m_CBoundaryBucketItr.find((ADDRESS)pBoundary)!=m_CBoundaryBucketItr.end());
	}

	//	m_dSumC	+=	(dNewC-dOldC);
}

void CLayer::GetStartBox(vector<int>* pX,vector<int>* pY)
{
	for (int i=0;i<m_iMaxBX;++i)			
		for (int j=0;j<m_iMaxBY;++j)
		{
			CBoundary*	pBoundary	=	GetBoundary(i,j);
			if(pBoundary->GetCongestion()>=1.0)
			{
				pX->push_back(pBoundary->X());
				pY->push_back(pBoundary->Y());
			}
		}				
}

void CLayer::Print(FILE *pFile, int iMode)
{
	if(pFile==NULL)	return;

	int i,j;
	switch(iMode){
	case PRINT_MODE_CGDUMP:
		for (i=0;i<m_iMaxBX;++i)			
			for (j=0;j<m_iMaxBY;++j)
			{
				GetBoundary(i,j)->Print(pFile,iMode);
				fprintf(pFile,"\n");
			}				
			break;
	case PRINT_MODE_RESULT:
	case PRINT_MODE_OFDUMP:
		for (i=0;i<m_iMaxBX;++i)
			for (j=0;j<m_iMaxBY;++j)
				GetBoundary(i,j)->Print(pFile,iMode);
		break;
	case PRINT_MODE_TEXT:
	case PRINT_MODE_CONGET:
		{	
			int	iMaxOF		=	GetOverFlow(GET_MODE_MAX);
			int	iOverFlow	=	GetOverFlow(GET_MODE_SUM);

			fprintf(pFile,"L%d(%c,%d):o(%3d,%3d)\n",
				Z(),IsHorizontal()? 'H':'V',GetCapacity(GET_MODE_MAX),iOverFlow,iMaxOF);
		}
		break;
	default:
		assert(FALSE);
		break;
	}

	fflush(pFile);
}

int CLayer::GetDirection()
{
	if (IsHorizontal())			return	DIR_HORIZONTAL;
	if (IsVertical())			return	DIR_VERTICAL;

	assert(FALSE);

	// this shouldn't happen, just to avoid warning [6/22/2006 thyeros]
	return	DIR_DIAGONAL;
}

int	CLayer::GetOverFlow(int iMode)
{
	int	iOverFlow		=	0;

	switch(iMode) {
	case GET_MODE_MAX:
		return	m_CBoundaryBucket.size()? GetMaxCBoundary(/*GET_MODE_PROP,PROP_BODR_ANY*/)->GetOverFlow():0;
		break;
	case GET_MODE_SUM:
		for(multimap<int, CBoundary*>::iterator itr=m_CBoundaryBucket.begin(),end=m_CBoundaryBucket.end();itr!=end;itr++)
			iOverFlow	+=	itr->second->GetOverFlow();
		return	iOverFlow;
	default:
		assert(FALSE);
		break;
	}

	return	-1;
}

void CLayer::GetAdjGrid()
{
	for (int i=0;i<GetDesign()->W();++i)
		for (int j=0;j<GetDesign()->H();++j)
		{
			assert(GetGrid(i,j));
			GetGrid(i,j)->GetAdjGrid(0);
		}
}

CBoundary* CLayer::GetMaxCBoundary(/*int iMode, int iValue*/)
{
	return	m_CBoundaryBucket.empty()? NULL:m_CBoundaryBucket.rbegin()->second;

	//if(iValue==PROP_BODR_ANY||iValue==STATE_BODR_ANY)	

	//switch(iMode) {
	//case GET_MODE_PROP:
	//	for(multimap<int, CBoundary*>::reverse_iterator ritr=m_CBoundaryBucket.rbegin(),rend=m_CBoundaryBucket.rend();ritr!=rend;++ritr)
	//		if(ritr->second->GetProp()&iValue)	return	ritr->second;
	//	break;
	//case GET_MODE_STATE:
	//	for(multimap<int, CBoundary*>::reverse_iterator ritr=m_CBoundaryBucket.rbegin(),rend=m_CBoundaryBucket.rend();ritr!=rend;++ritr)
	//		if(ritr->second->GetState()&iValue)	return	ritr->second;
	//	break;
	//default:
	//	assert(FALSE);
	//	break;
	//}
	//
	//return	NULL;
}

CBoundary* CLayer::GetMinCBoundary()
{
	return	m_CBoundaryBucket.empty()? NULL:m_CBoundaryBucket.begin()->second;
}

int CLayer::GetNumBoundary()
{
	return	m_iMaxBX*m_iMaxBY;
}

int CLayer::GetNumGrid()
{
	return	GetDesign()->W()*GetDesign()->H();
}

int CLayer::GetLength()
{
	return	m_iSumWL;
}

int CLayer::GetNumTrack(int iMode)
{
	int	iNumTrack	=	0;

	for (int i=0;i<m_iMaxBX;++i)
		for (int j=0;j<m_iMaxBY;++j)
			iNumTrack	+=	GetBoundary(i,j)->GetCapacity(iMode);

	return	iNumTrack;
}

void CLayer::GetCongestedBox(CBBox* pBBox)
{
	for(multimap<int, CBoundary*>::iterator itr=m_CBoundaryBucket.begin(),end=m_CBoundaryBucket.end();itr!=end;++itr)
	{
		CBoundary*	pBoundary	=	itr->second;

		pBBox->AddPoint(pBoundary);
	}
}

int CLayer::AddPenalty(int iPenalty)
{
	int	iMaxPenalty	=	0;

	for(multimap<int, CBoundary*>::iterator itr=m_CBoundaryBucket.begin(),end=m_CBoundaryBucket.end();itr!=end;++itr)
	{
		CBoundary*	pBoundary	=	itr->second;

		iMaxPenalty	=	MAX(iMaxPenalty,pBoundary->AddPenalty(iPenalty));
	}
	return	iMaxPenalty;
}

int CLayer::GetWireInOverFlowBoundary(hash_map<ADDRESS,int>* pWireList)
{
	int	iReRouting_Step		=	GetDesign()->m_Param.m_iReRouting_Step;
	for(multimap<int, CBoundary*>::iterator itr=m_CBoundaryBucket.begin(),end=m_CBoundaryBucket.end();itr!=end;++itr)
	{
		vector<CNet*>	NetList;
		CBoundary*	pBoundary	=	itr->second;

		int	iNumOF	=	pBoundary->GetOverFlow()*iReRouting_Step;

		sort(pBoundary->m_Wire.begin(),pBoundary->m_Wire.end(),CWireOpLen());
		for(vector<CWire*>::iterator itrw=pBoundary->m_Wire.begin(),end=pBoundary->m_Wire.end();itrw!=end;++itrw)
		{
			++(*pWireList)[(ADDRESS)(*itrw)];
			if(iNumOF--==0)	break;
		}
	}

	return	pWireList->size();
}

int CLayer::GetNetInOverFlowBoundary(hash_map<ADDRESS,int>* pNetList)
{
	for(multimap<int, CBoundary*>::iterator itr=m_CBoundaryBucket.begin(),end=m_CBoundaryBucket.end();itr!=end;++itr)
	{
		vector<CNet*>	NetList;
		CBoundary*	pBoundary	=	itr->second;

		for(vector<CWire*>::iterator itrw=pBoundary->m_Wire.begin(),end=pBoundary->m_Wire.end();itrw!=end;++itrw)
		{
			(*pNetList)[(ADDRESS)(*itrw)->GetParent()]	=	MAX(pBoundary->GetCongestion(),(*pNetList)[(ADDRESS)(*itrw)->GetParent()]);
		}
	}

	return	pNetList->size();
}

int CLayer::GetOverFlowBoundary(vector<CBoundary*>* pBoundaryList)
{
	for(multimap<int, CBoundary*>::iterator itr=m_CBoundaryBucket.begin(),end=m_CBoundaryBucket.end();itr!=end;++itr)
	{
		pBoundaryList->push_back(itr->second);
	}

	return	pBoundaryList->size();
}

