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
// GRouter.cpp: implementation of the CGRouter class.
//
//////////////////////////////////////////////////////////////////////

#include "GRouter.h"
#include "ILPSolver.h"
#include "Segment.h"
#include "MazeHeap.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGRouter::CGRouter()
{
	
}

CGRouter::~CGRouter()
{
	
}

void CGRouter::BoxRouting(CDesign *pDesign, CBBox *pBBoxS, CBBox* pBBoxE)
{
#define MAX_ILP_CONSTRAINT	20000

	int	iStep				=	pDesign->m_Param.m_iBoxRouting_Step;
	int	iNumWireRouted		=	pDesign->GetWireInBBox(NULL,pBBoxE,GET_MODE_STATE,STATE_WIRE_ROUTED);
	int	iNumWireWithinBBox	=	0;
	int	iNumWireCurRouted	=	0;
	int	iExit				=	0;
	vector<CWire*>	WireList;
		


	for(;!iExit;pDesign->m_iLoop++)
	{
		//pDesign->PrintFile("c.m",PRINT_MODE_MATLAB);
		//for(int i=LAYER_METAL1;i<=pDesign->T();i++)
		//{
		//	CLayer*	pLayer	=	pDesign->GetLayer(i);
		//	for (int i=0;i<pLayer->m_iMaxBX;++i)			
		//		for (int j=0;j<pLayer->m_iMaxBY;++j)
		//		{
		//			CBoundary*	pBoundary	=	pLayer->GetBoundary(i,j);
		//			if(pBoundary->GetCongestion()>=0.8)
		//			{
		//				pBoundary->PrintFile(">c.m",PRINT_MODE_MATLAB);

		//			}
		//		}
		//}

//		exit(0);


		//check if the  expanded box cover the target box (typically the whole circuit)
		iExit	=	pBBoxS->IsInside(pBBoxE);
	
		WireList.clear();
		//collect the unrouted wires in the current box
		iNumWireWithinBBox	=	pDesign->GetWireInBBox(&WireList,pBBoxS,GET_MODE_STATE,STATE_WIRE_UNROUTED);

		//unless this is the last expansion, try to get routed more wires at once (solve when more than MIN_ILP_CAPACITY)
		if(iNumWireWithinBBox>iStep || (iExit&&iNumWireWithinBBox))
		{
			CObject::Display(DISPLAY_MODE_NONE,"%d) routing %d wires within (%d,%d)-(%d,%d)\n",
				pDesign->m_iLoop,
				iNumWireWithinBBox,
				pBBoxS->X(),pBBoxS->Y(),
				pBBoxS->X()+pBBoxS->W(),pBBoxS->Y()+pBBoxS->H());

			iNumWireCurRouted	=	0;

			if(pDesign->m_Param.GetProp()&PROP_PARAM_BOXR_ILP)
			{
				if(pDesign->m_Param.GetProp()&PROP_PARAM_BOXR_ILPMIN)		CObject::PrintMsg2(" i-routing.");
				else if(pDesign->m_Param.GetProp()&PROP_PARAM_BOXR_ILPMAX)	CObject::PrintMsg2(" x-routing.");
				else if(pDesign->m_Param.GetProp()&PROP_PARAM_BOXR_ILPHYD)	CObject::PrintMsg2(" h-routing.");

				//if it is hybrid ILP mode, but there will be overflow, go for MAX-ILP
				if(pDesign->m_Param.GetProp()&PROP_PARAM_BOXR_ILPHYD&&pDesign->GetState()&STATE_DESN_OVERFLOW)
				{
					CObject::Display(DISPLAY_MODE_NONE,"..design will be overflowed\n");
				}
				else
				{
					iNumWireCurRouted	+=	CGRouter::ConcurrentRouting(pDesign,&WireList,pDesign->m_Param.GetProp());
				}

				if(pDesign->m_Param.GetProp()&PROP_PARAM_BOXR_ILPHYD&&iNumWireWithinBBox!=iNumWireCurRouted)
				{
					//it is hybrid ILP mode, was predicted there wouldn't be any overflow... but that was wrong
					pDesign->SetState(STATE_DESN_OVERFLOW);
					CObject::Display(DISPLAY_MODE_NONE," x-routing.");
					//solve MAX-ILP
					iNumWireCurRouted	+=	CGRouter::ConcurrentRouting(pDesign,&WireList,PROP_PARAM_BOXR_ILPMAX);
				}

				CObject::Display(DISPLAY_MODE_NONE,"=%4d (%.1f %%) \tOF:%4d\t\t%.2f sec (%.1f MB)\n",iNumWireCurRouted,100.0*iNumWireCurRouted/iNumWireWithinBBox,pDesign->GetOverFlow(GET_MODE_SUM),StopWatch(STOPWATCH_OPTION_SET),GetMemory()/1024.0);
			}

			if(iNumWireWithinBBox!=iNumWireCurRouted)
			{	
				//pick up all the unrouted wires by maze routing
				CObject::Display(DISPLAY_MODE_NONE," z-routing...");
				iNumWireCurRouted	+=	CGRouter::SequentialRouting(pDesign,&WireList,pBBoxS,pDesign->m_Param.m_iMazeRouting_Margin);		
				CObject::Display(DISPLAY_MODE_NONE,"\t\t\t\t\t=%4d (%.1f %%)\tOF:%4d\t\t%.2f sec (%.1f MB)\n",iNumWireCurRouted,100.0*iNumWireCurRouted/iNumWireWithinBBox,pDesign->GetOverFlow(GET_MODE_SUM),StopWatch(STOPWATCH_OPTION_SET),GetMemory()/1024.0);
			}

			assert(iNumWireWithinBBox==iNumWireCurRouted);
		}

		//box expansion
		pBBoxS->Expand(pDesign->W(),pDesign->H());
	}

	pDesign->SetState(~STATE_DESN_OVERFLOW);
}

void CGRouter::PreRouting(CDesign *pDesign)
{
	vector<CWire*>	WireList;
	WireList.reserve(pDesign->GetNumWire());

	pDesign->GetWireInBBox(&WireList,pDesign,GET_MODE_PROP,PROP_WIRE_FLAT);

	sort(WireList.begin(),WireList.end(),CWireOpLen());

	// two pin net first [9/12/2006 thyeros]
	int	i,j,s;
	for (i=0,s=WireList.size();i<s;++i)
	{
		CWire*	pWire	=	WireList[i];

		if(pWire->IsRouted())	continue;

		CNet*	pNet	=	pWire->GetParent();
		if(pNet->IsFlat()&&pNet->GetNumPin()==2)
		{
			for(j=LAYER_METAL1;j<=pDesign->T();++j)
			{
				if(pWire->IsRoutable(j))	
				{
					pWire->MakeRouted(j);
					break;					
				}
			}
		}
	}

	for (i=0,s=WireList.size();i<s;++i)
	{
		CWire*	pWire	=	WireList[i];

		if(pWire->IsRouted())	continue;

		CNet*	pNet	=	pWire->GetParent();
		if(pNet->IsFlat())
		{
			for(j=LAYER_METAL1;j<=pDesign->T();++j)
			{
				if(pWire->IsRoutable(j))	
				{
					pWire->MakeRouted(j);
					break;					
				}
			}
		}
	}

	for (i=0,s=WireList.size();i<s;++i)
	{
		CWire*	pWire	=	WireList[i];

		if(pWire->IsRouted())	continue;
		
		for (j=LAYER_METAL1;j<=pDesign->T();++j)
		{
			if(pWire->IsRoutable(j))	
			{
				pWire->MakeRouted(j);
				break;					
			}
		}
	}

	for(i=0,s=pDesign->GetNumNet();i<s;++i)
	{
		CNet*	pNet	=	pDesign->GetNet(i);
		if(pNet->IsRouted())
		{
			++pDesign->m_iNumPreRoutedNet;
		}
	}

//#ifdef _DEBUG
//	int	iPrevCapacity	=	pDesign->GetCapacity(GET_MODE_ACAP);
//	int	iPrevCapacity2	=	pDesign->GetCapacity(GET_MODE_OCAP);
//	int	iPrevWL			=	pDesign->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
//#endif
//
//	pDesign->PrintResult();
//
//	vector<CNet*>	NetList;
//	NetList.reserve(pDesign->GetNumNet());
//
//	for(i=0,s=pDesign->GetNumNet();i<s;++i)
//	{
//		CNet*	pNet	=	pDesign->GetNet(i);
//		if(pNet->IsFlat()&&pNet->IsRouted())
//		{
//			pDesign->m_iPreRoutedWL	+=	pNet->GetLength(GET_MODE_STATE,STATE_NET_ANY);
//			++pDesign->m_iNumPreRoutedNet;
//
//			for(j=0;j<pNet->GetNumWire();j++)
//			{
//				CWire*	pWire	=	pNet->m_Wire[j];
//
//				CLayer*	pLayer	=	pWire->GetLayer();
//				assert(pLayer);
//
//				//// make total wirelength unchanged [2/14/2007 thyeros]
//				pLayer->TrackWireLength(pWire->GetLength());
//
//				// adjust boundary capacities as if this is a blockage [2/14/2007 thyeros]
//				pWire->AdjustBoundary(pLayer,WIRE_WIDTH_SPACE);//pNet->GetWidth()+pLayer->GetDesignRule(GET_DR_MIN_WIR_SPACING));
//			}
//			SAFE_DEL(pNet);
//		}
//		else
//		{
//			NetList.push_back(pNet);
//		}
//	}
//
//	pDesign->CreateNet(&NetList);
//
//	CDesign::Display(DISPLAY_MODE_INFO,"flat prerouted wirelength: %d\n",pDesign->m_iPreRoutedWL);
//	CDesign::Display(DISPLAY_MODE_INFO,"total available capacity: %d\n",pDesign->GetCapacity(GET_MODE_ACAP));
//
//	assert(iPrevCapacity==pDesign->GetCapacity(GET_MODE_ACAP));
//	assert(iPrevCapacity2==pDesign->GetCapacity(GET_MODE_OCAP));
//	assert(iPrevWL==pDesign->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED));
}

