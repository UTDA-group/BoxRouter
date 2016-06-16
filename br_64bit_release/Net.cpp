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
// Net.cpp: implementation of the CNet class.
//
//////////////////////////////////////////////////////////////////////

#include "Net.h"
#include "flute.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNet::CNet()
{
	//m_cWidth	=	1;
	m_iNumPin	=	0;
	m_ppPin		=	NULL;
	m_iMinWL	=	MAX_NUMBER;
//	m_iMinVia	=	0;
}

CNet::~CNet()
{
	DelWires();

	for(int i=0;i<GetNumPin();++i)	SAFE_DEL(m_ppPin[i]);

	SAFE_DELA(m_ppPin);
}

void CNet::DelWires()
{
	while(!m_Wire.empty())
	{
		vector<CWire*>::iterator itr=m_Wire.begin();
		DelWire(*itr,DELWIRE_MODE_NOMAP);
		(*itr)->Delete();//SAFE_DEL(*itr);
		m_Wire.erase(itr);
	}
	m_Wire.clear();
}

void CNet::Initialize(int iIndex, /*char cWidth, */CDesign *pParent)
{
	m_iProp		=	PROP_INVALID;

//	m_cWidth	=	cWidth;
	m_Key		=	iIndex;
	m_pParent	=	(CObject*)pParent;
	
	SetState(STATE_NET_UNROUTED);
//	SetState(STATE_NET_UNREROUTED);
}

CPin* CNet::GetPin(int iX,int iY)
{
	CPoint	Point;
	Point.Initialize(iX,iY,0,NULL);

	return	GetPin(&Point);
}

CPin* CNet::GetPin(CPoint* pPoint, int* pIndex)
{
	for (int i=0;i<GetNumPin();++i)
		if(pPoint->IsSame2D(GetPin(i)))
		{
			if(pIndex)	*pIndex	=	i;
			return	GetPin(i);
		}

	return	NULL;
}

CPin* CNet::GetPin(int iIndex)
{
	return	m_ppPin[iIndex];
}

int CNet::GetLength(int iMode, int iValue)
{
	if(IsLocal())	return	0;
	
	int	iLength	=	0;
	switch(iMode) {
	case GET_MODE_PROP:
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			if(pWire->GetProp()&iValue)	iLength	+=	pWire->GetLength();
		}
		break;
	case GET_MODE_STATE:
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			if(pWire->GetState()&iValue)	iLength	+=	pWire->GetLength();
		}
		break;
	case GET_MODE_LAYER:
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			CLayer*	pLayer	=	pWire->GetLayer();

			if(pLayer&&pLayer->Z()==iValue)	iLength	+=	pWire->GetLength();
		}
		break;
	default:
		assert(FALSE);
		break;
	}

	return	iLength;
}

int CNet::DelWire(CWire *pWire, int iMode)
{
	assert(pWire);
	assert(pWire->GetParent()==this);
	assert(IsFound(pWire));
	
	SetState(STATE_NET_BBOXDIRTY);

	if(pWire->IsCompleted())
	{
		//thyeros: update boundary [6/18/2006 thyeros]
		pWire->UpdateBoundary(UPBOUND_MODE_DEL);
	}

	// need to check whether it works [6/18/2006 thyeros]
	if(iMode==DELWIRE_MODE_FRMAP)
	{
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			if((*itr)->GetKey()==pWire->GetKey())
			{
				m_Wire.erase(itr);
				break;
			}
		}
		assert(!IsFound(pWire));	
	}
	
	pWire->SetParent(NULL);


#ifdef _DEBUG
	if((CWire*)0xe984f0==pWire)
	{
		int a=0;
	}
#endif
	return	TRUE;
}

int CNet::AddWire(CWire *pWire)
{
	assert(pWire);
	assert(pWire->GetParent()==NULL);

	SetState(STATE_NET_BBOXDIRTY);

	CWire*	pExist	=	IsFound(pWire);
	if(pExist)
	{
		if(pWire->IsRouted()&&!pExist->IsRouted())
		{
			// make old wire routed and discard the new wire [6/30/2006 thyeros]
			pExist->MakeRouted(pWire->GetLayer()->Z());

			assert(pWire->GetParent()==NULL);
			pWire->Delete();//SAFE_DEL(pWire);

			return	FALSE;

		}
		else if(!pWire->IsRouted()||pExist->IsRouted())	
		{
			assert(pWire->GetParent()==NULL);
			pWire->Delete();//SAFE_DEL(pWire);

			return	FALSE;
		}
		else			
		{
			assert(FALSE);
		}
	}

	pWire->SetParent(this);
	assert(pWire->GetParent()==this);
	
	if(pWire->IsCompleted())
	{
		vector<CWire*>	AdjWire	=	pWire->AdjustByBoundary();
		for (int i=0,s=AdjWire.size();i<s;++i)
		{
			CWire*	pCurWire	=	AdjWire[i];
			//thyeros: update boundary [6/18/2006 thyeros]
			pCurWire->UpdateBoundary(UPBOUND_MODE_ADD);
		
			CWire*	pExist	=	IsFound(pCurWire);
			if(pExist)
			{
				if(pCurWire->IsRouted()&&!pExist->IsRouted())
				{
					// new wire will replace the old unrouted one [6/30/2006 thyeros]
					DelWire(pExist);
				}
				else			
				{
					assert(FALSE);
				}
			}

			assert(!IsFound(pCurWire));
			m_Wire.push_back(pCurWire);
			assert(IsFound(pCurWire));
		}
	}
	else
	{
		assert(!IsFound(pWire));
		m_Wire.push_back(pWire);
		assert(IsFound(pWire));
	}

	return	TRUE;
}

