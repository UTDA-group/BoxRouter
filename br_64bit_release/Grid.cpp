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
// Grid.cpp: implementation of the CGrid class.
//
//////////////////////////////////////////////////////////////////////

#include "Grid.h"
#include "Layer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGrid::CGrid()
{
	//	m_iNumPin				=	0;
	m_pParent				=	NULL;
}

CGrid::~CGrid()
{
}

void CGrid::Initialize(int iX, int iY, int iZ, CLayer *pParent)
{
	assert(iZ>=LAYER_METAL1);

	SetXYZ(iX,iY,iZ);

	m_pParent		=	(CObject*)pParent;

	CreateKey();

	memset(m_pAdjGrid,0x00,MAX_ADJ_GRID*sizeof(CGrid*));
}

CLayer* CGrid::GetParent()
{
	assert(m_pParent);
	return	(CLayer*)m_pParent;
}

CGrid* CGrid::GetAdjGrid(int iIndex)
{
	if(m_pAdjGrid[0]==NULL)
	{
		// collect adjacent grids [6/24/2006 thyeros]
		CLayer*		pLayer		=	GetParent();
		CBoundary*	pBoundary	=	NULL;
		CGrid*		pGrid		=	NULL;

		int	i	=	0;

		// same layer [6/24/2006 thyeros]
		if(pLayer->IsHorizontal())
		{
			pBoundary	=	pLayer->GetBoundary(this,X()-1,Y());
			pGrid		=	pLayer->GetGrid(X()-1,Y());
			if(pBoundary)
			{
				m_pAdjGrid[i]	=	pGrid;
				assert(m_pAdjGrid[i]);
				++i;
			}

			pBoundary	=	pLayer->GetBoundary(this,X()+1,Y());
			pGrid		=	pLayer->GetGrid(X()+1,Y());
			if(pBoundary)
			{
				m_pAdjGrid[i]	=	pGrid;
				assert(m_pAdjGrid[i]);
				++i;
			}
		}
		else
		{
			pBoundary	=	pLayer->GetBoundary(this,X(),Y()-1);
			pGrid		=	pLayer->GetGrid(X(),Y()-1);
			if(pBoundary)
			{
				m_pAdjGrid[i]	=	pGrid;
				assert(m_pAdjGrid[i]);
				++i;
			}

			pBoundary	=	pLayer->GetBoundary(this,X(),Y()+1);
			pGrid		=	pLayer->GetGrid(X(),Y()+1);
			if(pBoundary)
			{
				m_pAdjGrid[i]	=	pGrid;
				assert(m_pAdjGrid[i]);
				++i;
			}
		}

		// top and bottom layer [6/24/2006 thyeros]
		if(pLayer->Z()>LAYER_METAL1)		m_pAdjGrid[i++]	=	GetDesign()->GetLayer(pLayer->Z()-1)->GetGrid(X(),Y());
		if(pLayer->Z()<GetDesign()->T())	m_pAdjGrid[i++]	=	GetDesign()->GetLayer(pLayer->Z()+1)->GetGrid(X(),Y());
	}

	assert(iIndex>=0&&iIndex<MAX_ADJ_GRID);

	return	m_pAdjGrid[iIndex];
}