KEY CGRouter::m_iMazeCost[];
int	CGRouter::m_iViaCost;
void CGRouter::Routing(CDesign *pDesign)
{
	int		i,s;
	char	cTime[MAX_BUFFER_STR];

	m_iViaCost	=	pDesign->m_Param.m_iViaCost;
	
	StopWatch(STOPWATCH_OPTION_RESET);

	//////////////////////////////////////////////////////////////////////////
	// prerouting [9/11/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	if(pDesign->m_Param.GetProp()&PROP_PARAM_PRER)
	{
		CDesign::Display(DISPLAY_MODE_INFO,"prerouting started (%.1f MB)\n",GetMemory()/1024.0);
		PreRouting(pDesign);
		pDesign->Print(stdout,PRINT_MODE_TEXT);
		pDesign->Print(stdout,PRINT_MODE_CONGET);
	}
	else
	{
		GetTime(cTime);
		CDesign::Display(DISPLAY_MODE_INFO,"prerouting skipped due to %s (%.1f MB) at %s\n",pDesign->m_Param.m_cDump_File,GetMemory()/1024.0,cTime);
	}

	//////////////////////////////////////////////////////////////////////////
	// box routing [8/6/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	if(pDesign->m_Param.GetProp()&PROP_PARAM_BOXR)
	{
		GetTime(cTime);
		CDesign::Display(DISPLAY_MODE_INFO,"boxrouting started for %d nets (%.1f MB) at %s\n",pDesign->GetNumNet(),GetMemory()/1024.0,cTime);

		pDesign->m_iLoop=0;
		pDesign->SetState(STATE_DESN_ROUTING);
		
		//////////////////////////////////////////////////////////////////////////
		// pick the initial box [6/28/2006 thyeros]
		//////////////////////////////////////////////////////////////////////////
		vector<CBBox>	BBoxListS,BBoxListE;

		CBBox	StartBox	=	pDesign->GetStartBox();
		
		int i;
		for(i=0;i<MAX_MAZECOST;i++)
		{
			m_iMazeCost[i]	=	(int)(pDesign->GetCapacity(GET_MODE_MAX)*pow((i*0.01),pDesign->m_Param.m_iReRouting_Push))+1;
		}


		BBoxListS.push_back(StartBox);
		BBoxListE.push_back(*pDesign);

		assert(BBoxListS.size()==BBoxListE.size());


		for(i=0,s=BBoxListS.size();i<s;++i)
		{
			CBBox	BBoxS	=	BBoxListS[i];
			CBBox	BBoxE	=	BBoxListE[i];				
			BoxRouting(pDesign,&BBoxS,&BBoxE);
		}

		if(pDesign->m_Param.m_iReRouting_Repeat>=0)
		{
			vector<CNet*>	NetList;

			for(i=0,s=pDesign->GetNumNet();i<s;++i)
			{
				CNet*	pNet	=	pDesign->GetNet(i);
				if(!pNet->IsFlat())	NetList.push_back(pNet);
//				if(!pNet->IsShortest())	NetList.push_back(pNet);
			}
		
			pDesign->Print(stdout,PRINT_MODE_TEXT);
			sort(NetList.begin(),NetList.end(),CNetOpAreaR());		
			ReRoutingForWL(pDesign,&NetList);
		}
		else
		{
			pDesign->m_Param.m_iReRouting_Repeat = -pDesign->m_Param.m_iReRouting_Repeat;
		}

		pDesign->PrintResult();
		pDesign->WriteDump();	
	}
	else
	{
		GetTime(cTime);
		CDesign::Display(DISPLAY_MODE_INFO,"boxrouting skipped due to %s (%.1f MB) at %s\n",pDesign->m_Param.m_cDump_File,GetMemory()/1024.0,cTime);

	}

	pDesign->ReadDump(pDesign->m_Param.m_cDump_File);	

	//pDesign->PrintFile("a.map",PRINT_MODE_CGMAP);
	//exit(0);

	pDesign->SetState(STATE_DESN_ROUTED);
	pDesign->Print(stdout,PRINT_MODE_TEXT);
	pDesign->Print(stdout,PRINT_MODE_CONGET);


	if(pDesign->GetOverFlow(GET_MODE_SUM)==0)	pDesign->SetState(STATE_DESN_SUCCESS);

	//////////////////////////////////////////////////////////////////////////
	// relayering from 2D to 3D conversion [3/5/2007 thyeros]
	//////////////////////////////////////////////////////////////////////////
	if(pDesign->m_Param.GetProp()&PROP_PARAM_RELAYERING&&!(pDesign->GetState()&STATE_DESN_SUCCESS))
	{
		Layering(pDesign);

		if(pDesign->GetOverFlow(GET_MODE_SUM)==0)	pDesign->SetState(STATE_DESN_SUCCESS);
		else										pDesign->PrintResult();
	}

	int	iCurOF			=	pDesign->GetOverFlow(GET_MODE_SUM);
	int	iCurWL			=	pDesign->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
	int	iCurVia			=	pDesign->GetNumVia();
	int	iCurMOF			=	pDesign->GetOverFlow(GET_MODE_MAX);
	int	iResultPrinted	=	FALSE;

	if(pDesign->m_iBestOF>iCurOF 
		||(pDesign->m_iBestOF==iCurOF&&(iCurWL+iCurVia*pDesign->m_Param.m_iViaCost)<(pDesign->m_iBestWL+pDesign->m_iBestVia*pDesign->m_Param.m_iViaCost)))
	{
		pDesign->m_iBestOF		=	iCurOF;
		pDesign->m_iBestWL		=	iCurWL;
		pDesign->m_iBestVia		=	iCurVia;
	}

	//////////////////////////////////////////////////////////////////////////
	// rerouting [8/6/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	if(pDesign->m_Param.GetProp()&PROP_PARAM_RER)
	{
		int	iStuckCount		=	0;
		if(!(pDesign->GetState()&STATE_DESN_SUCCESS))
		{
			GetTime(cTime);
			CDesign::Display(DISPLAY_MODE_INFO,"rerouting started OF:%d WL:%d VIA:%d=%d (%.1f MB) at %s\n",iCurOF,iCurWL,iCurVia,iCurWL+iCurVia*pDesign->m_Param.m_iViaCost, GetMemory()/1024.0,cTime);

			pDesign->SetState(STATE_DESN_REROUTING);
			pDesign->m_iLoop	=	0;

			for(i=0;i<MAX_MAZECOST;i++)
			{
				m_iMazeCost[i]	=	(int)(pDesign->GetCapacity(GET_MODE_MAX)*pow((i*0.01),pDesign->m_Param.m_iReRouting_Push))+1;
			}

			for(i=0,s=pDesign->m_Param.m_iReRouting_Count;i<s;++i)
			{	
				//char cName[MAX_BUFFER_STR];
				//sprintf(cName,"cg_map%d.txt",i);
				//pDesign->PrintFile(cName,PRINT_MODE_CGMAP);

				CDesign::Display(DISPLAY_MODE_NONE,"*%3d/%3d P=%2d ",i,pDesign->m_Param.m_iReRouting_Count,(int)pDesign->m_Param.m_iReRouting_Push);

				ReRoutingForOF(pDesign,NULL);

				iCurOF				=	pDesign->GetOverFlow(GET_MODE_SUM);
				iCurWL				=	pDesign->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
				iCurVia				=	pDesign->GetNumVia();
				int	iBestUpdated	=	FALSE;

				if(pDesign->m_iBestOF>iCurOF 
					||(pDesign->m_iBestOF==iCurOF&&(iCurWL+iCurVia*pDesign->m_Param.m_iViaCost)<(pDesign->m_iBestWL+pDesign->m_iBestVia*pDesign->m_Param.m_iViaCost)))
				{
					pDesign->m_iBestOF		=	iCurOF;
					pDesign->m_iBestWL		=	iCurWL;
					pDesign->m_iBestVia		=	iCurVia;
					pDesign->m_iBestLoop	=	pDesign->m_iLoop;
					iBestUpdated			=	TRUE;
					iStuckCount				=	0;
				}
				else
				{
					++iStuckCount;
					if(iStuckCount>pDesign->m_Param.m_iReRouting_Stuck)
					{
						pDesign->m_Param.m_iReRouting_Push++;
						iStuckCount	=	0;
						pDesign->SetState(STATE_DESN_STUCK);
					}
				}

				if(iBestUpdated&&pDesign->m_Param.m_iReRouting_Push>30)
				{
					pDesign->PrintResult();
					iResultPrinted	=	TRUE;
				}

				CDesign::Display(DISPLAY_MODE_NONE," \tWL=%d,OF=%d,VIA=%d %s \t%.2f sec (%.1f MB)\n",iCurWL,iCurOF,iCurVia,iBestUpdated? "$":" ",StopWatch(STOPWATCH_OPTION_SET),GetMemory()/1024.0);

				++pDesign->m_iLoop;

				if(iCurOF==0)
				{
					pDesign->SetState(STATE_DESN_SUCCESS);
					pDesign->PrintResult();
					iResultPrinted	=	TRUE;
					break;
				}

				if(pDesign->m_Param.m_iReRouting_Push>100)
				{
					CDesign::Display(DISPLAY_MODE_INFO,"quit as push has been exhausted\n");
					break;
				}

				while(int iQuit=KBHit())
				{
					if(iQuit==0)	break;
					else if(iQuit=='q')
					{
						while(KBHit());
						CDesign::Display(DISPLAY_MODE_INFO,"q(uit) has been pressed\n");
						i	=	MAX_NUMBER;
						break;
					}
					else if(iQuit=='w')
					{
						while(KBHit());
						pDesign->PrintResult();
						CDesign::Display(DISPLAY_MODE_INFO,"w(rite) has been pressed\n");
						break;
					}
				}
			}
		}

		if(!iResultPrinted)		pDesign->PrintResult();


		// reduce wirelength further [2/8/2007 thyeros]
		if(pDesign->GetState()&STATE_DESN_SUCCESS)
		{
			pDesign->Print(stdout,PRINT_MODE_TEXT);

			memset(m_iMazeCost,0x00,sizeof(int)*MAX_MAZECOST);
			for(i=0;i<100;++i)	m_iMazeCost[i]	=	1;
			for(i=100;i<MAX_MAZECOST;++i)
			{
				m_iMazeCost[i]	=	MAX_PENALTY;
			}

			pDesign->m_Param.m_iMazeRouting_Margin	=	1;

			//collect nets not in the shortest [2/19/2007 thyeros]
			vector<CNet*>	NetList;

			if(pDesign->m_Param.m_iViaCost)
				for(i=0,s=pDesign->GetNumNet();i<s;++i)
				{
					CNet*	pNet	=	pDesign->GetNet(i);
					if(!pNet->IsOptimal2())
						NetList.push_back(pNet);
				}
			else
			for(i=0,s=pDesign->GetNumNet();i<s;++i)
			{
				CNet*	pNet	=	pDesign->GetNet(i);
					if(!pNet->IsShortest())
						NetList.push_back(pNet);
			}


			sort(NetList.begin(),NetList.end(),CNetOpArea());
	
			for(i=0,s=pDesign->m_Param.m_iReRouting_Repeat;i<s;++i)
			{		
				CDesign::Display(DISPLAY_MODE_NONE,"*%3d/%3d Bounded ...<%5d nets>",i,pDesign->m_Param.m_iReRouting_Repeat,NetList.size());

				// penalty is ignored [3/4/2007 thyeros]
				ReRoutingForWL(pDesign,&NetList);

				if(pDesign->m_Param.m_iViaCost)
					for(vector<CNet*>::iterator itr=NetList.begin(),end=NetList.end();itr!=end;)
					{
						CNet*	pNet	=	*itr;
						if(pNet->IsOptimal2())
						{
							NetList.erase(itr);
							end=	NetList.end();
						}
						else
						{
							++itr;
						}
					}
				else
					for(vector<CNet*>::iterator itr=NetList.begin(),end=NetList.end();itr!=end;)
					{
						CNet*	pNet	=	*itr;
						if(pNet->IsShortest())
						{
							NetList.erase(itr);
							end=	NetList.end();
						}
						else
						{
							++itr;
						}
					}


				iCurOF				=	pDesign->GetOverFlow(GET_MODE_SUM);
				iCurWL				=	pDesign->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
				iCurVia				=	pDesign->GetNumVia();
				int	iBestUpdated	=	FALSE;

//				if(pDesign->m_iBestOF>iCurOF ||(pDesign->m_iBestOF==iCurOF&&iCurWL<pDesign->m_iBestWL))
				if(pDesign->m_iBestOF>iCurOF 
					||(pDesign->m_iBestOF==iCurOF&&(iCurWL+iCurVia*pDesign->m_Param.m_iViaCost)<(pDesign->m_iBestWL+pDesign->m_iBestVia*pDesign->m_Param.m_iViaCost)))
				{
					pDesign->m_iBestOF		=	iCurOF;
					pDesign->m_iBestWL		=	iCurWL;
					pDesign->m_iBestVia		=	iCurVia;
					pDesign->m_iBestLoop	=	pDesign->m_iLoop;
					iBestUpdated			=	TRUE;
				}
				CDesign::Display(DISPLAY_MODE_NONE," \tOF:%d WL:%d VIA:%d=%d %s \t%.2f sec (%.1f MB)\n",iCurOF,iCurWL,iCurVia,iCurWL+iCurVia*pDesign->m_Param.m_iViaCost,iBestUpdated? "$":" ",StopWatch(STOPWATCH_OPTION_SET),GetMemory()/1024.0);

				++pDesign->m_iLoop;

				if(!iBestUpdated)	break;
				else				pDesign->PrintResult();

				while(int iQuit=KBHit())
				{
					if(iQuit==0)	break;
					else if(iQuit=='q')
					{
						while(KBHit());
						CDesign::Display(DISPLAY_MODE_INFO,"q(uit) has been pressed\n");
						i	=	MAX_NUMBER;
						break;
					}
				}
			}
		}
		else
		{
			//////////////////////////////////////////////////////////////////////////
			// recall the best solution [3/5/2007 thyeros]
			//////////////////////////////////////////////////////////////////////////
			// read the upto best [3/2/2007 thyeros]
			char	cDump[256];
			sprintf(cDump,"%s.dmp",pDesign->m_Param.m_cOutput_File);
			pDesign->ReadDump(cDump);	

			pDesign->m_iBestOF		=	pDesign->GetOverFlow(GET_MODE_SUM);;
			pDesign->m_iBestWL		=	pDesign->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);;
			pDesign->m_iBestVia		=	pDesign->GetNumVia();;
			pDesign->m_iBestMOF		=	pDesign->GetOverFlow(GET_MODE_MAX);;


//			CDesign::Display(DISPLAY_MODE_INFO,"current metric is %.1f with WL=%d,OF=%d\n",pDesign->GetMetric(),pDesign->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED),pDesign->GetOverFlow(GET_MODE_SUM));

			pDesign->Print(stdout,PRINT_MODE_TEXT);

			// need to optimize for metric [3/2/2007 thyeros]
			memset(m_iMazeCost,0x00,sizeof(int)*MAX_MAZECOST);
			for(i=0;i<100;++i)	m_iMazeCost[i]	=	1;
			for(i=100;i<MAX_MAZECOST;++i)
			{
				m_iMazeCost[i]	=	MAX_PENALTY*(i/100.0);
			}

			pDesign->m_Param.m_iMazeRouting_Margin	=	1;

			for(i=0,s=pDesign->m_Param.m_iReRouting_Repeat;i<s;++i)
			{		
				// collect nets either not in the shortest length or on the overflowed boundary [3/2/2007 thyeros]
				hash_map<ADDRESS,int>	ObjectList;
				int iObjectOverFlow	=	pDesign->GetNetInOverFlowBoundary(&ObjectList);

				vector<CNet*>	NetList;

				for(int j=0,s=pDesign->GetNumNet();j<s;++j)
				{
					CNet*	pNet	=	pDesign->GetNet(j);
					if(!pNet->IsOptimal()||ObjectList.find((ADDRESS)pNet)!=ObjectList.end())	NetList.push_back(pNet);
				}

				sort(NetList.begin(),NetList.end(),CNetOpAreaR());

				CDesign::Display(DISPLAY_MODE_NONE,"*%3d/%3d Bounded ...<%5d nets>",i,pDesign->m_Param.m_iReRouting_Repeat,NetList.size());

				// set boundary penalty [3/2/2007 thyeros]
				//pDesign->AddPenalty(-1);

				ReRoutingForWL(pDesign,&NetList);

				iCurOF				=	pDesign->GetOverFlow(GET_MODE_SUM);
				iCurWL				=	pDesign->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
				iCurVia				=	pDesign->GetNumVia();
				iCurMOF				=	pDesign->GetOverFlow(GET_MODE_MAX);
				int	iBestUpdated	=	FALSE;

				if(iCurOF<pDesign->m_iBestOF 
					|| (iCurOF==pDesign->m_iBestOF && iCurMOF<pDesign->m_iBestMOF )
					|| (iCurOF==pDesign->m_iBestOF && iCurMOF==pDesign->m_iBestMOF && iCurWL+iCurVia*pDesign->m_Param.m_iViaCost < pDesign->m_iBestWL+pDesign->m_iBestVia*pDesign->m_Param.m_iViaCost  )
					)
				{
					pDesign->m_iBestOF		=	iCurOF;
					pDesign->m_iBestWL		=	iCurWL;
					pDesign->m_iBestVia		=	iCurVia;
					pDesign->m_iBestMOF		=	iCurMOF;
					pDesign->m_iBestLoop	=	pDesign->m_iLoop;
					iBestUpdated			=	TRUE;
				}
				CDesign::Display(DISPLAY_MODE_NONE,"\tWL=%d,OF=%d,MOF=%d,VIA=%d %s \t%f sec (%.1f MB)\n",iCurWL,iCurOF,iCurMOF,iCurVia,iBestUpdated? "$":" ",StopWatch(STOPWATCH_OPTION_SET),GetMemory()/1024.0);

				++pDesign->m_iLoop;

				if(!iBestUpdated)	break;
				else				pDesign->PrintResult();

				while(int iQuit=KBHit())
				{
					if(iQuit==0)	break;
					else if(iQuit=='q')
					{
						while(KBHit());
						CDesign::Display(DISPLAY_MODE_INFO,"q(uit) has been pressed\n");
						i	=	MAX_NUMBER;
						break;
					}
				}
			}
		}
	}

	if(!iResultPrinted)		pDesign->PrintResult();

	pDesign->Print(stdout,PRINT_MODE_TEXT);
	pDesign->SetState(STATE_DESN_COMPLETED);
}


//void CGRouter::ReRoutingForWL(CDesign *pDesign, hash_map<ADDRESS,int>* pNetList)
//{
//		// no overflow, focus on wirelength [2/17/2007 thyeros]
//		vector<CNet*>	NetList;
//
//		for(hash_map<ADDRESS,int>::iterator itr=pNetList->begin();itr!=pNetList->end();++itr)
//		{
//			CNet*	pNet	=	(CNet*)itr->first;
//			pNet->UpdateBBox();
//
//			if(!pNet->IsShortest())	NetList.push_back(pNet);
//		}
//
//		sort(NetList.begin(),NetList.end(),CNetOpArea());
//
//		CDesign::Display(DISPLAY_MODE_NONE,"<%5d nets>",NetList.size());
//
//		ReRoutingForWL(pDesign,&NetList);
//}

void CGRouter::ReRoutingForOF(CDesign *pDesign, hash_map<ADDRESS,int>* pObjectList)
{
	//////////////////////////////////////////////////////////////////////////
	// reroute without rip-up [7/5/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////

	static int iTotalPenalty = 0;
	//static int iStep = pDesign->m_Param.m_iReRouting_Step;
	//static int iCount = pDesign->m_Param.m_iReRouting_Step;

	pDesign->AddPenalty(pDesign->m_iLoop);
	
	iTotalPenalty	+=	(pDesign->m_iLoop);

	//scaling
	for(int i=0;i<MAX_MAZECOST;i++)
	{
		m_iMazeCost[i]	=	(int)((pDesign->GetCapacity(GET_MODE_MAX)+iTotalPenalty )*pow((i*0.01),pDesign->m_Param.m_iReRouting_Push))+1;
	}

//	if(pDesign->m_iLoop%(iCount)==(iCount-1))
	if(pDesign->GetState()&STATE_DESN_STUCK)
	{
		pDesign->SetState(~STATE_DESN_STUCK);

		hash_map<ADDRESS,int>	ObjectList;
		int iObjectOverFlow	=	pDesign->GetNetInOverFlowBoundary(&ObjectList);
		vector<CNet*>	NetList;
		NetList.reserve(pDesign->GetNumNet());

//#define _CON_BOX_
#ifdef _CON_BOX_
		CBBox	CongestedBox	=	pDesign->GetCongestedBox();
		CongestedBox.Expand(pDesign->W(),pDesign->H(),MAX(pDesign->W(),pDesign->H())/50);

		for(int i=0,s=pDesign->GetNumNet();i<s;i++)
		{
			CNet*	pNet	=	pDesign->GetNet(i);
			pNet->UpdateBBox();
			if(CongestedBox.IsOverlapped(&pNet->m_BBox))
			{
				if(ObjectList.find((ADDRESS)pNet)==ObjectList.end()&&pNet->IsFlat())	continue;
				NetList.push_back(pNet);
			}
		}	
		sort(NetList.begin(),NetList.end(),CNetOpArea());
		CDesign::Display(DISPLAY_MODE_NONE,"(NOF:%5d)...* ",NetList.size());
		ReRoutingForOF(pDesign,&NetList,pDesign->m_Param.m_iMazeRouting_Margin);


#else
		vector<CBoundary*>	OverFlowBoundary;
		pDesign->GetOverFlowBoundary(&OverFlowBoundary);

		int*	pBoundaryGroup	=		new	int[OverFlowBoundary.size()];
		memset(pBoundaryGroup,0x00,OverFlowBoundary.size()*sizeof(int));

		int		iGroupIndex	=	1;
		pBoundaryGroup[0]	=	iGroupIndex++;

		for(int i=1,s=OverFlowBoundary.size();i<s;i++)
		{
			CPoint*	pPoint	=	OverFlowBoundary[i];
			for(int j=0;j<i;j++)
			{
				CPoint*	pCurPoint	=	OverFlowBoundary[j];
				if(pCurPoint->GetMDistance2D(pPoint)<=pDesign->m_Param.m_iMazeRouting_Margin)
				{
					if(pBoundaryGroup[i]&&pBoundaryGroup[i]!=pBoundaryGroup[j])
					{
						int	iCurIndex	=	pBoundaryGroup[j];
						for(int k=0;k<i;k++)
						{
							if(pBoundaryGroup[k]==iCurIndex)	pBoundaryGroup[k]	=	pBoundaryGroup[i];
						}
					}
					else
					{
						pBoundaryGroup[i]	=	pBoundaryGroup[j];
					}
				}				
			}		

			if(pBoundaryGroup[i]==0)	pBoundaryGroup[i]	=	iGroupIndex++;
		} 

		vector<CBBox>	BoxList;
		for(int i=0,s=OverFlowBoundary.size();i<s;i++)
		{
			if(pBoundaryGroup[i])
			{
				int	iCurIndex	=	pBoundaryGroup[i];
				CBBox	Box;

				for(int j=i;j<s;j++)
				{
					if(pBoundaryGroup[j]==iCurIndex)
					{
						Box.AddPoint(OverFlowBoundary[j]);
						pBoundaryGroup[j]	=	0;

					}
				}

				Box.Expand(pDesign->W(),pDesign->H(),MAX(pDesign->W(),pDesign->H())/50);


				BoxList.push_back(Box);
			}
		}

		if(pDesign->m_Param.m_iReRouting_Push>=50)
		{
			for(int i=0,s=pDesign->GetNumNet();i<s;i++)
			{
				CNet*	pNet	=	pDesign->GetNet(i);
				pNet->UpdateBBox();

				if(ObjectList.find((ADDRESS)pNet)==ObjectList.end()&&pNet->IsFlat())	continue;
				for(int j=0;j<BoxList.size();++j)
				{
					CBBox	Box	=	BoxList[j];
					if(Box.IsOverlapped(&pNet->m_BBox))
					{
						NetList.push_back(pNet);
						break;
					}
				}
			}	

			sort(NetList.begin(),NetList.end(),CNetOpArea());	
			CDesign::Display(DISPLAY_MODE_NONE,"(NOF:%5d)...* ",NetList.size());
			ReRoutingForOF(pDesign,&NetList,pDesign->m_Param.m_iMazeRouting_Margin);
		}
		else
		{
			vector<CWire*>	WireList;
			for(int j=0;j<BoxList.size();++j)
			{
				pDesign->GetWireInBBox(&WireList,&BoxList[j],GET_MODE_STATE,STATE_WIRE_ROUTED);
			}


			CDesign::Display(DISPLAY_MODE_NONE,"(WOF:%5d)...* ",WireList.size());
			sort(WireList.begin(),WireList.end(),CWireOpLen());		
			ReRoutingForOF(pDesign,&WireList,pDesign->m_Param.m_iMazeRouting_Margin);
		}

#endif


	}
	else
	{
		hash_map<ADDRESS,int>	ObjectList;
		int iObjectOverFlow	=	pDesign->GetWireInOverFlowBoundary(&ObjectList);
	
		vector<CWire*>	WireList;
		WireList.reserve(iObjectOverFlow);
		for(hash_map<ADDRESS,int>::iterator itr=ObjectList.begin();itr!=ObjectList.end();++itr)
		{
			WireList.push_back((CWire*)itr->first);
		}

		CDesign::Display(DISPLAY_MODE_NONE,"(WOF:%5d)... ",WireList.size());
		sort(WireList.begin(),WireList.end(),CWireOpLen());		
		ReRoutingForOF(pDesign,&WireList,pDesign->m_Param.m_iMazeRouting_Margin);
	}
}

