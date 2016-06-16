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
// Design.cpp: implementation of the CDesign class.
//
//////////////////////////////////////////////////////////////////////

#include "Design.h"
#include "flute.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDesign::CDesign()
{
#ifdef _DEBUG
	m_iDebugNet		=	-1;
#endif
	m_iNumBCap		=	0;
	m_iHighestRoutableLayer	=	MAX_NUMBER;
	m_iLowestRoutableLayer	=	1;
	//m_dTargetCongestion		=	1;

	m_iLowX	=	0;
	m_iLowY	=	0;
	m_iGridX	=	1;
	m_iGridY	=	1;
	m_iNumNet			=	0;
	m_iMaxCapacity		=	0;
	m_iLoop				=	0;
	m_pLayer			=	NULL;
	m_ppNet				=	NULL;

	m_iBestOF			=	MAX_NUMBER;
	m_iBestWL			=	MAX_NUMBER;
	m_iBestVia			=	MAX_NUMBER;
	m_iBestLoop			=	-1;
	m_iNumLocalNet		=	0;

	m_iNumPreRoutedNet	=	0;
	m_iPreRoutedWL		=	0;
}

CDesign::~CDesign()
{
	DelNets();
	SAFE_DELA(m_pLayer);
	SAFE_FCLOSE(m_pLog);

	//assert(CWire::Debug()==0);
	//assert(CSegment::m_iNumInstance==0);
}

void CDesign::Report(char* pName)
{
	if(strlen(m_Param.m_cEmail)>5)
	{
		char	cName[MAX_BUFFER_STR];
		sprintf(cName,">%s.rpt",m_Param.m_cOutput_File);

		PrintFile(cName,PRINT_MODE_TEXT);
		PrintFile(cName,PRINT_MODE_CONGET);

		char	cCmd[MAX_BUFFER_STR];
		sprintf(cCmd,"cat %s >> %s",cName+1,m_Param.m_cOutput_File);
		system(cCmd);

		sprintf(cCmd,"mail %s -s \"%s OF=%d WL:%d VIA:%d=%d in %.1f sec (%.1f MB)\" < %s",
			m_Param.m_cEmail,pName,m_iBestOF,m_iBestWL,m_iBestVia,
			m_iBestWL+m_iBestVia*m_Param.m_iViaCost,			
			StopWatch(STOPWATCH_OPTION_SET),GetMemory()/1024.0,cName+1);
#ifndef WIN32
		system(cCmd);
#endif
		Display(DISPLAY_MODE_EXEC,"%s\n",cCmd);
	}
}

// read in parameter file and store the parameters [1/31/2007 thyeros]
void CDesign::ReadParam(char* pName)
{
	FILE*	pFile		=	fopen(pName,"rt");

	if(!pFile)
	{
		Display(DISPLAY_MODE_ERRO,"file [%s] not found!\n",pName);
		exit(-7);
	}	

	char	cLine[MAX_BUFFER_STR];

	sprintf (cLine,"cat %s",pName);
	system(cLine);

	while (fgets(cLine,sizeof(cLine),pFile))
	{
		if(cLine[0]=='#')	continue;
		if(strlen(cLine)<2)	continue;

		char*	pTokenParam	=	strtok(cLine," \t\n");
		char*	pTokenValue	=	strtok(NULL," \t\n");

		if(pTokenParam==NULL||pTokenValue==NULL)	continue;

		if(STRICMP(pTokenParam,"REL_FILE")==0)
		{
			strcpy(m_Param.m_cRel_File,pTokenValue);
		}
		else if(STRICMP(pTokenParam,"INPUT_FILE")==0)
		{
			FILE*	pTest	=	fopen(pTokenValue,"rt");
			if(!pTest)
			{
				Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
				exit(-8);
			}
			fclose(pTest);

			strcpy(m_Param.m_cInput_File,pTokenValue);
		}
		else if(STRICMP(pTokenParam,"EMAIL")==0)
		{
			strcpy(m_Param.m_cEmail,pTokenValue);

			if(strlen(m_Param.m_cEmail)>5)
			{
				char	cName[MAX_BUFFER_STR];
				sprintf(cName,"%s.rpt",m_Param.m_cOutput_File);

				char	cCmd[MAX_BUFFER_STR];
				sprintf(cCmd,"hostname > %s",cName);
				system(cCmd);
#ifndef WIN32
				sprintf(cCmd,"date >> %s",cName);
				system(cCmd);
#endif
				sprintf(cCmd,"echo \"compiled as %s on %s (%d bit) at %s %s\" >> %s",
#ifdef _DEBUG
					"DEBUG"
#else
					"RELEASE"
#endif		
					,
#ifdef WIN32
					"WIN32"
#else
					"LINUX?"
#endif
					,
#ifdef __64BIT__
					64
#else
					32
#endif
					,__TIME__,__DATE__,cName);
				system(cCmd);

				sprintf(cCmd,"echo \"###########\" >> %s",cName);
				system(cCmd);

				sprintf(cCmd,"cat %s >> %s",pName,cName);
				system(cCmd);
			}
		}
		else if(STRICMP(pTokenParam,"OUTPUT_FILE")==0)
		{
			strcpy(m_Param.m_cOutput_File,pTokenValue);
		}
		else if(STRICMP(pTokenParam,"TREE_FILE")==0)
		{
			strcpy(m_Param.m_cTree_File,pTokenValue);
		}
		else if(STRICMP(pTokenParam,"DUMP_FILE")==0)
		{
			strcpy(m_Param.m_cDump_File,pTokenValue);
		}
		else if(STRICMP(pTokenParam,"MULTI_LAYER")==0)
		{
			switch(atoi(pTokenValue)){
			case 1:
				m_Param.m_iProperty	|=	PROP_PARAM_MULTILAYER;
				m_Param.m_iViaCost	=	3;
				Display(DISPLAY_MODE_PARM,"multilayer option enabled (via cost = %d)\n",m_Param.m_iViaCost);
				break;
			}
		}
		else if(STRICMP(pTokenParam,"PALL_CPU")==0)
		{
			m_Param.m_iParallelCPU	=	atoi(pTokenValue);
			if(m_Param.m_iBoxRouting_Step<1)
			{
				Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
				exit(-8);
			}		
		}
		else if(STRICMP(pTokenParam,"ILP_SOLVER")==0)
		{
			switch(atoi(pTokenValue)){
			case 1:
				m_Param.m_iProperty	|=	PROP_PARAM_SOLVER_GLPK;
				Display(DISPLAY_MODE_PARM,"GLPK as ILP solver\n");
				break;
			case 2:
				m_Param.m_iProperty	|=	PROP_PARAM_SOLVER_MOSEK;
				Display(DISPLAY_MODE_PARM,"MOSEK as ILP solver\n");
				break;
			default:
				assert(FALSE);
				break;
			}
		}
		else if(STRICMP(pTokenParam,"RELAYERING")==0)
		{
			switch(atoi(pTokenValue)){
			case 1:
				m_Param.m_iProperty	|=	PROP_PARAM_RELAYERING;
				break;
			}
		}
		else if(STRICMP(pTokenParam,"DISPLAY")==0)
		{
			switch(atoi(pTokenValue)){
			case 0:
				m_Param.m_iProperty	|=	PROP_PARAM_DISP_OFF;
				break;
			}
		}
		else if(STRICMP(pTokenParam,"BOXROUTING_STEP")==0)
		{
			m_Param.m_iBoxRouting_Step	=	atoi(pTokenValue);
			if(m_Param.m_iBoxRouting_Step<=0)
			{
				Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
				exit(-8);
			}
		}
		else if(STRICMP(pTokenParam,"REROUTING_STEP")==0)
		{
			m_Param.m_iReRouting_Step	=	atoi(pTokenValue);
			if(m_Param.m_iReRouting_Step<=1)
			{
				Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
				exit(-8);
			}
		}
		else if(STRICMP(pTokenParam,"MAZEROUTING_MARGIN")==0)
		{
			m_Param.m_iMazeRouting_Margin	=	atoi(pTokenValue);
			if(m_Param.m_iMazeRouting_Margin<=0)
			{
				Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
				exit(-8);
			}
		}
		else if(STRICMP(pTokenParam,"BOXROUTING_ILP")==0)
		{
			switch(atoi(pTokenValue)){
			case 0:
				m_Param.m_iProperty	|=	PROP_PARAM_BOXR_MAZE;
				break;
			case 1:
				m_Param.m_iProperty	|=	PROP_PARAM_BOXR_ILPMIN;
				break;
			case 2:
				m_Param.m_iProperty	|=	PROP_PARAM_BOXR_ILPMAX;
				break;
			case 3:
				m_Param.m_iProperty	|=	PROP_PARAM_BOXR_ILPHYD;
				break;
			case 4:
				m_Param.m_iProperty	|=	PROP_PARAM_BOXR_PATTERN;
				break;
			case 5:
				m_Param.m_iProperty	|=	PROP_PARAM_BOXR_ILPMAX;
				m_Param.m_iProperty	|=	PROP_PARAM_BOXR_ILP_RNDUP;
				break;
			default:
				assert(FALSE);
				break;
			}
		}
		else if(STRICMP(pTokenParam,"REROUTING")==0)
		{
			switch(atoi(pTokenValue)){
			case 1:
				m_Param.m_iProperty	|=	PROP_PARAM_RER;
				break;
			}
		}
		//else if(STRICMP(pTokenParam,"OUTPUT_RPIN")==0)
		//{
		//	switch(atoi(pTokenValue)){
		//	case 1:
		//		m_Param.m_iProperty	|=	PROP_PARAM_OUTPUT_RPIN;
		//		break;
		//	}
		//}
		else if(STRICMP(pTokenParam,"PREROUTING")==0)
		{
			switch(atoi(pTokenValue)){
			case 1:
				m_Param.m_iProperty	|=	PROP_PARAM_PRER;
				break;
			}
		}
		else if(STRICMP(pTokenParam,"BOXROUTING")==0)
		{
			switch(atoi(pTokenValue)){
			case 1:
				m_Param.m_iProperty	|=	PROP_PARAM_BOXR;
				break;
			}
		}
		else if(STRICMP(pTokenParam,"REROUTING_COUNT")==0)
		{
			m_Param.m_iReRouting_Count	=	atoi(pTokenValue);
//#ifndef _DEBUG
//			if(m_Param.m_iReRouting_Count<=0)
//			{
//				Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
//				exit(-8);
//			}
//#endif
			if(m_Param.m_iReRouting_Count>9999)				
			{
				m_Param.m_iReRouting_Count	=	9999;
				Display(DISPLAY_MODE_WARN,"%s is limited to %d\n",pTokenParam,m_Param.m_iReRouting_Count);
			}
		}
		else if(STRICMP(pTokenParam,"REROUTING_REPEAT")==0)
		{
			m_Param.m_iReRouting_Repeat	=	atoi(pTokenValue);
			//if(m_Param.m_iReRouting_Repeat<0)
			//{
			//	Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
			//	exit(-8);
			//}
			if(m_Param.m_iReRouting_Repeat>9999)				
			{
				m_Param.m_iReRouting_Repeat	=	9999;
				Display(DISPLAY_MODE_WARN,"%s is limited to %d\n",pTokenParam,m_Param.m_iReRouting_Repeat);
			}
		}
		else if(STRICMP(pTokenParam,"REROUTING_PUSH")==0)
		{
			m_Param.m_iReRouting_Push	=	atoi(pTokenValue);
			if(m_Param.m_iReRouting_Push<=1)
			{
				Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
				exit(-8);
			}
		}
		else if(STRICMP(pTokenParam,"REROUTING_STUCK")==0)
		{
			m_Param.m_iReRouting_Stuck	=	atoi(pTokenValue);
			if(m_Param.m_iReRouting_Stuck<=0)
			{
				Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
				exit(-8);
			}
		}

		//else if(STRICMP(pTokenParam,"REROUTING_K")==0)
		//{
		//	m_Param.m_iReRouting_K	=	atoi(pTokenValue);
		//	if(m_Param.m_iReRouting_K<=0)
		//	{
		//		Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
		//		exit(-8);
		//	}
		//}
		//else if(STRICMP(pTokenParam,"REROUTING_B")==0)
		//{
		//	m_Param.m_iReRouting_B	=	atoi(pTokenValue);
		//	if(m_Param.m_iReRouting_B<=0)
		//	{
		//		Display(DISPLAY_MODE_ERRO,"%s [%s]\n",pTokenParam,pTokenValue);
		//		exit(-8);
		//	}
		//}
	}

	Display(DISPLAY_MODE_INFO,"compiled as %s on %s (%d bit) at %s %s \n",
#ifdef _DEBUG
		"DEBUG"
#else
		"RELEASE"
#endif		
		,
#ifdef WIN32
		"WIN32"
#else
		"LINUX?"
#endif
		,
#ifdef __64BIT__
		64
#else
		32
#endif
		,__TIME__,__DATE__);

#ifdef __64BIT__
	if (sizeof(int*)*8!=64)
#else
	if (sizeof(int*)*8!=32)
#endif
	{
		Display(DISPLAY_MODE_INFO,"this is %d bit machine, but compiled for %d bit machine\n",sizeof(int*)*8,
#ifdef __64BIT__
			64
#else
			32
#endif
			);

		exit(-5);
	}

	Display(DISPLAY_MODE_INFO,"pid: %d @ ",getpid());
#ifdef WIN32
	Display(DISPLAY_MODE_NONE,"WIN32\n");
#else
	system("hostname");
#endif
	Display(DISPLAY_MODE_INFO,"running in ");
	system("pwd");

	SAFE_FCLOSE(pFile);
}