CWire* CNet::MergeWire(CWire* pWire)
{
	hash_map<KEY, int>				RedPoint;

	for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	{
		CWire*	pWire	=	*itr;

		++RedPoint[pWire->m_pPointS->GetKey()&0xFFFFFF00];
		++RedPoint[pWire->m_pPointE->GetKey()&0xFFFFFF00];
	}

	for(int i=0;i<GetNumPin();++i)	RedPoint.erase(GetPin(i)->GetKey()&0xFFFFFF00);


	hash_map<KEY, int>::iterator	itrp;

	while(TRUE)
	{
		int	iExit	=	TRUE;
		
		itrp	=	RedPoint.find(pWire->m_pPointS->GetKey()&0xFFFFFF00);
		if(itrp!=RedPoint.end()&&itrp->second==2)
		{
			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pCurWire	=	*itr;
				if(pCurWire!=pWire&&pCurWire->IsOn2D(pWire->m_pPointS))
				{
					pWire	=	pWire->MergeWire(pWire->m_pPointS,pCurWire);
					iExit	=	FALSE;
					break;
				}
			}
		}

		if(pWire==NULL||pWire->GetKey()==0)	return	NULL;

		itrp	=	RedPoint.find(pWire->m_pPointE->GetKey()&0xFFFFFF00);
		if(itrp!=RedPoint.end()&&itrp->second==2)
		{
			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pCurWire	=	*itr;
				if(pCurWire!=pWire&&pCurWire->IsOn2D(pWire->m_pPointE))
				{
					pWire	=	pWire->MergeWire(pWire->m_pPointE,pCurWire);			
					iExit	=	FALSE;
					break;
				}
			}
		}

		if(pWire==NULL||pWire->GetKey()==0)	return	NULL;

		if(iExit)	break;
	}

	return	pWire;
}

void CNet::MergeWire()
{
	hash_map<KEY, int>				RedPoint;

	for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	{
		CWire*	pWire	=	*itr;

		++RedPoint[pWire->m_pPointS->GetKey()&0xFFFFFF00];
		++RedPoint[pWire->m_pPointE->GetKey()&0xFFFFFF00];
	}

	for(int i=0;i<GetNumPin();++i)	RedPoint.erase(GetPin(i)->GetKey()&0xFFFFFF00);

	hash_map<KEY, int>::iterator		itrp,endp;
	for(itrp=RedPoint.begin(),endp=RedPoint.end();itrp!=endp;itrp++)
	{
		if(itrp->second==2)	// single path point [7/5/2006 thyeros]
		{
			KEY	P	=	itrp->first;
			
			CPoint	CurRedPoint;
			CurRedPoint.Initialize(CPoint::X(P),CPoint::Y(P),CPoint::Z(P),NULL);
			
			CWire*	pMergeWire	=	NULL;
			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pWire	=	*itr;
				if(pWire->IsOn2D(&CurRedPoint))
				{
					if(pMergeWire==NULL)
					{
						pMergeWire	=	pWire;
					}
					else
					{
						pMergeWire->MergeWire(&CurRedPoint,pWire);
						break;
					}
				}
			}
		}
	}
}

void CNet::SplitWire()
{
	hash_map<KEY, int>				RedPoint;
	
	for(int i=0;i<GetNumPin();++i)
	{
		++RedPoint[GetPin(i)->GetKey()&0xFFFFFF00];
	}

	vector<CWire*>::iterator	itrn;
	for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	{
		CWire*	pWire	=	*itr;
		if(pWire->IsRouted())
		{	
			++RedPoint[pWire->m_pPointS->GetKey()&0xFFFFFF00];
			++RedPoint[pWire->m_pPointE->GetKey()&0xFFFFFF00];
			
			itrn=itr;
			
			for(++itrn;itrn!=end;++itrn)
			{
				CPoint	CrossingPoint;
				if((*itrn)->IsRouted()&&pWire->IsCrossing((*itrn),&CrossingPoint))	++RedPoint[CrossingPoint.GetKey()&0xFFFFFF00];
			}
		}
	}

	for(hash_map<KEY, int>::iterator	itrp=RedPoint.begin(),endp=RedPoint.end();itrp!=endp;++itrp)
	{
		KEY	P	=	itrp->first;

		CPoint	CurRedPoint;
		CurRedPoint.Initialize(CPoint::X(P),CPoint::Y(P),0,NULL);
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;)
		{
			CWire*	pWire	=	*itr;
			++itr;

			if(pWire->IsRouted())
			{	
				CurRedPoint.SetZ(pWire->GetLayer()->Z());			
				if(pWire->SplitWire(&CurRedPoint))
				{
					pWire->Delete();//SAFE_DEL(pWire);
					itr	=	m_Wire.begin();
					end	=	m_Wire.end();
				}
			}
		}
	}
}