int CGRouter::ReRoutingForOF(CDesign *pDesign, vector<CNet*>* pNetList, int iBound)
{
	FP_MAZECOST*	pMazeCost	=	NULL;
	
	if(iBound)	pMazeCost	=	MazeCost_PenaltyBound;
	else		pMazeCost	=	MazeCost_Penalty;

	int	iNumNetRouted	=	0;

	int	i,s;
	for(i=0,s=pNetList->size();i<s;++i)
	{		
		if(i%MAX(1,(s/10))==1)	CDesign::Display(DISPLAY_MODE_NONE,"-");

		CNet*	pNet	=	(*pNetList)[i];

		if(pNet==NULL)			continue;

		assert(!pNet->IsLocal() && !pNet->IsPreRouted());

		pNet->MergeWire();

#ifdef _DEBUG
		pNet->PrintLog();
#endif
		vector<CWire*>	Wires	=	pNet->m_Wire;
		for(vector<CWire*>::iterator itr=Wires.begin(),end=Wires.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;

			if(pWire->IsRouted())
			{
				pNet->DelWire(pWire);
				pWire->SetState(STATE_WIRE_UNROUTED);
				pWire->SetState(STATE_WIRE_NOTASSGNED);
				pNet->AddWire(pWire);
			}
		}

		sort(pNet->m_Wire.begin(),pNet->m_Wire.end(),CWireOpLen());

		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;)
		{
			CWire*	pWire	=	*itr;
			++itr;
			
			if(!pWire->IsRouted())
			{				
				//////////////////////////////////////////////////////////////////////////
				// filtering the already routed wires [7/7/2006 thyeros]
				//////////////////////////////////////////////////////////////////////////
				if(!FilterRouting(pDesign,pWire))
				{						
					pWire->ClearSegmentList();

					MazeRouting(pMazeCost,pDesign,pWire,NULL,iBound);	
					
					assert(pWire->GetNumSegmentList()==1);
					pWire->m_pRoutedSegmentList	=	pWire->GetSegmentList(0);
					assert(pWire->m_pRoutedSegmentList);	
				
					pWire->Pupate();
					pWire->Delete();//SAFE_DEL(pWire);
				}				
				
				itr	=	pNet->m_Wire.begin();
				end	=	pNet->m_Wire.end();
			}
		}

		pNet->Refine();
		++iNumNetRouted;
	}
	
	return	iNumNetRouted;
}

#define _SAFE_REROUTING_
#ifdef _SAFE_REROUTING_


int CGRouter::ReRoutingForWL(CDesign *pDesign, vector<CNet*>* pNetList)
{
	FP_MAZECOST*	pMazeCost	=	MazeCost_BoxBound;

	int	iViaCost		=	pDesign->m_Param.m_iViaCost;
	int	iNumNetRouted	=	0;
	for(int i=0,s=pNetList->size();i<s;++i)
	{		
		CNet*	pNet	=	(*pNetList)[i];

		if(pNet==NULL)			continue;

		assert(!pNet->IsLocal() && !pNet->IsPreRouted());


		vector<CWire*>	CurrentWires;
		CurrentWires.reserve(pNet->GetNumWire());
		// back up the current routing path [2/14/2007 thyeros]
		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			CurrentWires.push_back(pWire->Clone());
		}

		try
		{
			int				iCurrentVia		=	0;
			int				iCurrentWL		=	0;
			int				iCurrentOF		=	0;
			int				iCurrentMOF		=	0;

			if(pDesign->GetState()&(STATE_DESN_ROUTING|STATE_DESN_SUCCESS))
			{
				iCurrentVia		=	(pDesign->m_Param.GetProp()&PROP_PARAM_MULTILAYER)?	pNet->GetNumVia():0;
				iCurrentWL		=	pNet->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
				iCurrentOF		=	pDesign->GetOverFlow(GET_MODE_SUM);
			}
			else
			{
				iCurrentVia		=	(pDesign->m_Param.GetProp()&PROP_PARAM_MULTILAYER)?	pNet->GetNumVia():0;
				iCurrentWL		=	pNet->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
				iCurrentOF		=	pDesign->GetOverFlow(GET_MODE_SUM);
				iCurrentMOF		=	pDesign->GetOverFlow(GET_MODE_MAX);
			}



			pNet->MergeWire();

			vector<CWire*>	Wires	=	pNet->m_Wire;
			for(vector<CWire*>::iterator itr=Wires.begin(),end=Wires.end();itr!=end;++itr)
			{
				CWire*	pWire	=	*itr;
				if(pWire->IsRouted())
				{
					pNet->DelWire(pWire);
					pWire->SetState(STATE_WIRE_UNROUTED);
					pWire->SetState(STATE_WIRE_NOTASSGNED);
					pNet->AddWire(pWire);
				}
			}

			for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;)
			{
				CWire*	pWire	=	*itr;
				++itr;

				if(!pWire->IsRouted())
				{					
					//////////////////////////////////////////////////////////////////////////
					// filtering the already routed wires [7/7/2006 thyeros]
					//////////////////////////////////////////////////////////////////////////
					if(!FilterRouting(pDesign,pWire))
					{						
						pWire->ClearSegmentList();

						MazeRouting(pMazeCost,pDesign,pWire,NULL,1);

						assert(pWire->GetNumSegmentList()==1);
						pWire->m_pRoutedSegmentList	=	pWire->GetSegmentList(0);
						assert(pWire->m_pRoutedSegmentList);	

						pWire->Pupate();
						pWire->Delete();//SAFE_DEL(pWire);
					}				

					itr	=	pNet->m_Wire.begin();
					end	=	pNet->m_Wire.end();
				}
			}


			int				iUpdate	=	FALSE;

			if(pDesign->GetState()&(STATE_DESN_ROUTING|STATE_DESN_SUCCESS))
			{
				int	iNewVia	=	(pDesign->m_Param.GetProp()&PROP_PARAM_MULTILAYER)?	pNet->GetNumVia():0;
				int	iNewWL	=	pNet->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
				int	iNewOF	=	pDesign->GetOverFlow(GET_MODE_SUM);

				iUpdate		=	!(iNewWL+iNewVia*iViaCost>iCurrentWL+iCurrentVia*iViaCost ||iNewOF>iCurrentOF);
			}
			else
			{
				int	iNewVia	=	(pDesign->m_Param.GetProp()&PROP_PARAM_MULTILAYER)?	pNet->GetNumVia():0;
				int	iNewWL	=	pNet->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
				int	iNewOF	=	pDesign->GetOverFlow(GET_MODE_SUM);
				int	iNewMOF	=	pDesign->GetOverFlow(GET_MODE_MAX);

				iUpdate		=	!(iNewWL+iNewVia*iViaCost>iCurrentWL+iCurrentVia*iViaCost ||iNewOF>iCurrentOF || iNewMOF>iCurrentMOF);
			}

			if(!iUpdate)
			{
				//////////////////////////////////////////////////////////////////////////
				// restore the backed up routing [2/14/2007 thyeros]
				//////////////////////////////////////////////////////////////////////////			
				pNet->DelWires();
				for(int i=0;i<CurrentWires.size();++i)
				{
					CurrentWires[i]->SetState(STATE_WIRE_ROUTED|STATE_WIRE_ASSGNED);
					pNet->AddWire(CurrentWires[i]);
				}			
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////
				// keep the new/better routing [2/14/2007 thyeros]
				//////////////////////////////////////////////////////////////////////////		
				for(int i=0;i<CurrentWires.size();++i)	CurrentWires[i]->Delete();//SAFE_DEL(CurrentWires[i]);
				pNet->Refine();
			}
		}
		catch (...)
		{
			CDesign::Display(DISPLAY_MODE_WARN,"\n#net (id:%d) gets exception!#\n",(int)pNet->GetKey());
			//////////////////////////////////////////////////////////////////////////
			// restore the backed up routing [2/14/2007 thyeros]
			//////////////////////////////////////////////////////////////////////////			
			pNet->DelWires();
			for(int i=0;i<CurrentWires.size();++i)
			{
				CurrentWires[i]->SetState(STATE_WIRE_ROUTED|STATE_WIRE_ASSGNED);
				pNet->AddWire(CurrentWires[i]);
			}			

		}

		++iNumNetRouted;
	}

	return	iNumNetRouted;
}


#else

int CGRouter::ReRoutingForWL(CDesign *pDesign, vector<CNet*>* pNetList)
{
	FP_MAZECOST*	pMazeCost	=	MazeCost_BoxBound;

	int	iViaCost		=	pDesign->m_Param.m_iViaCost;
	int	iNumNetRouted	=	0;
	for(int i=0,s=pNetList->size();i<s;++i)
	{		
		CNet*	pNet	=	(*pNetList)[i];

		if(pNet==NULL)			continue;

		assert(!pNet->IsLocal() && !pNet->IsPreRouted());
		
		int				iCurrentVia		=	0;
		int				iCurrentWL		=	0;
		int				iCurrentOF		=	0;
		int				iCurrentMOF		=	0;

		if(pDesign->GetState()&(STATE_DESN_ROUTING|STATE_DESN_SUCCESS))
		{
			iCurrentVia		=	(pDesign->m_Param.GetProp()&PROP_PARAM_MULTILAYER)?	pNet->GetNumVia():0;
			iCurrentWL		=	pNet->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
			iCurrentOF		=	pDesign->GetOverFlow(GET_MODE_SUM);
		}
		else
		{
			iCurrentVia		=	(pDesign->m_Param.GetProp()&PROP_PARAM_MULTILAYER)?	pNet->GetNumVia():0;
			iCurrentWL		=	pNet->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
			iCurrentOF		=	pDesign->GetOverFlow(GET_MODE_SUM);
			iCurrentMOF		=	pDesign->GetOverFlow(GET_MODE_MAX);
		}

		vector<CWire*>	CurrentWires;
		CurrentWires.reserve(pNet->GetNumWire());
		// back up the current routing path [2/14/2007 thyeros]
		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			CurrentWires.push_back(pWire->Clone());
		}

		pNet->MergeWire();

		vector<CWire*>	Wires	=	pNet->m_Wire;
		for(vector<CWire*>::iterator itr=Wires.begin(),end=Wires.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			if(pWire->IsRouted())
			{
				pNet->DelWire(pWire);
				pWire->SetState(STATE_WIRE_UNROUTED);
				pWire->SetState(STATE_WIRE_NOTASSGNED);
				pNet->AddWire(pWire);
			}
		}

		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;)
		{
			CWire*	pWire	=	*itr;
			++itr;

			if(!pWire->IsRouted())
			{					
				//////////////////////////////////////////////////////////////////////////
				// filtering the already routed wires [7/7/2006 thyeros]
				//////////////////////////////////////////////////////////////////////////
				if(!FilterRouting(pDesign,pWire))
				{						
					pWire->ClearSegmentList();

					MazeRouting(pMazeCost,pDesign,pWire,NULL,1);

					assert(pWire->GetNumSegmentList()==1);
					pWire->m_pRoutedSegmentList	=	pWire->GetSegmentList(0);
					assert(pWire->m_pRoutedSegmentList);	

					pWire->Pupate();
					pWire->Delete();//SAFE_DEL(pWire);
				}				

				itr	=	pNet->m_Wire.begin();
				end	=	pNet->m_Wire.end();
			}
		}


		int				iUpdate	=	FALSE;

		if(pDesign->GetState()&(STATE_DESN_ROUTING|STATE_DESN_SUCCESS))
		{
			int	iNewVia	=	(pDesign->m_Param.GetProp()&PROP_PARAM_MULTILAYER)?	pNet->GetNumVia():0;
			int	iNewWL	=	pNet->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
			int	iNewOF	=	pDesign->GetOverFlow(GET_MODE_SUM);
			
			iUpdate		=	!(iNewWL+iNewVia*iViaCost>iCurrentWL+iCurrentVia*iViaCost ||iNewOF>iCurrentOF);
		}
		else
		{
			int	iNewVia	=	(pDesign->m_Param.GetProp()&PROP_PARAM_MULTILAYER)?	pNet->GetNumVia():0;
			int	iNewWL	=	pNet->GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
			int	iNewOF	=	pDesign->GetOverFlow(GET_MODE_SUM);
			int	iNewMOF	=	pDesign->GetOverFlow(GET_MODE_MAX);

			iUpdate		=	!(iNewWL+iNewVia*iViaCost>iCurrentWL+iCurrentVia*iViaCost ||iNewOF>iCurrentOF || iNewMOF>iCurrentMOF);
		}

		if(!iUpdate)
		{
			//////////////////////////////////////////////////////////////////////////
			// restore the backed up routing [2/14/2007 thyeros]
			//////////////////////////////////////////////////////////////////////////			
			pNet->DelWires();
			for(int i=0;i<CurrentWires.size();++i)
			{
				CurrentWires[i]->SetState(STATE_WIRE_ROUTED|STATE_WIRE_ASSGNED);
				pNet->AddWire(CurrentWires[i]);
			}			
		}
		else
		{
			//////////////////////////////////////////////////////////////////////////
			// keep the new/better routing [2/14/2007 thyeros]
			//////////////////////////////////////////////////////////////////////////		
			for(int i=0;i<CurrentWires.size();++i)	CurrentWires[i]->Delete();//SAFE_DEL(CurrentWires[i]);
			pNet->Refine();
		}

		++iNumNetRouted;
	}

	return	iNumNetRouted;
}
#endif

int CGRouter::MazeCost_Box(CDesign *pDesign, CGrid *pGrid, CGrid* pNGrid, CBBox *pBBox)
{
	CBoundary*	pBoundary	=	pGrid->GetParent()->GetBoundary(pGrid,pNGrid);
	if(pBoundary)	return	m_iMazeCost[pBoundary->m_iCongestion];
	else			return	m_iViaCost;
}

int CGRouter::MazeCost_BoxBound(CDesign *pDesign, CGrid *pGrid, CGrid* pNGrid, CBBox *pBBox)
{
	CBoundary*	pBoundary	=	pGrid->GetParent()->GetBoundary(pGrid,pNGrid);
	if(pBoundary)	return	m_iMazeCost[pBoundary->m_iCongestion] + ((pNGrid->GetMaze()&MAZE_GRID_BOUND)? MAX_PENALTY:0);
	else			return	m_iViaCost;
}

int CGRouter::MazeCost_Penalty(CDesign *pDesign, CGrid *pGrid, CGrid* pNGrid, CBBox *pBBox)
{
	CBoundary*	pBoundary	=	pGrid->GetParent()->GetBoundary(pGrid,pNGrid);
	if(pBoundary)	return	pBoundary->GetPenalty()+m_iMazeCost[pBoundary->m_iCongestion];
	else			return	m_iViaCost;
}

int CGRouter::MazeCost_PenaltyBound(CDesign *pDesign, CGrid *pGrid, CGrid* pNGrid, CBBox *pBBox)
{
	CBoundary*	pBoundary	=	pGrid->GetParent()->GetBoundary(pGrid,pNGrid);
	if(pBoundary)	return	pBoundary->GetPenalty()+m_iMazeCost[pBoundary->m_iCongestion] + ((pNGrid->GetMaze()&MAZE_GRID_BOUND)? MAX_PENALTY:0);
	else			return	m_iViaCost;
}

int CGRouter::SequentialRouting(CDesign *pDesign, vector<CWire*>* pWireList, CBBox* pBBox, int iBound)
{
	FP_MAZECOST*	pMazeCost	=	NULL;

	if(iBound)	pMazeCost	=	MazeCost_BoxBound;
	else		pMazeCost	=	MazeCost_Box;

	int	iNumWireRouted	=	0;

	vector<CWire*>	Wires;
	Wires.reserve(pWireList->size());

	for(int i=0,s=pWireList->size();i<s;++i)
	{
		CWire*	pWire	=	(*pWireList)[i];

		// it is already routed, if it should be NULL [6/23/2006 thyeros]
		if(pWire==NULL)	continue;	
		
		assert(!pWire->IsRouted());

		CNet*	pNet	=	pWire->GetParent();

		// it is deleted from the net [7/1/2006 thyeros]
		if(pNet==NULL)
		{
			pWire->Delete();//SAFE_DEL(pWire);
			// mark it is routed and DELETED [6/23/2006 thyeros]
			(*pWireList)[i]	=	NULL;
			
			++iNumWireRouted;

			continue;
		}

		Wires.push_back(pWire);
	}

	sort(Wires.begin(),Wires.end(),CWireOpLen());

	for(int i=0,s=Wires.size();i<s;++i)
	{
		CWire*	pWire	=	Wires[i];

		// it is already routed, if it should be NULL [6/23/2006 thyeros]
		if(pWire==NULL)	continue;		
		
		CNet*	pNet	=	pWire->GetParent();

		// it is deleted from the net [7/1/2006 thyeros]
		if(pNet==NULL)
		{
			pWire->Delete();//SAFE_DEL(pWire);
			// mark it is routed and DELETED [6/23/2006 thyeros]
			(*pWireList)[i]	=	NULL;
			
			++iNumWireRouted;

			continue;
		}

		// if two wires from one net are in the list, routing one can make the other one routed as well [2/27/2007 thyeros]
		if(pWire->IsRouted())
		{
			++iNumWireRouted;
			continue;
		}

		assert(pNet);
		assert(!pNet->IsRouted());
		//////////////////////////////////////////////////////////////////////////
		// filtering the already routed wires [7/7/2006 thyeros]
		//////////////////////////////////////////////////////////////////////////
		if(!FilterRouting(pDesign,pWire))
		{			
			//pWire->AssignLayer(pDesign->T()/2);

			pWire->AssignLayer(LAYER_METAL1);
			
			// bounded maze routing [2/14/2007 thyeros]
			//CBBox	BoundBox	=	*pBBox;
			//BoundBox.Expand(pDesign->W(),pDesign->H(),4);
#ifdef _DEBUG
			if(pNet->GetKey()==pDesign->m_iDebugNet)
			{
				pDesign->PrintFile("debug.m",PRINT_MODE_MATLAB);
				pNet->PrintFile(">debug.m",PRINT_MODE_MATLAB);
			}
#endif
			MazeRouting(pMazeCost,pDesign,pWire,pBBox,iBound);

			assert(pNet->IsFound(pWire)); 
	
			// routing path must be found [6/26/2006 thyeros]
			assert(pWire->GetNumSegmentList()==1);
			pWire->m_pRoutedSegmentList	=	pWire->GetSegmentList(0);
			assert(pWire->m_pRoutedSegmentList);
			
			pWire->Pupate();
			pWire->Delete();//SAFE_DEL(pWire);
#ifdef _DEBUG
			if(pNet->GetKey()==pDesign->m_iDebugNet)
			{
				pDesign->PrintFile("debug2.m",PRINT_MODE_MATLAB);
				pNet->PrintFile(">debug2.m",PRINT_MODE_MATLAB);
			}
#endif
		}
		
		// mark it is routed and DELETED [6/23/2006 thyeros]
		(*pWireList)[i]	=	NULL;
		
		++iNumWireRouted;
		
		pNet->Refine();
	}
	
	return	iNumWireRouted;
}

