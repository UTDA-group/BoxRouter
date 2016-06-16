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
// Wire.cpp: implementation of the CWire class.
//
//////////////////////////////////////////////////////////////////////

#include "Wire.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
int	CWire::m_iNumInstance	=	0;
set<CWire*>	CWire::m_InstanceList;
int CWire::Debug()
{
	set<CWire*>::iterator	itr,end;
	for (itr=m_InstanceList.begin(),end=m_InstanceList.end();itr!=end;++itr)
	{
		CWire*	pWire	=	*itr;
		pWire->Print(stdout,PRINT_MODE_TEXT);
	}
	
	return	m_iNumInstance;
}

#endif

stack<CWire*> CWire::m_InstancePool;
CWire::CWire()
{
	m_pRoutedSegmentList	=	NULL;
	m_pPointS				=	NULL;
	m_pPointE				=	NULL;
#ifdef _DEBUG
	m_iNumInstance++;
	m_InstanceList.insert(this);
	assert(m_iNumInstance==m_InstanceList.size());
#endif
}

CWire::~CWire()
{
	//thyeros- make sure that this doesn't belong to any NET [6/16/2006]
	assert(m_pParent==NULL);

	ClearSegmentList();
#ifdef _DEBUG
	m_iNumInstance--;
	m_InstanceList.erase(this);
	assert(m_iNumInstance==m_InstanceList.size());
#endif
}

void CWire::Initialize(int iX1, int iY1, int iX2, int iY2, int iZ, CWire *pWire)
{
	Initialize(iX1,iY1,iX2,iY2,iZ);
	
	SetParent(pWire->GetParent());

	// overwrite the state [7/5/2006 thyeros]
	m_iState	=	pWire->m_iState;
}

void CWire::Initialize(int iX1, int iY1, int iZ1, int iX2, int iY2, int iZ2)
{
	m_iProp	=	PROP_INVALID;

	if(iX1<iX2 || (iX1==iX2 && iY1<iY2))
	{
		m_pPointS	=	(CPoint*)GetDesign()->GetLayer(iZ1)->GetGrid(iX1,iY1);
		m_pPointE	=	(CPoint*)GetDesign()->GetLayer(iZ2)->GetGrid(iX2,iY2);

		//m_pPointS->Initialize(iX1,iY1,iZ1,this);
		//m_pPointE->Initialize(iX2,iY2,iZ2,this);
	}
	else
	{
		m_pPointS	=	(CPoint*)GetDesign()->GetLayer(iZ2)->GetGrid(iX2,iY2);
		m_pPointE	=	(CPoint*)GetDesign()->GetLayer(iZ1)->GetGrid(iX1,iY1);

		//m_pPointS->Initialize(iX2,iY2,iZ2,this);	
		//m_pPointE->Initialize(iX1,iY1,iZ1,this);
	}

	if (m_pPointS->IsHorizontal(m_pPointE))			m_iProp	|=	PROP_WIRE_HORIZONTAL;
	else if (m_pPointS->IsVertical(m_pPointE))		m_iProp	|=	PROP_WIRE_VERTICAL;
	else if (m_pPointS->IsPerpendicular(m_pPointE))	m_iProp	|=	PROP_WIRE_PERPENDICULAR;
	else if (m_pPointS->IsSame3D(m_pPointE))		m_iProp	|=	PROP_WIRE_POINT;
	else											m_iProp	|=	PROP_WIRE_BEND;

	assert(!(IsHorizontal()&&IsVertical()));

//	m_BBox.Initialize(&m_PointS,&m_PointE);

	SetState(STATE_WIRE_UNROUTED|STATE_WIRE_NOTASSGNED);

	CreateKey();
}

void CWire::Initialize(int iX1, int iY1, int iX2, int iY2, int iZ)
{
	Initialize(iX1,iY1,iZ,iX2,iY2,iZ);
}

void CWire::SetState(int iState)
{
	switch(iState) {
	case STATE_WIRE_REROUTED:
		m_iState	|=	STATE_WIRE_REROUTED;
		break;
	case STATE_WIRE_ROUTED:
		m_iState	&=	~STATE_WIRE_UNROUTED;	
		m_iState	|=	STATE_WIRE_ROUTED;
		break;
	case STATE_WIRE_UNROUTED:
		m_iState	&=	~STATE_WIRE_ROUTED;	
		m_iState	|=	STATE_WIRE_UNROUTED;
		break;
	case STATE_WIRE_ASSGNED:
		m_iState	&=	~STATE_WIRE_NOTASSGNED;	
		m_iState	|=	STATE_WIRE_ASSGNED;
		break;
	case STATE_WIRE_NOTASSGNED:
		m_iState	&=	~STATE_WIRE_ASSGNED;	
		m_iState	|=	STATE_WIRE_NOTASSGNED;
		break;
	case STATE_WIRE_UNROUTED|STATE_WIRE_NOTASSGNED:
		m_iState	&=	~STATE_WIRE_ROUTED;	
		m_iState	|=	STATE_WIRE_UNROUTED;
		m_iState	&=	~STATE_WIRE_ASSGNED;	
		m_iState	|=	STATE_WIRE_NOTASSGNED;
		break;
	case STATE_WIRE_ROUTED|STATE_WIRE_ASSGNED:
		m_iState	&=	~STATE_WIRE_UNROUTED;	
		m_iState	|=	STATE_WIRE_ROUTED;
		m_iState	&=	~STATE_WIRE_NOTASSGNED;	
		m_iState	|=	STATE_WIRE_ASSGNED;
		break;
	default:
		assert(false);
		break;
	}
}

int CWire::IsPerpendicular()
{
	return	m_iProp&PROP_WIRE_PERPENDICULAR;
}

int CWire::IsHorizontal()
{
	return	m_iProp&PROP_WIRE_HORIZONTAL;
}

int CWire::IsVertical()
{
	return	m_iProp&PROP_WIRE_VERTICAL;
}

int CWire::IsPoint()
{
	return	m_iProp&PROP_WIRE_POINT;
}

int CWire::IsFlat()
{
	return	m_iProp&PROP_WIRE_FLAT;
}


int CWire::GetLength()
{
	if(IsPoint())	return	0;
	
	return	m_pPointS->GetMDistance(m_pPointE);
}

int CWire::GetLength2D()
{
	if(IsPoint())	return	0;
	
	return	m_pPointS->GetMDistance2D(m_pPointE);
}

