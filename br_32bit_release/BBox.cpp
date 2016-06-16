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
// BBox.cpp: implementation of the CBBox class.
//
//////////////////////////////////////////////////////////////////////

#include "BBox.h"
#include "Layer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBBox::CBBox()
{
	m_iMinX	=	MAX_NUMBER;
	m_iMaxX	=	0;
	m_iMinY	=	MAX_NUMBER;
	m_iMaxY	=	0;
	m_cMinZ	=	MAX_NUMBER;
	m_cMaxZ	=	0;
}

CBBox::~CBBox()
{

}

void CBBox::Initialize(CPoint* pPointS, CPoint* pPointE)
{
	Initialize(pPointS->X(),pPointS->Y(),pPointS->Z(),pPointE->X(),pPointE->Y(),pPointE->Z());
}

int CBBox::IsInside(int iX, int iY)
{
	return	iX>=m_iMinX&&iX<=m_iMaxX&&iY>=m_iMinY&&iY<=m_iMaxY;
}

int CBBox::IsInside(int iX, int iY, int iZ)
{
	return	iX>=m_iMinX&&iX<=m_iMaxX&&iY>=m_iMinY&&iY<=m_iMaxY&&iZ>=m_cMinZ&&iZ<=m_cMaxZ;
}

int CBBox::IsInside(CPoint *pPoint)
{
	return	IsInside(pPoint->X(),pPoint->Y());
}

void CBBox::Expand(int iMaxX, int iMaxY, int iStep)
{
	m_iMinX	=	MAX(0,m_iMinX-iStep);
	m_iMinY	=	MAX(0,m_iMinY-iStep);
	m_iMaxX	=	MIN(iMaxX,m_iMaxX+iStep);
	m_iMaxY	=	MIN(iMaxY,m_iMaxY+iStep);
}

void CBBox::Initialize(int iX1, int iY1, int iZ1, int iX2, int iY2, int iZ2)
{
	m_iMaxX	=	MAX(iX1,iX2);
	m_iMinX	=	MIN(iX1,iX2);
	m_iMaxY	=	MAX(iY1,iY2);
	m_iMinY	=	MIN(iY1,iY2);
	m_cMaxZ	=	MAX(iZ1,iZ2);
	m_cMinZ	=	MIN(iZ1,iZ2);
}

int CBBox::IsInside(CBBox *pBBox)
{
	return	X()<=pBBox->X()&&Y()<=pBBox->Y()&&Z()<=pBBox->Z()
			&&m_iMaxX>=pBBox->m_iMaxX
			&&m_iMaxY>=pBBox->m_iMaxY
			&&m_cMaxZ>=pBBox->m_cMaxZ;
}

int CBBox::IsOverlapped(CBBox *pBBox)
{
	if(IsInside(pBBox)||pBBox->IsInside(this))	return	TRUE;
	if(pBBox->X()+pBBox->W()<m_iMinX)			return	FALSE;
	if(pBBox->Y()+pBBox->H()<m_iMinY)			return	FALSE;
	if(pBBox->Z()+pBBox->T()<m_cMinZ)			return	FALSE;
	if(pBBox->X()>m_iMaxX)						return	FALSE;
	if(pBBox->Y()>m_iMaxY)						return	FALSE;
	if(pBBox->Z()>m_cMaxZ)						return	FALSE;

	return	FALSE;
}

int CBBox::IsOverlapped(CWire* pWire)
{
	if((pWire->m_pPointS->X()<=m_iMinX&&pWire->m_pPointE->X()<=m_iMinX)||
		(pWire->m_pPointS->X()>=m_iMaxX&&pWire->m_pPointE->X()>=m_iMaxX)||
		(pWire->m_pPointS->Y()<=m_iMinY&&pWire->m_pPointE->Y()<=m_iMinY)||
		(pWire->m_pPointS->Y()>=m_iMaxY&&pWire->m_pPointE->Y()>=m_iMaxY))	
		return	FALSE;


	return	TRUE;
}

CPoint CBBox::GetCenter()
{
	int	iX	=	(m_iMinX+m_iMaxX)/2;
	int	iY	=	(m_iMinY+m_iMaxY)/2;
	int	iZ	=	(m_cMinZ+m_cMaxZ)/2;

	CPoint	Ret;
	Ret.Initialize(iX,iY,iZ,NULL);

	return	Ret;
}