void CGRouter::SetBoundary(CDesign* pDesign, CBBox* pBBox)
{
	int	iMinX	=	pBBox->X();
	int	iMaxX	=	iMinX+pBBox->W();
	int	iMinY	=	pBBox->Y();
	int	iMaxY	=	iMinY+pBBox->H();
	//int	iMinZ	=	pBBox->Z();
	//int	iMaxZ	=	iMinZ+pBBox->T();

	for(int iZ=LAYER_METAL1;iZ<=pDesign->T();iZ++)
	{
		CLayer*	pLayer	=	pDesign->GetLayer(iZ);

		if(iMinX)					for(int iY=iMinY;iY<=iMaxY;iY++)	pLayer->GetGrid(iMinX,iY)->SetMaze(MAZE_GRID_BOUND);
		if(iMaxX!=pDesign->W()-1)	for(int iY=iMinY;iY<=iMaxY;iY++)	pLayer->GetGrid(iMaxX,iY)->SetMaze(MAZE_GRID_BOUND);

		if(iMinY)					for(int iX=iMinX;iX<=iMaxX;iX++)	pLayer->GetGrid(iX,iMinY)->SetMaze(MAZE_GRID_BOUND);
		if(iMaxY!=pDesign->H()-1)	for(int iX=iMinX;iX<=iMaxX;iX++)	pLayer->GetGrid(iX,iMaxY)->SetMaze(MAZE_GRID_BOUND);
	}
}

#ifdef _DEBUG
int	iMazeCount	=	0;
#endif
int CGRouter::MazeRouting(FP_MAZECOST* pMazeCost,CDesign* pDesign, CWire* pWire, CBBox* pBBox, int iBound)
{
	assert(pWire);

	CNet*	pNet		=	pWire->GetParent();

	vector<CPoint*>		Source,Target,Bridge;
	int					iBridgeGrpIndex	=	1;
	int					iNumLayer		=	pDesign->T();
	CGrid*				pGrid			=	NULL;
	CLayer*				pLayer			=	NULL;
	CBBox				TargetBBox;
	CBBox				BoundBox;
	CMazeHeap			Queue;

#ifdef _DEBUG
	if(pNet->GetKey()==332&&pWire==(CWire*)0x06898d50)
	{
		pNet->PrintLog();
		pWire->PrintLog();
	}
#endif

	int i,j,s;
	for(i=LAYER_METAL1;i<=iNumLayer;++i)	pDesign->GetLayer(i)->ResetMaze();

	Source.reserve(pNet->m_iMinWL);
	Target.reserve(pNet->m_iMinWL);
	Bridge.reserve(pNet->m_iMinWL);
	// mark some routed wires isolated from source and target [6/24/2006 thyeros]
	if(pNet->GetPoints(&Bridge,NULL,GET_MODE_STATE,STATE_WIRE_ROUTED))
	{		
		for (i=0,s=Bridge.size();i<s;++i)
		{
			pGrid	=	(CGrid*)Bridge[i];
			pGrid->SetMaze(MAZE_GRID_BRIDGE);
			if(iBound)	BoundBox.AddPoint(pGrid);
		}
	}

	// mark some routed wires connected to source and target [6/24/2006 thyeros]
	if(pNet->GetPoints(&Source,pWire->m_pPointS,GET_MODE_STATE,STATE_WIRE_ROUTED)
		&&pNet->GetPoints(&Target,pWire->m_pPointE,GET_MODE_STATE,STATE_WIRE_ROUTED))
	{	
		assert(Source.size());
		assert(Target.size());

		for (i=0,s=Target.size();i<s;++i)
		{
			pGrid	=	(CGrid*)Target[i];
			pGrid->SetMaze(MAZE_GRID_TARGET);
			if(iBound)	BoundBox.AddPoint(pGrid);
		}

		for (i=0,s=Source.size();i<s;++i)
		{
			pGrid	=	(CGrid*)Source[i];
			assert(!(pGrid->GetMaze()&MAZE_GRID_TARGET));
			pGrid->SetMaze(MAZE_GRID_SOURCE);
			if(iBound)	BoundBox.AddPoint(pGrid);
		}


		for (i=0,s=Source.size();i<s;++i)
		{
			pGrid	=	(CGrid*)Source[i];
			pGrid->SetMazeCostGrid(0,pGrid->GetMDistance(pWire->m_pPointE),NULL);
			Queue.Push(pGrid);
		}
	}
	else
	{
		assert(FALSE);
	}


	if(iBound)
	{
		BoundBox.Expand(pDesign->W()-1,pDesign->H()-1,iBound);
		SetBoundary(pDesign,&BoundBox);
	}

	multimap<ADDRESS, int>	BridgeGrp;
	//////////////////////////////////////////////////////////////////////////
	// flooding [6/24/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	CGrid*			pTarget		=	NULL;
	int				iTargetCost	=	0;
	int				iGridCost	=	0;

	while (Queue.GetCount())
	{
		pGrid	=	Queue.Top();Queue.Pop();

		iGridCost	=	pGrid->GetMazeCost();
		if(pTarget&&iGridCost>=iTargetCost)	break;

#define PCOST_FACTOR	(4)
//		int	PCOST_FACTOR	=	iGridCost/MAX(1,pGrid->GetMDistance(pWire->m_pPointS))/2;

		int	iPCost		=	pGrid->GetMDistance(pWire->m_pPointE)*PCOST_FACTOR;

		for (i=0;i<MAX_ADJ_GRID;++i)
		{
			CGrid*	pNGrid	=	pGrid->GetAdjGrid(i);

			if(!pNGrid)	break;

#ifdef _DEBUG
			iMazeCount++;
			if(iMazeCount==1704)
			{
				int a=0;
			}
#endif
			int	iCost		=	(*pMazeCost)(pDesign,pGrid,pNGrid,/*pNet,*/pBBox)+iGridCost;
			assert(iCost>=0);

			switch(pNGrid->GetMaze()){
			case MAZE_GRID_SOURCE:
				// do nothing [6/24/2006 thyeros]
				break;
			case MAZE_GRID_TARGET:
				if(!pTarget || iTargetCost>iCost)
				{
					pNGrid->SetMazeCostGrid(iCost,pGrid);
					pTarget		=	pNGrid;	
					iTargetCost	=	iCost;
				}
				break;
			case MAZE_GRID_BRIDGE:
				if(pNGrid->GetMazeCost()>iCost)
				{					
					multimap<ADDRESS,int>::iterator itr = BridgeGrp.find((ADDRESS)pNGrid);
					if(itr!=BridgeGrp.end())
					{
						int	iBridgeGrp	=	itr->second;                                   

						multimap<ADDRESS,int>::iterator itrl	=	BridgeGrp.lower_bound(iBridgeGrp);
						multimap<ADDRESS,int>::iterator itru	=	BridgeGrp.upper_bound(iBridgeGrp);

						for(itr=itrl;itr!=itru;++itr)
						{
							CGrid*	pBGrid	=	(CGrid*)itr->first;
							if(pBGrid!=pNGrid)
							{
								assert(pBGrid->GetMaze()&MAZE_GRID_BRIDGE);
								pBGrid->SetMazeCostGrid(iCost,pBGrid->GetMDistance2D(pWire->m_pPointE)*PCOST_FACTOR,pNGrid);
								Queue.Push(pBGrid);
							}
						}					
					}
					else
					{
						Bridge.clear();
						if(pNet->GetPoints(&Bridge,pNGrid,GET_MODE_STATE,STATE_WIRE_ROUTED))
						{
							for (j=0,s=Bridge.size();j<s;++j)
							{
								CGrid*	pBGrid	=	(CGrid*)Bridge[j];
								if(pBGrid!=pNGrid)
								{
									assert(pBGrid->GetMaze()&MAZE_GRID_BRIDGE);
									pBGrid->SetMazeCostGrid(iCost,pBGrid->GetMDistance2D(pWire->m_pPointE)*PCOST_FACTOR,pNGrid);
									Queue.Push(pBGrid);
								}

								BridgeGrp.insert(pair<ADDRESS, int>((ADDRESS)pBGrid,iBridgeGrpIndex));
							}
							iBridgeGrpIndex++;
						}
					}

					pNGrid->SetMazeCostGrid(iCost,iPCost,pGrid);
					Queue.Push(pNGrid);
				}
				break;
			case MAZE_GRID_FLOODED|MAZE_GRID_BOUND:
			case MAZE_GRID_FLOODED:
				if(pNGrid->GetMazeCost()>iCost)
				{
					pNGrid->SetMazeCostGrid(iCost,iPCost,pGrid);
					Queue.Push(pNGrid);
				}
				break;
			case MAZE_GRID_BOUND:
			case MAZE_GRID_NULL:
				pNGrid->SetMazeCostGrid(iCost,iPCost,pGrid);
				pNGrid->SetMaze(MAZE_GRID_FLOODED);
				Queue.Push(pNGrid);
				break;
			default:
				assert(FALSE);
				break;
			}
		}
	}// while(Queue) [6/24/2006 thyeros]

	//////////////////////////////////////////////////////////////////////////
	// traceback [6/24/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	if(pTarget==NULL)
	{
		CDesign::Display(DISPLAY_MODE_ERRO,"target is not found!!! ==> current routable layers are %d - %d\n",pDesign->GetRoutableLayer(GET_MODE_MIN),pDesign->T());
		pNet->PrintLog();
		pWire->PrintLog();

		assert(pTarget);
	}

	vector<CSegment*>	SegmentList;
	vector<CGrid*>		Trace;	
	Trace.reserve(pNet->m_iMinWL);
	for (pGrid=pTarget;pGrid;pGrid=pGrid->GetMazeGrid())
	{
		assert(pGrid->GetMazeGrid()==NULL||pGrid->GetMazeGrid()->GetMazeGrid()!=pGrid);

		Trace.push_back(pGrid);
		if(pGrid->GetMaze()&MAZE_GRID_BRIDGE)
		{
			TraceBack(pWire,&Trace,&SegmentList);

			assert(pGrid->GetMazeGrid());

			while(pGrid->GetMazeGrid()->GetMaze()&MAZE_GRID_BRIDGE)
			{
				if(BridgeGrp.find((ADDRESS)pGrid)->second==BridgeGrp.find((ADDRESS)pGrid->GetMazeGrid())->second)		pGrid=pGrid->GetMazeGrid();
				else																									break;
			}

			Trace.push_back(pGrid);
		}
		else if(pGrid->GetMaze()&MAZE_GRID_SOURCE)
		{
			break;
		}
	}

	TraceBack(pWire,&Trace,&SegmentList);

	//////////////////////////////////////////////////////////////////////////
	// create segmentlist [6/24/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////

	assert(SegmentList.size());
	pWire->AddSegmentList(SegmentList);

	return	TRUE;
}

int CGRouter::FilterRouting(CDesign* pDesign, CWire* pWire)
{
	assert(pWire);
	
	if(pWire->IsCompleted())	
	{
		return	TRUE;
	}
	
	CNet*	pNet	=	pWire->GetParent();
	// it is deleted from the net [7/1/2006 thyeros]
	if(pNet==NULL)	return	TRUE;

	pNet->SplitWire();
	pNet	=	pWire->GetParent();
	
	// it is deleted from the net [7/1/2006 thyeros]
	if(pNet==NULL)	return	TRUE;
	if(pWire->IsCompleted())		return	TRUE;
	
	if(pNet->IsConnected(pWire->m_pPointS,pWire->m_pPointE))
	{
		pNet->DelWire(pWire);
		pNet->Refine();

		pWire->Delete();//SAFE_DEL(pWire);

		return	TRUE;
	}

	return	FALSE;
}