int	CDesign::Initialize(char* pName)
{
#ifdef _DEBUG
	m_iDebugNet	=	188;
#endif
	ReadParam(pName);

	//SetState(STATE_DESN_COMPLETED);
	//PrintResult();

	//vector<CGrid*>	Queue;

	//CGrid	G[4];
	//CGrid* pG[4];

	//G[0].SetMazeCostGrid(1,NULL);
	//G[1].SetMazeCostGrid(7,NULL);
	//G[2].SetMazeCostGrid(4,NULL);
	//G[3].SetMazeCostGrid(3,NULL);

	//Queue.push_back(&G[0]);

	//make_heap(Queue.begin(),Queue.end(),CGridOpMaze());

	//Queue.push_back(&G[1]);
	//Queue.push_back(&G[2]);
	//Queue.push_back(&G[3]);

	//push_heap(Queue.begin(), Queue.end(), CGridOpMaze());

	//pG[0]	=	Queue.front();	pop_heap(Queue.begin(), Queue.end(), CGridOpMaze());Queue.pop_back();
	//pG[1]	=	Queue.front();	pop_heap(Queue.begin(), Queue.end(), CGridOpMaze());Queue.pop_back();
	//pG[2]	=	Queue.front();	pop_heap(Queue.begin(), Queue.end(), CGridOpMaze());Queue.pop_back();
	//pG[3]	=	Queue.front();	pop_heap(Queue.begin(), Queue.end(), CGridOpMaze());Queue.pop_back();

	//G[1].SetMazeCostGrid(2,NULL);

	//push_heap(Queue.begin(), Queue.end(), CGridOpMaze());


	//pG[0]	=	Queue.front();	pop_heap(Queue.begin(), Queue.end(), CGridOpMaze());Queue.pop_back();
	//pG[1]	=	Queue.front();	pop_heap(Queue.begin(), Queue.end(), CGridOpMaze());Queue.pop_back();
	//pG[2]	=	Queue.front();	pop_heap(Queue.begin(), Queue.end(), CGridOpMaze());Queue.pop_back();
	//pG[3]	=	Queue.front();	pop_heap(Queue.begin(), Queue.end(), CGridOpMaze());Queue.pop_back();

	char	cName[MAX_BUFFER_STR];
	char	cLine[MAX_BUFFER_STR];

	//open log fiile
	SAFE_FCLOSE(m_pLog);
	sprintf(cName,"%s.log",m_Param.m_cInput_File);	
	m_pLog		=	fopen(cName,"wt");
	m_pDesign	=	this;

	Display(DISPLAY_MODE_INFO,"input file [%s]\n",m_Param.m_cInput_File);
	Display(DISPLAY_MODE_INFO,"tree file [%s]\n",m_Param.m_cTree_File);

	FILE*	pInput_File		=	fopen(m_Param.m_cInput_File,"rt");
	FILE*	pTree_File		=	fopen(m_Param.m_cTree_File,"rt");

	if(pTree_File==NULL)
	{
		Display(DISPLAY_MODE_INFO,"tree file will be written as ");

#ifdef WIN32

		// open in write mode [1/31/2007 thyeros]
		pTree_File		=	fopen(m_Param.m_cTree_File,"wt");
		readLUT();
#else
		
		char	cCmd[MAX_BUFFER_STR];

		//if(STRICMP(m_Param.m_cTree_File,"flute")==0 )
		//{
			// open in write mode [1/31/2007 thyeros]
		  	if(strlen(m_Param.m_cTree_File)==0)
			  {
			    sprintf(m_Param.m_cTree_File,"%s.stt",m_Param.m_cInput_File);
			  }

			Display(DISPLAY_MODE_NONE,"%s",m_Param.m_cTree_File);


			pTree_File		=	fopen(m_Param.m_cTree_File,"wt");
			readLUT();

			Display(DISPLAY_MODE_INFO,"building steiner tree by *pure flute\n");
			//}

		/*
		else
		{

			if(strlen(m_Param.m_cTree_File)==0)
			{
				sprintf(m_Param.m_cTree_File,"%s.stt",m_Param.m_cInput_File);
			}

			Display(DISPLAY_MODE_NONE,"%s\n",m_Param.m_cTree_File);

			sprintf(cCmd,"./cflute.x -bt %s %s 1.0 > /tmp/stt",m_Param.m_cInput_File,m_Param.m_cTree_File);
			Display(DISPLAY_MODE_EXEC,"%s\n",cCmd);
			Display(DISPLAY_MODE_INFO,"building steiner tree in %s...",m_Param.m_cTree_File);
			Display(DISPLAY_MODE_NONE,"done (%d)!\n",system(cCmd));

			pTree_File		=	fopen(m_Param.m_cTree_File,"rt");
		}
		*/
#endif

	}

	while (fgets(cLine,sizeof(cLine),pInput_File))
	{
		if(cLine[0]=='#')	continue;
		if(strlen(cLine)<2)	continue;

		char*	pToken	=	strtok(cLine," \t\n");

		if(STRICMP(pToken,"grid")==0)
		{
			m_iSizeX		=	atoi(strtok(NULL," \t\n"));
			m_iSizeY		=	atoi(strtok(NULL," \t\n"));

			pToken			=	strtok(NULL," \t\n");
			if(pToken)	m_iSizeZ	=	max(2,atoi(pToken));		// multilayer routing [1/31/2007 thyeros]
			else		m_iSizeZ	=	2;							// planar routing [1/31/2007 thyeros]

			CBBox::Initialize(0,0,0,m_iSizeX,m_iSizeY,m_iSizeZ);

			CreateLayer();
		}
		else if(STRICMP(pToken,"vertical")==0&&strtok(NULL," \t\n"))
		{
			int	iIndex	=	LAYER_METAL1;	
			for(pToken=strtok(NULL," \t\n");pToken;pToken=strtok(NULL," \t\n"))
			{
				int	iCapacity	=	atoi(pToken);
				if(iCapacity)
				{
					for(;GetLayer(iIndex)->GetKey();iIndex++);	
					GetLayer(iIndex)->Configure(iIndex,iCapacity,DIR_VERTICAL,0,0,0,this);
				}

				++iIndex;
			}
		}
		else if(STRICMP(pToken,"horizontal")==0&&strtok(NULL," \t\n"))
		{
			int	iIndex	=	LAYER_METAL1;	
			for(pToken=strtok(NULL," \t\n");pToken;pToken=strtok(NULL," \t\n"))
			{
				int	iCapacity	=	atoi(pToken);
				if(iCapacity)
				{
					for(;GetLayer(iIndex)->GetKey();iIndex++);
					GetLayer(iIndex)->Configure(iIndex,iCapacity,DIR_HORIZONTAL,0,0,0,this);
				}

				++iIndex;
			}
		}
		else if(STRICMP(pToken,"minimum")==0)
		{
			pToken			=	strtok(NULL," \t\n");
			if(STRICMP(pToken,"width")==0)
			{
				int	iIndex	=	LAYER_METAL1;	
				for(pToken=strtok(NULL," \t\n");pToken;pToken=strtok(NULL," \t\n"),iIndex++)
				{
					GetLayer(iIndex)->Configure(-1,-1,-1,atoi(pToken),-1,-1,this);
				}

			}
			else if(STRICMP(pToken,"spacing")==0)
			{
				int	iIndex	=	LAYER_METAL1;	
				for(pToken=strtok(NULL," \t\n");pToken;pToken=strtok(NULL," \t\n"),iIndex++)
				{
					GetLayer(iIndex)->Configure(-1,-1,-1,-1,atoi(pToken),-1,this);
				}
			}
		}
		else if(STRICMP(pToken,"via")==0)
		{
			pToken			=	strtok(NULL," \t\n");
			if(STRICMP(pToken,"spacing")==0)
			{
				int	iIndex	=	LAYER_METAL1;	
				for(pToken=strtok(NULL," \t\n");pToken;pToken=strtok(NULL," \t\n"),iIndex++)
				{
					GetLayer(iIndex)->Configure(-1,-1,-1,-1,-1,atoi(pToken),this);
				}
			}
		}
		else if(STRICMP(pToken,"num")==0)
		{
			pToken			=	strtok(NULL," \t\n");
			if(STRICMP(pToken,"net")==0)
			{
				CreateNet(atoi(strtok(NULL," \t\n")));

				for (int i=LAYER_METAL1;i<=T();++i)	
				{
					CLayer*	pLayer	=	GetLayer(i);

					if(!pLayer->GetKey())
					{
						int	iDirection	=	-1;

						if(i==LAYER_METAL1)										iDirection	=	DIR_VERTICAL;
						else if(GetLayer(i-1)->GetDirection()==DIR_VERTICAL)	iDirection	=	DIR_HORIZONTAL;
						else													iDirection	=	DIR_VERTICAL;

						pLayer->Configure(i,0,iDirection,0,0,0,this);
					}

					pLayer->Initialize();
					m_iMaxCapacity		=	MAX(m_iMaxCapacity,pLayer->GetCapacity(GET_MODE_MAX));
				}

				SetHighestRoutableLayer(T());
				SetLowestRoutableLayer(LAYER_METAL1);

				break;
			}
		}
		else
		{
			m_iLowX		=	atoi(pToken);
			m_iLowY		=	atoi(strtok(NULL," \t\n"));
			m_iGridX	=	atoi(strtok(NULL," \t\n"));
			m_iGridY	=	atoi(strtok(NULL," \t\n"));
		}
	}

	Display(DISPLAY_MODE_INFO,"total available capacity: %d\n",GetCapacity(GET_MODE_ACAP));
	Display(DISPLAY_MODE_INFO,"reading nets started (%.1f MB)\n",GetMemory()/1024.0);

	//////////////////////////////////////////////////////////////////////////
	// start read nets [1/31/2007 thyeros]
	//////////////////////////////////////////////////////////////////////////
	int	iNetIndex	=	0;
	for(int i=0;i<m_iNumNet;++i)
	{
		while(fgets(cLine,sizeof(cLine),pInput_File))
		{
			if(cLine[0]=='#')	continue;
			if(strlen(cLine)<2)	continue;
			break;
		}

		char	cNetName[MAX_BUFFER_STR];
		sprintf(cNetName,strtok(cLine," \t\n"));
		int		iNetID	 		=	atoi(strtok(NULL," \t\n"));
		int		iNumPin			=	atoi(strtok(NULL," \t\n"));
		int		iWidth			=	1;

		char*	pToken			=	strtok(NULL," \t\n");
		if(pToken)	iWidth		=	atoi(pToken);	

		m_ppNet[iNetIndex]		=	new	CNet;
		assert(m_ppNet[iNetIndex]);

		CNet*	pNet			=	GetNet(iNetIndex);
		pNet->Initialize(iNetID,/*iWidth,*/this);

		if(iNumPin>1000)
		{
			Display(DISPLAY_MODE_WARN,"net (%s) with too many pins (%d) will be ignored\n",cNetName,iNumPin);

			for(int j=0;j<iNumPin;++j)
			{
				while(fgets(cLine,sizeof(cLine),pInput_File))
				{
					if(cLine[0]=='#')	continue;
					if(strlen(cLine)<2)	continue;
					break;
				}
			}

			iNumPin	=	0;
		}

		// get pins [1/31/2007 thyeros]
		vector<CPin*>	PinList;
		for(int j=0;j<iNumPin;++j)
		{
			while(fgets(cLine,sizeof(cLine),pInput_File))
			{
				if(cLine[0]=='#')	continue;
				if(strlen(cLine)<2)	continue;
				break;
			}

			// this is the real location [1/31/2007 thyeros]
			int	iRX				=	atoi(strtok(cLine," \t\n"));
			int	iRY				=	atoi(strtok(NULL," \t\n"));
			int	iX				=	(iRX-m_iLowX)/m_iGridX;
			int	iY				=	(iRY-m_iLowY)/m_iGridY;
			int	iZ				=	LAYER_METAL1;

			pToken				=	strtok(NULL," \t\n");
			if(pToken)	iZ		=	atoi(pToken);

			CPin*		pPin	=	NULL;

			for(int k=0;k<PinList.size()&&!pPin;k++)
			{
				CPin*	pCurPin	=	PinList[k];

				if(pCurPin->X()==iX&&pCurPin->Y()==iY)	pPin		=	pCurPin;
			}

			if(pPin==NULL)
			{
				pPin	=	new	CPin;
				pPin->Initialize(PinList.size(),iX,iY,iZ,pNet);
				PinList.push_back(pPin);
			}
		}

		pNet->AddPin(&PinList);
//		pNet->m_iMinVia	+=	PinList.size();

		if(IsLUTup())
		{
			if(pNet->GetNumPin()<2)
			{
				if(pTree_File) fprintf(pTree_File,"%s %d %d\n!\n",cNetName,(int)pNet->GetKey(),0);
			}
			else
			{
				// create and save a tree topolgy [1/31/2007 thyeros]
				static int iArrX[MAX_BUFFER];
				static int iArrY[MAX_BUFFER];

				for(int j=0;j<pNet->GetNumPin();++j)
				{
					iArrX[j]	=	pNet->GetPin(j)->X();
					iArrY[j]	=	pNet->GetPin(j)->Y();
				}			

				Tree FluteTree;

				if(pNet->GetNumPin()<=9)	FluteTree = flute(pNet->GetNumPin(),iArrX,iArrY,ACCURACY);
				else						FluteTree = flute(pNet->GetNumPin(),iArrX,iArrY,ACCURACY+25);

				int	iNumWire	=	0;
				for(int j=0;j<2*FluteTree.deg-2;++j) 
				{
					int     iNeighbor = FluteTree.branch[j].n;

					if(iNeighbor==j)        continue;
					if(FluteTree.branch[j].x==FluteTree.branch[iNeighbor].x&&FluteTree.branch[j].y==FluteTree.branch[iNeighbor].y)   continue;

					++iNumWire;
				}  

				if(pTree_File) fprintf(pTree_File,"%s %d %d\n",cNetName,(int)pNet->GetKey(),iNumWire);
				for(int j=0;j<2*FluteTree.deg-2;++j) 
				{
					int     iNeighbor = FluteTree.branch[j].n;

					if(iNeighbor==j)        continue;
					if(FluteTree.branch[j].x==FluteTree.branch[iNeighbor].x&&FluteTree.branch[j].y==FluteTree.branch[iNeighbor].y)   continue;

					if(pTree_File) fprintf(pTree_File,"%d %d - %d %d\n",  FluteTree.branch[j].x,FluteTree.branch[j].y,FluteTree.branch[iNeighbor].x,FluteTree.branch[iNeighbor].y);

					CWire*	pWire	=	CWire::New();//new	CWire;


					assert(pWire);

					int	iX1			=	FluteTree.branch[j].x;
					int	iY1			=	FluteTree.branch[j].y;
					int	iX2			=	FluteTree.branch[iNeighbor].x;
					int	iY2			=	FluteTree.branch[iNeighbor].y;

					CPin*	pPin1	=	pNet->GetPin(iX1,iY1);
					CPin*	pPin2	=	pNet->GetPin(iX2,iY2);
					int	iZ			=	MAX(pPin1? pPin1->Z():GetRoutableLayer(GET_MODE_MIN),pPin2? pPin2->Z():GetRoutableLayer(GET_MODE_MIN));

					pWire->Initialize(iX1,iY1,iX2,iY2,iZ);
					pNet->AddWire(pWire);
//					pNet->m_iMinVia	+=	(2-pWire->GetNumPinOn()+pWire->IsFlat()? 0:1);			
				}
				if(pTree_File) fprintf(pTree_File,"!\n");fflush(pTree_File);
			}
		}
		else
		{
			// read the tree toplogy [1/31/2007 thyeros]
			while(fgets(cLine,sizeof(cLine),pTree_File))
			{
				if(cLine[0]=='!')	break;
				if(cLine[0]=='#')	continue;
				if(strlen(cLine)<2)	continue;

				char*	pName			=	strtok(cLine," \t\n");
				int		iNetIndex 		=	atoi(strtok(NULL," \t\n"));
				int		iNumWire		=	atoi(strtok(NULL," \t\n"));

				for(int j=0;j<iNumWire;++j)
				{
					while(fgets(cLine,sizeof(cLine),pTree_File))
					{
						if(cLine[0]=='#')	continue;
						if(strlen(cLine)<2)	continue;
						break;
					}

					// larget nets with over 1000pins have ZERO pins [2/20/2007 thyeros]
					if(pNet->GetNumPin()>=2)
					{
						CWire*	pWire	=	CWire::New();//new	CWire;
						assert(pWire);

						int	iX1			=	atoi(strtok(cLine," \t\n"));
						int	iY1			=	atoi(strtok(NULL," \t\n"));
						strtok(NULL," \t\n");	//thyeros- dash.. [6/16/2006]
						int	iX2			=	atoi(strtok(NULL," \t\n"));
						int	iY2			=	atoi(strtok(NULL," \t\n"));

						CPin*	pPin1	=	pNet->GetPin(iX1,iY1);
						CPin*	pPin2	=	pNet->GetPin(iX2,iY2);
						int	iZ			=	MAX(pPin1? pPin1->Z():GetRoutableLayer(GET_MODE_MIN),pPin2? pPin2->Z():GetRoutableLayer(GET_MODE_MIN));

						pWire->Initialize(iX1,iY1,iX2,iY2,iZ);
						pNet->AddWire(pWire);
//						pNet->m_iMinVia	+=	(2-pWire->GetNumPinOn()+pWire->IsFlat()? 0:1);			
					}
				}				
			}
		}

		if(pNet->IsLocal())
		{
			assert(pNet==m_ppNet[iNetIndex]);

			SAFE_DEL(m_ppNet[iNetIndex]);
			++m_iNumLocalNet;
		}
		else
		{
			if(pNet->GetNumWire()==0)	Display(DISPLAY_MODE_ERRO,"net %s should have at least one wire\n",cNetName);
			pNet->m_iMinWL	=	pNet->GetLength(GET_MODE_STATE,STATE_WIRE_UNROUTED);
			++iNetIndex;
		}
	}

	// save some memory by removing local nets [2/13/2007 thyeros]
	CreateNet(iNetIndex);

	//////////////////////////////////////////////////////////////////////////
	// read speical net/blockages [1/31/2007 thyeros]
	//////////////////////////////////////////////////////////////////////////
	while(fgets(cLine,sizeof(cLine),pInput_File))
	{
		//// temporarily ignore blocakges  [2/27/2007 thyeros]
		//Display(DISPLAY_MODE_WARN,"blockages are ignored for test purpose!\n");
		//break;

		if(cLine[0]=='#')	continue;
		if(strlen(cLine)<2)	continue;

		int	iNumBlock	=	atoi(cLine);

		if(iNumBlock)
		{
			Display(DISPLAY_MODE_INFO,"%d blocakges are specified\n",iNumBlock);

			for(int i=0;i<iNumBlock;++i)
			{
				while(fgets(cLine,sizeof(cLine),pInput_File))
				{
					if(cLine[0]=='#')	continue;
					if(strlen(cLine)<2)	continue;
					break;
				}

				int	iX1		=	atoi(strtok(cLine," \t\n"));
				int	iY1		=	atoi(strtok(NULL," \t\n"));
				int	iZ1		=	atoi(strtok(NULL," \t\n"));

				int	iX2		=	atoi(strtok(NULL," \t\n"));
				int	iY2		=	atoi(strtok(NULL," \t\n"));
				int	iZ2		=	atoi(strtok(NULL," \t\n"));

				assert(iZ1==iZ2);
				assert(iX1==iX2||iY1==iY2);
				assert(abs((iX1-iX2))==1||abs((iY1-iY2))==1);

				CBoundary*	pBoundary	=	GetLayer(iZ1)->GetBoundary(iX1,iY1,iX2,iY2);

				if(!pBoundary)				Display(DISPLAY_MODE_ERRO,"boundary (%d,%d,%d)-(%d,%d,%d) not found\n",iX1,iY1,iZ1,iX2,iY2,iZ2);
				if(pBoundary->m_iNumOCap)	Display(DISPLAY_MODE_ERRO,"boundary (%d,%d,%d)-(%d,%d,%d) is overwritten\n",iX1,iY1,iZ1,iX2,iY2,iZ2);

				int	iBlockage	=	pBoundary->GetParent()->GetCapacity(GET_MODE_MAX)-atoi(strtok(NULL," \t\n"));
				m_iNumBCap	+=		iBlockage;
				pBoundary->AdjustCapacity(iBlockage);
			}
			Display(DISPLAY_MODE_INFO,"total available capacity: %d\n",GetCapacity(GET_MODE_ACAP));
		}

		break;
	}

	SAFE_FCLOSE(pInput_File);
	SAFE_FCLOSE(pTree_File);

	Display(DISPLAY_MODE_INFO,"input [%s with %d nets] is loaded (%.1f MB)\n",m_Param.m_cInput_File,GetNumNet(),GetMemory()/1024.0);

	if(strlen(m_Param.m_cDump_File))
	{
		FILE*	pDumpFile	=	fopen(m_Param.m_cDump_File,"rt");
		if(pDumpFile)
		{
			SAFE_FCLOSE(pDumpFile);
			m_Param.m_iProperty	&=	~(PROP_PARAM_PRER|PROP_PARAM_BOXR);
		}
	}

	if(m_Param.m_iMazeRouting_Margin<0)	m_Param.m_iMazeRouting_Margin	=	0.1*MAX(W(),H());
	Display(DISPLAY_MODE_PARM,"mazerouting margin %d\n",m_Param.m_iMazeRouting_Margin);

	Print(stdout,PRINT_MODE_TEXT);

	return	TRUE;
}