void CGrid::Print(FILE *pFile, int iMode)
{
	if(pFile==NULL)	return;

	switch(iMode) {
	case PRINT_MODE_GNUPLOT:
		fprintf(pFile,"plot '-' w lines lt 1\n %.1f %.1f\n %.1f %.1f\n\n %.1f %.1f\n %.1f %.1f\ne\n",
			X()-0.5,Y()-0.5,
			X()+0.5,Y()-0.5,
			X()-0.5,Y()-0.5,
			X()-0.5,Y()+0.5);
		break;
	case PRINT_MODE_GNUPLOT3D:
		fprintf(pFile,"splot '-' w lines lt 1\n %.1f %.1f %d\n %.1f %.1f %d\n\n %.1f %.1f %d\n %.1f %.1f %d\ne\n",
			X()-0.5,Y()-0.5,Z(),
			X()+0.5,Y()-0.5,Z(),
			X()-0.5,Y()-0.5,Z(),
			X()-0.5,Y()+0.5,Z());
		if(Z()==1)
		{
			fprintf(pFile,"splot '-' w lines lt 1\n %.1f %.1f %d\n %.1f %.1f %d\n\n %.1f %.1f %d\n %.1f %.1f %d\ne\n",
				X()-0.5,Y()-0.5,0,
				X()+0.5,Y()-0.5,0,
				X()-0.5,Y()-0.5,0,
				X()-0.5,Y()+0.5,0);
		}		
		break;
	case PRINT_MODE_MATLAB:
		fprintf(pFile,"plot([%.1f,%.1f],[%.1f,%.1f],'%s');\n",
			X()-0.5,X()+0.5,
			Y()-0.5,Y()-0.5,
			"-r");
		fprintf(pFile,"plot([%.2f,%.2f],[%.2f,%.2f],'%s');\n",
			X()-0.5,X()-0.5,
			Y()-0.5,Y()+0.5,
			"-r");
		break;
	case PRINT_MODE_MATLAB3D:
		fprintf(pFile,"plot3([%.1f,%.1f],[%.1f,%.1f],[%d,%d],'%s');\n",
			X()-0.5,X()+0.5,
			Y()-0.5,Y()-0.5,
			Z(),Z(),
			"-r");
		fprintf(pFile,"plot3([%.2f,%.2f],[%.2f,%.2f],[%d,%d],'%s');\n",
			X()-0.5,X()-0.5,
			Y()-0.5,Y()+0.5,
			Z(),Z(),
			"-r");
		if(Z()==1)
		{
			fprintf(pFile,"plot3([%.1f,%.1f],[%.1f,%.1f],[%d,%d],'%s');\n",
				X()-0.5,X()+0.5,
				Y()-0.5,Y()-0.5,
				0,0,
				"-r");
			fprintf(pFile,"plot3([%.2f,%.2f],[%.2f,%.2f],[%d,%d],'%s');\n",
				X()-0.5,X()-0.5,
				Y()-0.5,Y()+0.5,
				0,0,
				"-r");
		}
		break;
	case PRINT_MODE_CGDUMP:
		fprintf(pFile,"%s(%3d,%3d,%3d) %.2f\t",GetParent()->IsHorizontal()? "H":"V",X(),Y(),Z(),GetCongestion()/100.0);
		break;
	//case PRINT_MODE_RESULT:
	//	fprintf(pFile,"b %d %d %d\t%d\n",X(),Y(),Z(),GetNumWire(GET_MODE_SPECIAL));
	//	break;
	case PRINT_MODE_OFDUMP:
		if(GetOverFlow())
		{
			fprintf(pFile,"bnd(%d,%d,%d): %d OF -%d\n",
				X(),Y(),Z(),GetOverFlow(),GetPenalty());
		}
		break;
	case PRINT_MODE_TEXT:
		{
#ifdef _DEBUG
			fprintf(pFile,"grid(%d,%d,%d)\n",X(),Y(),Z());
#else
			fprintf(pFile,"grid(%d,%d,%d) 0x%x\n",X(),Y(),Z(),this);
#endif
			
			fprintf(pFile,"bnd(%d,%d,%d): ocu_cap:%d\tavb_cap:%d\tmax_cap:%d\toverflow:%d\n",
				X(),Y(),Z(),
				GetCapacity(GET_MODE_OCAP),
				GetCapacity(GET_MODE_ACAP),
				GetParent()->GetCapacity(GET_MODE_MAX),
				GetOverFlow());

			for(vector<CWire*>::iterator itr=m_Wire.begin(),end=m_Wire.end();itr!=end;++itr)	(*itr)->Print(pFile,iMode);
		}
		break;
	default:
		assert(FALSE);
		break;
	}
	fflush(pFile);
}