int CGRouter::ConcurrentRouting(CDesign* pDesign, vector<CWire*>* pWireList, int iMode)
{
	// make ILP solver ready [6/22/2006 thyeros]
	CILPSolver ILPSolver;
	
	ILPSolver.Delete();
	ILPSolver.Create((char*)pDesign->m_Param.m_cInput_File);

	char cName[MAX_BUFFER_STR];

	int	i,j,k,x,y,s,iD,iNumVariable;
	int	iNumWireRouted		=	0;

	//////////////////////////////////////////////////////////////////////////
	// filtering the already routed wires [7/7/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	for(i=0,s=pWireList->size();i<s;++i)
	{
		CWire*	pWire	=	(*pWireList)[i];
		if(pWire==NULL)	continue;
		
		if(FilterRouting(pDesign,pWire))
		{	
			// mark it is routed and DELETED [6/23/2006 thyeros]
			(*pWireList)[i]	=	NULL;
			
			++iNumWireRouted;			
		}
	}

	vector<vector<CSegment*>* >		InverseVariable;
	hash_map<ADDRESS, int>				Variable;
	set<CBoundary*>					Boundary;

	InverseVariable.reserve(pWireList->size()*2*pDesign->T());

	//thyeros: make the first empty as GLPK is doing [10/29/2005]
	InverseVariable.push_back((vector<CSegment*>*)NULL);
	
	//////////////////////////////////////////////////////////////////////////
	// variable constraints [6/24/2006 thyeros]
	// set objective function [6/24/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	for(i=0,s=pWireList->size();i<s;++i)
	{
		CWire*	pWire	=	(*pWireList)[i];
		if(pWire==NULL)	continue;

		assert(!pWire->IsRouted());

		CNet*	pNet	=	pWire->GetParent();
		// it is deleted from the net [7/1/2006 thyeros]
		if(pNet==NULL)
		{
			pWire->Delete();//SAFE_DEL(pWire);
			// mark it is routed and DELETED [6/23/2006 thyeros]
			(*pWireList)[i]	=	NULL;			
			++iNumWireRouted;

			continue;
		}
		
		assert(pNet);
		assert(!pNet->IsRouted());
		assert(!pWire->IsPoint());

		pWire->Enumerate(ENUM_MODE_L);
		
		int	iNumSegmentList	=	pWire->GetNumSegmentList();
		
		for(j=0;j<iNumSegmentList;++j)
		{		
			vector<CSegment*>*	pSegmentList	=	pWire->GetSegmentList(j);
			for (k=0;k<pSegmentList->size();k++)
			{
				CSegment*	pCurSegment	=	(*pSegmentList)[k];
				CBoundary*	pBoundary	=	NULL;
				CGrid*		pGrid		=	NULL;
				
				if(pCurSegment->IsHorizontal())
				{
					assert(pCurSegment->m_pPointS->Y()==pCurSegment->m_pPointE->Y());
					assert(pCurSegment->m_pPointS->Z()==pCurSegment->m_pPointE->Z());
					int iZ		=	pCurSegment->m_pPointS->Z();
					int	iY		=	pCurSegment->m_pPointS->Y();
					int	iMinX	=	pCurSegment->m_pPointS->X();
					int	iMaxX	=	pCurSegment->m_pPointE->X();
					
					assert(iMinX<=iMaxX);
					
					for (x=iMinX;x<iMaxX;x++)
					{
						pBoundary	=	pDesign->GetLayer(iZ)->GetBoundary(x,iY);					
						assert(pBoundary);
						
						if(pBoundary->IsFound(pNet))	continue;

						pBoundary->AddSegmentList(pSegmentList);
						//Boundary[pBoundary->GetKey()]	=	pBoundary;

						Boundary.insert(pBoundary);
					}
				}
				else if(pCurSegment->IsVertical())
				{
					assert(pCurSegment->m_pPointS->X()==pCurSegment->m_pPointE->X());
					assert(pCurSegment->m_pPointS->Z()==pCurSegment->m_pPointE->Z());
					int iZ		=	pCurSegment->m_pPointS->Z();
					int	iX		=	pCurSegment->m_pPointS->X();
					int	iMinY	=	pCurSegment->m_pPointS->Y();
					int	iMaxY	=	pCurSegment->m_pPointE->Y();
					
					assert(iMinY<=iMaxY);
					
					for (y=iMinY;y<iMaxY;y++)
					{
						pBoundary	=	pDesign->GetLayer(iZ)->GetBoundary(iX,y);						
						assert(pBoundary);
						
						if(pBoundary->IsFound(pNet))	continue;

						pBoundary->AddSegmentList(pSegmentList);
						Boundary.insert(pBoundary);
					}
				}
				else
				{
					assert(FALSE);
				}
			}// for each segment [6/22/2006 thyeros]
				
			if(iMode&PROP_PARAM_SOLVER_GLPK)
			{
#ifdef _DEBUG
				sprintf(cName,"v-seg%d.(%d,%d,%d)-(%d,%d,%d).%d",
					(int)pNet->GetKey(),
					pWire->m_pPointS->X(),pWire->m_pPointS->Y(),pWire->m_pPointS->Z(),
					pWire->m_pPointE->X(),pWire->m_pPointE->Y(),pWire->m_pPointE->Z(),j);
				for (k=0;k<pSegmentList->size();k++)
				{
					CSegment*	pCurSegment	=	(*pSegmentList)[k];
					sprintf(&cName[strlen(cName)],"=(%d,%d,%d)->(%d,%d,%d)",
						pCurSegment->m_pPointS->X(),pCurSegment->m_pPointS->Y(),pCurSegment->m_pPointS->Z(),
						pCurSegment->m_pPointE->X(),pCurSegment->m_pPointE->Y(),pCurSegment->m_pPointE->Z());
				}
#else
				cName[0]	=	NULL;		
#endif
			}
			else if(iMode&PROP_PARAM_SOLVER_MOSEK)
			{
				sprintf(cName,"X%d",ILPSolver.GetNumVariable()+1);
			}

			// each variable means a set of Segments which is a possible routing for this Wire [6/22/2006 thyeros]			
			// Variable[xxx] contains its index in problem matrix (column) [6/22/2006 thyeros]

			// layer assignment cost [9/22/2006 thyeros]
			if(iMode&PROP_PARAM_BOXR_ILPMAX)
			{
				double	dCost = 1+((pDesign->m_Param.GetProp()&PROP_PARAM_MULTILAYER)? 1.0/CWire::GetSegmentListCost(pSegmentList)/s:0);
				
				Variable[(ADDRESS)pSegmentList]	=	ILPSolver.AddVariable(cName,dCost,TRUE,0,TRUE,1);
			}
			else if(iMode&(PROP_PARAM_BOXR_ILPHYD|PROP_PARAM_BOXR_ILPMIN))
			{
				double	dCost =	0;
				
				Variable[(ADDRESS)pSegmentList]	=	ILPSolver.AddVariable(cName,dCost,TRUE,0,TRUE,1);
			}
			
			InverseVariable.push_back(pSegmentList);
			
		}// for each segment list [6/22/2006 thyeros]
				
		ILPSolver.PrepareCoeffBuffer(iNumSegmentList+1,Variable.size());
		

		if(iMode&PROP_PARAM_SOLVER_GLPK)
		{
#ifdef _DEBUG
			sprintf(cName,"v-wire%d.(%d,%d)-(%d,%d)",(int)pNet->GetKey(),pWire->m_pPointS->X(),pWire->m_pPointS->Y(),pWire->m_pPointE->X(),pWire->m_pPointE->Y());
#else
			cName[0]	=	NULL;		
#endif
		}
		else if(iMode&PROP_PARAM_SOLVER_MOSEK)
		{
			sprintf(cName,"Z%d",ILPSolver.GetNumConstraint()+1);
		}

		// add constraint such that one of segment lists for each wire should be selected [6/22/2006 thyeros]
		if(iMode&PROP_PARAM_BOXR_ILPMAX)								ILPSolver.AddConstraint(cName,iNumSegmentList,TRUE,0,TRUE,1);
		else if(iMode&(PROP_PARAM_BOXR_ILPHYD|PROP_PARAM_BOXR_ILPMIN))	ILPSolver.AddConstraint(cName,iNumSegmentList,TRUE,1,TRUE,1);	
	}// for each wire [6/22/2006 thyeros]
	
	InverseVariable.resize(InverseVariable.size());
	assert(Variable.size()==InverseVariable.size()-1);
	iNumVariable	=	Variable.size();

	//////////////////////////////////////////////////////////////////////////
	// depending on the obj, needs max utilization variable [6/28/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	if(iMode&PROP_PARAM_BOXR_ILPMAX)
	{
		iD	=	-1;
	}
	else if(iMode&(PROP_PARAM_BOXR_ILPHYD|PROP_PARAM_BOXR_ILPMIN))
	{
		iD	=	ILPSolver.AddVariable("v-D",1,TRUE,0,FALSE,MAX_NUMBER);

		if(iMode&PROP_PARAM_BOXR_ILPHYD)
		{
			ILPSolver.SetCoeffIndex(1,iD);
			ILPSolver.AddConstraint("c-D",1,TRUE,0,TRUE,pDesign->GetCapacity(GET_MODE_MAX)+1);
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	// congestion constraint for each boundary [6/22/2006 thyeros]
	//////////////////////////////////////////////////////////////////////////
	for(set<CBoundary*>::iterator itr=Boundary.begin(),end=Boundary.end();itr!=end;++itr)
	{
		CBoundary*	pBoundary	=	*itr;//->second;
		int	iNumSegmentList		=	pBoundary->GetNumSegmentList();
		
		ILPSolver.PrepareCoeffBuffer(iNumSegmentList+2);
		
		for (i=0;i<iNumSegmentList;++i)
		{
			vector<CSegment*>*	pSegmentList	=	pBoundary->GetSegmentList(i);
			
			assert(Variable.find((ADDRESS)pSegmentList)!=Variable.end());			
			ILPSolver.SetCoeffIndex(i+1,Variable.find((ADDRESS)pSegmentList)->second);

			// each segment needs its own required width and spacing [2/3/2007 thyeros]
			ILPSolver.SetCoeff(i+1,2);//pBoundary->GetParent()->GetDesignRule(GET_DR_MIN_WIR_SPACING)+pSegmentList->at(0)->GetWidth());
		}

		if(iMode&PROP_PARAM_SOLVER_GLPK)
		{
#ifdef _DEBUG
			sprintf(cName,"v-bnd(%d,%d,%d)_of_%d",pBoundary->X(),pBoundary->Y(),pBoundary->Z(),pBoundary->GetCapacity(GET_MODE_ACAP));
#else
			cName[0]	=	NULL;		
#endif
		}
		else if(iMode&PROP_PARAM_SOLVER_MOSEK)
		{
			sprintf(cName,"Y%d",ILPSolver.GetNumConstraint());
		}

		if(iMode&PROP_PARAM_BOXR_ILPMAX)
		{
			assert(iD==-1);
			ILPSolver.AddConstraint(cName,iNumSegmentList,TRUE,0,TRUE,pBoundary->GetCapacity(GET_MODE_ACAP));
		}
		else if(iMode&(PROP_PARAM_BOXR_ILPHYD|PROP_PARAM_BOXR_ILPMIN))
		{
			assert(iD>0);
			ILPSolver.SetCoeffIndex(i+1,iD);

			ILPSolver.SetCoeff(i+1,-1.0*pBoundary->GetParent()->GetCapacity(GET_MODE_MAX)/pDesign->GetCapacity(GET_MODE_MAX));	
			ILPSolver.AddConstraint(cName,iNumSegmentList+1,FALSE,0,TRUE,-pBoundary->GetCapacity(GET_MODE_OCAP));

			// make it back to one for other variables [6/26/2006 thyeros]
			ILPSolver.SetCoeff(i+1,1);
		}
		
		pBoundary->ClearSegmentList();	
		assert(pBoundary->GetNumSegmentList()==0);
	}
	
	CObject::PrintMsg2("..#v=%d #c=%d ",ILPSolver.GetNumVariable(),ILPSolver.GetNumConstraint());
	
	int	iSolMode	=	0;
	if(iMode&PROP_PARAM_SOLVER_GLPK)
	{
		if(iMode&PROP_PARAM_BOXR_ILPMAX)
		{
			if(iMode&PROP_PARAM_BOXR_ILP_RNDUP)		iSolMode	=	SOL_OPTION_MAX|SOL_OPTION_SIMPLEX;
			else									iSolMode	=	SOL_OPTION_MAX|SOL_OPTION_MIP;
		}
		else if(iMode&PROP_PARAM_BOXR_ILPMIN)
		{
			if(iMode&PROP_PARAM_BOXR_ILP_RNDUP)		iSolMode	=	SOL_OPTION_MIN|SOL_OPTION_SIMPLEX;
			else									iSolMode	=	SOL_OPTION_MIN|SOL_OPTION_MIP;
		}
		else if(iMode&PROP_PARAM_BOXR_ILPHYD)
		{
			//////////////////////////////////////////////////////////////////////////
			// heuristically decide the better approach [6/29/2006 thyeros]
			// check # of constratins [6/29/2006 thyeros]
			//////////////////////////////////////////////////////////////////////////
			if(iMode&PROP_PARAM_BOXR_ILP_RNDUP)		iSolMode	=	ILPSolver.GetNumConstraint()>MAX_ILP_CONSTRAINT? 0:SOL_OPTION_MIN|SOL_OPTION_SIMPLEX;
			else									iSolMode	=	ILPSolver.GetNumConstraint()>MAX_ILP_CONSTRAINT? 0:SOL_OPTION_MIN|SOL_OPTION_MIP;
		}

		if(iSolMode)
		{
			//////////////////////////////////////////////////////////////////////////
			// solve the problem [6/24/2006 thyeros]
			//////////////////////////////////////////////////////////////////////////
			double	dObjectValue	=	0;
			ILPSolver.PrepareSolutionBuffer(InverseVariable.size());			

#ifdef _DEBUG
			//sprintf(cName,"%s_p%d.txt",pDesign->m_Param.m_cInput_File,pDesign->m_iLoop-1);
			//ILPSolver.Print(cName,SOL_PRINT_PRO);
#endif
	
			if(ILPSolver.Solve(iSolMode,&dObjectValue))
			{
#ifdef _DEBUG
				//sprintf(cName,"%s_s%d.txt",pDesign->GetName(),pDesign->m_iLoop-1);
				//ILPSolver.Print(cName,SOL_PRINT_SOL);
#endif
				CObject::PrintMsg2("*o=%.2f\t",dObjectValue);

				if(iMode&PROP_PARAM_BOXR_ILP_RNDUP)		ILPSolver.RoundUp(0.5);

				if(iMode&PROP_PARAM_BOXR_ILPHYD&&(int)dObjectValue>pDesign->GetCapacity(GET_MODE_MAX))
				{
					CObject::PrintMsg2("=give up solution\n");
				}
				else
				{
					//////////////////////////////////////////////////////////////////////////
					// retrieve the solution [6/24/2006 thyeros]
					//////////////////////////////////////////////////////////////////////////	
					for (i=1;i<=iNumVariable;++i)
					{
						if(ILPSolver.GetSolution(i))
						{	
							CSegment*	pSegment	=	(*InverseVariable[i])[0];

//							assert(pSegment->GetParent()->m_pRoutedSegmentList==NULL);
							pSegment->GetParent()->m_pRoutedSegmentList	=	InverseVariable[i];	
							assert(pSegment->GetParent()->m_pRoutedSegmentList->size());
						}
					}		
				}
#ifdef _DEBUG
				if(iD>0)
				{
					if((int)ILPSolver.GetSolution(iD)!=(int)dObjectValue)
					{
						printf("ERROR: ILP objective iD=%d(%f) vs dObjectValue=%d(%f)\n",
							(int)ILPSolver.GetSolution(iD),ILPSolver.GetSolution(iD),
							(int)dObjectValue,dObjectValue);
					}
					assert((int)ILPSolver.GetSolution(iD)==(int)dObjectValue);
				}
#endif
			}
		}
		else
		{
			CObject::PrintMsg2("\t= discarded (maxc %d)\n",MAX_ILP_CONSTRAINT);
		}
	}
	else if(iMode&PROP_PARAM_SOLVER_MOSEK)
	{
		sprintf(cName,"%s_l%d.mps",pDesign->m_Param.m_cInput_File,pDesign->m_iLoop-1);
		//sprintf(cName,"br.mps");
		ILPSolver.Write(cName,SOL_FORMAT_FREEMPS);

		// run mosek [10/13/2006 thyeros]
		char cLine[MAX_BUFFER_STR];

		sprintf(cLine,"mosek -max %s -d MSK_IPAR_READ_MPS_FORMAT 2 -d MSK_IPAR_WRITE_SOL_CONSTRAINTS 0 -d MSK_IPAR_WRITE_BAS_CONSTRAINTS 0 -d MSK_IPAR_WRITE_BAS_HEAD 0 -d MSK_IPAR_WRITE_BAS_VARIABLES 0 -d MSK_IPAR_LOG 0",cName);

		CreateAndWait(cLine);

		sprintf(cName,"%s_l%d.sol",pDesign->m_Param.m_cInput_File,pDesign->m_iLoop-1);
		for(FILE* pFile=fopen(cName,"rt");fgets(cLine,sizeof(cLine),pFile);)
		{
			char*	pToken	=	strtok(cLine," \t\n");
			pToken	=	strtok(NULL," \t\n");

			if(pToken==NULL)	continue;

			switch(pToken[0]){
				case 'X':
					{
						int		iIndex			=	atoi(pToken+1);
						strtok(NULL," \t\n");
						if(atof(strtok(NULL," \t\n"))>=0.5)
						{
							CSegment*	pSegment	=	(*InverseVariable[iIndex])[0];

							assert(pSegment->GetParent()->m_pRoutedSegmentList==NULL);
							pSegment->GetParent()->m_pRoutedSegmentList	=	InverseVariable[iIndex];	
							assert(pSegment->GetParent()->m_pRoutedSegmentList->size());
						}
					}
					break;
				default:
					break;
			}	
		}
	}

	for(i=0,s=pWireList->size();i<s;++i)
	{
		CWire*	pWire	=	(*pWireList)[i];
		if(pWire==NULL)	continue;

		CNet*	pNet	=	pWire->GetParent();

		if(pNet==NULL)
		{
			pWire->Delete();//SAFE_DEL(pWire);

			// mark it is routed and DELETED [6/23/2006 thyeros]
			(*pWireList)[i]	=	NULL;
			
			++iNumWireRouted;

			// must be discarded [7/1/2006 thyeros]
			continue;
		}

		if(pWire->m_pRoutedSegmentList)
		{
			assert(pWire->m_pRoutedSegmentList->size());

/*			if(pNet->GetKey()==11)
			{
				pNet->Print(stdout,PRINT_MODE_TEXT);
				pWire->Print(stdout,PRINT_MODE_TEXT);

				printf("==============\n");				

				vector<CSegment*>* pSegmentList	=	pWire->m_pRoutedSegmentList;
				for(int k=0;k<pSegmentList->size();k++)
				{
					pSegmentList->at(k)->Print(stdout,PRINT_MODE_TEXT);	
				}
			}
*/

			pWire->Pupate();
			pWire->Delete();//SAFE_DEL(pWire);

			// mark it is routed and DELETED [6/23/2006 thyeros]
			(*pWireList)[i]	=	NULL;
	
			++iNumWireRouted;

			pNet->Refine();
		}
		else
		{
			pWire->ClearSegmentList();
		}

#ifdef _DEBUG
		if(pNet->GetKey()==pDesign->m_iDebugNet)
		{
			pDesign->PrintFile("debug.m",PRINT_MODE_MATLAB);
			pNet->PrintFile(">debug.m",PRINT_MODE_MATLAB);
		}
#endif
	}
	
	return	iNumWireRouted;
}

void CGRouter::TraceBack(CWire* pWire, vector<CGrid*>* pTrace, vector<CSegment*>*	pSegmentList)
{
	assert(pTrace->size()>=2);	// at least, source and target [6/24/2006 thyeros]
	
	CSegment*	pSegment	=	CSegment::New();//new CSegment;
	pSegment->Initialize((*pTrace)[0]->X(),(*pTrace)[0]->Y(),(*pTrace)[0]->Z(),(*pTrace)[1]->X(),(*pTrace)[1]->Y(),(*pTrace)[1]->Z(),pWire);
	
	pSegmentList->reserve(pTrace->size());

	for (int i=2,s=pTrace->size();i<s;++i)
	{
		if(pSegment->GetDirection()==pSegment->m_pPointE->GetDirection((*pTrace)[i]))
		{
			pSegment->Initialize(pSegment->m_pPointS,(*pTrace)[i]->X(),(*pTrace)[i]->Y(),(*pTrace)[i]->Z(),pWire);
		}
		else
		{
			CPoint*	pPoint	=	pSegment->m_pPointE;
			if(!pSegment->IsPerpendicular())
			{
				pSegment->FixSE();
				pSegmentList->push_back(pSegment);
			}
			else				
			{
				pSegment->Delete();//SAFE_DEL(pSegment);
			}
			
			pSegment	=	CSegment::New();//new CSegment;
			pSegment->Initialize(pPoint,(*pTrace)[i]->X(),(*pTrace)[i]->Y(),(*pTrace)[i]->Z(),pWire);
		}
	}
	
	if(!pSegment->IsPerpendicular())
	{
		pSegment->FixSE();
		pSegmentList->push_back(pSegment);
	}
	else				
	{
		pSegment->Delete();//SAFE_DEL(pSegment);
	}
	
	pTrace->clear();
}
//
//int CGRouter::PatternRouting(FP_MAZECOST* pMazeCost,CDesign *pDesign, CWire *pWire)
//{
//	assert(pWire);
//	
//	CNet*	pNet	=	pWire->GetParent();
//	assert(pNet);
//	
//	int	iOptimalCost	=	pWire->GetLength2D();
//	int	iMinSegmentList	=	-1;
//	int	iNumSegmentList	=	-1;
//	int	iMinCost		=	MAX_NUMBER;
//	int	j,k;
//		
//	pWire->Enumerate(ENUM_MODE_L);
//	iNumSegmentList	=	pWire->GetNumSegmentList();
//	for(j=0;j<iNumSegmentList;++j)
//	{		
//		vector<CSegment*>*	pSegmentList	=	pWire->GetSegmentList(j);
//		
//		int	iCurCost	=	0;
//		for (k=0;k<pSegmentList->size();++k)
//		{
//			CSegment*	pCurSegment	=	(*pSegmentList)[k];
//			CBoundary*	pBoundary	=	NULL;
//			CGrid*		pGrid		=	NULL;
//			
//			if(pCurSegment->IsHorizontal())
//			{
//				assert(pCurSegment->m_pPointS->Y()==pCurSegment->m_pPointE->Y());
//				assert(pCurSegment->m_pPointS->Z()==pCurSegment->m_pPointE->Z());
//				int iZ		=	pCurSegment->m_pPointS->Z();
//				int	iY		=	pCurSegment->m_pPointS->Y();
//				int	iMinX	=	pCurSegment->m_pPointS->X();
//				int	iMaxX	=	pCurSegment->m_pPointE->X();
//				
//				assert(iMinX<=iMaxX);
//				
//				for (int x=iMinX;x<iMaxX;x++)
//				{
//					CGrid*	pGrid	=	pDesign->GetLayer(iZ)->GetGrid(x,iY);
//					CGrid*	pNGrid	=	pDesign->GetLayer(iZ)->GetGrid(x+1,iY);
//
//					iCurCost		+=	MazeCost(pDesign,pGrid,pNGrid,/*pNet,*/NULL);
//				}
//			}
//			else if(pCurSegment->IsVertical())
//			{
//				assert(pCurSegment->m_pPointS->X()==pCurSegment->m_pPointE->X());
//				assert(pCurSegment->m_pPointS->Z()==pCurSegment->m_pPointE->Z());
//				int iZ		=	pCurSegment->m_pPointS->Z();
//				int	iX		=	pCurSegment->m_pPointS->X();
//				int	iMinY	=	pCurSegment->m_pPointS->Y();
//				int	iMaxY	=	pCurSegment->m_pPointE->Y();
//				
//				assert(iMinY<=iMaxY);
//				
//				for (int y=iMinY;y<iMaxY;y++)
//				{
//					CGrid*	pGrid	=	pDesign->GetLayer(iZ)->GetGrid(iX,y);
//					CGrid*	pNGrid	=	pDesign->GetLayer(iZ)->GetGrid(iX,y+1);
//
//					iCurCost		+=	MazeCost(pDesign,pGrid,pNGrid,/* pNet,*/NULL);
//				}
//			}
//			else
//			{
//				assert(FALSE);
//			}			
//		}
//
//		if(iCurCost<iMinCost)
//		{
//			iMinSegmentList	=	j;
//			iMinCost		=	iCurCost;
//
//			if(iMinCost==iOptimalCost)
//			{
//				pWire->m_pRoutedSegmentList	=	pWire->GetSegmentList(iMinSegmentList);
//				return	iMinCost;
//			}
//		}
//	}
//
//	pWire->Enumerate(ENUM_MODE_Z);
//	iNumSegmentList	=	pWire->GetNumSegmentList();
//	for(;j<iNumSegmentList;++j)
//	{		
//		vector<CSegment*>*	pSegmentList	=	pWire->GetSegmentList(j);
//		
//		int	iCurCost	=	0;
//		for (k=0;k<pSegmentList->size();++k)
//		{
//			CSegment*	pCurSegment	=	(*pSegmentList)[k];
//			CBoundary*	pBoundary	=	NULL;
//			CGrid*		pGrid		=	NULL;
//			
//			if(pCurSegment->IsHorizontal())
//			{
//				assert(pCurSegment->m_pPointS->Y()==pCurSegment->m_pPointE->Y());
//				assert(pCurSegment->m_pPointS->Z()==pCurSegment->m_pPointE->Z());
//				int iZ		=	pCurSegment->m_pPointS->Z();
//				int	iY		=	pCurSegment->m_pPointS->Y();
//				int	iMinX	=	pCurSegment->m_pPointS->X();
//				int	iMaxX	=	pCurSegment->m_pPointE->X();
//				
//				assert(iMinX<=iMaxX);
//				
//				for (int x=iMinX;x<iMaxX;x++)
//				{
//					CGrid*	pGrid	=	pDesign->GetLayer(iZ)->GetGrid(x,iY);
//					CGrid*	pNGrid	=	pDesign->GetLayer(iZ)->GetGrid(x+1,iY);
//
//					iCurCost		+=	MazeCost(pDesign,pGrid,pNGrid, /*pNet,*/ NULL);
//				}
//			}
//			else if(pCurSegment->IsVertical())
//			{
//				assert(pCurSegment->m_pPointS->X()==pCurSegment->m_pPointE->X());
//				assert(pCurSegment->m_pPointS->Z()==pCurSegment->m_pPointE->Z());
//				int iZ		=	pCurSegment->m_pPointS->Z();
//				int	iX		=	pCurSegment->m_pPointS->X();
//				int	iMinY	=	pCurSegment->m_pPointS->Y();
//				int	iMaxY	=	pCurSegment->m_pPointE->Y();
//				
//				assert(iMinY<=iMaxY);
//				
//				for (int y=iMinY;y<iMaxY;y++)
//				{
//					CGrid*	pGrid	=	pDesign->GetLayer(iZ)->GetGrid(iX,y);
//					CGrid*	pNGrid	=	pDesign->GetLayer(iZ)->GetGrid(iX,y+1);
//
//					iCurCost		+=	MazeCost(pDesign,pGrid,pNGrid, /*pNet,*/ NULL);
//				}
//			}
//			else
//			{
//				assert(FALSE);
//			}						
//		}
//
//		if(iCurCost<iMinCost)
//		{
//			iMinSegmentList	=	j;
//			iMinCost		=	iCurCost;
//
//			if(iMinCost==iOptimalCost)
//			{
//				pWire->m_pRoutedSegmentList	=	pWire->GetSegmentList(iMinSegmentList);
//				return	iMinCost;
//			}
//		}
//	}
//
//	pWire->m_pRoutedSegmentList	=	pWire->GetSegmentList(iMinSegmentList);
//	return	iMinCost;
//}
//

int CGRouter::ReRoutingForOF(CDesign *pDesign, vector<CWire*>* pWireList, int iBound)
{
	FP_MAZECOST*	pMazeCost;

	if(iBound)	pMazeCost	=	MazeCost_PenaltyBound;
	else		pMazeCost	=	MazeCost_Penalty;

	int	iNumWireRouted	=	0;

	int	i,s;

	sort(pWireList->begin(),pWireList->end(),CWireOpLen());
	//for(i=0,s=pWireList->size()<100? pWireList->size():(pWireList->size()*0.9);i<s;++i)
	for(i=0,s=pWireList->size();i<s;++i)
	{
		if(i%MAX(1,(s/10))==1)	CDesign::Display(DISPLAY_MODE_NONE,"-");

		CWire*	pWire	=	(*pWireList)[i];
		if(pWire==NULL)	continue;
		CNet*	pNet	=	pWire->GetParent();
		if(pNet==NULL)	continue;

#ifdef _DEBUG
		if(pNet->GetKey()==pDesign->m_iDebugNet&&pDesign->m_iLoop>=0)
		{
			pDesign->PrintFile("mer.m",PRINT_MODE_MATLAB);
			pNet->PrintFile(">mer.m",PRINT_MODE_MATLAB);
		}
#endif
		pWire	=	pNet->MergeWire(pWire);

		if(pWire)
		{
			if(pWire->IsRouted())
			{
				pNet->DelWire(pWire);
				pWire->SetState(STATE_WIRE_UNROUTED);
				pWire->SetState(STATE_WIRE_NOTASSGNED);
				pNet->AddWire(pWire);
#ifdef _DEBUG
				if(pNet->GetKey()==pDesign->m_iDebugNet&&pDesign->m_iLoop>=139)
				{
					pDesign->PrintFile("mer2.m",PRINT_MODE_MATLAB);
					pNet->PrintFile(">mer2.m",PRINT_MODE_MATLAB);
				}
#endif	
			}
			//////////////////////////////////////////////////////////////////////////
			// filtering the already routed wires [7/7/2006 thyeros]
			//////////////////////////////////////////////////////////////////////////
			if(!FilterRouting(pDesign,pWire))
			{						
				pWire->ClearSegmentList();

				MazeRouting(pMazeCost,pDesign,pWire,NULL,iBound);	

				assert(pWire->GetNumSegmentList()==1);
				pWire->m_pRoutedSegmentList	=	pWire->GetSegmentList(0);
				assert(pWire->m_pRoutedSegmentList);	

				pWire->Pupate();
				pWire->Delete();//SAFE_DEL(pWire);
			}				

			pNet->Refine();
		}

#ifdef _DEBUG
		if(pNet->GetKey()==pDesign->m_iDebugNet&&pDesign->m_iLoop>=139)
		{
			pDesign->PrintFile("debug2.m",PRINT_MODE_MATLAB);
			pNet->PrintFile(">debug2.m",PRINT_MODE_MATLAB);
		}	
#endif
		++iNumWireRouted;

	}

	return	iNumWireRouted;
}

class CWireOpLayering {
public:
	bool operator () (const CWire* pL, const CWire* pR) const
	{ 
		CWire*	pWireL	=	(CWire*)pL;
		CWire*	pWireR	=	(CWire*)pR;

		CNet*	pNetL	=	pWireL->GetParent();
		CNet*	pNetR	=	pWireR->GetParent();

		if(pNetL->IsFlat()&&!pNetR->IsFlat())		return	true;
		else if(!pNetL->IsFlat()&&pNetR->IsFlat())	return	false;

		int		iNumWL	=	pNetL->GetNumWire();
		int		iNumWR	=	pNetL->GetNumWire();

		if(iNumWL!=iNumWR)	return	iNumWL<iNumWR;

		int		iNumPinL	=	pWireL->GetNumPinOn();
		int		iNumPinR	=	pWireR->GetNumPinOn();

		if(iNumPinL!=iNumPinR)	return	iNumPinL>iNumPinR;

		return	pNetL->GetLength(GET_MODE_STATE,STATE_WIRE_ANY)<pNetL->GetLength(GET_MODE_STATE,STATE_WIRE_ANY);
	}
};

//#define _GREEDY_

#ifdef _GREEDY_
int	CGRouter::Layering(CDesign* pDesign, vector<CWire*>* pWireList, int iPos, int iDirection)
{
	vector<CLayer*>	Layers;

	int	iTotalBnd	=	iDirection==DIR_HORIZONTAL?	pDesign->W()-1:pDesign->H()-1;
	int	iTotalCap	=	0;
	int	i,j,k,s;
	for(i=LAYER_METAL1;i<=pDesign->T();++i)	
	{
		CLayer*	pLayer	=	pDesign->GetLayer(i);
		if(pLayer->GetDirection()==iDirection)
		{
			Layers.push_back(pLayer);
			iTotalCap	+=	pLayer->GetCapacity(GET_MODE_MAX)/WIRE_WIDTH_SPACE;
		}
	}

	// setup memory for each boundary [3/5/2007 thyeros]
	CWire***	pppLayer		=	new	CWire**[iTotalCap];
	for(i=0;i<iTotalCap;++i)
	{
		pppLayer[i]	=	new	CWire*[iTotalBnd];
		memset(pppLayer[i],NULL,iTotalBnd*sizeof(CWire*));
	}

	int*		pMapping		=	new int[iTotalCap];
	int	iIndex	=	0;
	for(i=0,s=Layers.size();i<s;i++)
	{
		CLayer*	pLayer	=	Layers[i];
		int	iCap	=	pLayer->GetCapacity(GET_MODE_MAX)/WIRE_WIDTH_SPACE;
		for(j=0;j<iCap;j++)
		{
			pMapping[iIndex++]	=	pLayer->GetKey();
		}
	}


	// mark blockages [3/5/2007 thyeros]
	iIndex	=	0;
	for(i=0,s=Layers.size();i<s;i++)
	{
		CLayer*	pLayer	=	Layers[i];
		int	iCap	=	pLayer->GetCapacity(GET_MODE_MAX)/WIRE_WIDTH_SPACE;

		CBoundary*	pBoundary;
		switch(iDirection){
		case DIR_HORIZONTAL:
			for(j=0;j<iTotalBnd;++j)
			{
				pBoundary	=	pLayer->GetBoundary(j,iPos);
				assert(pBoundary);

				int	iBlockage	=	pBoundary->GetCapacity(GET_MODE_BCAP)/WIRE_WIDTH_SPACE;

				for(k=0;k<iBlockage;++k)
					pppLayer[k+iIndex][j]	=	(CWire*)0xF;

			}
			break;
		case DIR_VERTICAL:
			for(j=0;j<iTotalBnd;++j)
			{
				pBoundary	=	pLayer->GetBoundary(iPos,j);
				assert(pBoundary);

				int	iBlockage	=	pBoundary->GetCapacity(GET_MODE_BCAP)/WIRE_WIDTH_SPACE;

				for(k=0;k<iBlockage;++k)
					pppLayer[k+iIndex][j]	=	(CWire*)0xF;
			}
			break;
		default:
			assert(FALSE);
			break;
		}

		iIndex	+=	iCap;
	}

	sort(pWireList->begin(),pWireList->end(),CWireOpLayering());

	// reassign the layer [3/5/2007 thyeros]
	while(!pWireList->empty())
	{
		CWire*	pWire	=	pWireList->front();pWireList->erase(pWireList->begin());
		
		assert(pWire->GetDirection()==iDirection);

		CNet*	pNet	=	pWire->GetParent();
		assert(pNet);

		if(pWire->IsHorizontal())
		{
			assert(pWire->m_pPointS->Y()==pWire->m_pPointE->Y());
			int	iY		=	pWire->m_pPointS->Y();
			int iMinX	=	pWire->m_pPointS->X();
			int	iMaxX	=	pWire->m_pPointE->X();

			// the last point doesn't require to update boundary [6/18/2006 thyeros]
			assert(iMinX<iMaxX);

			int	iBestPos	=	0;
			int	iAssigned	=	FALSE;
			for(k=0;k<iTotalCap;++k)
			{
				int	iRoutable	=	TRUE;
				for (j=iMinX;j<iMaxX;++j)
				{
					if(pppLayer[k][j])	
					{	
						iBestPos	=	MAX(iBestPos,j);
						iRoutable	=	FALSE;
						break;
					}
				}

				if(iRoutable)
				{
					for (j=iMinX;j<iMaxX;++j)
					{
						pppLayer[k][j]	=	pWire;
					}

					pWire->AssignLayer(pMapping[k]);	
					iAssigned	=	TRUE;
					break;
				}
			}

			if(!iAssigned)
			{
				if(iBestPos<=pWire->m_pPointS->X())
				{
					if(pWire->GetLength()>1)	iBestPos	=	pWire->m_pPointS->X()+1;				
				}

				if(iBestPos>pWire->m_pPointS->X())
				{
					CWire*	pWireS	=	NULL;
					CWire*	pWireE	=	NULL;

					CPoint	Point;
					Point.Initialize(iBestPos,pWire->m_pPointS->Y(),pWire->m_pPointS->Z(),NULL);

					pWire->SplitWire(&Point,&pWireS,&pWireE);
					pWire->Delete();

					assert(pWireS&&pWireE);

					pWireList->insert(pWireList->begin(),pWireE);
					pWireList->insert(pWireList->begin(),pWireS);
				}
			}
		}
		else if(pWire->IsVertical())
		{
			assert(pWire->m_pPointS->X()==pWire->m_pPointE->X());
			int	iX		=	pWire->m_pPointS->X();
			int iMinY	=	pWire->m_pPointS->Y();
			int	iMaxY	=	pWire->m_pPointE->Y();

			assert(iMinY<iMaxY);

			int	iBestPos	=	0;
			int	iAssigned	=	FALSE;
			for(k=0;k<iTotalCap;++k)
			{
				int	iRoutable	=	TRUE;
				for (j=iMinY;j<iMaxY;++j)
				{
					if(pppLayer[k][j])	
					{	
						iBestPos	=	MAX(iBestPos,j);
						iRoutable	=	FALSE;
						break;
					}
				}

				if(iRoutable)
				{
					for (j=iMinY;j<iMaxY;++j)
					{
						pppLayer[k][j]	=	pWire;
					}

					pWire->AssignLayer(pMapping[k]);
					iAssigned	=	TRUE;
					break;
				}
			}

			if(!iAssigned)
			{
				if(iBestPos<=pWire->m_pPointS->Y())
				{
					if(pWire->GetLength()>1)	iBestPos	=	pWire->m_pPointS->Y()+1;
				}

				if(iBestPos>pWire->m_pPointS->Y())
				{
					CWire*	pWireS	=	NULL;
					CWire*	pWireE	=	NULL;

					CPoint	Point;
					Point.Initialize(pWire->m_pPointS->X(),iBestPos,pWire->m_pPointS->Z(),NULL);

					pWire->SplitWire(&Point,&pWireS,&pWireE);
					pWire->Delete();

					assert(pWireS&&pWireE);

					pWireList->insert(pWireList->begin(),pWireE);
					pWireList->insert(pWireList->begin(),pWireS);
				}
			}
		}
	}

	// release memory [3/5/2007 thyeros]
	for(i=0;i<iTotalCap;++i)	SAFE_DELA(pppLayer[i]);
	SAFE_DELA(pppLayer);

	return	0;
}

#else

int	CGRouter::Layering(CDesign* pDesign, vector<CWire*>* pWireList, int iPos, int iDirection)
{
	vector<CLayer*>	Layers;

	int	iTotalBnd	=	iDirection==DIR_HORIZONTAL?	pDesign->W()-1:pDesign->H()-1;
	int	iTotalCap	=	0;
	int	i,j,k,s;
	for(i=LAYER_METAL1;i<=pDesign->T();++i)	
	{
		CLayer*	pLayer	=	pDesign->GetLayer(i);
		if(pLayer->GetDirection()==iDirection)
		{
			Layers.push_back(pLayer);
			iTotalCap	+=	pLayer->GetCapacity(GET_MODE_MAX)/WIRE_WIDTH_SPACE;
		}
	}

	// setup memory for each boundary [3/5/2007 thyeros]
	CWire***	pppLayer		=	new	CWire**[iTotalCap];
	for(i=0;i<iTotalCap;++i)
	{
		pppLayer[i]	=	new	CWire*[iTotalBnd];
		memset(pppLayer[i],NULL,iTotalBnd*sizeof(CWire*));
	}

	int*		pMapping		=	new int[iTotalCap];
	int	iIndex	=	0;
	for(i=0,s=Layers.size();i<s;i++)
	{
		CLayer*	pLayer	=	Layers[i];
		int	iCap	=	pLayer->GetCapacity(GET_MODE_MAX)/WIRE_WIDTH_SPACE;
		for(j=0;j<iCap;j++)
		{
			pMapping[iIndex++]	=	pLayer->GetKey();
		}
	}


	// mark blockages [3/5/2007 thyeros]
	iIndex	=	0;
	for(i=0,s=Layers.size();i<s;i++)
	{
		CLayer*	pLayer	=	Layers[i];
		int	iCap	=	pLayer->GetCapacity(GET_MODE_MAX)/WIRE_WIDTH_SPACE;

		CBoundary*	pBoundary;
		switch(iDirection){
		case DIR_HORIZONTAL:
			for(j=0;j<iTotalBnd;++j)
			{
				pBoundary	=	pLayer->GetBoundary(j,iPos);
				assert(pBoundary);

				int	iBlockage	=	MIN(pLayer->GetCapacity(GET_MODE_MAX),pBoundary->GetCapacity(GET_MODE_OCAP))/WIRE_WIDTH_SPACE;

				for(k=0;k<iBlockage;++k)
					pppLayer[k+iIndex][j]	=	(CWire*)0xF;

			}
			break;
		case DIR_VERTICAL:
			for(j=0;j<iTotalBnd;++j)
			{
				pBoundary	=	pLayer->GetBoundary(iPos,j);
				assert(pBoundary);

				int	iBlockage	=	MIN(pLayer->GetCapacity(GET_MODE_MAX),pBoundary->GetCapacity(GET_MODE_OCAP))/WIRE_WIDTH_SPACE;

				for(k=0;k<iBlockage;++k)
					pppLayer[k+iIndex][j]	=	(CWire*)0xF;
			}
			break;
		default:
			assert(FALSE);
			break;
		}

		iIndex	+=	iCap;
	}

	sort(pWireList->begin(),pWireList->end(),CWireOpLayering());

	// reassign the layer [3/5/2007 thyeros]
	while(!pWireList->empty())
	{
		CWire*	pWire	=	pWireList->front();pWireList->erase(pWireList->begin());

		if(pWire->IsCompleted())	continue;

		assert(pWire->GetDirection()==iDirection);

		CNet*	pNet	=	pWire->GetParent();
		assert(pNet);

		if(pWire->IsHorizontal())
		{
			assert(pWire->m_pPointS->Y()==pWire->m_pPointE->Y());
			int	iY		=	pWire->m_pPointS->Y();
			int iMinX	=	pWire->m_pPointS->X();
			int	iMaxX	=	pWire->m_pPointE->X();

			// the last point doesn't require to update boundary [6/18/2006 thyeros]
			assert(iMinX<iMaxX);

			int	iBestPos	=	0;
			int	iAssigned	=	FALSE;
			for(k=0;k<iTotalCap;++k)
			{
				int	iRoutable	=	TRUE;
				for (j=iMinX;j<iMaxX;++j)
				{
					if(pppLayer[k][j])	
					{	
						iBestPos	=	MAX(iBestPos,j);
						iRoutable	=	FALSE;
						break;
					}
				}

				if(iRoutable)
				{
					for (j=iMinX;j<iMaxX;++j)
					{
						pppLayer[k][j]	=	pWire;
					}

					pWire->MakeRouted(pMapping[k]);	
					iAssigned	=	TRUE;
					break;
				}
			}

			if(!iAssigned)
			{
				if(iBestPos<=pWire->m_pPointS->X())
				{
					if(pWire->GetLength()>1)	iBestPos	=	pWire->m_pPointS->X()+1;				
					else				
					{
						assert(pWire->GetLength()==1);
						int	iMinOF		=	MAX_PENALTY;
						int	iMinLayer	=	LAYER_DEVICE;
						for(int i=LAYER_METAL1;i<=pDesign->T();++i)	
						{
							CLayer*	pLayer	=	pDesign->GetLayer(i);

							if(pLayer->GetDirection()==iDirection)
							{
								int	iCurOF		=	pLayer->GetBoundary(pWire->m_pPointS->X(),pWire->m_pPointS->Y())->GetOverFlow();

								if(iCurOF<iMinOF)
								{
									iMinOF		=	iCurOF;
									iMinLayer	=	i;
								//	break;
								}
							}
						}

						assert(iMinLayer!=LAYER_DEVICE);

						pWire->MakeRouted(iMinLayer);
					}
				}

				if(iBestPos>pWire->m_pPointS->X())
				{
					CWire*	pWireS	=	NULL;
					CWire*	pWireE	=	NULL;

					CPoint	Point;
					Point.Initialize(iBestPos,pWire->m_pPointS->Y(),pWire->m_pPointS->Z(),NULL);

					pWire->SplitWire(&Point,&pWireS,&pWireE);
					pWire->Delete();

					assert(pWireS&&pWireE);

					pWireList->insert(pWireList->begin(),pWireE);
					pWireList->insert(pWireList->begin(),pWireS);
				}
			}
		}
		else if(pWire->IsVertical())
		{
			assert(pWire->m_pPointS->X()==pWire->m_pPointE->X());
			int	iX		=	pWire->m_pPointS->X();
			int iMinY	=	pWire->m_pPointS->Y();
			int	iMaxY	=	pWire->m_pPointE->Y();

			assert(iMinY<iMaxY);

			int	iBestPos	=	0;
			int	iAssigned	=	FALSE;
			for(k=0;k<iTotalCap;++k)
			{
				int	iRoutable	=	TRUE;
				for (j=iMinY;j<iMaxY;++j)
				{
					if(pppLayer[k][j])	
					{	
						iBestPos	=	MAX(iBestPos,j);
						iRoutable	=	FALSE;
						break;
					}
				}

				if(iRoutable)
				{
					for (j=iMinY;j<iMaxY;++j)
					{
						pppLayer[k][j]	=	pWire;
					}

					pWire->MakeRouted(pMapping[k]);
					iAssigned	=	TRUE;
					break;
				}
			}

			if(!iAssigned)
			{
				if(iBestPos<=pWire->m_pPointS->Y())
				{
					if(pWire->GetLength()>1)	iBestPos	=	pWire->m_pPointS->Y()+1;
					else
					{
						assert(pWire->GetLength()==1);
						int	iMinOF		=	MAX_PENALTY;
						int	iMinLayer	=	LAYER_DEVICE;
						for(int i=LAYER_METAL1;i<=pDesign->T();++i)	
						{
							CLayer*	pLayer	=	pDesign->GetLayer(i);

							if(pLayer->GetDirection()==iDirection)
							{
								int	iCurOF		=	pLayer->GetBoundary(pWire->m_pPointS->X(),pWire->m_pPointS->Y())->GetOverFlow();

								if(iCurOF<iMinOF)
								{
									iMinOF		=	iCurOF;
									iMinLayer	=	i;
								//	break;
								}
							}
						}

						assert(iMinLayer!=LAYER_DEVICE);

						pWire->MakeRouted(iMinLayer);
					}
				}

				if(iBestPos>pWire->m_pPointS->Y())
				{
					CWire*	pWireS	=	NULL;
					CWire*	pWireE	=	NULL;

					CPoint	Point;
					Point.Initialize(pWire->m_pPointS->X(),iBestPos,pWire->m_pPointS->Z(),NULL);

					pWire->SplitWire(&Point,&pWireS,&pWireE);
					pWire->Delete();

					assert(pWireS&&pWireE);

					pWireList->insert(pWireList->begin(),pWireE);
					pWireList->insert(pWireList->begin(),pWireS);
				}
			}
		}
	}

	// release memory [3/5/2007 thyeros]
	for(i=0;i<iTotalCap;++i)	SAFE_DELA(pppLayer[i]);
	SAFE_DELA(pppLayer);

	return	0;
}
#endif

void CGRouter::Layering(CDesign* pDesign,vector<CNet*>* pNetList)
{
	char	cLine[MAX_BUFFER_STR];

#ifdef WIN32
	sprintf(cLine,"layer.mps");
#else
	sprintf(cLine,"/tmp/mosek_%d.mps",getpid());
#endif
	FILE*	pFile	=	fopen(cLine,"wt");

	//#define _VIEW_NETID_

	fprintf(pFile,"NAME %s_%d\n","layer",getpid());
	fprintf(pFile,"ROWS\n");
	fprintf(pFile," N O\n");

	hash_map<ADDRESS,int>	WireMap;
	vector<CWire*>			WireList;
	set<CBoundary*>			Boundary;

	for(int i=0,s=pNetList->size();i<s;i++)
	{
		CNet*	pNet	=	pNetList->at(i);

		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;

			WireList.push_back(pWire);
		}
	}

	for(int i=0,s=WireList.size();i<s;i++)
	{
		WireMap[(ADDRESS)WireList[i]]	=	i;
	}

	int	iNumWire	=	0;
	for(int i=0,s=pNetList->size();i<s;i++)
	{
		CNet*	pNet	=	pNetList->at(i);

		multimap<KEY, CWire*>	ViaBucket;
		ViaBucket.clear();

		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;

			assert(pWire->IsFlat());

			if(pWire->IsCompleted())	continue;
			iNumWire++;

			pWire->Enumerate(ENUM_MODE_L);
			int	iNumSegmentList	=	pWire->GetNumSegmentList();
			assert(iNumSegmentList==pDesign->T()/2);

			int j,k,x,y;
			for(j=0;j<iNumSegmentList;++j)
			{		
				vector<CSegment*>*	pSegmentList	=	pWire->GetSegmentList(j);
				for (k=0;k<pSegmentList->size();k++)
				{
					CSegment*	pCurSegment	=	(*pSegmentList)[k];
					CBoundary*	pBoundary	=	NULL;
					CGrid*		pGrid		=	NULL;

					if(pCurSegment->IsHorizontal())
					{
						assert(pCurSegment->m_pPointS->Y()==pCurSegment->m_pPointE->Y());
						assert(pCurSegment->m_pPointS->Z()==pCurSegment->m_pPointE->Z());
						int iZ		=	pCurSegment->m_pPointS->Z();
						int	iY		=	pCurSegment->m_pPointS->Y();
						int	iMinX	=	pCurSegment->m_pPointS->X();
						int	iMaxX	=	pCurSegment->m_pPointE->X();

						assert(iMinX<=iMaxX);

						for (x=iMinX;x<iMaxX;x++)
						{
							pBoundary	=	pDesign->GetLayer(iZ)->GetBoundary(x,iY);					
							assert(pBoundary);

							pBoundary->AddSegmentList(pSegmentList);
							Boundary.insert(pBoundary);
						}
					}
					else if(pCurSegment->IsVertical())
					{
						assert(pCurSegment->m_pPointS->X()==pCurSegment->m_pPointE->X());
						assert(pCurSegment->m_pPointS->Z()==pCurSegment->m_pPointE->Z());
						int iZ		=	pCurSegment->m_pPointS->Z();
						int	iX		=	pCurSegment->m_pPointS->X();
						int	iMinY	=	pCurSegment->m_pPointS->Y();
						int	iMaxY	=	pCurSegment->m_pPointE->Y();

						assert(iMinY<=iMaxY);

						for (y=iMinY;y<iMaxY;y++)
						{
							pBoundary	=	pDesign->GetLayer(iZ)->GetBoundary(iX,y);						
							assert(pBoundary);

							pBoundary->AddSegmentList(pSegmentList);
							Boundary.insert(pBoundary);
						}
					}
					else
					{
						assert(FALSE);
					}
				}// for each segment [6/22/2006 thyeros]
			}
		}

		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			int	iWireIndex	=	WireMap[(ADDRESS)pWire];

			if(!pWire->IsCompleted())
			{
				// to calculate the layer number [3/30/2007 thyeros]
				fprintf(pFile," E L%d\n",iWireIndex);

				// one wire can be assigned to only one layer [3/30/2007 thyeros]
				fprintf(pFile," L C%d\n",iWireIndex);
			}

			// smaller than max layer [4/1/2007 thyeros]
			fprintf(pFile," G A%ds\n",iWireIndex);
			fprintf(pFile," G A%de\n",iWireIndex);

			// greater than min layer [4/1/2007 thyeros]	
			if(!pNet->GetPin(pWire->m_pPointS))	fprintf(pFile," L I%ds\n",iWireIndex);
			if(!pNet->GetPin(pWire->m_pPointE))	fprintf(pFile," L I%de\n",iWireIndex);
		}

#ifdef _DEBUG
		fflush(pFile);
#endif
	}

	// boundary constraint [3/30/2007 thyeros]
	for(set<CBoundary*>::iterator itr=Boundary.begin(),end=Boundary.end();itr!=end;++itr)
	{
		CBoundary*	pBoundary	=	*itr;//->second;
		int	iNumSegmentList		=	pBoundary->GetNumSegmentList();

		if(iNumSegmentList<=pBoundary->GetCapacity()/WIRE_WIDTH_SPACE)	continue;

		// one wire can be assigned to only one layer [3/30/2007 thyeros]
		fprintf(pFile," L B%d,%d,%d\n",pBoundary->X(),pBoundary->Y(),pBoundary->Z());
	}

	fprintf(pFile,"COLUMNS\n");

	for(int i=0,s=pNetList->size();i<s;i++)
	{
		CNet*	pNet	=	pNetList->at(i);
		int		iNetID	=	(int)pNet->GetKey();

		multimap<KEY, CWire*>	ViaBucket;

		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;

			ViaBucket.insert(pair<const KEY, CWire*>(pWire->m_pPointS->GetKey()&0xFFFFFF00, pWire));
			ViaBucket.insert(pair<const KEY, CWire*>(pWire->m_pPointE->GetKey()&0xFFFFFF00, pWire));

			// to calculate the layer number [3/30/2007 thyeros]
			int	iWireIndex	=	WireMap[(ADDRESS)pWire];

			if(!pWire->IsCompleted())
			{
				fprintf(pFile," W%d L%d -1\n",iWireIndex,iWireIndex,-1);
				for(int i=LAYER_METAL1;i<=pDesign->T();++i)
				{
					if(pWire->GetDirection()==pDesign->GetLayer(i)->GetDirection())
					{
						// to calculate the layer number [3/30/2007 thyeros]
						// one wire can be assigned to only one layer [3/30/2007 thyeros]
#ifdef _VIEW_NETID_
						fprintf(pFile," Z%d.%d-%d L%d %d C%d 1\n",iWireIndex,i,iNetID,iWireIndex,i,iWireIndex);
#else
						fprintf(pFile," Z%d.%d L%d %d C%d 1\n",iWireIndex,i,iWireIndex,i,iWireIndex);
#endif

#ifdef _VIEW_NETID_
						fprintf(pFile," Z%d.%d-%d O 1\n",iWireIndex,i,iNetID));
#else
						fprintf(pFile," Z%d.%d O 1\n",iWireIndex,i);
#endif
					}
				}
			}

			fprintf(pFile," W%d A%ds -1 A%de -1\n",iWireIndex,iWireIndex,iWireIndex);	

			if(pNet->GetPin(pWire->m_pPointS))
			{
				if(pNet->GetPin(pWire->m_pPointE))	;
				else								fprintf(pFile," W%d I%de -1\n",iWireIndex,iWireIndex);
			}
			else
			{
				if(pNet->GetPin(pWire->m_pPointE))	fprintf(pFile," W%d I%ds -1\n",iWireIndex,iWireIndex);
				else								fprintf(pFile," W%d I%ds -1 I%de -1\n",iWireIndex,iWireIndex,iWireIndex);
			}
		}

		for(int i=0;i<pNet->GetNumPin();++i) 
		{
			CWire*	pPinWire	=	CWire::New();
			CPin*	pPin		=	pNet->GetPin(i);

			pPinWire->Initialize(pPin->X(),pPin->Y(),pPin->X(),pPin->Y(),pPin->Z());

			ViaBucket.insert(pair<const KEY, CWire*>(pPin->GetKey()&0xFFFFFF00,pPinWire));
		}

		multimap<KEY, CWire*>::iterator itrl;
		multimap<KEY, CWire*>::iterator itru;
		multimap<KEY, CWire*>::iterator itrv;

		int	iIndex	=	0;
		for(itrv=ViaBucket.begin();itrv!=ViaBucket.end();itrv=itrl)
		{
			// get number of via [3/30/2007 thyeros]
			// max layer [4/1/2007 thyeros]
			fprintf(pFile," A%d.%d O %.0g\n",iNetID,iIndex,-1.0/iNumWire);

			int	iPinFound	=	FALSE;

			CPoint	ViaPoint;
			ViaPoint.Initialize(CPoint::X(itrv->first),CPoint::Y(itrv->first),LAYER_METAL1,NULL);

			itrl	=	ViaBucket.lower_bound(itrv->first);
			itru	=	ViaBucket.upper_bound(itrv->first);
			for (;itrl!=itru;++itrl)
			{
				CWire*	pWire	=	itrl->second;
				int		iWireIndex	=	WireMap[(ADDRESS)pWire];

				if(pWire->IsPoint())
				{
					iPinFound	=	TRUE;
				}
				else
				{
					if(ViaPoint.IsSame2D(pWire->m_pPointS))	fprintf(pFile," A%d.%d A%ds 1\n",iNetID,iIndex,iWireIndex);
					else									fprintf(pFile," A%d.%d A%de 1\n",iNetID,iIndex,iWireIndex);
				}
			}

			if(!iPinFound)
			{
				fprintf(pFile," I%d.%d O %.0g\n",iNetID,iIndex,1.0/iNumWire);

				itrl	=	ViaBucket.lower_bound(itrv->first);
				itru	=	ViaBucket.upper_bound(itrv->first);
				for (;itrl!=itru;++itrl)
				{
					CWire*	pWire	=	itrl->second;
					int		iWireIndex	=	WireMap[(ADDRESS)pWire];

					if(pWire->IsPoint())
					{
						pWire->Delete();
					}
					else
					{
						if(ViaPoint.IsSame2D(pWire->m_pPointS))	 fprintf(pFile," I%d.%d I%ds 1\n",iNetID,iIndex,iWireIndex);
						else									 fprintf(pFile," I%d.%d I%de 1\n",iNetID,iIndex,iWireIndex);
					}
				}	
			}

			iIndex++;
		}		
	}

	// boundary constraint [3/30/2007 thyeros]
	for(set<CBoundary*>::iterator itr=Boundary.begin(),end=Boundary.end();itr!=end;++itr)
	{
		CBoundary*	pBoundary	=	*itr;//->second;
		int	iNumSegmentList		=	pBoundary->GetNumSegmentList();

		if(iNumSegmentList<=pBoundary->GetCapacity()/WIRE_WIDTH_SPACE)	continue;
		for (int i=0;i<iNumSegmentList;++i)
		{
			vector<CSegment*>*	pSegmentList	=	pBoundary->GetSegmentList(i);

			assert(pSegmentList->size()==1);
			CWire*	pWire	=	pSegmentList->at(0)->GetParent();
#ifdef _VIEW_NETID_
			fprintf(pFile," Z%d.%d-%d B%d,%d,%d 1\n",WireMap[(ADDRESS)pWire],pSegmentList->at(0)->m_pPointS->Z(),(int)pWire->GetParent()->GetKey(),pBoundary->X(),pBoundary->Y(),pBoundary->Z());
#else
			fprintf(pFile," Z%d.%d B%d,%d,%d 1\n",WireMap[(ADDRESS)pWire],pSegmentList->at(0)->m_pPointS->Z(),pBoundary->X(),pBoundary->Y(),pBoundary->Z());
#endif
		}
	}

	fprintf(pFile,"RHS\n");
	for(int i=0,s=pNetList->size();i<s;i++)
	{
		CNet*	pNet	=	pNetList->at(i);

		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;

			if(pWire->IsCompleted())	continue;

			// one wire must be assigned to only one layer [3/30/2007 thyeros]
			fprintf(pFile," R C%d 1\n",WireMap[(ADDRESS)pWire]);

			assert(pWire->IsFlat());
		}
	}

	// boundary constraint [3/30/2007 thyeros]
	for(set<CBoundary*>::iterator itr=Boundary.begin(),end=Boundary.end();itr!=end;++itr)
	{
		CBoundary*	pBoundary	=	*itr;//->second;

		int	iNumSegmentList		=	pBoundary->GetNumSegmentList();
		int	iNumACap			=	pBoundary->GetCapacity()/WIRE_WIDTH_SPACE;

		if(iNumSegmentList<=iNumACap||iNumACap==0);
		else	fprintf(pFile," R B%d,%d,%d %d\n",pBoundary->X(),pBoundary->Y(),pBoundary->Z(),iNumACap);

		pBoundary->ClearSegmentList();	
	}


	fprintf(pFile,"BOUNDS\n");
	for(int i=0,s=pNetList->size();i<s;i++)
	{
		CNet*	pNet	=	pNetList->at(i);
		int		iNetID	=	(int)pNet->GetKey();

		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;++itr)
		{
			CWire*	pWire	=	*itr;
			int		iWireIndex	=	WireMap[(ADDRESS)pWire];

			if(pWire->IsCompleted())
			{
				fprintf(pFile," FX B W%d %d\n",iWireIndex,pWire->GetLayer()->Z());
			}
			//			else
			//			{
			//				for(int i=LAYER_METAL1;i<=pDesign->T();++i)
			//				{
			//					if(pWire->GetDirection()==pDesign->GetLayer(i)->GetDirection())
			//					{
			//						// to calculate the layer number [3/30/2007 thyeros]
			//#ifdef _VIEW_NETID_
			//						fprintf(pFile," UP B Z%d.%d-%d 1\n",iWireIndex,i,iNetID);
			//#else
			//						fprintf(pFile," UP B Z%d.%d 1\n",iWireIndex,i);
			//#endif
			//					}
			//				}
			//
			//			}

			pWire->ClearSegmentList();
		}
	}

	fprintf(pFile,"ENDATA\n");

	SAFE_FCLOSE(pFile);
	// run mosek [10/13/2006 thyeros]