void CWire::AssignLayer(int iLayer)
{
	if(IsRouted())
	{
		assert((IsLayerAssigned()&&GetLayer())||(!IsLayerAssigned()&&!GetLayer()));
		
		if(IsLayerAssigned()&&GetLayer()->Z()==iLayer)	return;
		
		CNet*	pNet	=	GetParent();
		assert(pNet);
		
		pNet->DelWire(this);
		
		m_pPointS	=	GetDesign()->GetLayer(iLayer)->GetGrid(m_pPointS->X(),m_pPointS->Y());
		m_pPointE	=	GetDesign()->GetLayer(iLayer)->GetGrid(m_pPointE->X(),m_pPointE->Y());
		//m_pPointS->SetZ(iLayer);
		//m_pPointE->SetZ(iLayer);
		CreateKey();	
		
		iLayer?	SetState(STATE_WIRE_ASSGNED):SetState(STATE_WIRE_NOTASSGNED);
		
		pNet->AddWire(this);
	}
	else
	{
		m_pPointS	=	GetDesign()->GetLayer(iLayer)->GetGrid(m_pPointS->X(),m_pPointS->Y());
		m_pPointE	=	GetDesign()->GetLayer(iLayer)->GetGrid(m_pPointE->X(),m_pPointE->Y());
		//m_pPointS->SetZ(iLayer);
		//m_pPointE->SetZ(iLayer);
		
		CreateKey();	
	}
}

void CWire::CreateKey()
{
	// for wire, we don't need Z as a part of KEY [6/30/2006 thyeros]
	// the same wire, but with different Z is not allowed [6/30/2006 thyeros]
	KEY S	=	m_pPointS->GetKey()&0xFFFFFF00;
	KEY	E	=	m_pPointE->GetKey()&0xFFFFFF00;

	m_Key	=	(S<<32 | E);

}

void CWire::SetParent(CNet *pParent)
{
	m_pParent	=	(CObject*)pParent;
}

int CWire::IsLayerAssigned()
{
	return	GetState()&STATE_WIRE_ASSGNED;
}

vector<CWire*> CWire::AdjustByBoundary()
{
	assert(IsRouted());
	assert(IsLayerAssigned());

	vector<CWire*>		AdjWire;
	AdjWire.reserve(GetLength());

	CLayer*	pLayer		=	GetLayer();
	assert(pLayer);
	CNet*	pNet		=	GetParent();
	assert(pNet);

	int	iNumBoundary	=	0;
	if(IsHorizontal())
	{
		iNumBoundary			=	m_pPointE->X()-m_pPointS->X();
		assert(iNumBoundary>0&&(iNumBoundary<=GetDesign()->W()||iNumBoundary<=GetDesign()->H()));

		int*	pBoundary		=	new	int[iNumBoundary+1];
		assert(pBoundary);

		memset(pBoundary,0x00,iNumBoundary*sizeof(int));
		pBoundary[iNumBoundary]	=	1;	// mark as the last element [7/5/2006 thyeros]

		int	iSum	=	0;
		int	i,j;
		for (i=0;i<iNumBoundary;++i)
		{
			for (j=1;j<=GetDesign()->T();++j)
			{
				CLayer*	pCurLayer	=	GetDesign()->GetLayer(j);

				if(pCurLayer->GetDirection()==pLayer->GetDirection())
				{
					pBoundary[i]	|=	pCurLayer->GetBoundary(i+m_pPointS->X(),m_pPointS->Y())->IsFound(pNet);	
				}
			}

			iSum	+=	pBoundary[i];
		}

		if(iSum==0)
		{	
			// whole wire is needed [7/5/2006 thyeros]
			AdjWire.push_back(this);
		}
		else if(iSum==iNumBoundary)
		{
			// already wires are there, discard this wire [7/5/2006 thyeros]
			assert(AdjWire.empty());

			m_pParent	=	NULL;
			Delete();//delete	this;
		}
		else
		{
			// partially needed [7/5/2006 thyeros]
			int	iSX		=	m_pPointS->X();
			int	iEX		=	m_pPointE->X();
			int	iY		=	m_pPointS->Y();
			int	iZ		=	m_pPointS->Z();

			int	iLast	=	-1;
			for (i=0;i<iNumBoundary+1;++i)
			{
				if(pBoundary[i])
				{
					if(iLast>=0)
					{
						// create wire between iLast and i [7/5/2006 thyeros]
						int	iNSX	=	iSX+iLast;
						int	iNEX	=	iSX+i;
						
						CWire*	pWire	=	new CWire;
						pWire->Initialize(iNSX,iY,iNEX,iY,iZ,this);
						
						AdjWire.push_back(pWire);
						
						iLast	=	-1;
					}
				}
				else
				{
					if(iLast<0)		iLast	=	i;
				}
			}

			m_pParent	=	NULL;
			Delete();//delete	this;
		}

		SAFE_DELA(pBoundary);
	}
	else if(IsVertical())
	{
		iNumBoundary	=	m_pPointE->Y()-m_pPointS->Y();
		assert(iNumBoundary>0);

		int*	pBoundary		=	new	int[iNumBoundary+1];
		memset(pBoundary,0x00,iNumBoundary*sizeof(int));
		pBoundary[iNumBoundary]	=	1;	// mark as the last element [7/5/2006 thyeros]
		assert(pBoundary);

		int	iSum	=	0;
		int	i,j;
		for (i=0;i<iNumBoundary;++i)
		{
			for (j=1;j<=GetDesign()->T();++j)
			{
				CLayer*	pCurLayer	=	GetDesign()->GetLayer(j);

				if(pCurLayer->GetDirection()==pLayer->GetDirection())
				{
					pBoundary[i]	|=	pCurLayer->GetBoundary(m_pPointS->X(),i+m_pPointS->Y())->IsFound(pNet);	
				}
			}
			iSum	+=	pBoundary[i];
		}

		if(iSum==0)
		{	
			// whole wire is needed [7/5/2006 thyeros]
			AdjWire.push_back(this);
		}
		else if(iSum==iNumBoundary)
		{
			// already wires are there, discard this wire [7/5/2006 thyeros]
			assert(AdjWire.empty());

			m_pParent	=	NULL;
			Delete();//delete	this;
		}
		else
		{
			// partially needed [7/5/2006 thyeros]
			int	iSY		=	m_pPointS->Y();
			int	iEY		=	m_pPointE->Y();
			int	iX		=	m_pPointS->X();
			int	iZ		=	m_pPointS->Z();

			int	iLast	=	-1;
			for (i=0;i<iNumBoundary+1;++i)
			{
				if(pBoundary[i])
				{
					if(iLast>=0)
					{
						// create wire between iLast and i [7/5/2006 thyeros]
						int	iNSY	=	iSY+iLast;
						int	iNEY	=	iSY+i;
						
						assert(iNSY<iNEY);
						
						CWire*	pWire	=	new CWire;
						pWire->Initialize(iX,iNSY,iX,iNEY,iZ,this);
						
						AdjWire.push_back(pWire);
						
						iLast	=	-1;	
					}				
				}
				else
				{
					if(iLast<0)		iLast	=	i;
				}
			}

			m_pParent	=	NULL;
			Delete();//delete	this;
		}

		SAFE_DELA(pBoundary);
	}
	else
	{
		assert(IsPoint());
		AdjWire.push_back(this);
	}

	return	AdjWire;
}

