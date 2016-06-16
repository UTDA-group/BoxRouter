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
// Boundary.cpp: implementation of the CBoundary class.
//
//////////////////////////////////////////////////////////////////////

#include "Layer.h"
#include "Boundary.h"
#include "GRouter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBoundary::CBoundary()
{
	m_iNumOCap		=	0;
	m_iNumACap		=	0;
}

CBoundary::~CBoundary()
{

}

CLayer* CBoundary::GetParent()
{
	return	(CLayer*)m_pParent;
}

void CBoundary::Initialize(int iX, int iY, int iZ, CLayer* pParent)
{
	SetXYZ(iX,iY,iZ);

	m_pParent	=	(CObject*)pParent;

	CreateKey();

	m_iNumOCap		=	0;
	m_iNumACap		=	GetParent()->GetCapacity(GET_MODE_MAX);
	m_iCongestion	=	0;

	m_Wire.reserve(m_iNumACap/2);
}

int	CBoundary::AdjustCapacity(int iBlockage)
{
	//save old congestion number for TrackCongestion
	int		iOldC	=	GetCongestion();

	m_iNumOCap		+=	iBlockage;
	m_iNumACap		-=	iBlockage;
	m_iCongestion	=	100*m_iNumOCap/GetParent()->m_iMaxCapacity;

	assert(m_iNumOCap>=0);

	//update the congestion statictics in the parent layer
	GetParent()->TrackCongestion(this,iOldC,m_iCongestion);
	
	return	m_iNumOCap;
}

int CBoundary::AddWire(CWire *pWire)
{
	//this is called from CNet::Addwire when a new *ROUTED* wire is added
	assert(pWire->GetParent());
	assert(pWire->IsRouted());
	assert(pWire->IsFlat());
	assert(pWire->GetLayer()->Z()==Z());

	//save old congestion number for TrackCongestion
	int		iOldC	=	GetCongestion();

	// each boundary has only one single pass from every net [6/30/2006 thyeros]
	assert(!IsFound(pWire));

	m_Wire.push_back(pWire);

	assert(IsFound(pWire));

	// update occupied capacity [2/9/2007 thyeros]
	m_iNumOCap		+=	WIRE_WIDTH_SPACE;//(pWire->GetParent()->GetWidth()+GetParent()->GetDesignRule(GET_DR_MIN_WIR_SPACING));
	m_iNumACap		-=	WIRE_WIDTH_SPACE;//GetParent()->GetCapacity(GET_MODE_MAX)-m_iNumOCap;
	m_iCongestion	=	100*m_iNumOCap/GetParent()->m_iMaxCapacity;

	//update the congestion statictics in the parent layer
	GetParent()->TrackCongestion(this,iOldC,m_iCongestion);

	return	TRUE;
}

void CBoundary::DelWire(CWire *pWire)
{
	//this is called from CNet::DelWire when an existing  *ROUTED* wire is deleted
	assert(pWire->GetParent());

	//save old congestion numberf or TrackCongestion
	int		iOldC	=	GetCongestion();

	assert(IsFound(pWire));

	for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	{
		if(*itr==pWire)
		{
			m_Wire.erase(itr);
			break;
		}
	}

	assert(!IsFound(pWire));

	// update occupied capacity [2/9/2007 thyeros]
	m_iNumOCap		-=	WIRE_WIDTH_SPACE;//(pWire->GetParent()->GetWidth()+GetParent()->GetDesignRule(GET_DR_MIN_WIR_SPACING));
	m_iNumACap		+=	WIRE_WIDTH_SPACE;//GetParent()->GetCapacity(GET_MODE_MAX)-m_iNumOCap;
	m_iCongestion	=	100*m_iNumOCap/GetParent()->m_iMaxCapacity;
	
	assert(m_iNumOCap>=0);

	//update the congestion statictics in the parent layer
	GetParent()->TrackCongestion(this,iOldC,m_iCongestion);
}

int CBoundary::GetCapacity(int iMode)
{
	switch(iMode){
	case GET_MODE_ACAP:	return	MAX(0,m_iNumACap);	//# of available tracks
	case GET_MODE_OCAP:	return	m_iNumOCap;			//# of occupied tracks considering target congestion
	case GET_MODE_BCAP:	return	m_iNumOCap-m_Wire.size()*WIRE_WIDTH_SPACE;	//# of blockages
	//case GET_MODE_ECAP:	return	MAX(0,GetParent()->GetCapacity(GET_MODE_MAX)-m_iNumOCap);	//# of empty tracks w.r.t. the real maximum routing capacity
	default:
		assert(FALSE);
		break;
	}

	return	-1;
}