CLayer* CDesign::GetLayer(int iLayer)
{
#ifdef _DEBUG
	if(iLayer>T()||iLayer<1)
	{
		int a=0;
	}
#endif
	assert(iLayer<=T()&&iLayer>=LAYER_METAL1);
	//thyeros- iLayer start from ONE (M1), but array index starts from ZERO [6/16/2006]
	return	&m_pLayer[iLayer-LAYER_METAL1];
}

void CDesign::CreateLayer()
{
	//initialize the memory for layers
	assert(T());
	m_pLayer	=	new	CLayer[T()];
	assert(m_pLayer);
}

void CDesign::CreateNet(int	iNumNet)
{
	m_iNumNet	=	iNumNet;

	if(m_ppNet)
	{
		// save some memory by removing local nets [2/13/2007 thyeros]
		CNet**	ppNet	=	new	CNet*[m_iNumNet];
		memcpy(ppNet,m_ppNet,sizeof(CNet*)*m_iNumNet);
		SAFE_DELA(m_ppNet);
		m_ppNet	=	ppNet;
	}
	else
	{
		//initialize the memory for nets
		assert(m_iNumNet);
		m_ppNet	=	new	CNet*[m_iNumNet];
		assert(m_ppNet);
	}
}

void CDesign::CreateNet(vector<CNet*>*	pNetList)
{
	m_iNumNet	=	pNetList->size();

	// save some memory by removing prerouted nets [2/13/2007 thyeros]
	if(m_ppNet)		SAFE_DELA(m_ppNet);

	//initialize the memory for nets
	assert(m_iNumNet);
	m_ppNet	=	new	CNet*[m_iNumNet];
	assert(m_ppNet);

	for(int i=0;i<m_iNumNet;i++)
	{
		m_ppNet[i]	=	(*pNetList)[i];	
	}	
}