void CNet::Refine()
{
	if(IsLocal())	return;
	if(!IsRouted())	return;

	SplitWire();

	hash_map<KEY, int>				RedPoint;
	hash_map<KEY, int>::iterator		itrp;

	vector<CWire*>::iterator itr,end;
	for(itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	{
		CWire*	pWire	=	*itr;

		++RedPoint[pWire->m_pPointS->GetKey()&0xFFFFFF00];
		++RedPoint[pWire->m_pPointE->GetKey()&0xFFFFFF00];
	}

	for(int i=0;i<GetNumPin();++i)
		RedPoint.erase(GetPin(i)->GetKey()&0xFFFFFF00);

	for(itrp=RedPoint.begin();itrp!=RedPoint.end();)
	{
		if(itrp->second==1)	// dangling point [7/5/2006 thyeros]
		{
			KEY	P	=	itrp->first;
	
			for(itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pWire	=	*itr;

				KEY S	=	pWire->m_pPointS->GetKey()&0xFFFFFF00;
				KEY	E	=	pWire->m_pPointE->GetKey()&0xFFFFFF00;

				if(P==S||P==E)
				{
					DelWire(pWire);
					pWire->Delete();//SAFE_DEL(pWire);

					--RedPoint[S];
					--RedPoint[E];

					break;
				}
			}

			itrp=RedPoint.begin();
		}
		else
		{
			++itrp;
		}
	}

	m_iMinWL	=	MIN(m_iMinWL,GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED));
//	m_iMinVia	=	MIN(m_iMinVia,GetNumVia());

	assert(m_Wire.size());
}

int CNet::GetNumWire(int iMode, int iValue)
{
	int	iNum	=	0;
	switch(iMode) {
	case GET_MODE_PROP:
		if(iValue==PROP_WIRE_ANY)	return	m_Wire.size();
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			if(pWire->GetProp()&iValue)	++iNum;
		}
		break;
	case GET_MODE_STATE:
		if(iValue==STATE_WIRE_ANY)	return	m_Wire.size();			
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			if(pWire->GetState()&iValue)	++iNum;
		}
		break;
	case GET_MODE_LAYER:
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;

			if(pWire->IsCompleted()&&
				!pWire->IsPoint()&&
				pWire->GetLayer()->Z()==iValue)	++iNum;
		}
		break;
	default:
		assert(FALSE);
		break;
	}

	return	iNum;
}

void CNet::SetState(int iState)
{
	switch(iState) {
	case STATE_NET_PREROUTED:
		m_iState	|=	STATE_NET_PREROUTED;
		break;
	//case STATE_NET_REROUTED:
	//	m_iState	&=	~STATE_NET_UNREROUTED;	
	//	m_iState	|=	STATE_NET_REROUTED;
	//	break;
	//case STATE_NET_UNREROUTED:
	//	m_iState	&=	~STATE_NET_REROUTED;	
	//	m_iState	|=	STATE_NET_UNREROUTED;
	//	break;		
	case STATE_NET_ROUTED:
		m_iState	&=	~STATE_NET_UNROUTED;	
		m_iState	|=	STATE_NET_ROUTED;
		break;
	case STATE_NET_UNROUTED:
		m_iState	&=	~STATE_NET_ROUTED;	
		m_iState	|=	STATE_NET_UNROUTED;
		break;
	case STATE_NET_BBOXDIRTY:
		m_iState	|=	STATE_NET_BBOXDIRTY;
		m_iState	&=	~STATE_NET_BBOXCLEAN;
		break;
	case STATE_NET_BBOXCLEAN:
		m_iState	|=	STATE_NET_BBOXCLEAN;
		m_iState	&=	~STATE_NET_BBOXDIRTY;
		break;
	default:
		assert(false);
		break;
	}
}

int CNet::IsShortest(int iMargin)
{
	return	(m_iMinWL+iMargin)>=GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
}

int	CNet::IsOptimal2(int iMargin)
{
//	if(GetDesign()->T()==2)	return	(m_iMinWL+iMargin)>=GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);

	if(IsFlat() && m_BBox.T()<2)																		return	TRUE;		

	return	FALSE;
}

int	CNet::IsOptimal(int iMargin)
{
	if(GetDesign()->T()==2)	return	(m_iMinWL+iMargin)>=GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);

	if(IsFlat() && m_BBox.T()<2)																		return	TRUE;
	if(GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED)<=(m_iMinWL+iMargin)	&& GetNumVia()<=GetNumPin())	return	TRUE;
	
	//if(GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED)<=(m_iMinWL+iMargin)	&& GetNumVia()<=(m_iMinVia+iMargin))	return	TRUE;
	//


	return	FALSE;

	//if(GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED)<=(m_iMinWL+iMargin)	&& GetNumVia()<=(m_iMinVia+iMargin))	
	//	return	TRUE;
	
	

	//return	FALSE;

	//if((m_iMinWL+iMargin)>=GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED))
	//{
	//	multimap<KEY, int>	ViaBucket;
	//	GetViaBucket(&ViaBucket);


	//	int	iNumViaWire	=	0;
	//	multimap<KEY, int>::iterator itrl;
	//	multimap<KEY, int>::iterator itru;
	//	multimap<KEY, int>::iterator itrv;

	//	for(itrv=ViaBucket.begin();itrv!=ViaBucket.end();itrv=itrl)
	//	{
	//		itrl	=	ViaBucket.lower_bound(itrv->first);
	//		itru	=	ViaBucket.upper_bound(itrv->first);

	//		int	iMaxZ	=	0;
	//		int	iMinZ	=	MAX_NUMBER;
	//		for (;itrl!=itru;++itrl)
	//		{
	//			iMaxZ	=	MAX(itrl->second,iMaxZ);
	//			iMinZ	=	MIN(itrl->second,iMinZ);
	//		}	

	//		if(iMaxZ-iMinZ>1)	return	FALSE;
	//	}

	//	return	TRUE;

	//}

	//return	FALSE;
}

