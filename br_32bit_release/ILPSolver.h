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
// ILPSolver.h: interface for the ILPSolver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ILPSOLVER_H__2C943DAA_6F90_46D5_A9A9_76A18F28190B__INCLUDED_)
#define AFX_ILPSOLVER_H__2C943DAA_6F90_46D5_A9A9_76A18F28190B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <assert.h>


//this is merely wrapper class of GLPK apis
//refer GLPK manual directly
extern "C" {
#include "glpk-4.10/include/glpk.h"
}

#ifdef WIN32
#pragma comment(lib, "glpk-4.10/glpk.lib")
//#pragma comment(lib, "glpk-4.10/glpkd.lib")
//#pragma comment(lib, "glpk-4.13/w32/glpk413.lib")
#endif

#define	SOL_OPTION_SIMPLEX		(0x0010000)
#define	SOL_OPTION_IPT			(0x0020000)
#define	SOL_OPTION_MIP			(0x0100000)
#define SOL_OPTION_MIP_COVER	(SOL_OPTION_MIP|LPX_C_COVER)
#define SOL_OPTION_MIP_CLIQUE	(SOL_OPTION_MIP|LPX_C_CLIQUE)
#define SOL_OPTION_MIP_GOMORY	(SOL_OPTION_MIP|LPX_C_GOMORY)

#define	SOL_OPTION_MAX			(0x1000000)
#define	SOL_OPTION_MIN			(0x2000000)

#define	SOL_FORMAT_CPLEX		(0x0000000)
#define SOL_FORMAT_FREEMPS		(0x0000001)
#define	SOL_PRINT_PRO			(0x0000002)
#define	SOL_PRINT_SOL			(0x0000003)

class CILPSolver  
{
public:
	void RoundUp(double dThreshold);
	int GetNumConstraint();
	int GetNumVariable();
	void SetCoeff(int iIndex, int iValue);
	void SetCoeff(int iIndex, double dValue);
	void SetCoeffIndex(int iIndex, int iValue);
	double GetSolution(int iIndex);
	int Solve(int iOption,double* pObjectValue);
	void Print(char* pFileName,int iOptioin);
	void Read(char* pFileName,int iOption=SOL_FORMAT_CPLEX);
	int Write(char* pFileName,int iOption=SOL_FORMAT_CPLEX);
	void Sample();
	void Sample2();
	int Solve(int iOption,double* pObjectValue,double* pVariableValue);
	int AddConstraint(char *pName, int iNumCoeff, int iLowerBound, double dLowerBound, int iUpperBound, double dUpperBound);
	int AddConstraint(char *pName, int* pCoeffIndex, double* pCoeff, int iNumCoeff, int iLowerBound, double dLowerBound, int iUpperBound, double dUpperBound);
	int AddVariable(char* pName, double dCoeff, int iLowerBound,double dLowerBound,int iUpperBound,double dUpperBound);
	int IsCreated();
	void Delete();
	void Create(char* pName=0x00);
	CILPSolver();
	virtual ~CILPSolver();

	void			PrepareCoeffBuffer(int iNumCoeff, int iNumVariable=0);
	void			PrepareSolutionBuffer(int iNumSolution);
	int				m_iNumSolution;
	int				m_iNumCoeff;
	int*			m_pCoeffIndex;
	double*			m_pCoeff;
	double*			m_pSolution;	
private:
	LPX*		m_pLPX;
};

#endif // !defined(AFX_ILPSOLVER_H__2C943DAA_6F90_46D5_A9A9_76A18F28190B__INCLUDED_)