CNet* CDesign::GetNet(int iIndex)
{
	return	m_ppNet[iIndex];
}

int CDesign::GetNumNet(int iMode, int iValue)
{
	if(iValue==STATE_NET_ANY||iValue==PROP_NET_ANY)	return	m_iNumNet;

	int	iNum	=	0;
	switch(iMode) {
	case GET_MODE_PROP:	//count the nets which has iValue as the property
		for(int i=0;i<m_iNumNet;++i)	if(GetNet(i)->GetProp()&iValue)	++iNum;
		break;
	case GET_MODE_STATE://count the nets which has iValue as the state
		for(int i=0;i<m_iNumNet;++i)	if(GetNet(i)->GetState()&iValue)	++iNum;
		break;
	default:
		assert(FALSE);
		break;
	}

	return	iNum;
}

void CDesign::Print(FILE *pFile, int iMode)
{
	if(pFile==NULL)	return;

	switch(iMode) {
	case PRINT_MODE_RESULT:
		for (int i=0;i<m_iNumNet;++i)	
		{
			GetNet(i)->Print(pFile,iMode);
			fprintf(pFile,"!\n");
		}
		break;
	case PRINT_MODE_PIN:
		for (int i=0;i<m_iNumNet;++i)	
		{
			fprintf(pFile,"net(id:%d) %d\n",(int)GetNet(i)->GetKey(),i);
			GetNet(i)->Print(pFile,iMode);
			fprintf(pFile,"!\n");
		}
		break;
	case PRINT_MODE_DUMP:
		for (int i=0;i<m_iNumNet;++i)	
		{
			GetNet(i)->Print(pFile,iMode);
		}
		break;
	case PRINT_MODE_MATLAB:
		fprintf(pFile,"%%Benchmark: %s\n",m_Param.m_cInput_File);
		fprintf(pFile,"%%Grid Size: %d x %d\n",m_iSizeX,m_iSizeY);
		fprintf(pFile,"figure;\naxis ([-1 %d -1 %d]);\nhold on;\n",W()+1,H()+1);
		fprintf(pFile,"title ('%s')\n",m_Param.m_cInput_File);
		break;
	case PRINT_MODE_MATLAB3D:
		fprintf(pFile,"%%Benchmark: %s\n",m_Param.m_cInput_File);
		fprintf(pFile,"%%Grid Size: %d x %d x %d\n",W(),H(),T());
		fprintf(pFile,"figure;\naxis ([-1 %d -1 %d 0 %d]);\nhold on;\n",W()+1,H()+1,T()+1);
		fprintf(pFile,"title ('%s')\n",m_Param.m_cInput_File);
		fprintf(pFile,"view(2)\n");
		break;	
	case PRINT_MODE_GNUPLOT:
		fprintf(pFile,"#Benchmark: %s\n",m_Param.m_cInput_File);
		fprintf(pFile,"#Grid Size: %d x %d\n",W(),H());
		fprintf(pFile,"reset\n");
		fprintf(pFile,"set nokey\n");
		fprintf(pFile,"set xrange [0:%d]\n",W()+1);
		fprintf(pFile,"set yrange [0:%d]\n",H()+1);
		fprintf(pFile,"set multiplot\n");
		break;
	case PRINT_MODE_GNUPLOT3D:
		fprintf(pFile,"#Benchmark: %s\n",m_Param.m_cInput_File);
		fprintf(pFile,"#Grid Size: %d x %d\n",W(),H());
		fprintf(pFile,"reset\n");
		fprintf(pFile,"set nokey\n");
		fprintf(pFile,"set xrange [0:%d]\n",W()+1);
		fprintf(pFile,"set yrange [0:%d]\n",H()+1);
		fprintf(pFile,"set zrange [0:%d]\n",T()+1);
		fprintf(pFile,"set multiplot\n");
		break;
	case PRINT_MODE_TEXT:
		{
			int	iNumNet		=	GetNumNet()+m_iNumLocalNet;
			int	iNumLNet	=	m_iNumLocalNet;
			int	iNumTNet	=	GetNumNet(GET_MODE_PROP,PROP_NET_TWOPIN);
			int	iNumPNet	=	m_iNumPreRoutedNet;
			int	iNumVia		=	GetNumVia();

			int	iNumWire	=	GetNumWire();
			int	iNumUWire	=	GetNumWire(GET_MODE_STATE,STATE_WIRE_UNROUTED);
			int	iNumRWire	=	GetNumWire(GET_MODE_STATE,STATE_WIRE_ROUTED);
			int	iNumAWire	=	GetNumWire(GET_MODE_STATE,STATE_WIRE_ASSGNED);
			int	iNumNWire	=	GetNumWire(GET_MODE_STATE,STATE_WIRE_NOTASSGNED);
			int	iNumHWire	=	GetNumWire(GET_MODE_PROP,PROP_WIRE_HORIZONTAL);
			int	iNumVWire	=	GetNumWire(GET_MODE_PROP,PROP_WIRE_VERTICAL);

			// for multilayer, iLenWire includes the # of vias [2/22/2007 thyeros]
			int	iLenWire	=	GetLength(GET_MODE_STATE,STATE_WIRE_ANY);
			int	iLenUWire	=	GetLength(GET_MODE_STATE,STATE_WIRE_UNROUTED);
			int	iLenRWire	=	GetLength(GET_MODE_STATE,STATE_WIRE_ROUTED);
			int	iLenAWire	=	GetLength(GET_MODE_STATE,STATE_WIRE_ASSGNED);
			int	iLenNWire	=	GetLength(GET_MODE_STATE,STATE_WIRE_NOTASSGNED);
			//int	iLenHWire	=	GetLength(GET_MODE_PROP,PROP_WIRE_HORIZONTAL);
			//int	iLenVWire	=	GetLength(GET_MODE_PROP,PROP_WIRE_VERTICAL);

			int	iNumOF		=	GetOverFlow(GET_MODE_SUM);
			int	iNumMaxOF	=	GetOverFlow(GET_MODE_MAX);
			int	iNumACap	=	GetCapacity(GET_MODE_ACAP);
			int	iNumOCap	=	GetCapacity(GET_MODE_OCAP);

			fprintf(pFile,"------------ statistics ------------\n");
			fprintf(pFile,"design name\t: %s\n",m_Param.m_cInput_File);
			fprintf(pFile,"size\t\t: %d x %d x %d\n",W(),H(),T());
			fprintf(pFile,"# totl net\t: %d\n",iNumNet);
			fprintf(pFile,"# locl net\t: %d (%.1f %%)\n",iNumLNet,iNumLNet*100.0/iNumNet);
			fprintf(pFile,"# 2pin net\t: %d (%.1f %%)\n",iNumTNet,iNumTNet*100.0/iNumNet);
			fprintf(pFile,"# prep net\t: %d (%.1f %%)\n",iNumPNet,iNumPNet*100.0/iNumNet);
			fprintf(pFile,"# totl wire\t: %d\n",iNumWire);		
			fprintf(pFile,"# totl len\t: %d\n",iLenWire);
			fprintf(pFile,"# blockage\t: %d\n",m_iNumBCap);
			if(m_Param.GetProp()&PROP_PARAM_MULTILAYER)
			{
				fprintf(pFile,"# totl via\t: %d\n",iNumVia);
				fprintf(pFile,"# diffculty\t: %d vs. %d (%.1f %%)\n",iLenUWire*2+GetOverFlow(GET_MODE_SUM),iNumACap,(iLenUWire*2+GetOverFlow(GET_MODE_SUM))*100.0/iNumACap);
			}
			else
			{
				fprintf(pFile,"# diffculty\t: %d vs. %d (%.1f %%)\n",iLenUWire*2+GetOverFlow(GET_MODE_SUM),iNumACap,(iLenUWire*2+GetOverFlow(GET_MODE_SUM))*100.0/iNumACap);
			}

			fprintf(pFile,"# rout wire\t: %d (%.1f %%) %d\n",iNumRWire,iNumRWire*100.0/iNumWire,iLenRWire);
			fprintf(pFile,"# unrt wire\t: %d (%.1f %%) %d\n",iNumUWire,iNumUWire*100.0/iNumWire,iLenUWire);
			fprintf(pFile,"# lasg wire\t: %d (%.1f %%) %d\n",iNumAWire,iNumAWire*100.0/iNumWire,iLenAWire);
			fprintf(pFile,"# lnas wire\t: %d (%.1f %%) %d\n",iNumNWire,iNumNWire*100.0/iNumWire,iLenNWire);
			//fprintf(pFile,"# horz wire\t: %d (%.1f %%) %d\n",iNumHWire,iNumHWire*100.0/iNumWire,iLenHWire);
			//fprintf(pFile,"# vert wire\t: %d (%.1f %%) %d\n",iNumVWire,iNumVWire*100.0/iNumWire,iLenVWire);
			fprintf(pFile,"# overflow\t: total %d, max %d\n",iNumOF,iNumMaxOF);

			assert(GetCapacity(GET_MODE_OCAP)==m_iNumBCap+iLenRWire*2);

		}
		break;
	case PRINT_MODE_OFDUMP:
	case PRINT_MODE_CGDUMP:
	case PRINT_MODE_CONGET:
		for (int i=LAYER_METAL1;i<=T();++i)	GetLayer(i)->Print(pFile,iMode);
		break;
	case PRINT_MODE_CGMAP:
		for(int k=0;k<H();k++)
		{
			for(int j=0;j<W();j++)
			{
				int	iCongestion	=	0;
				for (int i=LAYER_METAL1;i<=T();++i)
				{
					CBoundary*	pBoundary	=	GetLayer(i)->GetBoundary(j,k);
					iCongestion	+=	pBoundary? pBoundary->GetCongestion():0;
				}

//				if(k==0&&j==0)	fprintf(pFile,"%d ",4);
//				else			
				fprintf(pFile,"%.3f ",iCongestion/100.0);
			}
			fprintf(pFile,"\n");
		}
		break;
	default:
		assert(FALSE);
		break;
	}

	fflush(pFile);
}