//int CNet::IsRerouted()
//{
//	return	GetState()&STATE_NET_REROUTED;
//}

int CNet::IsRouted()
{
	for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	{
		CWire*	pWire	=	*itr;
		if(!pWire->IsRouted())	return	FALSE;
	}

	return	TRUE;
}

int CNet::IsFlat()
{
	vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();

	int	iDirection	=	(*itr)->GetDirection();
	for(++itr;itr!=end;++itr)
		if((*itr)->GetDirection()!=iDirection)	return	FALSE;

	return	TRUE;
}

//int CNet::CompareByLength(const void* A, const void* B)
//{
//	CNet* pNetA		=	*((CNet**)A);
//	CNet* pNetB		=	*((CNet**)B);
//
//	return	pNetB->GetLength(GET_MODE_STATE,STATE_WIRE_ANY)-pNetA->GetLength(GET_MODE_STATE,STATE_WIRE_ANY);
//}

int CNet::IsLocal()
{
	return	m_iProp&PROP_NET_LOCAL;
}

CDesign* CNet::GetParent()
{
	return	(CDesign*)m_pParent;
}

int CNet::GetNumPin()
{
	return	m_iNumPin;
}

void CNet::AddPin(vector<CPin*>* pPinList)
{
	if(pPinList->size())
	{
		m_ppPin	=	new	CPin*[pPinList->size()];
		for(m_iNumPin=0;m_iNumPin<pPinList->size();m_iNumPin++)	m_ppPin[m_iNumPin]	=	(*pPinList)[m_iNumPin];
	}

	switch(GetNumPin()){
	case 0:
	case 1:
		m_iProp	|=	PROP_NET_LOCAL;
		break;
	case 2:
		m_iProp	&=	~PROP_NET_LOCAL;
		m_iProp	|=	PROP_NET_TWOPIN;
		break;
	default:
		m_iProp	&=	~(PROP_NET_LOCAL|PROP_NET_TWOPIN);
		m_iProp	|=	PROP_NET_GLOBAL;
		break;
	}
}

int CNet::GetWireInBBox(vector<CWire*>* pWireList, CBBox* pBBox, int iMode, int iValue)
{
	int	iNumWire	=	0;
	for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	{
		CWire*	pWire	=	*itr;

		assert(pWire);
		assert(pWire->GetParent());

		int		iContinue	=	TRUE;

		switch(iMode){
		case GET_MODE_STATE:
			if(pWire->GetState()&iValue)	iContinue	=	FALSE;
			break;
		case GET_MODE_PROP:
			if(pWire->GetProp()&iValue)		iContinue	=	FALSE;
		    break;
		case GET_MODE_LENGTH:
			if(pWire->GetLength()>=iValue)	iContinue	=	FALSE;
			break;
		default:
			assert(FALSE);
		    break;
		}
		
		if(iContinue)	continue;

		if(pBBox->IsInside(pWire->m_pPointS)&&pBBox->IsInside(pWire->m_pPointE))
		{
			++iNumWire;
			if(pWireList)	pWireList->push_back(pWire);
		}
	}

	return	iNumWire;
}

int CNet::GetPoints(vector<CPoint*>* pPoints, CPoint *pPoint, int iMode, int iValue)
{
	vector<CWire*> Wires;
	if(GetWires2D(&Wires,pPoint,iMode,iValue))
	{
		for(vector<CWire*>::iterator itr=Wires.begin(),end=Wires.end();itr!=end;++itr)
		{
			assert(!(*itr)->IsPoint());
			(*itr)->GetPoints(pPoints);			
		}
	}
	else if(pPoint)
	{
		pPoints->push_back(pPoint);	
	}

	return	pPoints->size();
}

int CNet::IsConnected(CPoint *pPoint1, CPoint *pPoint2)
{
	hash_map<KEY,CWire*>	Wires;
	
	deque<CPoint*> Queue;
	
	Queue.push_front(pPoint1);

	while(!Queue.empty())
	{
		CPoint*	pPoint	=	Queue.front();Queue.pop_front();

		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			assert(pWire);
			
			if(Wires.find(pWire->GetKey())!=Wires.end())	continue;
				
			if(pWire->IsCompleted())
			{
				int	iRet	=	pWire->IsOn2D(pPoint);
			
				if(iRet&RET_WIRE_SE)
				{
					if(pWire->IsOn2D(pPoint2))	return	TRUE;
					Wires[pWire->GetKey()]	=	pWire;
				}

				if(iRet&RET_WIRE_S)		Queue.push_front(pWire->m_pPointE);				
				if(iRet&RET_WIRE_E)		Queue.push_front(pWire->m_pPointS);
			}
		}
	}

	return	FALSE;
}