void CBBox::AddPoint(int iX, int iY, int iZ)
{
	m_iMaxX	=	MAX(m_iMaxX,iX);
	m_iMinX	=	MIN(m_iMinX,iX);
	m_iMaxY	=	MAX(m_iMaxY,iY);
	m_iMinY	=	MIN(m_iMinY,iY);
	m_cMaxZ	=	MAX(m_cMaxZ,iZ);
	m_cMinZ	=	MIN(m_cMinZ,iZ);
}

void CBBox::AddPoint(CPoint* pPoint)
{
	AddPoint(pPoint->X(),pPoint->Y(),pPoint->Z());
}

int CBBox::Distance(CPoint *pPoint)
{
	if(pPoint->X()<=m_iMaxX&&pPoint->X()>=m_iMinX)
	{
		return	MIN(abs(pPoint->Y()-m_iMinY),abs(pPoint->Y()-m_iMaxY));
	}
	else if(pPoint->Y()<=m_iMaxY&&pPoint->Y()>=m_iMinY)
	{
		return	MIN(abs(pPoint->X()-m_iMinX),abs(pPoint->X()-m_iMaxX));
	}
	else if(IsInside(pPoint->X(),pPoint->Y()))
	{
		return	0;
	}

	return	MIN(abs(pPoint->X()-m_iMinX),abs(pPoint->X()-m_iMaxX))+MIN(abs(pPoint->Y()-m_iMinY),abs(pPoint->Y()-m_iMaxY));
}

void CBBox::Print(FILE *pFile, int iMode)
{
	if(pFile==NULL)	return;

	switch(iMode) {
	case PRINT_MODE_MATLAB:
		fprintf(pFile,"plot([%d,%d],[%d,%d],'%s');\n",
			X(),X()+W(),
			Y(),Y(),
			"-r");
		fprintf(pFile,"plot([%d,%d],[%d,%d],'%s');\n",
			X(),X()+W(),
			Y()+H(),Y()+H(),
			"-r");
		fprintf(pFile,"plot([%d,%d],[%d,%d],'%s');\n",
			X(),X(),
			Y(),Y()+H(),
			"-r");
		fprintf(pFile,"plot([%d,%d],[%d,%d],'%s');\n",
			X()+W(),X()+W(),
			Y(),Y()+H(),
			"-r");
		break;

		/*fprintf(pFile,"plot([%.1f,%.1f],[%.1f,%.1f],'%s');\n",
			X()-0.5,X()+0.5,
			Y()-0.5,Y()-0.5,
			"-r");
		fprintf(pFile,"plot([%.2f,%.2f],[%.2f,%.2f],'%s');\n",
			X()-0.5,X()-0.5,
			Y()-0.5,Y()+0.5,
			"-r");
*/
	//case PRINT_MODE_MATLAB3D:
	//	fprintf(pFile,"plot3([%.1f,%.1f],[%.1f,%.1f],[%d,%d],'%s');\n",
	//		X()-0.5,X()+0.5,
	//		Y()-0.5,Y()-0.5,
	//		Z(),Z(),
	//		"-r");
	//	fprintf(pFile,"plot3([%.2f,%.2f],[%.2f,%.2f],[%d,%d],'%s');\n",
	//		X()-0.5,X()-0.5,
	//		Y()-0.5,Y()+0.5,
	//		Z(),Z(),
	//		"-r");
	//	if(Z()==1)
	//	{
	//		fprintf(pFile,"plot3([%.1f,%.1f],[%.1f,%.1f],[%d,%d],'%s');\n",
	//			X()-0.5,X()+0.5,
	//			Y()-0.5,Y()-0.5,
	//			0,0,
	//			"-r");
	//		fprintf(pFile,"plot3([%.2f,%.2f],[%.2f,%.2f],[%d,%d],'%s');\n",
	//			X()-0.5,X()-0.5,
	//			Y()-0.5,Y()+0.5,
	//			0,0,
	//			"-r");
	//	}
		break;
	default:
		assert(FALSE);
		break;
	}
	fflush(pFile);
}