int CDesign::GetNumWire(int iMode, int iValue)
{
	int	iNumWire	=	0;
	for (int i=0;i<m_iNumNet;++i)	iNumWire	+=	GetNet(i)->GetNumWire(iMode,iValue);

	return	iNumWire;
}

int CDesign::GetOverFlow(int iMode)
{
	int	iOverFlow		=	0;

	switch(iMode) {
	case GET_MODE_MAX:
		for (int i=LAYER_METAL1;i<=T();++i)	iOverFlow	=	MAX(iOverFlow,GetLayer(i)->GetOverFlow(iMode));
		break;
	case GET_MODE_SUM:
		for (int i=LAYER_METAL1;i<=T();++i)	iOverFlow	+=	GetLayer(i)->GetOverFlow(iMode);
		break;
	}

	return	iOverFlow;
}

int CDesign::GetLength(int iMode, int iValue)
{
	int	iLength	=	0;

	if(iMode==GET_MODE_STATE&&iValue==STATE_WIRE_ROUTED)	for (int i=LAYER_METAL1;i<=T();++i)	iLength	+=	GetLayer(i)->GetLength();
	else													for (int i=0;i<m_iNumNet;++i)		iLength	+=	GetNet(i)->GetLength(iMode,iValue);

	switch(iValue){
	case STATE_WIRE_ASSGNED:
	case STATE_WIRE_ANY:
		iLength	+=	m_iPreRoutedWL;
		break;
	}

	return	iLength;
}