int CNet::GetWires2D(vector<CWire*>* pWires, CPoint* pPoint, int iMode, int iValue)
{
	if(!pPoint)
	{
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			assert(find(pWires->begin(),pWires->end(),pWire)==pWires->end());

			int	iRet	=	RET_WIRE_NONE;
			switch(iMode) {
			case GET_MODE_PROP:
				if(pWire->GetProp()&iValue)		pWires->push_back(pWire);
				break;
			case GET_MODE_STATE:
				if(pWire->GetState()&iValue)	pWires->push_back(pWire);
				break;
			default:
				assert(FALSE);
				break;
			}
		}
	}
	else
	{
		deque<CPoint*> Queue;

		Queue.push_front(pPoint);

		while(!Queue.empty())
		{
			pPoint	=	Queue.front();Queue.pop_front();
			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pWire	=	*itr;

				if(find(pWires->begin(),pWires->end(),pWire)!=pWires->end())	continue;

				int	iRet	=	RET_WIRE_NONE;
				switch(iMode) {
				case GET_MODE_PROP:
					if(pWire->GetProp()&iValue)		iRet	=	pWire->IsOn2D(pPoint);
					break;
				case GET_MODE_STATE:
					if(pWire->GetState()&iValue)	iRet	=	pWire->IsOn2D(pPoint);
					break;
				default:
					assert(FALSE);
					break;
				}

				if(iRet&RET_WIRE_SE)	pWires->push_back(pWire);
				if(iRet&RET_WIRE_E)		Queue.push_front(pWire->m_pPointS);				
				if(iRet&RET_WIRE_S)		Queue.push_front(pWire->m_pPointE);
			}
		}
	}
	return	pWires->size();
}

int CNet::GetWires3D(vector<CWire*>* pWires, CPoint* pPoint, int iMode, int iValue)
{
	if(!pPoint)
	{
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			assert(find(pWires->begin(),pWires->end(),pWire)==pWires->end());

			int	iRet	=	RET_WIRE_NONE;
			switch(iMode) {
			case GET_MODE_PROP:
				if(pWire->GetProp()&iValue)		pWires->push_back(pWire);
				break;
			case GET_MODE_STATE:
				if(pWire->GetState()&iValue)	pWires->push_back(pWire);
				break;
			default:
				assert(FALSE);
				break;
			}
		}
	}
	else
	{
		deque<CPoint*> Queue;
		
		Queue.push_front(pPoint);
		
		while(!Queue.empty())
		{
			pPoint	=	Queue.front();Queue.pop_front();
			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pWire	=	*itr;

				if(find(pWires->begin(),pWires->end(),pWire)!=pWires->end())	continue;
				
				int	iRet	=	RET_WIRE_NONE;
				switch(iMode) {
				case GET_MODE_PROP:
					if(pWire->GetProp()&iValue)		iRet	=	pWire->IsOn3D(pPoint);
					break;
				case GET_MODE_STATE:
					if(pWire->GetState()&iValue)	iRet	=	pWire->IsOn3D(pPoint);
					break;
				default:
					assert(FALSE);
					break;
				}
				
				if(iRet&RET_WIRE_SE)	pWires->push_back(pWire);
				if(iRet&RET_WIRE_E)		Queue.push_front(pWire->m_pPointS);				
				if(iRet&RET_WIRE_S)		Queue.push_front(pWire->m_pPointE);
			}
		}
	}
	return	pWires->size();
}

CWire* CNet::IsFound(CWire *pWire)
{
	for(vector<CWire*>::iterator itr=m_Wire.begin();itr!=m_Wire.end();itr++)
	{
		if((*itr)->GetKey()==pWire->GetKey())	return	*itr;
	}
	return	NULL;
}