void CWire::UpdateBoundary(int iMode)
{
	// need to have a valid layer [6/18/2006 thyeros]
	assert(GetLayer());
	assert(m_pPointS->X()<=m_pPointE->X());
	assert(IsFlat()||IsPoint());

	if(IsPoint())	return;

	//////////////////////////////////////////////////////////////////////////
	// congestion and density are updated here [6/28/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	CLayer*		pLayer		=	GetLayer();
	CDesign*	pDesign		=	GetDesign();


	if(IsHorizontal())
	{
		assert(m_pPointS->Y()==m_pPointE->Y());
		int	iY		=	m_pPointS->Y();
		int iMinX	=	m_pPointS->X();
		int	iMaxX	=	m_pPointE->X();

		// the last point doesn't require to update boundary [6/18/2006 thyeros]
		assert(iMinX<iMaxX);

		switch(iMode){
		case UPBOUND_MODE_ADD:
			pLayer->TrackWireLength(GetLength());
			for (int i=iMinX;i<iMaxX;++i)
			{
				assert(!pLayer->GetBoundary(i,iY)->IsFound(this));
				pLayer->GetBoundary(i,iY)->AddWire(this);
			}
			break;
		case UPBOUND_MODE_DEL:
			pLayer->TrackWireLength(-GetLength());
			for (int i=iMinX;i<iMaxX;++i)
				pLayer->GetBoundary(i,iY)->DelWire(this);
			break;
		default:
			assert(false);
		    break;
		}
	}
	else if(IsVertical())
	{
		assert(m_pPointS->X()==m_pPointE->X());
		int	iX		=	m_pPointS->X();
		int iMinY	=	m_pPointS->Y();
		int	iMaxY	=	m_pPointE->Y();

		assert(iMinY<iMaxY);

		switch(iMode){
		case UPBOUND_MODE_ADD:
			pLayer->TrackWireLength(GetLength());
			for (int i=iMinY;i<iMaxY;++i)
			{
				assert(!pLayer->GetBoundary(iX,i)->IsFound(this));
				pLayer->GetBoundary(iX,i)->AddWire(this);
			}	
			break;
		case UPBOUND_MODE_DEL:
			pLayer->TrackWireLength(-GetLength());
			for (int i=iMinY;i<iMaxY;++i)
				pLayer->GetBoundary(iX,i)->DelWire(this);
			break;
		default:
			assert(FALSE);
			break;
		}
	}
}

void CWire::AddSegmentList(vector<CSegment*> Segment)
{
	m_SegmentList.push_back(Segment);
}

double CWire::GetSegmentListCost(vector<CSegment*>* pSegmentList)
{
	assert(pSegmentList);

	// via cost [2/10/2007 thyeros]
	int	iMaxZ	=	LAYER_METAL1;
	int	iMinZ	=	GetDesign()->T();

	for(int i=0,s=pSegmentList->size();i<s;++i)
	{
		iMaxZ	=	MAX(iMaxZ,pSegmentList->at(i)->m_pPointE->Z());
		iMinZ	=	MIN(iMinZ,pSegmentList->at(i)->m_pPointE->Z());
	}

	return	iMaxZ-iMinZ+1;
}

vector<CSegment*>* CWire::GetSegmentList(int iIndex)
{
	assert(iIndex>=0&&iIndex<GetNumSegmentList());
	return	&m_SegmentList[iIndex];
}

int CWire::GetNumSegmentList()
{
	return	m_SegmentList.size();
}

CWire*	CWire::New()
{
	CWire*	pWire	=	NULL;

	if(m_InstancePool.empty())	pWire	=	new	CWire;
	else		
	{
		pWire	=	m_InstancePool.top();m_InstancePool.pop();
	}
	
	assert(pWire);
	return	pWire;
}

void CWire::Delete()
{
	assert(m_pParent==NULL);
	m_iState	=	STATE_INVALID;
	m_iProp		=	PROP_INVALID;
	m_Key		=	0;

	m_pPointE	=	NULL;
	m_pPointS	=	NULL;

	ClearSegmentList();
	m_InstancePool.push(this);
}

void CWire::ClearSegmentList()
{
	int	iNumSegmentList	=	GetNumSegmentList();

	for (int i=0;i<iNumSegmentList;++i)
	{
		vector<CSegment*>*	pSegmentList	=	GetSegmentList(i);
		for (int j=0,s=pSegmentList->size();j<s;++j)
		{
			(*pSegmentList)[j]->Delete();//SAFE_DEL((*pSegmentList)[j]);
		}

		pSegmentList->clear();
	}

	m_SegmentList.clear();
	m_pRoutedSegmentList	=	NULL;
}