CBBox CDesign::GetStartBox(int* pNumEdge)
{
	//compute the starting point of box expansion, based on the # of pins
	//it also can be done any congestion analysis

	vector<int>	X;
	vector<int>	Y;

	for(int i=LAYER_METAL1;i<=T();++i)	GetLayer(i)->GetStartBox(&X,&Y);


	sort(X.begin(),X.end());
	sort(Y.begin(),Y.end());

	int	iStartX		=	X.size()? X[X.size()/2]:W()/2;
	int	iStartY		=	Y.size()? Y[Y.size()/2]:H()/2;

	Display(DISPLAY_MODE_INFO,"(%d,%d) predicted as the most congestion region\n",iStartX,iStartY);

	CPoint	PointS;
	CPoint	PointE;

	PointS.SetXYZ(MAX(0,iStartX-1),MAX(0,iStartY-1),0);
	PointE.SetXYZ(MIN(m_iSizeX-1,iStartX+1),MIN(m_iSizeY-1,iStartY+1),T());

	CBBox	BBox;

	BBox.Initialize(&PointS,&PointE);

	if(pNumEdge)	*pNumEdge = X.size();
	return	BBox;
}

CBBox CDesign::GetCongestedBox()
{
	CBBox	BBox;

	for(int i=LAYER_METAL1;i<=T();++i)	GetLayer(i)->GetCongestedBox(&BBox);

	return	BBox;
}

