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
// Pin.cpp: implementation of the CPin class.
//
//////////////////////////////////////////////////////////////////////

#include "Net.h"
#include "Pin.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPin::CPin()
{

}

CPin::~CPin()
{

}

void CPin::Initialize(int iIndex, int iX, int iY, int iZ, CNet *pParent)
{
	SetXYZ(iX,iY,iZ);
	m_pParent	=	(CObject*)pParent;

	CreateKey();

	if(iX>=GetDesign()->W()||iY>=GetDesign()->H()||iZ>GetDesign()->T()||iX<GetDesign()->X()||iY<GetDesign()->Y()||iZ<GetDesign()->Z())
	{
		Display(DISPLAY_MODE_ERRO,"net (id:%d) has a pin outside the circuit (%d,%d,%d)\n",(int)GetParent()->GetKey(),iX,iY,iZ);
	}
}

CNet* CPin::GetParent()
{
	return	(CNet*)m_pParent;
}

//void CPin::Print(FILE *pFile, int iMode)
//{
//	if(pFile==NULL)	return;
//
//	switch(iMode){
//	case PRINT_MODE_GNUPLOT:
//		fprintf(pFile,"plot '-' w points pt 4 ps 1.5\n %d %d\n %d %d\ne\n",
//			X(),Y(),
//			X(),Y());
//		break;
//	case PRINT_MODE_GNUPLOT3D:
//		fprintf(pFile,"splot '-' w points pt 4 ps 1.5\n %d %d %d\n %d %d %d\ne\n",
//			X(),Y(),Z(),
//			X(),Y(),Z());
//		break;
//	case PRINT_MODE_MATLAB:
//		fprintf(pFile,"plot([%d,%d],[%d,%d],'.');\n",
//			X(),X(),
//			Y(),Y());
//		break;
//	case PRINT_MODE_MATLAB3D:
//		fprintf(pFile,"plot3([%d,%d],[%d,%d],[%d,%d],'.');\n",
//			X(),X(),
//			Y(),Y(),
//			Z(),Z());
//		break;
//	case PRINT_MODE_TEXT:
//		fprintf(pFile,"p(%d,%d,%d)\n",
//			X(),Y(),Z());
//		break;
//	default:
//		assert(FALSE);
//		break;
//	}
//
//	fflush(pFile);
//}