#ifdef WIN32
	sprintf(cLine,"perl fix_mps.pl layer.mps layer_fixed.mps");
#else
	sprintf(cLine,"perl fix_mps.pl /tmp/mosek_%d.mps /tmp/mosek_%d_fixed.mps",getpid(),getpid());
#endif
	system(cLine);

#ifdef WIN32
	sprintf(cLine,"mosek -max %s -d MSK_IPAR_BI_MAX_ITERATIONS 1000 -d MSK_IPAR_INTPNT_NUM_THREADS %d -d MSK_IPAR_INFEAS_REPORT_AUTO 1 -d MSK_IPAR_READ_MPS_FORMAT 2 -d MSK_IPAR_WRITE_SOL_CONSTRAINTS 0 -d MSK_IPAR_WRITE_BAS_CONSTRAINTS 0 -d MSK_IPAR_WRITE_BAS_HEAD 0 -d MSK_IPAR_WRITE_BAS_VARIABLES 0 -d MSK_IPAR_LOG 1 > mosek.log ","layer_fixed.mps",pDesign->m_Param.m_iParallelCPU);	
#else
	sprintf(cLine,"mosek -max /tmp/mosek_%d_fixed.mps -d MSK_IPAR_BI_MAX_ITERATIONS 1000 -d MSK_IPAR_INTPNT_NUM_THREADS %d -d MSK_IPAR_INFEAS_REPORT_AUTO 1 -d MSK_IPAR_READ_MPS_FORMAT 2 -d MSK_IPAR_WRITE_SOL_CONSTRAINTS 0 -d MSK_IPAR_WRITE_BAS_CONSTRAINTS 0 -d MSK_IPAR_WRITE_BAS_HEAD 0 -d MSK_IPAR_WRITE_BAS_VARIABLES 0 -d MSK_IPAR_LOG 1 > /tmp/mosek.log",getpid(),pDesign->m_Param.m_iParallelCPU);	 