int CDesign::GetNetInBBox(vector<CNet*>* pNetList, CBBox* pBBox/*, int iMode, int iValue*/)
{
	//collect the all nets within the give box, satisfying the given condition (property/state)
	assert(pNetList==NULL||pNetList->size()==0);
	
	int	iNumNet	=	0;
	for (int i=0;i<m_iNumNet;++i)
	{
		CNet*	pNet	=	GetNet(i);
		
		if(!pNet)	continue;
		assert(pNet);

		//int		iContinue	=	TRUE;

		//switch(iMode){
		//case GET_MODE_STATE:
		//	if(pNet->GetState()&iValue)	iContinue	=	FALSE;
		//	break;
		//case GET_MODE_PROP:
		//	if(pNet->GetProp()&iValue)	iContinue	=	FALSE;
		//    break;
		//default:
		//    break;
		//}
		
		if(pNet->IsRouted())	continue;

		pNet->UpdateBBox();
		if(pBBox->IsInside(&pNet->m_BBox))
		{
			++iNumNet;
			if(pNetList)	pNetList->push_back(pNet);
		}		
	}
	
	return	iNumNet;
}
//
int CDesign::AddPenalty(int iPenalty)
{
	int	iMaxPenalty	=	0;
	for (int i=LAYER_METAL1;i<=T();++i)
		iMaxPenalty	=	MAX(iMaxPenalty,GetLayer(i)->AddPenalty(iPenalty));

	return	iMaxPenalty;
}

int CDesign::GetWireInOverFlowBoundary(hash_map<ADDRESS,int>* pWireList)
{
	for (int i=LAYER_METAL1;i<=T();++i)	GetLayer(i)->GetWireInOverFlowBoundary(pWireList);

	return	pWireList->size();
}

int CDesign::GetNetInOverFlowBoundary(hash_map<ADDRESS,int>* pNetList)
{
	for (int i=LAYER_METAL1;i<=T();++i)	GetLayer(i)->GetNetInOverFlowBoundary(pNetList);

	return	pNetList->size();
}


int	CDesign::GetOverFlowBoundary(vector<CBoundary*>* pBoundaryList)
{
	for (int i=LAYER_METAL1;i<=T();++i)	GetLayer(i)->GetOverFlowBoundary(pBoundaryList);

	return	pBoundaryList->size();
}


int CDesign::GetWireInBBox(vector<CWire*>* pWireList, CBBox* pBBox, int iMode, int iValue)
{
	//collect the all wires within the give box, satisfying the given condition (property/state)
	assert(pWireList!=NULL);//||pWireList->size()==0);

	int	iNumWire	=	0;
	for (int i=0;i<m_iNumNet;++i)	iNumWire	+=	GetNet(i)->GetWireInBBox(pWireList,pBBox,iMode,iValue);

	return	iNumWire;
}