int CBoundary::GetNumWire()
{
	return	m_Wire.size();
}

void CBoundary::AddSegmentList(vector<CSegment*>* pSegment)
{
	//this is from CGRouter::ConcurrentRouting. the vector of segment is passing this boundary
	m_SegmentList.push_back(pSegment);
}

vector<CSegment*>* CBoundary::GetSegmentList(int iIndex)
{
	assert(iIndex>=0&&iIndex<GetNumSegmentList());
	return	m_SegmentList[iIndex];
}

int CBoundary::GetNumSegmentList()
{
	return	m_SegmentList.size();
}

void CBoundary::ClearSegmentList()
{
	m_SegmentList.clear();
}

void CBoundary::Print(FILE *pFile, int iMode)
{
	//print function for debugging/loggin g
	if(pFile==NULL)	return;

	assert(FALSE);

	//switch(iMode){
	//case PRINT_MODE_CGDUMP:
	//	fprintf(pFile,"%s(%3d,%3d,%3d) %.2f\t",GetParent()->IsHorizontal()? "H":"V",X(),Y(),Z(),GetCongestion(GET_MODE_ALL));
	//	break;
	//case PRINT_MODE_RESULT:
	//	fprintf(pFile,"b %d %d %d\t%d\n",X(),Y(),Z(),GetNumWire(GET_MODE_SPECIAL));
	//	break;
	//case PRINT_MODE_OFDUMP:
	//	if(GetOverFlow(GET_MODE_SUM))
	//	{
	//		fprintf(pFile,"bnd(%d,%d,%d): %d OF -%d\n",
	//			X(),Y(),Z(),GetOverFlow(GET_MODE_SUM),GetParent()->m_BoundaryPenalty[(ADDRESS)this]);
	//	}
	//	break;
	//case PRINT_MODE_TEXT:
	//	{
	//		fprintf(pFile,"bnd(%d,%d,%d): %d tracks\t%d max_cap\t%d tag_cap\t%d overflow\n",
	//			X(),Y(),Z(),
	//			GetNumWire(GET_MODE_ALL),
	//			GetParent()->GetCapacity(GET_MODE_MAX),
	//			GetParent()->GetCapacity(GET_MODE_TARGET),
	//			GetOverFlow(GET_MODE_SUM));
	//		fprintf(pFile,"-spnet takes %d tracks\n",GetNumWire(GET_MODE_SPECIAL));

	//		int	iIndex	=	1;

	//		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	//		{
	//			(*itr)->Print(pFile,iMode);
	//		}
	//	}
	//	break;
	//default:
	//	assert(FALSE);
	//	break;
	//}

	fflush(pFile);
}

int CBoundary::GetOverFlow()
{
	return	abs(MIN(0,m_iNumACap));
	//switch(iMode){
	//case GET_MODE_SUM:
	//	//get # of overflow w.r.t the real routing capaicity
	//case GET_MODE_TARGET:
	//	//get # of overflow w.r.t the target routing capacity
	//	return	MAX(0,GetCapacity(GET_MODE_OCAP)-GetParent()->GetCapacity(GET_MODE_TARGET));
	//default:
	//	assert(FALSE);
	//    break;
	//}
	//
	//return	-1;
}

int CBoundary::IsFound(CWire *pWire)
{
	//see if this wire is already crossing the boundary
	assert(pWire);
	assert(pWire->GetParent());
	return	IsFound(pWire->GetParent());
}

int CBoundary::IsFound(CNet *pNet)
{
	//see if any wire in this net is already crossing the boundary
	assert(pNet);
	for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		if((*itr)->GetParent()==pNet)	return	TRUE;

	return	FALSE;
}

int CBoundary::GetCongestion()
{ 
	return	m_iCongestion;
//	return	1.0*m_iNumOCap/GetParent()->m_iMaxCapacity;
}

int CBoundary::AddPenalty(int iPenalty)
{
	//if(iPenalty<0)
	//{
	//	m_iProp	=	0;
	//	if(GetOverFlow())
	//	{
	//		double	dPenalty	=	0;
	//		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	//		{
	//			dPenalty	+=	(*itr)->GetParent()->m_fPenalty;
	//		}
	//		m_iProp	=	dPenalty;
	//	}
	//}
	//else
	//{
		m_iProp	+=	iPenalty*SQRE(m_iCongestion)/10000.0;
	//}

	return	m_iProp;
}

int CBoundary::GetPenalty()
{
	return	m_iProp;
	//if(m_iNumACap<2)	return	m_iProp;
	//else				return	0;
}