void CNet::Print(FILE *pFile, int iMode)
{
	if(pFile==NULL)	return;

	switch(iMode){
	case PRINT_MODE_DUMP:
		{
			KEY	Key		=	GetKey();
			if(!fwrite(&Key,1,sizeof(KEY),pFile))		Display(DISPLAY_MODE_ERRO,"print key in net(id:%d)\n",(int)GetParent()->GetKey());

			multimap<KEY, int>	ViaBucket;
			GetViaBucket(&ViaBucket);

		
			int	iNumViaWire	=	0;
			multimap<KEY, int>::iterator itrl;
			multimap<KEY, int>::iterator itru;
			multimap<KEY, int>::iterator itrv;

			// be careful about itrv=itru [3/5/2007 thyeros]
			for(itrv=ViaBucket.begin();itrv!=ViaBucket.end();itrv=itrl)
			{
				itrl	=	ViaBucket.lower_bound(itrv->first);
				itru	=	ViaBucket.upper_bound(itrv->first);

				int	iMaxZ	=	0;
				int	iMinZ	=	MAX_NUMBER;
				for (;itrl!=itru;++itrl)
				{
					iMaxZ	=	MAX(itrl->second,iMaxZ);
					iMinZ	=	MIN(itrl->second,iMinZ);
				}	

				if(iMaxZ!=iMinZ)	iNumViaWire++;
			}

			int	iNumWire	=	GetNumWire()+iNumViaWire;
			if(!fwrite(&iNumWire,1,sizeof(int),pFile))	Display(DISPLAY_MODE_ERRO,"print #(wire+via) in net(id:%d)\n",(int)GetParent()->GetKey());

			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pWire	=	*itr;
				pWire->Print(pFile,iMode);
			}

			if(iNumViaWire)
			{
				for(itrv=ViaBucket.begin();itrv!=ViaBucket.end();itrv=itrl)
				{
					itrl	=	ViaBucket.lower_bound(itrv->first);
					itru	=	ViaBucket.upper_bound(itrv->first);

					int	iMaxZ	=	0;
					int	iMinZ	=	MAX_NUMBER;
					for (;itrl!=itru;++itrl)
					{
						iMaxZ	=	MAX(itrl->second,iMaxZ);
						iMinZ	=	MIN(itrl->second,iMinZ);
					}	

					if(iMaxZ!=iMinZ)
					{
						int	iX	=	CPoint::X(itrv->first);
						int	iY	=	CPoint::Y(itrv->first);

						KEY	Key;
						Key		=	CPoint::MakeKey(iX,iY,iMinZ);
						if(!fwrite(&Key,1,sizeof(KEY),pFile))	Display(DISPLAY_MODE_ERRO,"print a via(B) in net(id:%d)\n",(int)GetParent()->GetKey());

						Key		=	CPoint::MakeKey(iX,iY,iMaxZ);
						if(!fwrite(&Key,1,sizeof(KEY),pFile))	Display(DISPLAY_MODE_ERRO,"print a via(T) in net(id:%d)\n",(int)GetParent()->GetKey());
					}
				}
			}
		}
		break;
	case PRINT_MODE_PIN:
		for(int i=0;i<GetNumPin();++i)	GetPin(i)->Print(pFile,PRINT_MODE_TEXT);		
		break;
	//case PRINT_MODE_DEBUG:
	//	fprintf(pFile,"net(id:%d)(0x%x)..L:%d\n",(int)GetKey(),(ADDRESS)this,GetLength(GET_MODE_STATE,STATE_WIRE_ANY));
	//	break;
	case PRINT_MODE_TEXT:
		{
			if(IsLocal())	fprintf(pFile,"#local net\n");
	
#ifdef _DEBUG
			fprintf(pFile,"net(id:%d)(0x%x)\n",(int)GetKey(),(ADDRESS)this);
#else
			fprintf(pFile,"net(id:%d)\n",(int)GetKey());
#endif

			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pWire	=	*itr;
				pWire->Print(pFile,iMode);
			}
			
			for(int i=0;i<GetNumPin();++i)	GetPin(i)->Print(pFile,iMode);		
		}
		break;
	case PRINT_MODE_GNUPLOT3D:
		{
			fprintf(pFile,"#net(id:%d) \n",(int)GetKey());

			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pWire	=	*itr;
				pWire->Print(pFile,iMode);
			}

			int i,j,k;
			for(i=0;i<GetNumPin();++i)	GetPin(i)->Print(pFile,iMode);		

			UpdateBBox();

			int	iMinX	=	m_BBox.X();
			int	iMaxX	=	iMinX+m_BBox.W();
			int	iMinY	=	m_BBox.Y();
			int	iMaxY	=	iMinY+m_BBox.H();
			int	iMinZ	=	m_BBox.Z();
			int	iMaxZ	=	iMinZ+m_BBox.T();
									
			for(i=iMinX;i<=iMaxX;++i)
				for(j=iMinY;j<=iMaxY;++j)
					for(k=MAX(1,iMinZ);k<=iMaxZ;++k)
						GetDesign()->GetLayer(k)->GetGrid(i,j)->Print(pFile,iMode);
			
			multimap<KEY, int>	ViaBucket;
			GetViaBucket(&ViaBucket);
					
			multimap<KEY, int>::iterator itrl;
			multimap<KEY, int>::iterator itru;
			multimap<KEY, int>::iterator itrv;
			for(itrv=ViaBucket.begin();itrv!=ViaBucket.end();itrv=itrl)
			{
				itrl	=	ViaBucket.lower_bound(itrv->first);
				itru	=	ViaBucket.upper_bound(itrv->first);
				
				int	iMaxZ	=	0;
				int	iMinZ	=	MAX_NUMBER;
				for (;itrl!=itru;++itrl)
				{
					iMaxZ	=	MAX(itrl->second,iMaxZ);
					iMinZ	=	MIN(itrl->second,iMinZ);
				}	
				
				int	iX	=	CPoint::X(itrv->first);
				int	iY	=	CPoint::Y(itrv->first);
				
				fprintf(pFile,"splot '-' w lines lt 2 lw 1\n %d %d %d\n %d %d %d\ne\n",iX,iY,iMaxZ,iX,iY,iMinZ);
			}
		}
	case PRINT_MODE_MATLAB3D:
		{
			fprintf(pFile,"%%net(id:%d) \n",(int)GetKey());

			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pWire	=	*itr;
				pWire->Print(pFile,iMode);
			}

			int i,j,k;
			for(i=0;i<GetNumPin();++i)	GetPin(i)->Print(pFile,iMode);		

			UpdateBBox();

			int	iMinX	=	m_BBox.X();
			int	iMaxX	=	iMinX+m_BBox.W();
			int	iMinY	=	m_BBox.Y();
			int	iMaxY	=	iMinY+m_BBox.H();
			int	iMinZ	=	m_BBox.Z();
			int	iMaxZ	=	iMinZ+m_BBox.T();
									
			for(i=iMinX;i<=iMaxX;++i)
				for(j=iMinY;j<=iMaxY;++j)
					for(k=MAX(1,iMinZ);k<=iMaxZ;++k)
						GetDesign()->GetLayer(k)->GetGrid(i,j)->Print(pFile,iMode);
					
			multimap<KEY, int>	ViaBucket;
			GetViaBucket(&ViaBucket);

			multimap<KEY, int>::iterator itrl;
			multimap<KEY, int>::iterator itru;
			multimap<KEY, int>::iterator itrv;
			for(itrv=ViaBucket.begin();itrv!=ViaBucket.end();itrv=itrl)
			{
				itrl	=	ViaBucket.lower_bound(itrv->first);
				itru	=	ViaBucket.upper_bound(itrv->first);
				
				int	iMaxZ	=	0;
				int	iMinZ	=	MAX_NUMBER;
				for (;itrl!=itru;itrl++)
				{
					iMaxZ	=	MAX(itrl->second,iMaxZ);
					iMinZ	=	MIN(itrl->second,iMinZ);
				}	
				
				int	iX	=	CPoint::X(itrv->first);
				int	iY	=	CPoint::Y(itrv->first);
				
				fprintf(pFile,"plot3([%d,%d],[%d,%d],[%d,%d],'g');\n",iX,iX,iY,iY,iMaxZ,iMinZ);
				fprintf(pFile,"plot3([%d,%d],[%d,%d],[%d,%d],'.g');\n",iX,iX,iY,iY,iMaxZ,iMaxZ);
				
				if(iMinZ)	fprintf(pFile,"plot3([%d,%d],[%d,%d],[%d,%d],'.g');\n",iX,iX,iY,iY,iMinZ,iMinZ);
			}
		}
		break;	
	case PRINT_MODE_MATLAB:
		fprintf(pFile,"%%net(id:%d) \n",(int)GetKey());
	case PRINT_MODE_GNUPLOT:
		{
			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
			{
				CWire*	pWire	=	*itr;
				pWire->Print(pFile,iMode);
			}
			
			int i,j;
			for(i=0;i<GetNumPin();++i)	GetPin(i)->Print(pFile,iMode);		
			
			UpdateBBox();

			int	iMinX	=	m_BBox.X();
			int	iMaxX	=	iMinX+m_BBox.W();
			int	iMinY	=	m_BBox.Y();
			int	iMaxY	=	iMinY+m_BBox.H();

			for(i=iMinX;i<=iMaxX;++i)
				for(j=iMinY;j<=iMaxY;++j)
					GetDesign()->GetLayer(1)->GetGrid(i,j)->Print(pFile,iMode);
		}
		break;
	case PRINT_MODE_RESULT:
		fprintf(pFile,"net(id:%d) %d\n",(int)GetKey(),GetNumWire());		
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			pWire->Print(pFile,iMode);
		}
		break;
	default:
		assert(FALSE);
		break;
	}

	fflush(pFile);
}

