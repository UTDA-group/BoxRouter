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
// Segment.cpp: implementation of the CSegment class.
//
//////////////////////////////////////////////////////////////////////

#include "Segment.h"
#include "Wire.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
int	CSegment::m_iNumInstance	=	0;
#endif

stack<CSegment*> CSegment::m_InstancePool;

CSegment::CSegment()
{
	//m_cWidth	=	0;
	m_pPointE	=	NULL;
	m_pPointS	=	NULL;

#ifdef _DEBUG
	m_iNumInstance++;
#endif
}

CSegment::~CSegment()
{
#ifdef _DEBUG
	m_iNumInstance--;
#endif
}

CSegment*	CSegment::New()
{
	CSegment*	pSegment	=	NULL;

	if(m_InstancePool.empty())	pSegment	=	new	CSegment;
	else		
	{
		pSegment	=	m_InstancePool.top();m_InstancePool.pop();
	}

	assert(pSegment);
	return	pSegment;
}

void CSegment::Delete()
{
	m_InstancePool.push(this);
}


int CSegment::GetLength()
{
	return	m_pPointS->GetMDistance(m_pPointE);
}

CWire* CSegment::GetParent()
{
	return	(CWire*)m_pParent;
}

void CSegment::Initialize(int iX1, int iY1, int iX2, int iY2, int iZ, CWire* pParent)
{
	m_iProp	=	PROP_INVALID;

	if(iX1<iX2 || (iX1==iX2 && iY1<iY2))
	{
		m_pPointS	=	(CPoint*)GetDesign()->GetLayer(iZ)->GetGrid(iX1,iY1);
		m_pPointE	=	(CPoint*)GetDesign()->GetLayer(iZ)->GetGrid(iX2,iY2);

		//m_pPointS->Initialize(iX1,iY1,iZ,NULL);
		//m_pPointE->Initialize(iX2,iY2,iZ,NULL);
	}
	else
	{
		m_pPointS	=	(CPoint*)GetDesign()->GetLayer(iZ)->GetGrid(iX2,iY2);
		m_pPointE	=	(CPoint*)GetDesign()->GetLayer(iZ)->GetGrid(iX1,iY1);

		//m_pPointS->Initialize(iX2,iY2,iZ,NULL);	
		//m_pPointE->Initialize(iX1,iY1,iZ,NULL);
	}

	if (m_pPointS->IsHorizontal(m_pPointE))			m_iProp	|=	PROP_SEG_HORIZONTAL;
	else if (m_pPointS->IsVertical(m_pPointE))		m_iProp	|=	PROP_SEG_VERTICAL;

	assert(IsHorizontal()||IsVertical());

	m_pParent	=	(CObject*)pParent;

	//m_cWidth	=	GetParent()->GetParent()->GetWidth();
}

void CSegment::FixSE()
{
	int	iX1	=	m_pPointS->X();
	int	iY1	=	m_pPointS->Y();
	int	iZ1	=	m_pPointS->Z();
	int	iX2	=	m_pPointE->X();
	int	iY2	=	m_pPointE->Y();
	int	iZ2	=	m_pPointE->Z();

	if(iX1<iX2 || (iX1==iX2 && iY1<iY2))
	{
		//m_pPointS	=	(CPoint*)GetDesign()->GetLayer(iZ1)->GetGrid(iX1,iY1);
		//m_pPointE	=	(CPoint*)GetDesign()->GetLayer(iZ2)->GetGrid(iX2,iY2);

		//m_pPointS->Initialize(iX1,iY1,iZ1,NULL);
		//m_pPointE->Initialize(iX2,iY2,iZ2,NULL);
	}
	else
	{
		CPoint*	pPoint	=	m_pPointS;
		m_pPointS		=	m_pPointE;
		m_pPointE		=	pPoint;
		//m_pPointS	=	(CPoint*)GetDesign()->GetLayer(iZ2)->GetGrid(iX2,iY2);
		//m_pPointE	=	(CPoint*)GetDesign()->GetLayer(iZ1)->GetGrid(iX1,iY1);

		//m_pPointS->Initialize(iX2,iY2,iZ2,NULL);	
		//m_pPointE->Initialize(iX1,iY1,iZ1,NULL);
	}
}

void CSegment::Initialize(int iX1, int iY1, int iZ1, int iX2, int iY2, int iZ2, CWire* pParent)
{
	m_iProp	=	PROP_INVALID;

	// need to be fixed the order (FixSE) before actually used [6/25/2006 thyeros]
	//m_pPointS->Initialize(iX1,iY1,iZ1,NULL);
	//m_pPointE->Initialize(iX2,iY2,iZ2,NULL);

	m_pPointS	=	(CPoint*)GetDesign()->GetLayer(iZ1)->GetGrid(iX1,iY1);
	m_pPointE	=	(CPoint*)GetDesign()->GetLayer(iZ2)->GetGrid(iX2,iY2);


	if (m_pPointS->IsHorizontal(m_pPointE))			m_iProp	|=	PROP_SEG_HORIZONTAL;
	else if (m_pPointS->IsVertical(m_pPointE))		m_iProp	|=	PROP_SEG_VERTICAL;
	else if (m_pPointS->IsPerpendicular(m_pPointE))	m_iProp	|=	PROP_SEG_PERPENDICULAR;
	
	assert(IsHorizontal()||IsVertical()||IsPerpendicular());

	m_pParent	=	(CObject*)pParent;
}

int CSegment::IsHorizontal()
{
	return	m_iProp&PROP_SEG_HORIZONTAL;
}

int CSegment::IsVertical()
{
	return	m_iProp&PROP_SEG_VERTICAL;
}

int CSegment::IsPerpendicular()
{
	return	m_iProp&PROP_SEG_PERPENDICULAR;
}

void CSegment::Initialize(CPoint* pPoint1, int iX2, int iY2, int iZ2, CWire* pParent)
{
	Initialize(pPoint1->X(),pPoint1->Y(),pPoint1->Z(),iX2,iY2,iZ2,pParent);
}

void CSegment::Print(FILE *pFile, int Mode)
{
	if(pFile==NULL)	return;

	switch(Mode){
	case PRINT_MODE_TEXT:
		fprintf(pFile,"(%d,%d,%d)-(%d,%d,%d)\n",
			m_pPointS->X(),m_pPointS->Y(),m_pPointS->Z(),
			m_pPointE->X(),m_pPointE->Y(),m_pPointE->Z());
		break;
	default:
		assert(FALSE);
		break;
	}

	fflush(pFile);
}

CWire* CSegment::Pupate()
{
	CWire*	pWire	=	CWire::New();//new	CWire;

	assert(pWire);
	assert(m_pPointS->Z()==m_pPointE->Z());

	pWire->Initialize(m_pPointS->X(),m_pPointS->Y(),m_pPointE->X(),m_pPointE->Y(),m_pPointS->Z());

	// this will be deleted by CWire [6/30/2006 thyeros]
	return	pWire;
}

int CSegment::GetDirection()
{
	if (IsHorizontal())		return	DIR_HORIZONTAL;
	if (IsVertical())		return	DIR_VERTICAL;
	if (IsPerpendicular())	return	DIR_PERPENDICULAR;

	return	DIR_DIAGONAL;
}

//int CSegment::GetWidth()
//{
//	assert(m_cWidth>=1);
//	return	m_cWidth;
//}
//