#endif
	system(cLine);

#ifdef WIN32
	sprintf(cLine,"layer_fixed.sol");
#else
	sprintf(cLine,"/tmp/mosek_%d_fixed.sol",getpid());
#endif

	pFile=fopen(cLine,"rt");

	if(pFile==NULL)
	{
#ifdef WIN32
		sprintf(cLine,"cp mosek.log mosek_%d.err",getpid());
		system(cLine);
#else
		sprintf(cLine,"cp /tmp/mosek.log /tmp/mosek_%d.err",getpid());
		system(cLine);
#endif
	}
	else
	{
		for(;fgets(cLine,sizeof(cLine),pFile);)
		{
			if(strstr(cLine," PRIMAL_INFEASIBLE"))
			{
#ifdef WIN32
				system("cat mosek.log");
#else
				system("cat /tmp/mosek.log");
#endif
				//				exit(0);
				break;
			}

			char*	pToken	=	strtok(cLine," \t\n");
			pToken	=	strtok(NULL," \t\n");

			if(pToken==NULL)	continue;

			switch(pToken[0]){
		case 'W':
			{
				CWire*	pWire		=	WireList[atoi(pToken+1)];

				strtok(NULL," \t\n");
				double	dLayer		=	atof(strtok(NULL," \t\n"));	
				int		iLayer		=	LAYER_DEVICE;
				double	dError		=	0.5;

				for(int i=LAYER_METAL1;i<=pDesign->T();i++)
				{
					if(pDesign->GetLayer(i)->GetDirection()==pWire->GetDirection())
					{
						double	dCurError	=	fabs(i-dLayer)/i;
						if(dCurError<dError)
						{
							dError	=	dCurError;
							iLayer	=	i;

						}
					}
				}

				if(dError<0.15&&iLayer>LAYER_DEVICE)
				{
					if(pWire->IsRoutable(iLayer))	pWire->MakeRouted(iLayer);
					else
					{
						if(pWire->IsVertical())
						{
							int	iBestPos	=	pWire->m_pPointS->X()+pWire->GetRoutableLength(iLayer);
							if(iBestPos<=pWire->m_pPointS->X()&&pWire->GetLength()>1)	iBestPos	=	pWire->m_pPointS->X()+1;				
							if(iBestPos>pWire->m_pPointS->X())
							{
								CPoint	Point;
								Point.Initialize(iBestPos,pWire->m_pPointS->Y(),pWire->m_pPointS->Z(),NULL);

								if(pWire->SplitWire(&Point))	pWire->Delete();
							}
						}
						else if(pWire->IsHorizontal())
						{
							int	iBestPos	=	pWire->m_pPointS->Y()+pWire->GetRoutableLength(iLayer);
							if(iBestPos<=pWire->m_pPointS->Y()&&pWire->GetLength()>1)	iBestPos	=	pWire->m_pPointS->Y()+1;
							if(iBestPos>pWire->m_pPointS->Y())
							{
								CPoint	Point;
								Point.Initialize(pWire->m_pPointS->X(),iBestPos,pWire->m_pPointS->Z(),NULL);

								if(pWire->SplitWire(&Point))	pWire->Delete();
							}
						}
					}
				}
			}
			break;
		default:
			break;
			}	
		}
	}

	SAFE_FCLOSE(pFile);