int CNet::GetNumVia()
{
	if(IsRouted())
	{		
		int	iNumVia	=	0;

		//if(!IsLocal())
		//{
		multimap<KEY, int>	ViaBucket;
		GetViaBucket(&ViaBucket);

		multimap<KEY, int>::iterator itrl;
		multimap<KEY, int>::iterator itru;
		multimap<KEY, int>::iterator itrv;
		for(itrv=ViaBucket.begin();itrv!=ViaBucket.end();itrv=itrl)
		{
			itrl	=	ViaBucket.lower_bound(itrv->first);
			itru	=	ViaBucket.upper_bound(itrv->first);

			int	iMaxZ	=	0;
			int	iMinZ	=	MAX_NUMBER;
			for (;itrl!=itru;++itrl)
			{
				iMaxZ	=	MAX(itrl->second,iMaxZ);
				iMinZ	=	MIN(itrl->second,iMinZ);
			}	

			iNumVia	+=	iMaxZ-iMinZ;
		}		
		//		}

		return	iNumVia;
	}

	return	0;
}

void CNet::UpdateBBox()
{
	if(GetState()&STATE_NET_BBOXCLEAN)	return;

	int	iMaxX	=	-1;
	int	iMinX	=	MAX_NUMBER;
	int	iMaxY	=	-1;
	int	iMinY	=	MAX_NUMBER;
	int	iMaxZ	=	-1;
	int	iMinZ	=	MAX_NUMBER;	
	
	for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	{
		CWire*	pWire	=	*itr;
		assert(pWire->GetParent()==this);
		
		iMinX	=	MIN(iMinX,pWire->m_pPointS->X());
		iMaxX	=	MAX(iMaxX,pWire->m_pPointE->X());
		
		iMinY	=	MIN(MIN(iMinY,pWire->m_pPointS->Y()),pWire->m_pPointE->Y());
		iMaxY	=	MAX(MAX(iMaxY,pWire->m_pPointS->Y()),pWire->m_pPointE->Y());
		
		iMinZ	=	MIN(MIN(iMinZ,pWire->m_pPointS->Z()),pWire->m_pPointE->Z());
		iMaxZ	=	MAX(MAX(iMaxZ,pWire->m_pPointS->Z()),pWire->m_pPointE->Z());
	}
	
	m_BBox.Initialize(iMinX,iMinY,iMinZ,iMaxX,iMaxY,iMaxZ);

	SetState(STATE_NET_BBOXCLEAN);
}