int CDesign::GetCapacity(int iMode)
{
	switch(iMode){
	case GET_MODE_ACAP:	
	case GET_MODE_OCAP:	
		//	case GET_MODE_ECAP:	
		{
			int	iCapacity	=	0;
			for (int i=LAYER_METAL1;i<=T();++i)		iCapacity	+=	GetLayer(i)->GetCapacity(iMode);

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

int CDesign::GetNumVia()
{
	int	iNumVia	=	0;
	for (int i=0;i<GetNumNet();++i)	iNumVia	+=	GetNet(i)->GetNumVia();

	return	iNumVia;
}

void CDesign::DelNets()
{
	// need to delete Net before Layer (Boundary management issue) [6/18/2006 thyeros]
	for (int i=0;i<m_iNumNet;++i)	SAFE_DEL(m_ppNet[i]);	

	SAFE_DELA(m_ppNet);
	m_iNumNet	=	0;
}

void CDesign::WriteDump()
{
	if(strlen(m_Param.m_cDump_File))
	{
		char	cCmd[MAX_BUFFER_STR];
		sprintf(cCmd,"cp %s.dmp %s",m_Param.m_cOutput_File,m_Param.m_cDump_File);
		CreateAndWait(cCmd);
	}
}

void CDesign::WriteRel()
{
	if(strlen(m_Param.m_cRel_File))
	{
		char	cCmd[MAX_BUFFER_STR];
		sprintf(cCmd,"cp %s.dmp %s",m_Param.m_cOutput_File,m_Param.m_cRel_File);
		CreateAndWait(cCmd);
	}
}


int CDesign::ReadDump(char* pName)
{
	if(strlen(pName))
	{
		FILE*	pDump_File	=	fopen(pName,"rb");

		if(!pDump_File)	return	FALSE;

		// assume both are in the same order [2/17/2007 thyeros]
		for(int i=0,s=GetNumNet();i<s;++i)
		{
			CNet*	pNet	=	GetNet(i);

			KEY	Key;
			if(fread(&Key,1,sizeof(KEY),pDump_File))
			{	
				if((int)Key==pNet->GetKey())
				{
					pNet->DelWires();
					int		iNumWire;

					fread(&iNumWire,1,sizeof(int),pDump_File);

					KEY	Key[2];
					for(int j=0;j<iNumWire;++j)
					{
						fread(Key,sizeof(KEY),2,pDump_File);

						int	iX1	=	CPoint::X(Key[0]);
						int	iY1	=	CPoint::Y(Key[0]);
						int	iZ1	=	CPoint::Z(Key[0]);

						int	iX2	=	CPoint::X(Key[1]);
						int	iY2	=	CPoint::Y(Key[1]);
						int	iZ2	=	CPoint::Z(Key[1]);

						CWire*	pWire	=	CWire::New();//new	CWire;
						assert(pWire);

						// ignore via [3/5/2007 thyeros]

						if(iX1==iX2&&iY1==iY2)	continue;

						if(iZ1==0 && iZ2 ==0)
						{
							iZ1	=	1;
							iZ2	=	1;
							pWire->Initialize(iX1,iY1,iX2,iY2,iZ1);
							pNet->AddWire(pWire);
						}
						else
						{
							pWire->Initialize(iX1,iY1,iX2,iY2,iZ1);
							pNet->AddWire(pWire);
							pWire->MakeRouted(iZ1);

							assert(pWire->IsCompleted());
						}
					}

					assert(pNet->IsRouted());

					pNet->Refine();
		//			pNet->Check();
				}
				else
				{
					int		iNumWire;
					fread(&iNumWire,1,sizeof(int),pDump_File);

					KEY	Key[2];
					for(int j=0;j<iNumWire;++j)
					{
						fread(Key,sizeof(KEY),2,pDump_File);
					}

					--i;
				}
			}
			else
			{
				assert(FALSE);
			}
		
		}

		SAFE_FCLOSE(pDump_File);

		return	TRUE;
	}

	return	FALSE;
}

void CDesign::PrintResult()
{
	if(!strlen(m_Param.m_cOutput_File))	return;

	if(GetState()&STATE_DESN_COMPLETED)
	{

		char	cCmd[MAX_BUFFER_STR];

#ifdef WIN32
		sprintf(cCmd,"dmp2res.exe %s %s.dmp %s",
			m_Param.m_cInput_File,
			m_Param.m_cOutput_File,
			m_Param.m_cOutput_File);
#else
		sprintf(cCmd,"./dmp2res.x %s %s.dmp %s",
			m_Param.m_cInput_File,
			m_Param.m_cOutput_File,
			m_Param.m_cOutput_File);
#endif

		Display(DISPLAY_MODE_EXEC,"%s\n",cCmd);
		Display(DISPLAY_MODE_INFO,"writing result to %s...",m_Param.m_cOutput_File);
		

		Display(DISPLAY_MODE_NONE,"done (%d)!\n",system(cCmd));
		Display(DISPLAY_MODE_INFO,"OF:%d WL:%d VIA:%d=%d (%d) in %.2f sec\n",m_iBestOF,m_iBestWL,m_iBestVia,m_iBestWL+m_iBestVia*m_Param.m_iViaCost,m_iBestLoop,StopWatch(STOPWATCH_OPTION_SET));


		sprintf(cCmd,"%s.map",m_Param.m_cOutput_File);
        	PrintFile(cCmd,PRINT_MODE_CGMAP); 

	}
	else
	{
		char	cDump[256];
		sprintf(cDump,"%s.dmp",m_Param.m_cOutput_File);
		PrintFile(cDump,PRINT_MODE_DUMP);
	}
}

void CDesign::SetState(int iState)
{
	switch(iState){
	//case STATE_DESN_CONVERGING:
	//	m_iState	|=	STATE_DESN_CONVERGING;
	//	break;
	case STATE_DESN_COMPLETED:
		m_iState	|=	STATE_DESN_COMPLETED;
		break;
	case STATE_DESN_STUCK:
		m_iState	|=	STATE_DESN_STUCK;
		break;
	case ~STATE_DESN_STUCK:
		m_iState	&=	~STATE_DESN_STUCK;
		break;
	case STATE_DESN_SUCCESS:
		m_iState	|=	STATE_DESN_SUCCESS;
		//m_iState	&=	~STATE_DESN_STUCK;
		Display(DISPLAY_MODE_INFO,"routing success!\n");
		break;
	case STATE_DESN_REROUTING:	//design is being rerouted
		m_iState	&=	~STATE_DESN_REROUTED;
		m_iState	|=	STATE_DESN_REROUTING;
		break;
	case STATE_DESN_REROUTED:	//design is rerouted
		m_iState	|=	STATE_DESN_REROUTED;
		m_iState	&=	~STATE_DESN_REROUTING;
		break;
	case STATE_DESN_ROUTING:	//design is being routed
		m_iState	|=	STATE_DESN_ROUTING;
		m_iState	&=	~STATE_DESN_ROUTED;
		break;
	case STATE_DESN_OVERFLOW:	//design has overflow
		m_iState	|=	STATE_DESN_OVERFLOW;
		break;
	case ~STATE_DESN_OVERFLOW:	//the overflow in the design is removed
		m_iState	&=	~STATE_DESN_OVERFLOW;
		break;
	case STATE_DESN_ROUTED:		//design is routed
		m_iState	|=	STATE_DESN_ROUTED;
		m_iState	&=	~STATE_DESN_ROUTING;
		break;
	default:
		assert(FALSE);
		break;
	}
}

void CDesign::SetProp(int iProp)
{
	m_iProp	|=	iProp;
}

void CDesign::SetHighestRoutableLayer(int iLayer)
{
	m_iHighestRoutableLayer	=	iLayer;
}


void CDesign::SetLowestRoutableLayer(int iLayer)
{
	m_iLowestRoutableLayer	=	MAX(LAYER_METAL1,iLayer);
}

int	CDesign::GetRoutableLayer(int iMode)
{
	switch(iMode){
	case GET_MODE_MAX:	return	m_iHighestRoutableLayer;
	case GET_MODE_MIN:	return	m_iLowestRoutableLayer;
	default:
		assert(FALSE);
		break;
	}

	return	-1;
}

//void CDesign::SetTargetCongestion(double dTargetCongestion)
//{
//	m_dTargetCongestion	=	dTargetCongestion;
//}

double CDesign::GetCongestion(int iMode)
{
	switch(iMode){
		//	case GET_MODE_TARGET:	return	m_dTargetCongestion;
	case GET_MODE_MAX:
		{
			double	dMaxCongestion	=	-1;
			for (int i=LAYER_METAL1;i<=T();++i)	
				dMaxCongestion	=	MAX(dMaxCongestion,GetLayer(i)->GetCongestion(iMode));

			return	dMaxCongestion;
		}
	case GET_MODE_AVG:
		{
			double	dAvgCongestion	=	-1;
			for (int i=LAYER_METAL1;i<=T();++i)	
				dAvgCongestion	+=	GetLayer(i)->GetCongestion(iMode);

			return	dAvgCongestion/T();
		}
	default:
		assert(FALSE);
		break;
	}

	return	-1;
}

void CDesign::UpdateSteinerTree()
{
	char	cName[MAX_BUFFER_STR];
	sprintf(cName,">%s.cft",m_Param.m_cInput_File);

	FILE*	pFile	=	fopen(cName+1,"wt");
	fprintf(pFile,"%d %d %d %d\n",W(),H(),GetLayer(LAYER_METAL1)->GetCapacity(GET_MODE_MAX),GetLayer(LAYER_METAL2)->GetCapacity(GET_MODE_MAX));
	SAFE_FCLOSE(pFile);

	PrintFile(cName,PRINT_MODE_CGDUMP);
	PrintFile(cName,PRINT_MODE_PIN);

	pFile	=	fopen(cName+1,"at");
	fprintf(pFile,"#");
	SAFE_FCLOSE(pFile);

	return;

	char	cCmd[MAX_BUFFER_STR];
	sprintf(cCmd,"cflute_ispd07 -x %s %s.nst",cName,m_Param.m_cInput_File);

	CreateAndWait(cCmd);

	sprintf(cName,"%s.nst",m_Param.m_cInput_File);

	pFile	=	fopen(cName,"rt");

	char	cLine[MAX_BUFFER_STR];
	while(fgets(cLine,sizeof(cLine),pFile))
	{
		if(cLine[0]=='!')	continue;
		if(cLine[0]=='#')	continue;
		if(strlen(cLine)<1)	continue;


		char	cNetName[MAX_BUFFER_STR];
		sprintf(cNetName,strtok(cLine," \t\n"));
		int		iNetIndex			=	atoi(strtok(NULL," \t\n"));
		int		iNumWire			=	atoi(strtok(NULL," \t\n"));

		CNet*	pNet	=	GetNet(iNetIndex);
		assert(pNet);

		pNet->DelWires();

		for(int i=0;i<iNumWire;++i)
		{
			fgets(cLine,sizeof(cLine),pFile);

			int	iX1	=	atoi(strtok(cLine," \t\n"));
			int	iY1	=	atoi(strtok(NULL," \t\n"));

			strtok(NULL," \t\n");	// dash [2/20/2007 thyeros]

			int	iX2	=	atoi(strtok(NULL," \t\n"));
			int	iY2	=	atoi(strtok(NULL," \t\n"));

			CWire*	pWire	=	CWire::New();

			CPin*	pPin1	=	pNet->GetPin(iX1,iY1);
			CPin*	pPin2	=	pNet->GetPin(iX2,iY2);
			int	iZ			=	MAX(pPin1? pPin1->Z():GetRoutableLayer(GET_MODE_MIN),pPin2? pPin2->Z():GetRoutableLayer(GET_MODE_MIN));

			pWire->Initialize(iX1,iY1,iX2,iY2,iZ);
			pNet->AddWire(pWire);
		}
	}
}

//double CDesign::GetMetric()
//{
//	double	dMetric	=	GetLength(GET_MODE_STATE,STATE_WIRE_ANY);
//
//	dMetric	+=	GetNumVia()*m_Param.m_iViaCost;
//
//	hash_map<ADDRESS,int>	ObjectList;
//	int iObjectOverFlow	=	GetNetInOverFlowBoundary(&ObjectList);
//
//	for(hash_map<ADDRESS,int>::iterator itr=ObjectList.begin();itr!=ObjectList.end();++itr)
//	{
//		CNet*	pNet		=	(CNet*)itr->first;
//
//		double	dAdjustedWL	=	(itr->second+1)*pNet->m_fPenalty;
//
//		dMetric	+=	(dAdjustedWL-pNet->GetLength(GET_MODE_STATE,STATE_DESN_ROUTED)-pNet->GetNumVia()*m_Param.m_iViaCost);
//	}
//
//	return	dMetric;
//}

int CDesign::GetWireInDirection(vector<CWire*>* pWireList, int iPos, int iDirection, int iMode, int iValue)
{
	switch(iValue){
		case STATE_WIRE_ANY:
		case STATE_WIRE_ROUTED:
		case STATE_WIRE_UNROUTED:
		case PROP_WIRE_ANY:
			for(int i=0,s=GetNumNet(i);i<s;i++)
			{
				CNet*	pNet	=	GetNet(i);
				pNet->GetWireInDirection(pWireList,iPos,iDirection,iMode,iValue);
			}
			break;
		case PROP_NET_FLAT|PROP_NET_TWOPIN:
			for(int i=0,s=GetNumNet(i);i<s;i++)
			{
				CNet*	pNet	=	GetNet(i);
				if(pNet->IsFlat()&&pNet->GetProp()&PROP_NET_TWOPIN)
					pNet->GetWireInDirection(pWireList,iPos,iDirection);
			}
			break;
	}
	

	return	pWireList->size();
}

int CDesign::GetWireInDirection(vector<CNet*>* pNetList, vector<CWire*>* pWireList, int iPos, int iDirection, int iMode, int iValue)
{
	switch(iValue){
		case STATE_WIRE_ANY:
		case STATE_WIRE_ROUTED:
		case STATE_WIRE_UNROUTED:
		case PROP_WIRE_ANY:
			for(int i=0,s=pNetList->size();i<s;i++)
			{
				CNet*	pNet	=	pNetList->at(i);
				pNet->GetWireInDirection(pWireList,iPos,iDirection,iMode,iValue);
			}
			break;
		case PROP_NET_FLAT|PROP_NET_TWOPIN:
			for(int i=0,s=pNetList->size();i<s;i++)
			{
				CNet*	pNet	=	pNetList->at(i);
				if(pNet->IsFlat()&&pNet->GetProp()&PROP_NET_TWOPIN)
					pNet->GetWireInDirection(pWireList,iPos,iDirection);
			}
			break;
	}


	return	pWireList->size();
}