#ifdef WIN32

#else
	sprintf(cLine,"rm /tmp/mosek_%d*.sol",getpid());
	system(cLine);
#endif

}


void CGRouter::Layering(CDesign* pDesign)
{
	char	cTime[MAX_BUFFER_STR];


	GetTime(cTime);
	CDesign::Display(DISPLAY_MODE_INFO,"relayering started (%.1f MB) at %s\n",GetMemory()/1024.0,cTime);

#ifdef _GREEDY_

	CDesign::Display(DISPLAY_MODE_NONE," H greedy-layering: ");
	// horizontal direction [3/5/2007 thyeros]
	for(int iY=0,s=pDesign->H();iY<s;++iY)
	{
		if(iY%MAX(1,(s/10))==1)	CDesign::Display(DISPLAY_MODE_NONE,"h");

		vector<CWire*>	WireList;
		pDesign->GetWireInDirection(&WireList, iY, DIR_HORIZONTAL);
		Layering(pDesign,&WireList,iY,DIR_HORIZONTAL);
	}

	CDesign::Display(DISPLAY_MODE_NONE,"\tOF=%d,VIA=%d\n",pDesign->GetOverFlow(GET_MODE_SUM),pDesign->GetNumVia());

	CDesign::Display(DISPLAY_MODE_NONE," V greedy-layering: ");
	// vertical direction [3/5/2007 thyeros]
	for(int iX=0,s=pDesign->W();iX<s;++iX)
	{
		if(iX%MAX(1,(s/10))==1)	CDesign::Display(DISPLAY_MODE_NONE,"v");

		vector<CWire*>	WireList;
		pDesign->GetWireInDirection(&WireList, iX, DIR_VERTICAL);
		Layering(pDesign,&WireList,iX,DIR_VERTICAL);
	}
	CDesign::Display(DISPLAY_MODE_NONE,"\tOF=%d,VIA=%d\n",pDesign->GetOverFlow(GET_MODE_SUM),pDesign->GetNumVia());

#else
	for(int i=0,s=pDesign->GetNumNet();i<s;++i)
	{	
		CNet*	pNet	=	pDesign->GetNet(i);

		for(vector<CWire*>::iterator itr=pNet->m_Wire.begin(),end=pNet->m_Wire.end();itr!=end;)
		{
			CWire*	pWire	=	*itr;

			if(!pWire->IsRouted())
			{
				++itr;
				continue;
			}

			pNet->DelWire(pWire);
			pWire->SetState(STATE_WIRE_UNROUTED);
			pWire->SetState(STATE_WIRE_NOTASSGNED);
			pNet->AddWire(pWire);

			itr=pNet->m_Wire.begin();
		}
	}

	if(pDesign->ReadDump(pDesign->m_Param.m_cRel_File))
	{
		GetTime(cTime);
		CDesign::Display(DISPLAY_MODE_INFO,"LP layering skipped due to %s (%.1f MB) at %s\n",pDesign->m_Param.m_cRel_File,GetMemory()/1024.0,cTime);

		pDesign->ReadDump(pDesign->m_Param.m_cRel_File);
	}
	else
	{

		GetTime(cTime);
		CDesign::Display(DISPLAY_MODE_NONE," LP layering: %d wires ",pDesign->GetNumWire(GET_MODE_STATE,STATE_WIRE_ROUTED));
		CDesign::Display(DISPLAY_MODE_NONE," =>  %d wires unrouted at %s\n",pDesign->GetNumWire(GET_MODE_STATE,STATE_WIRE_UNROUTED),cTime);

		int	iStep				=	pDesign->m_Param.m_iBoxRouting_Step;

		for(int i=0;i<1;++i)
		{
			vector<CNet*>	NetList;

			CBBox	LastBox;
			LastBox.Initialize(0,0,0,0,0,0);
			CBBox	StartBox	=	pDesign->GetStartBox();
			int		iExit		=	FALSE;
			int		iTotalNumWire		=	pDesign->GetNumWire();
			for(;!iExit;pDesign->m_iLoop++)
			{
				//check if the  expanded box cover the target box (typically the whole circuit)
				iExit	=	StartBox.IsInside(pDesign);

				NetList.clear();

				int iNumNetWithinBBox	=	pDesign->GetNetInBBox(&NetList,&StartBox);

				if(iNumNetWithinBBox>iStep || (iExit&&iNumNetWithinBBox))
				{
					for(int j=0,s=NetList.size();j<s;++j)
					{	
						CNet*	pNet	=	NetList[j];
						if(pNet->IsRouted())	continue;
					}

					int	iCurNumRoutedWire	=	pDesign->GetNumWire(GET_MODE_STATE,STATE_WIRE_ROUTED);
					CDesign::Display(DISPLAY_MODE_NONE," %.1f%%) %8d (%2.1f%%) wires are routed and routing\t%d nets",
						StartBox.A()*100.0/pDesign->A()-0.01,iCurNumRoutedWire,iCurNumRoutedWire*100.0/iTotalNumWire,NetList.size());
					Layering(pDesign,&NetList);

					//					CDesign::Display(DISPLAY_MODE_NONE,"\n H layering: ");
					// horizontal direction [3/5/2007 thyeros]
					for(int iY=LastBox.Y(),s=LastBox.Y()+LastBox.H();iY<s;++iY)
					{
						//						if(iY%MAX(1,(s/10))==1)	CDesign::Display(DISPLAY_MODE_NONE,"h");

						vector<CWire*>	WireList;
						pDesign->GetWireInDirection(&NetList,&WireList, iY, DIR_HORIZONTAL,GET_MODE_STATE,STATE_WIRE_UNROUTED);
						Layering(pDesign,&WireList,iY,DIR_HORIZONTAL);
					}

					//					CDesign::Display(DISPLAY_MODE_NONE,"\tOF=%d,VIA=%d\n",pDesign->GetOverFlow(GET_MODE_SUM),pDesign->GetNumVia());

					//					CDesign::Display(DISPLAY_MODE_NONE," V layering: ");
					// vertical direction [3/5/2007 thyeros]
					for(int iX=LastBox.X(),s=LastBox.X()+LastBox.W();iX<s;++iX)
					{
						//						if(iX%MAX(1,(s/10))==1)	CDesign::Display(DISPLAY_MODE_NONE,"v");

						vector<CWire*>	WireList;
						pDesign->GetWireInDirection(&NetList,&WireList, iX, DIR_VERTICAL,GET_MODE_STATE,STATE_WIRE_UNROUTED);
						Layering(pDesign,&WireList,iX,DIR_VERTICAL);
					}
					//					CDesign::Display(DISPLAY_MODE_NONE,"\tOF=%d,VIA=%d\n",pDesign->GetOverFlow(GET_MODE_SUM),pDesign->GetNumVia());

					GetTime(cTime);
					CDesign::Display(DISPLAY_MODE_NONE,"\tOF=%d,VIA=%d at %s\n",pDesign->GetOverFlow(GET_MODE_SUM),pDesign->GetNumVia(),cTime);

					//pDesign->GetLayer(2)->GetBoundary(42,39)->Print(stdout,PRINT_MODE_TEXT);
					//pDesign->GetLayer(4)->GetBoundary(42,39)->Print(stdout,PRINT_MODE_TEXT);

					LastBox	=	StartBox;

				}

				StartBox.Expand(pDesign->W(),pDesign->H());
			}		
		}

		pDesign->PrintResult();
		pDesign->WriteRel();
	}

//	pDesign->ReadDump(pDesign->m_Param.m_cRel_File);

	CDesign::Display(DISPLAY_MODE_NONE," H layering: ");
	// horizontal direction [3/5/2007 thyeros]
	for(int iY=0,s=pDesign->H();iY<s;++iY)
	{
		if(iY%MAX(1,(s/10))==1)	CDesign::Display(DISPLAY_MODE_NONE,"h");

		vector<CWire*>	WireList;
		pDesign->GetWireInDirection(&WireList, iY, DIR_HORIZONTAL,GET_MODE_STATE,STATE_WIRE_UNROUTED);
		Layering(pDesign,&WireList,iY,DIR_HORIZONTAL);
	}

	CDesign::Display(DISPLAY_MODE_NONE,"\tOF=%d,VIA=%d\n",pDesign->GetOverFlow(GET_MODE_SUM),pDesign->GetNumVia());

	CDesign::Display(DISPLAY_MODE_NONE," V layering: ");
	// vertical direction [3/5/2007 thyeros]
	for(int iX=0,s=pDesign->W();iX<s;++iX)
	{
		if(iX%MAX(1,(s/10))==1)	CDesign::Display(DISPLAY_MODE_NONE,"v");

		vector<CWire*>	WireList;
		pDesign->GetWireInDirection(&WireList, iX, DIR_VERTICAL,GET_MODE_STATE,STATE_WIRE_UNROUTED);
		Layering(pDesign,&WireList,iX,DIR_VERTICAL);
	}
	CDesign::Display(DISPLAY_MODE_NONE,"\tOF=%d,VIA=%d\n",pDesign->GetOverFlow(GET_MODE_SUM),pDesign->GetNumVia());

	//vector<CWire*>	WireList;
	//pDesign->GetWireInBBox(&WireList,pDesign,GET_MODE_STATE,STATE_WIRE_UNROUTED);
	//if(WireList.size())
	//{
	//	int i;
	//	for(i=0;i<MAX_MAZECOST;i++)
	//	{
	//		m_iMazeCost[i]	=	(int)(pDesign->GetCapacity(GET_MODE_MAX)*pow((i*0.01),pDesign->m_Param.m_iReRouting_Push))+1;
	//	}

	//	CDesign::Display(DISPLAY_MODE_NONE," Wrapping up %d wires...",WireList.size());
	//	CGRouter::SequentialRouting(pDesign,&WireList,pDesign,pDesign->m_Param.m_iMazeRouting_Margin);	
	//	CDesign::Display(DISPLAY_MODE_NONE,"...done! ");
	//}
#endif

	pDesign->Print(stdout,PRINT_MODE_CONGET);
}