void CNet::Check()
{
	if(IsLocal())	return;

	vector<CWire*>::iterator	itr,end;
	for(itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	{
		CWire*	pWire	=	*itr;
		if(!pWire->IsFlat())
		{
			PrintLog();
			Display(DISPLAY_MODE_ERRO,"net id(%d) has a non-flat wire!\n",(int)GetKey());
		}

		if(!pWire->IsCompleted())
		{
			PrintLog();
			Display(DISPLAY_MODE_ERRO,"net id(%d) has a non-completed wire!\n",(int)GetKey());
		}
	}

	vector<CPoint*>	PointList;
	GetPoints(&PointList,GetPin(0),GET_MODE_STATE,STATE_WIRE_ROUTED);

	for(int i=0;i<GetNumPin();i++)
	{
		CPin*	pPin	=	GetPin(i);
		
		int		iFound	=	FALSE;
		for(int j=0;j<PointList.size();j++)
		{
			if(pPin->IsSame2D(PointList[j]))
			{
				iFound	=	TRUE;
				break;
			}
		}

		if(!iFound)
		{
			PrintLog();
			Display(DISPLAY_MODE_ERRO,"net id(%d) is not completely connected!\n",(int)GetKey());
		}
	}
}

void CNet::GetViaBucket(multimap<KEY, int>*	pViaBucket)
{
	if(IsRouted())
	{
		for(int i=0;i<GetNumPin();++i) 
		{
			pViaBucket->insert(pair<const KEY, int>(GetPin(i)->GetKey()&0xFFFFFF00, GetPin(i)->Z()));
		}
	}

	for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
	{
		CWire*	pWire	=	*itr;
		if(pWire->IsRouted())
		{
			if(pWire->IsLayerAssigned())
			{
				pViaBucket->insert(pair<const KEY, int>(pWire->m_pPointS->GetKey()&0xFFFFFF00, pWire->GetLayer()->Z()));
				pViaBucket->insert(pair<const KEY, int>(pWire->m_pPointE->GetKey()&0xFFFFFF00, pWire->GetLayer()->Z()));
			}
			else
			{
				assert(FALSE);
				assert(IsLocal());
				assert(pWire->IsPoint());
				
				pViaBucket->insert(pair<const KEY, int>(pWire->m_pPointS->GetKey()&0xFFFFFF00, 0));
				pViaBucket->insert(pair<const KEY, int>(pWire->m_pPointS->GetKey()&0xFFFFFF00, GetDesign()->T()));
			}
		}
	}
}
//
//int CNet::IsFlat()
//{
//	return	GetNumPin()==2&&(*m_Wire.begin())->IsFlat();
//}

int CNet::IsPreRouted()
{
	return	GetState()&STATE_NET_PREROUTED;
}

//int CNet::GetWidth()
//{
//	assert(m_cWidth>=1);
//	return	m_cWidth;
//}
//


int CNet::GetWireInDirection(vector<CWire*>* pWireList, int iPos, int iDirection, int iMode, int iValue)
{

	switch(iMode) {
	case GET_MODE_PROP:
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			if(pWire->GetProp()&iValue)
			{
				int	iWireDirection	=	pWire->GetDirection();

				assert(iWireDirection&(DIR_VERTICAL|DIR_HORIZONTAL));

				if(iWireDirection==iDirection)
				{
					switch(iDirection){
					case DIR_HORIZONTAL:
						if(pWire->m_pPointE->Y()==iPos)	pWireList->push_back(pWire);
						break;
					case DIR_VERTICAL:
						if(pWire->m_pPointE->X()==iPos)	pWireList->push_back(pWire);
						break;
					default:
						assert(FALSE);
						break;
					}
				}
			}
		}
		break;
	case GET_MODE_STATE:
		for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			if(pWire->GetState()&iValue)
			{
				int	iWireDirection	=	pWire->GetDirection();

				assert(iWireDirection&(DIR_VERTICAL|DIR_HORIZONTAL));

				if(iWireDirection==iDirection)
				{
					switch(iDirection){
					case DIR_HORIZONTAL:
						if(pWire->m_pPointE->Y()==iPos)	pWireList->push_back(pWire);
						break;
					case DIR_VERTICAL:
						if(pWire->m_pPointE->X()==iPos)	pWireList->push_back(pWire);
						break;
					default:
						assert(FALSE);
						break;
					}
				}
			}
		}
		break;
	}

	return	pWireList->size();
}