void CWire::Print(FILE *pFile, int Mode)
{
	if(pFile==NULL)	return;

	switch(Mode){
	case PRINT_MODE_DUMP:
		{
			KEY	Key;

			if(IsCompleted())
			{
				Key		=	m_pPointS->GetKey();
				if(!fwrite(&Key,1,sizeof(KEY),pFile))	Display(DISPLAY_MODE_ERRO,"print a wire(S) in net(id:%d)\n",(int)GetParent()->GetKey());

				Key		=	m_pPointE->GetKey();
				if(!fwrite(&Key,1,sizeof(KEY),pFile))	Display(DISPLAY_MODE_ERRO,"print a wire(E) in net(id:%d)\n",(int)GetParent()->GetKey());
			}
			else
			{
				Key		=	m_pPointS->GetKey()&0xFFFFFF00;
				if(!fwrite(&Key,1,sizeof(KEY),pFile))	Display(DISPLAY_MODE_ERRO,"print a wire(S) in net(id:%d)\n",(int)GetParent()->GetKey());

				Key		=	m_pPointE->GetKey()&0xFFFFFF00;
				if(!fwrite(&Key,1,sizeof(KEY),pFile))	Display(DISPLAY_MODE_ERRO,"print a wire(E) in net(id:%d)\n",(int)GetParent()->GetKey());
			}
		}
		break;
	case PRINT_MODE_GNUPLOT:
		fprintf(pFile,"plot '-' w linespoints lt %d lw 1 pt %d ps 1\n %d %d\n %d %d\ne\n",
			IsRouted()? 3:0,
			IsHorizontal()? 3:4,
			m_pPointS->X(),m_pPointS->Y(),
			m_pPointE->X(),m_pPointE->Y());
		break;
	case PRINT_MODE_GNUPLOT3D:
		fprintf(pFile,"splot '-' w linespoints lt %d lw 1 pt %d ps 1\n %d %d %d\n %d %d %d\ne\n",
			IsRouted()? 3:0,
			IsHorizontal()? 3:4,
			m_pPointS->X(),m_pPointS->Y(),m_pPointS->Z(),
			m_pPointE->X(),m_pPointE->Y(),m_pPointE->Z());
		break;
	case PRINT_MODE_MATLAB:
		fprintf(pFile,"plot([%d,%d],[%d,%d],'%c');\n",
			m_pPointS->X(),m_pPointS->X(),m_pPointS->Y(),m_pPointS->Y(), 
			IsHorizontal()? 's':'x');
		fprintf(pFile,"plot([%d,%d],[%d,%d],'%c');\n",
			m_pPointE->X(),m_pPointE->X(),m_pPointE->Y(),m_pPointE->Y(), 
			IsHorizontal()? 's':'x');
		fprintf(pFile,"plot([%d,%d],[%d,%d],'%s');\n",
			m_pPointS->X(),m_pPointE->X(),
			m_pPointS->Y(),m_pPointE->Y(),
			IsRouted()? "-":"-.");			
		break;
	case PRINT_MODE_MATLAB3D:
		fprintf(pFile,"plot3([%d,%d],[%d,%d],[%d,%d],'%c');\n",
			m_pPointS->X(),m_pPointS->X(),
			m_pPointS->Y(),m_pPointS->Y(),
			m_pPointS->Z(),m_pPointS->Z(), 
			IsHorizontal()? 's':'x');
		fprintf(pFile,"plot3([%d,%d],[%d,%d],[%d,%d],'%c');\n",
			m_pPointE->X(),m_pPointE->X(),
			m_pPointE->Y(),m_pPointE->Y(),
			m_pPointE->Z(),m_pPointE->Z(), 
			IsHorizontal()? 's':'x');
		fprintf(pFile,"plot3([%d,%d],[%d,%d],[%d,%d],'%s');\n",
			m_pPointS->X(),m_pPointE->X(),
			m_pPointS->Y(),m_pPointE->Y(),
			m_pPointS->Z(),m_pPointE->Z(),
			IsRouted()? "-":"-.");			
		break;
	case PRINT_MODE_SEGMENT:
		{
			fprintf(pFile,"-------------net(id:%d) - (%d,%d)-(%d,%d)\n",
				(int)GetParent()->GetKey(),
				m_pPointS->X(),m_pPointS->Y(),
				m_pPointE->X(),m_pPointE->Y());fflush(pFile);

			for (int i=0;i<GetNumSegmentList();++i)
			{
				vector<CSegment*>*	pSegmentList	=	GetSegmentList(i);
				fprintf(pFile,"*segment[%d]\n",i);
				for (int j=0;j<pSegmentList->size();++j)
					(*pSegmentList)[j]->Print(pFile,PRINT_MODE_TEXT);
			}
		}
		break;
	case PRINT_MODE_TEXT:	
#ifdef _DEBUG
		fprintf(pFile,"-w[0x%x] (%d,%d,%d)-(%d,%d,%d) %c",(ADDRESS)this,
#else
		fprintf(pFile,"-w (%d,%d,%d)-(%d,%d,%d) %c",
#endif
			m_pPointS->X(),m_pPointS->Y(),m_pPointS->Z(),
			m_pPointE->X(),m_pPointE->Y(),m_pPointE->Z(),
			IsRouted()? 'o':'x');
		if(GetParent())	fprintf(pFile,":net(id:%d) ",(int)GetParent()->GetKey());
		fprintf(pFile,"\n");
		break;
	case PRINT_MODE_RESULT:
		if(GetDesign()->m_Param.GetProp()&PROP_PARAM_MULTILAYER)
		{
			fprintf(pFile,"(%d,%d,%d)-(%d,%d,%d)\n",
				m_pPointS->X(),m_pPointS->Y(),m_pPointS->Z(),
				m_pPointE->X(),m_pPointE->Y(),m_pPointE->Z());
		}
		else
		{
			fprintf(pFile,"(%d,%d,0)-(%d,%d,0)\n",
				m_pPointS->X(),m_pPointS->Y(),
				m_pPointE->X(),m_pPointE->Y());
		}
		break;
	default:
		assert(FALSE);
		break;
	}

	fflush(pFile);
}

void CWire::Enumerate(int iMode)
{
	CDesign*	pDesign		=	GetDesign();
	int			iNumLayer	=	pDesign->T();
	int			iDirection	=	GetDirection();
	int			i,j,k;

	if(IsPoint())
	{
		
	}
	else if(IsFlat())
	{
		for (i=1;i<=iNumLayer;++i)
		{
			if(pDesign->GetLayer(i)->GetDirection()==iDirection)
			{	
				AddSegmentList(Bend(i));
			}
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// L-shape routing [6/19/2006 thyeros]
		//////////////////////////////////////////////////////////////////////////
		if(iMode&ENUM_MODE_L)
		{
			int	i1stElbowDir	=	DIR_DIAGONAL;
			int	i2ndElbowDir	=	DIR_DIAGONAL;
			
			CPoint	MidPoint;
			
			MidPoint.Initialize(m_pPointS->X(),m_pPointE->Y(),m_pPointS->Z(),NULL);			
			i1stElbowDir	=	m_pPointS->GetDirection(&MidPoint);
			MidPoint.SetZ(m_pPointE->Z());
			i2ndElbowDir	=	MidPoint.GetDirection(m_pPointE);
			
			for (i=LAYER_METAL1;i<=iNumLayer;++i)
			{
				if(pDesign->GetLayer(i)->GetDirection()==i1stElbowDir)
				{				
					for (j=LAYER_METAL1;j<=iNumLayer;++j)
					{
						if(pDesign->GetLayer(j)->GetDirection()==i2ndElbowDir)
						{
							AddSegmentList(Bend(&MidPoint,i,j));	
						}
					}
				}
			}
			
			MidPoint.Initialize(m_pPointE->X(),m_pPointS->Y(),m_pPointS->Z(),NULL);
			i1stElbowDir	=	m_pPointS->GetDirection(&MidPoint);
			MidPoint.SetZ(m_pPointE->Z());
			i2ndElbowDir	=	MidPoint.GetDirection(m_pPointE);
			
			for (i=LAYER_METAL1;i<=iNumLayer;++i)
			{
				if(pDesign->GetLayer(i)->GetDirection()==i1stElbowDir)
				{				
					for (j=LAYER_METAL1;j<=iNumLayer;++j)
					{
						if(pDesign->GetLayer(j)->GetDirection()==i2ndElbowDir)
						{
							AddSegmentList(Bend(&MidPoint,i,j));	
						}
					}
				}
			}

			//// minimum via enumeration [2/9/2007 thyeros]
			//for (i=LAYER_METAL1;i<=iNumLayer;++i)
			//{
			//	if(pDesign->GetLayer(i)->GetDirection()==i1stElbowDir)
			//	{				
			//		int	iLayerBelow	=	i-1;
			//		int	iLayerAbove	=	i+1;

			//		if(iLayerBelow>=LAYER_METAL1&&pDesign->GetLayer(iLayerBelow)->GetDirection()==i2ndElbowDir)	AddSegmentList(Bend(&MidPoint,i,iLayerBelow));	
			//		if(iLayerAbove<=iNumLayer&&pDesign->GetLayer(iLayerAbove)->GetDirection()==i2ndElbowDir)	AddSegmentList(Bend(&MidPoint,i,iLayerAbove));	
			//	}
			//}
			//
			//MidPoint.Initialize(m_pPointE->X(),m_pPointS->Y(),m_pPointS->Z(),NULL);
			//i1stElbowDir	=	m_pPointS->GetDirection(&MidPoint);
			//MidPoint.SetZ(m_pPointE->Z());
			//i2ndElbowDir	=	MidPoint.GetDirection(m_pPointE);
			//
			//for (i=LAYER_METAL1;i<=iNumLayer;++i)
			//{
			//	if(pDesign->GetLayer(i)->GetDirection()==i1stElbowDir)
			//	{				
			//		int	iLayerBelow	=	i-1;
			//		int	iLayerAbove	=	i+1;

			//		if(iLayerBelow>=LAYER_METAL1&&pDesign->GetLayer(iLayerBelow)->GetDirection()==i2ndElbowDir)	AddSegmentList(Bend(&MidPoint,i,iLayerBelow));	
			//		if(iLayerAbove<=iNumLayer&&pDesign->GetLayer(iLayerAbove)->GetDirection()==i2ndElbowDir)	AddSegmentList(Bend(&MidPoint,i,iLayerAbove));	
			//	}
			//}
		}
		
		//////////////////////////////////////////////////////////////////////////
		// Z-shape routing [6/19/2006 thyeros]
		//////////////////////////////////////////////////////////////////////////
		if(iMode&ENUM_MODE_Z)
		{
			int	i1stElbowDir	=	DIR_DIAGONAL;
			int	i2ndElbowDir	=	DIR_DIAGONAL;
			int	i3rdElbowDir	=	DIR_DIAGONAL;
			
			CPoint	MidPoint1,MidPoint2;
			
			// slide X [9/6/2006 thyeros]
			int	iMinX	=	m_pPointS->X();
			int	iMaxX	=	m_pPointE->X();

			for (int iX=iMinX+1;iX<iMaxX;iX++)
			{
				MidPoint1.Initialize(iX,m_pPointS->Y(),m_pPointS->Z(),NULL);
				MidPoint2.Initialize(iX,m_pPointE->Y(),m_pPointS->Z(),NULL);
				
				i1stElbowDir	=	m_pPointS->GetDirection(&MidPoint1);
				i2ndElbowDir	=	MidPoint1.GetDirection(&MidPoint2);
				MidPoint2.SetZ(m_pPointE->Z());
				i3rdElbowDir	=	MidPoint2.GetDirection(m_pPointE);

				for (i=LAYER_METAL1;i<=iNumLayer;++i)
				{
					if(pDesign->GetLayer(i)->GetDirection()==i1stElbowDir)
					{				
						for (j=LAYER_METAL1;j<=iNumLayer;++j)
						{
							if(pDesign->GetLayer(j)->GetDirection()==i2ndElbowDir)
							{
								for (k=LAYER_METAL1;k<=iNumLayer;k++)
								{
									if(pDesign->GetLayer(k)->GetDirection()==i3rdElbowDir)
									{
										AddSegmentList(Bend(&MidPoint1,&MidPoint2,i,j,k));	
									}
								}
							}
						}
					}
				}
			}

			// slide Y [9/6/2006 thyeros]
			int	iMinY	=	MIN(m_pPointS->Y(),m_pPointE->Y());
			int	iMaxY	=	MAX(m_pPointS->Y(),m_pPointE->Y());

			for (int iY=iMinY+1;iY<iMaxY;iY++)
			{
				MidPoint1.Initialize(m_pPointS->X(),iY,m_pPointS->Z(),NULL);
				MidPoint2.Initialize(m_pPointE->X(),iY,m_pPointS->Z(),NULL);
				
				i1stElbowDir	=	m_pPointS->GetDirection(&MidPoint1);
				i2ndElbowDir	=	MidPoint1.GetDirection(&MidPoint2);
				MidPoint2.SetZ(m_pPointE->Z());
				i3rdElbowDir	=	MidPoint2.GetDirection(m_pPointE);

				for (i=LAYER_METAL1;i<=iNumLayer;++i)
				{
					if(pDesign->GetLayer(i)->GetDirection()==i1stElbowDir)
					{				
						for (j=LAYER_METAL1;j<=iNumLayer;++j)
						{
							if(pDesign->GetLayer(j)->GetDirection()==i2ndElbowDir)
							{
								for (k=LAYER_METAL1;k<=iNumLayer;k++)
								{
									if(pDesign->GetLayer(k)->GetDirection()==i3rdElbowDir)
									{
										AddSegmentList(Bend(&MidPoint1,&MidPoint2,i,j,k));	
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

vector<CSegment*> CWire::Bend(int iZ)
{
	vector<CSegment*>	Candidate;
	
	CSegment*	pSegment	=	CSegment::New();//new	CSegment;
	
	pSegment->Initialize(m_pPointS->X(),m_pPointS->Y(),m_pPointE->X(),m_pPointE->Y(),iZ,this);

	Candidate.push_back(pSegment);

	return	Candidate;
}

vector<CSegment*> CWire::Bend(CPoint* pPoint, int iZ1, int iZ2)
{
	return	Bend(pPoint->X(),pPoint->Y(),iZ1,iZ2);
}

vector<CSegment*> CWire::Bend(CPoint* pPoint1, CPoint* pPoint2, int iZ1, int iZ2, int iZ3)
{
	return	Bend(pPoint1->X(),pPoint1->Y(),pPoint2->X(),pPoint2->Y(),iZ1,iZ2,iZ3);
}

vector<CSegment*> CWire::Bend(int iX, int iY, int iZ1, int iZ2)
{ 
	vector<CSegment*>	Candidate;

	CNet*	pNet		=	GetParent();
	assert(pNet);

	CSegment*	pSegment	=	CSegment::New();//new	CSegment;
	CSegment*	pNSegment	=	CSegment::New();//new	CSegment;

	assert(pSegment);
	assert(pNSegment);

	pSegment->Initialize(m_pPointS->X(),m_pPointS->Y(),iX,iY,iZ1,this);
	pNSegment->Initialize(iX,iY,m_pPointE->X(),m_pPointE->Y(),iZ2,this);

	Candidate.push_back(pSegment);
	Candidate.push_back(pNSegment);

	return	Candidate;
}

vector<CSegment*> CWire::Bend(int iX1, int iY1, int iX2, int iY2, int iZ1, int iZ2, int iZ3)
{
	vector<CSegment*>	Candidate;

	CSegment*	pSegment	=	CSegment::New();//new	CSegment;
	CSegment*	pNSegment	=	CSegment::New();//new	CSegment;
	CSegment*	pNNSegment	=	CSegment::New();//new	CSegment;
	
	assert(pSegment);
	assert(pNSegment);
	assert(pNNSegment);
	
	pSegment->Initialize(m_pPointS->X(),m_pPointS->Y(),iX1,iY1,iZ1,this);
	pNSegment->Initialize(iX1,iY1,iX2,iY2,iZ2,this);
	pNNSegment->Initialize(iX2,iY2,m_pPointE->X(),m_pPointE->Y(),iZ3,this);
	
	Candidate.push_back(pSegment);
	Candidate.push_back(pNSegment);
	Candidate.push_back(pNNSegment);
	
	return	Candidate;
}

int CWire::GetDirection()
{
	if (IsHorizontal())		return	DIR_HORIZONTAL;
	if (IsVertical())		return	DIR_VERTICAL;
	if (IsPerpendicular())	return	DIR_PERPENDICULAR;

	return	DIR_DIAGONAL;
}

int CWire::IsOn2D(CPoint *pPoint)
{
	// the other point is returned to expand the search [6/25/2006 thyeros]
	if(pPoint->IsSame2D(m_pPointS))			return	RET_WIRE_S;
	else if(pPoint->IsSame2D(m_pPointE))	return	RET_WIRE_E;
	else if(IsFlat())
	{
		if(pPoint->X()>=m_pPointS->X()
			&&pPoint->X()<=m_pPointE->X()
			&&pPoint->Y()>=m_pPointS->Y()
			&&pPoint->Y()<=m_pPointE->Y())	return	RET_WIRE_SE;
	}

	return	RET_WIRE_NONE;
}

int CWire::IsOn3D(CPoint *pPoint)
{
	// the other point is returned to expand the search [6/25/2006 thyeros]
	if(pPoint->IsSame3D(m_pPointS))			return	RET_WIRE_S;
	else if(pPoint->IsSame3D(m_pPointE))	return	RET_WIRE_E;
	else if(IsFlat())
	{
		if(pPoint->X()>=m_pPointS->X()
			&&pPoint->X()<=m_pPointE->X()
			&&pPoint->Y()>=m_pPointS->Y()
			&&pPoint->Y()<=m_pPointE->Y()
			&&pPoint->Z()>=m_pPointS->Z()
			&&pPoint->Z()<=m_pPointE->Z())	return	RET_WIRE_SE;
	}

	return	RET_WIRE_NONE;
}

void CWire::GetPoints(vector<CPoint*>* pPoints)
{
	assert(IsFlat());
	assert(m_pPointS->Z()==m_pPointE->Z());

	if(IsHorizontal())
	{
		assert(m_pPointS->Y()==m_pPointE->Y());
		int	iY		=	m_pPointS->Y();
		int iMinX	=	m_pPointS->X();
		int	iMaxX	=	m_pPointE->X();
		int	iZ		=	m_pPointE->Z();

		CLayer*	pLayer	=	GetDesign()->GetLayer(iZ);

		// the last point doesn't require to update boundary [6/18/2006 thyeros]
		assert(iMinX<iMaxX);
		
		for (int i=iMinX;i<=iMaxX;++i)	pPoints->push_back(pLayer->GetGrid(i,iY));
	}
	else
	{
		assert(m_pPointS->X()==m_pPointE->X());
		int	iX		=	m_pPointS->X();
		int iMinY	=	m_pPointS->Y();
		int	iMaxY	=	m_pPointE->Y();
		int	iZ		=	m_pPointE->Z();
		
		CLayer*	pLayer	=	GetDesign()->GetLayer(iZ);

		assert(iMinY<iMaxY);
		
		for (int i=iMinY;i<=iMaxY;++i)	pPoints->push_back(pLayer->GetGrid(iX,i));
	}
}

CWire* CWire::MergeWire(CPoint *pPoint, CWire* pWire)
{
	assert(pWire->GetParent()==GetParent());
	assert(pWire->GetParent());

	CNet*	pNet		=	GetParent();
	CWire*	pNewWire	=	NULL;

	switch(IsOn2D(pPoint)){
	case RET_WIRE_E:
		switch(pWire->IsOn2D(pPoint)){
		case RET_WIRE_E:
			pNewWire	=	CWire::New();//new	CWire;
			pNewWire->Initialize(m_pPointS->X(),m_pPointS->Y(),m_pPointS->Z(),pWire->m_pPointS->X(),pWire->m_pPointS->Y(),pWire->m_pPointS->Z());
			break;
		case RET_WIRE_S:
			pNewWire	=	CWire::New();//new	CWire;
			pNewWire->Initialize(m_pPointS->X(),m_pPointS->Y(),m_pPointS->Z(),pWire->m_pPointE->X(),pWire->m_pPointE->Y(),pWire->m_pPointE->Z());
			break;
		default:
			return	FALSE;
		}
		break;
	case RET_WIRE_S:
		switch(pWire->IsOn2D(pPoint)){
		case RET_WIRE_E:
			pNewWire	=	CWire::New();//new	CWire;
			pNewWire->Initialize(m_pPointE->X(),m_pPointE->Y(),m_pPointE->Z(),pWire->m_pPointS->X(),pWire->m_pPointS->Y(),pWire->m_pPointS->Z());
			break;
		case RET_WIRE_S:
			pNewWire	=	CWire::New();//new	CWire;
			pNewWire->Initialize(m_pPointE->X(),m_pPointE->Y(),m_pPointE->Z(),pWire->m_pPointE->X(),pWire->m_pPointE->Y(),pWire->m_pPointE->Z());
			break;
		default:
			return	FALSE;
		}
		break;
	default:
		return	FALSE;
	}

	pNet->DelWire(this);
	pNet->DelWire(pWire);
	pNet->AddWire(pNewWire);

	// fix memory leak [10/4/2006 thyeros]	
	Delete();//delete this;
	pWire->Delete();//SAFE_DEL(pWire);

	return	pNewWire;
}

int CWire::SplitWire(CPoint *pPoint, CWire** ppWireS, CWire** ppWireE)
{	
	// make sure it is checked in into the net [6/30/2006 thyeros]
	//assert(IsRouted());

	// replace by parent checking [3/31/2007 thyeros]
	assert(GetParent());

	assert(IsLayerAssigned()? pPoint->Z()==GetLayer()->Z():TRUE);

	if(!IsOn3D(pPoint)||m_pPointS->IsSame3D(pPoint)||m_pPointE->IsSame3D(pPoint))	return	FALSE;

	CNet*	pNet		=	GetParent();
	assert(pNet);

	CWire*	pNewWireS	=	CWire::New();//new	CWire;
	assert(pNewWireS);
	pNewWireS->Initialize(m_pPointS->X(),m_pPointS->Y(),pPoint->X(),pPoint->Y(),pPoint->Z(),this);
	pNewWireS->SetParent(NULL);

	CWire*	pNewWireE	=	CWire::New();//new	CWire;
	assert(pNewWireE);
	pNewWireE->Initialize(pPoint->X(),pPoint->Y(),m_pPointE->X(),m_pPointE->Y(),m_pPointE->Z(),this);
	pNewWireE->SetParent(NULL);

	// check out the current one first [7/6/2006 thyeros]
	// DO NOT delete here [7/8/2006 thyeros]
	pNet->DelWire(this);

	// check in the new two ones [7/6/2006 thyeros]
	pNet->AddWire(pNewWireS);
	pNet->AddWire(pNewWireE);

	if(ppWireS)	*ppWireS	=	pNewWireS;
	if(ppWireE)	*ppWireE	=	pNewWireE;

	return	TRUE;
}

void CWire::Pupate()
{
	CNet*	pNet	=	GetParent();

	assert(pNet);
	assert(m_pRoutedSegmentList->size());

	// first, check out the wire from the net [6/30/2006 thyeros]
	pNet->DelWire(this);
	int	i,s;

	for(i=0,s=m_pRoutedSegmentList->size();i<s;++i)
	{	
		CSegment*	pSegment	=	(*m_pRoutedSegmentList)[i];
		
		CWire*		pNewWire	=	pSegment->Pupate();	
		pNewWire->SetState(STATE_WIRE_ROUTED|STATE_WIRE_ASSGNED);
		
		// then, check in the wire into the net [6/30/2006 thyeros]
		pNet->AddWire(pNewWire);							
	}
}

#ifdef _DEBUG
void CWire::Check()
{
	if(IsLayerAssigned())	
	{
		if(GetLayer()==NULL)
		{
			printf("layer is not assigned!\n");
			Print(stdout,PRINT_MODE_TEXT);
		}

		if(GetLayer()->Z()<=GetDesign()->T()&&GetLayer()->Z()>=1)
		{

		}
		else
		{
			printf("layer is not incorrect!\n");
			Print(stdout,PRINT_MODE_TEXT);
		}

		if(GetDirection()!=GetLayer()->GetDirection())
		{
			printf("layer is not in different direction!\n");
			Print(stdout,PRINT_MODE_TEXT);
		}
	}

	if(GetParent())
	{
		if(!GetParent()->IsFound(this))
		{
			printf("parent is incorrect!\n");
			Print(stdout,PRINT_MODE_TEXT);
		}
	}
	else
	{
		printf("parent is NULLl!\n");
		Print(stdout,PRINT_MODE_TEXT);
	}
}
#endif

int CWire::IsCrossing(CWire *pWire, CPoint *pPoint)
{
	CPoint	TestPoint;		

	if(IsHorizontal()&&pWire->IsVertical())			TestPoint.SetXYZ(pWire->m_pPointS->X(),m_pPointS->Y(),0);
	else if(IsVertical()&&pWire->IsHorizontal())	TestPoint.SetXYZ(m_pPointS->X(),pWire->m_pPointS->Y(),0);

	
	if(IsOn2D(&TestPoint)&&pWire->IsOn2D(&TestPoint))
	{
		if(pPoint)	pPoint->Initialize(TestPoint.X(),TestPoint.Y(),TestPoint.Z(),NULL);		
		return	TRUE;
	}

	return	FALSE;
}

int CWire::GetNumPinOn()
{
	CNet*	pNet	=	GetParent();
	int	iNumPin	=	0;
	if(pNet)
	{
		for (int i=0;i<pNet->GetNumPin();++i)
		{
			if(m_pPointS->IsSame2D(pNet->GetPin(i)))	iNumPin++;
			if(m_pPointE->IsSame2D(pNet->GetPin(i)))	iNumPin++;
		}

		//if(m_pPointS->IsPin())
		//{
		//	*pZ	=	MAX(m_pPointS->Z(),*pZ);
		//	return	TRUE;
		//}

		//if(m_pPointE->IsPin())
		//{
		//	*pZ	=	MAX(m_pPointE->Z(),*pZ);
		//	return	TRUE;
		//}
	}
	return	iNumPin;
}

int CWire::MakeRouted(int iLayer)
{
	CNet*	pNet	=	GetParent();
	assert(pNet);

	if(IsPoint())
	{
		// no reason to delete and add wire [8/6/2006 thyeros]
		AssignLayer(iLayer);
		SetState(STATE_WIRE_ROUTED|STATE_WIRE_ASSGNED);
	}
	else
	{
		if(!IsFlat())	return	FALSE;
		if(GetDirection()!=GetDesign()->GetLayer(iLayer)->GetDirection())	return	FALSE;

		pNet->DelWire(this);
		AssignLayer(iLayer);
		SetState(STATE_WIRE_ROUTED|STATE_WIRE_ASSGNED);
		pNet->AddWire(this);		
	}

	return	TRUE;				
}

int CWire::IsRoutable(int iLayer)
{
	CLayer*	pLayer		=	GetDesign()->GetLayer(iLayer);

	if(IsHorizontal())
	{
		if(pLayer->IsVertical())	return	FALSE;

		assert(m_pPointS->Y()==m_pPointE->Y());
		int	iY		=	m_pPointS->Y();
		int iMinX	=	m_pPointS->X();
		int	iMaxX	=	m_pPointE->X();
		
		// the last point doesn't require to update boundary [6/18/2006 thyeros]
		assert(iMinX<iMaxX);

		for (int i=iMinX;i<iMaxX;++i)	
		//	if(pLayer->GetBoundary(i,iY)->GetCongestion()>0.8) 	return	FALSE;
			
			if(pLayer->GetBoundary(i,iY)->GetCapacity(GET_MODE_ACAP)<WIRE_WIDTH_SPACE)	return	FALSE;
	}
	else if(IsVertical())
	{
		if(pLayer->IsHorizontal())	return	FALSE;

		assert(m_pPointS->X()==m_pPointE->X());
		int	iX		=	m_pPointS->X();
		int iMinY	=	m_pPointS->Y();
		int	iMaxY	=	m_pPointE->Y();
		
		assert(iMinY<iMaxY);
		
		for (int i=iMinY;i<iMaxY;++i)
		//	if(pLayer->GetBoundary(iX,i)->GetCongestion()>0.8) 	return	FALSE;

			if(pLayer->GetBoundary(iX,i)->GetCapacity(GET_MODE_ACAP)<WIRE_WIDTH_SPACE)	return	FALSE;
	}	

	return	TRUE;
}

int CWire::GetRoutableLength(int iLayer)
{
	CLayer*	pLayer		=	GetDesign()->GetLayer(iLayer);

	if(IsHorizontal())
	{
		if(pLayer->IsVertical())	return	FALSE;

		assert(m_pPointS->Y()==m_pPointE->Y());
		int	iY		=	m_pPointS->Y();
		int iMinX	=	m_pPointS->X();
		int	iMaxX	=	m_pPointE->X();

		// the last point doesn't require to update boundary [6/18/2006 thyeros]
		assert(iMinX<iMaxX);

		for (int i=iMinX;i<iMaxX;++i)	
			if(pLayer->GetBoundary(i,iY)->GetCapacity(GET_MODE_ACAP)<WIRE_WIDTH_SPACE)	return	i-iMinX;
	}
	else if(IsVertical())
	{
		if(pLayer->IsHorizontal())	return	FALSE;

		assert(m_pPointS->X()==m_pPointE->X());
		int	iX		=	m_pPointS->X();
		int iMinY	=	m_pPointS->Y();
		int	iMaxY	=	m_pPointE->Y();

		assert(iMinY<iMaxY);

		for (int i=iMinY;i<iMaxY;++i)
			//	if(pLayer->GetBoundary(iX,i)->GetCongestion()>0.8) 	return	FALSE;

			if(pLayer->GetBoundary(iX,i)->GetCapacity(GET_MODE_ACAP)<WIRE_WIDTH_SPACE)	return	i-iMinY;
	}	

	return	-1;
}

CWire*	CWire::Clone()
{
	CWire*	pWire	=	CWire::New();//new	CWire;

	pWire->Initialize(m_pPointS->X(),m_pPointS->Y(),m_pPointE->X(),m_pPointE->Y(),GetLayer()->Z());

	return	pWire;
}

CLayer* CWire::GetLayer()
{
	if(GetState()&STATE_WIRE_ASSGNED)
	{
		assert(m_pPointE->Z()==m_pPointS->Z());

		if(m_pPointE->Z()==0)	return	NULL;

		return	GetDesign()->GetLayer(m_pPointE->Z());
	}

	return	NULL;
}

void CWire::AdjustBoundary(CLayer* pLayer, int iBlocakge)
{
	// need to have a valid layer [6/18/2006 thyeros]
	assert(m_pPointS->X()<=m_pPointE->X());
	assert(IsFlat()||IsPoint());

	if(IsPoint())	return;

	//////////////////////////////////////////////////////////////////////////
	// congestion and density are updated here [6/28/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	CDesign*	pDesign		=	GetDesign();

	if(IsHorizontal())
	{
		assert(m_pPointS->Y()==m_pPointE->Y());
		int	iY		=	m_pPointS->Y();
		int iMinX	=	m_pPointS->X();
		int	iMaxX	=	m_pPointE->X();

		// the last point doesn't require to update boundary [6/18/2006 thyeros]
		assert(iMinX<iMaxX);

		for (int i=iMinX;i<iMaxX;++i)	pLayer->GetBoundary(i,iY)->AdjustCapacity(iBlocakge);
	}
	else if(IsVertical())
	{
		assert(m_pPointS->X()==m_pPointE->X());
		int	iX		=	m_pPointS->X();
		int iMinY	=	m_pPointS->Y();
		int	iMaxY	=	m_pPointE->Y();

		assert(iMinY<iMaxY);

		for (int i=iMinY;i<iMaxY;++i)	pLayer->GetBoundary(iX,i)->AdjustCapacity(iBlocakge);
	}
}

bool CWire::CompareLength(CWire* pWire)
{
	//return	((CWire*)pL)->GetLength()<((CWire*)pR)->GetLength();

	int	iLL		=	GetLength();
	int	iLR		=	pWire->GetLength();

	// break tie all the time [3/4/2007 thyeros]
	if(iLL==iLR)
	{
		if(GetKey()!=pWire->GetKey())	return	GetKey()<pWire->GetKey();
		return	GetParent()->GetKey()<pWire->GetParent()->GetKey();
	}

	return	iLL<iLR;

}

int	CWire::IsCompleted()
{
	return	IsRouted()&&IsLayerAssigned();
}


