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
// CILPSolver.cpp: implementation of the CILPSolver class.
//
//////////////////////////////////////////////////////////////////////

#include "ILPSolver.h"
#include <stdio.h>
#include <memory.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CILPSolver::CILPSolver()
{
	m_pLPX				=	0x00;
	m_iNumSolution		=	0;
	m_iNumCoeff			=	0;
	m_pCoeff			=	NULL;
	m_pCoeffIndex		=	NULL;
	m_pSolution			=	NULL;
}

CILPSolver::~CILPSolver()
{
	Delete();
	
	if(m_pCoeff)		delete[] m_pCoeff;
	if(m_pCoeffIndex)	delete[] m_pCoeffIndex;
	if(m_pSolution)		delete[] m_pSolution;
}

void CILPSolver::Create(char* pName)
{
	assert(m_pLPX==0x00);
	
	m_pLPX	=	lpx_create_prob();
	if(pName)	lpx_set_prob_name(m_pLPX,pName);
}

void CILPSolver::Delete()
{
	if(IsCreated())
	{
		lpx_delete_prob(m_pLPX);
		m_pLPX	=	0x00;
	}
}

int CILPSolver::IsCreated()
{
	return	(m_pLPX!=0x00);
}

int CILPSolver::AddVariable(char *pName, double dCoeff, int iLowerBound, double dLowerBound, int iUpperBound, double dUpperBound)
{
	int	iNumVariable	=	lpx_get_num_cols(m_pLPX)+1;
	int	iBoundType		=	-1;
	
	if(iLowerBound&&iUpperBound)		iBoundType	=	(dLowerBound==dUpperBound)? LPX_FX:LPX_DB;
	else if(!iLowerBound&&!iUpperBound)	iBoundType	=	LPX_FR;
	else								iBoundType	=	(iLowerBound)?	LPX_LO:LPX_UP;
	
	assert(iBoundType>0);
	
	lpx_add_cols(m_pLPX,1);	
	
	if(pName)
	{
		lpx_set_col_name(m_pLPX,iNumVariable,pName);
	}
	else
	{
		char	cName[100];
		sprintf(cName,"X%d",iNumVariable);
		lpx_set_col_name(m_pLPX,iNumVariable,cName);
	}
	
	lpx_set_col_bnds(m_pLPX,iNumVariable,iBoundType,dLowerBound,dUpperBound);
	lpx_set_obj_coef(m_pLPX,iNumVariable,dCoeff);
	
	assert(lpx_get_num_cols(m_pLPX)==iNumVariable);
	
	return	iNumVariable;
}

int CILPSolver::AddConstraint(char *pName, int iNumCoeff, int iLowerBound, double dLowerBound, int iUpperBound, double dUpperBound)
{
	return	AddConstraint(pName,m_pCoeffIndex,m_pCoeff,iNumCoeff,iLowerBound,dLowerBound,iUpperBound,dUpperBound);
}

int CILPSolver::AddConstraint(char *pName, int* pCoeffIndex, double *pCoeff, int iNumCoeff, int iLowerBound, double dLowerBound, int iUpperBound, double dUpperBound)
{
	int	iNumConstraint	=	lpx_get_num_rows(m_pLPX)+1;
	int	iBoundType		=	-1;
	
	if(iLowerBound&&iUpperBound)		iBoundType	=	(dLowerBound==dUpperBound)? LPX_FX:LPX_DB;
	else if(!iLowerBound&&!iUpperBound)	iBoundType	=	LPX_FR;
	else								iBoundType	=	(iLowerBound)?	LPX_LO:LPX_UP;
	
	assert(iBoundType>0);
	
	lpx_add_rows(m_pLPX,1);	
	
	if(pName)
	{
		lpx_set_row_name(m_pLPX,iNumConstraint,pName);
	}
	else
	{
		char	cName[100];
		sprintf(cName,"Y%d",iNumConstraint);
		lpx_set_row_name(m_pLPX,iNumConstraint,cName);
	}
	
	lpx_set_row_bnds(m_pLPX,iNumConstraint,iBoundType,dLowerBound,dUpperBound);	
	lpx_set_mat_row(m_pLPX,iNumConstraint,iNumCoeff,pCoeffIndex,pCoeff);
		
	assert(lpx_get_num_rows(m_pLPX)==iNumConstraint);
		
	return	iNumConstraint;
}

int CILPSolver::Solve(int iOption,double* pObjectValue)
{
	return	Solve(iOption,pObjectValue,m_pSolution);
}

int CILPSolver::Solve(int iOption,double* pObjectValue,double* pVariableValue)
{
	lpx_set_obj_dir(m_pLPX, iOption&SOL_OPTION_MAX? LPX_MAX:LPX_MIN);

	iOption	&=	~SOL_OPTION_MAX;
	iOption	&=	~SOL_OPTION_MIN;
	
	int	iResult;

	lpx_set_int_parm(m_pLPX,LPX_K_ROUND,1);
	lpx_set_int_parm(m_pLPX,LPX_K_MSGLEV,1);
	lpx_set_int_parm(m_pLPX,LPX_K_OUTFRQ,1000);
	
	switch(iOption&0xFFFF0000) {
	case SOL_OPTION_SIMPLEX:
		lpx_set_int_parm(m_pLPX,LPX_K_PRESOL,1);
		lpx_adv_basis(m_pLPX);
		iResult	=	lpx_simplex(m_pLPX);
		if(iResult==LPX_E_OK)
		{
			if(pObjectValue)	*pObjectValue	=	lpx_get_obj_val(m_pLPX);
			if(pVariableValue)	
			{
				pVariableValue[0]	=	lpx_get_num_cols(m_pLPX);
				for(int i=1;i<=pVariableValue[0];++i)
				{
					pVariableValue[i]	=	lpx_get_col_prim(m_pLPX,i);
				}				
			}
		}
		break;
	case SOL_OPTION_IPT:
		iResult	=	lpx_interior(m_pLPX);
		if(iResult==LPX_E_OK)
		{
			if(pObjectValue)	*pObjectValue	=	lpx_ipt_obj_val(m_pLPX);
			if(pVariableValue)	
			{
				pVariableValue[0]	=	lpx_get_num_cols(m_pLPX);
				for(int i=1;i<=pVariableValue[0];++i)
				{
					pVariableValue[i]	=	lpx_ipt_col_prim(m_pLPX,i);
				}				
			}
		}
		break;
	case SOL_OPTION_MIP:
		{
			lpx_set_class(m_pLPX,LPX_MIP);

			int	iNumVariable	=	lpx_get_num_cols(m_pLPX);			
			for(int i=1;i<=iNumVariable;++i)	lpx_set_col_kind(m_pLPX,i,LPX_IV);

			lpx_set_int_parm(m_pLPX,LPX_K_USECUTS,LPX_C_ALL);
			lpx_set_real_parm(m_pLPX,LPX_K_TMLIM,1800);	
			lpx_set_int_parm(m_pLPX,LPX_K_ROUND,1);
			lpx_set_int_parm(m_pLPX,LPX_K_MSGLEV,1);
			lpx_set_int_parm(m_pLPX,LPX_K_OUTFRQ,1000);

			iResult	=	lpx_intopt(m_pLPX);
			if(iResult==LPX_E_OK)
			{
				if(pObjectValue)	*pObjectValue	=	lpx_mip_obj_val(m_pLPX);
				if(pVariableValue)	
				{
					pVariableValue[0]	=	lpx_get_num_cols(m_pLPX);	
					for(int i=1;i<=pVariableValue[0];++i)	pVariableValue[i]	=	lpx_mip_col_val(m_pLPX,i);
				}
			}	
			else
			{
				printf("\n\tlpx_intopt..failed (%d)\n",iResult);
			}
		}
		break;
	default:
		assert(false);
		break;
	}
	
	switch(iResult) {
	case LPX_E_OK     : 
		return	1;
		break;
	case LPX_E_EMPTY  : printf("ILP error: empty problem\n");break;
	case LPX_E_BADB   : printf("ILP error: invalid initial basis\n");break;
	case LPX_E_INFEAS : printf("ILP error: infeasible initial solution\n");break;
	case LPX_E_FAULT  : printf("ILP error: unable to start the search\n");break;
	case LPX_E_OBJLL  : printf("ILP error: objective lower limit reached\n");break;
	case LPX_E_OBJUL  : printf("ILP error: objective upper limit reached\n");break;
	case LPX_E_ITLIM  : printf("ILP error: iterations limit exhausted\n");break;
	case LPX_E_TMLIM  : printf("ILP error: time limit exhausted\n");break;
	case LPX_E_NOFEAS : printf("ILP error: no feasible solution\n");break;
	case LPX_E_INSTAB : printf("ILP error: numerical instability\n");break;
	case LPX_E_SING   : printf("ILP error: problems with basis matrix\n");break;
	case LPX_E_NOCONV : printf("ILP error: no convergence (interior)\n");break;
	case LPX_E_NOPFS  : printf("ILP error: no primal feas. sol. (LP presolver)\n");break;
	case LPX_E_NODFS  : printf("ILP error: no dual feas. sol. (LP presolver)\n");break;
	default:			printf("ILP error: unknown error code (%d)\n",iResult);break;
	}

	return	0;
}

void CILPSolver::Sample()
{
	Create(); 
	
	int	iX1	=	AddVariable("X1",10,1,0,0,0);	assert(lpx_get_col_type(m_pLPX,iX1)==LPX_LO);
	int	iX2	=	AddVariable("X2",6,1,0,0,0);		assert(lpx_get_col_type(m_pLPX,iX2)==LPX_LO);
	int	iX3	=	AddVariable("X3",4,1,0,0,0);		assert(lpx_get_col_type(m_pLPX,iX2)==LPX_LO);
	
	double	dCoeff[4];
	int		iCoeffIndex[4];
	int		iIndex;
	
	iIndex				=	1;
	iCoeffIndex[iIndex]	=	1;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	2;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	3;		dCoeff[iIndex++]	=	1;
	
	int	iP	=	AddConstraint("p",iCoeffIndex,dCoeff,iIndex-1,0,0,1,100);	assert(lpx_get_row_type(m_pLPX,iP)==LPX_UP);
	
	iIndex				=	1;
	iCoeffIndex[iIndex]	=	1;		dCoeff[iIndex++]	=	10;
	iCoeffIndex[iIndex]	=	2;		dCoeff[iIndex++]	=	4;
	iCoeffIndex[iIndex]	=	3;		dCoeff[iIndex++]	=	5;
	
	int	iQ	=	AddConstraint("q",iCoeffIndex,dCoeff,iIndex-1,0,0,1,600);	assert(lpx_get_row_type(m_pLPX,iQ)==LPX_UP);
	
	iIndex				=	1;
	iCoeffIndex[iIndex]	=	1;		dCoeff[iIndex++]	=	2;
	iCoeffIndex[iIndex]	=	2;		dCoeff[iIndex++]	=	2;
	iCoeffIndex[iIndex]	=	3;		dCoeff[iIndex++]	=	6;
	
	int iR	=	AddConstraint("r",iCoeffIndex,dCoeff,iIndex-1,0,0,1,300);	assert(lpx_get_row_type(m_pLPX,iR)==LPX_UP);
	
	
	double	dObj	=	0;
	Solve(SOL_OPTION_IPT|SOL_OPTION_MAX,&dObj,dCoeff);
	
	printf("Z = %f, X1:%f, X2:%f, X3:%f\n",dObj,dCoeff[iX1],dCoeff[iX2],dCoeff[iX3]);
}

void CILPSolver::Sample2()
{
	Create(); 
	
	int	iX11	=	AddVariable("X11",2,1,0,1,1);	
	int	iX12	=	AddVariable("X12",3,1,0,1,1);		
	int	iX13	=	AddVariable("X13",3,1,0,1,1);		

	int	iX21	=	AddVariable("X21",2,1,0,1,1);	
	int	iX22	=	AddVariable("X22",3,1,0,1,1);		
	int	iX23	=	AddVariable("X23",3,1,0,1,1);		
	
	int	iX31	=	AddVariable("X31",2,1,0,1,1);	
	int	iX32	=	AddVariable("X32",2,1,0,1,1);		

	double	dCoeff[10];
	int		iCoeffIndex[10];
	int		iIndex;
	
	iIndex				=	1;
	iCoeffIndex[iIndex]	=	1;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	2;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	3;		dCoeff[iIndex++]	=	1;
	
	int	iX1	=	AddConstraint("X1",iCoeffIndex,dCoeff,iIndex-1,1,1,1,1);	
	
	iIndex				=	1;
	iCoeffIndex[iIndex]	=	4;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	5;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	6;		dCoeff[iIndex++]	=	1;

	int	iX2	=	AddConstraint("X2",iCoeffIndex,dCoeff,iIndex-1,1,1,1,1);	
	
	iIndex				=	1;
	iCoeffIndex[iIndex]	=	7;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	8;		dCoeff[iIndex++]	=	1;
	
	int iX3	=	AddConstraint("X3",iCoeffIndex,dCoeff,iIndex-1,1,1,1,1);	

	iIndex				=	1;
	iCoeffIndex[iIndex]	=	2;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	3;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	4;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	6;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	7;		dCoeff[iIndex++]	=	1;

	AddConstraint("p",iCoeffIndex,dCoeff,iIndex-1,1,0,1,2);	

	iIndex				=	1;
	iCoeffIndex[iIndex]	=	1;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	3;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	4;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	5;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	6;		dCoeff[iIndex++]	=	1;

	AddConstraint("q",iCoeffIndex,dCoeff,iIndex-1,1,0,1,2);	

	iIndex				=	1;
	iCoeffIndex[iIndex]	=	2;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	3;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	4;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	5;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	8;		dCoeff[iIndex++]	=	1;

	AddConstraint("r",iCoeffIndex,dCoeff,iIndex-1,1,0,1,2);	
	
	iIndex				=	1;
	iCoeffIndex[iIndex]	=	1;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	2;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	5;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	6;		dCoeff[iIndex++]	=	1;
	iCoeffIndex[iIndex]	=	8;		dCoeff[iIndex++]	=	1;

	AddConstraint("s",iCoeffIndex,dCoeff,iIndex-1,1,0,1,2);	
	
	double	dObj	=	0;
	Solve(SOL_OPTION_MIP|SOL_OPTION_MAX,&dObj,dCoeff);
	
	printf("Z = %f\nX11:%f, X12:%f, X13:%f\nX21:%f, X22:%f, X23:%f\nX31:%f, X32:%f\n"
													,dObj
													,dCoeff[iX11],dCoeff[iX12],dCoeff[iX13]
													,dCoeff[iX21],dCoeff[iX22],dCoeff[iX23]
													,dCoeff[iX31],dCoeff[iX32]);
}


int CILPSolver::Write(char *pFileName, int iOption)
{
	switch(iOption) {
	case SOL_FORMAT_CPLEX:		return	lpx_write_cpxlp(m_pLPX,pFileName);
	case SOL_FORMAT_FREEMPS:	return	lpx_write_freemps(m_pLPX,pFileName);
	default:
		assert(false);
		break;
	}

	return	-1;
}

void CILPSolver::Read(char *pFileName, int iOption)
{
	Delete();

	switch(iOption) {
	case SOL_FORMAT_CPLEX:	
		m_pLPX	=	lpx_read_cpxlp(pFileName);	
		break;
	default:
		assert(false);
		break;
	}

	iOption	=	0;
	
	if(lpx_get_obj_dir(m_pLPX)==LPX_MAX)		iOption	|=	SOL_OPTION_MAX;
	else if(lpx_get_obj_dir(m_pLPX)==LPX_MIN)	iOption	|=	SOL_OPTION_MIN;
	
	if(lpx_get_class(m_pLPX)==LPX_LP)			iOption	|=	SOL_OPTION_SIMPLEX;
	else if(lpx_get_class(m_pLPX)==LPX_MIP)		iOption	|=	SOL_OPTION_MIP;

	Solve(iOption,0x00,0x00);
}

void CILPSolver::Print(char* pFileName, int iOptioin)
{
	switch(iOptioin) {
	case SOL_PRINT_PRO:	
		lpx_print_prob(m_pLPX,pFileName);
		break;
	case SOL_PRINT_SOL:	
		lpx_print_mip(m_pLPX,pFileName);
		break;
	default:
		assert(0);
		break;
	}
}

void CILPSolver::PrepareSolutionBuffer(int iNumSolution)
{
	if(iNumSolution>m_iNumSolution)
	{
#define SAFE_DELA(p)				{if(p) {delete[] p; p=NULL;}}
		SAFE_DELA(m_pSolution);

		m_iNumSolution	=	iNumSolution;
		m_pSolution		=	new	double[m_iNumSolution+1];

		assert(m_pSolution);

#ifdef _DEBUG
		m_pSolution[0]					=	-99;
		m_pSolution[m_iNumSolution]		=	-99;
#endif
	}
}


void CILPSolver::PrepareCoeffBuffer(int iNumCoeff, int iNumVariable)
{
	assert(iNumCoeff);

	if(iNumCoeff>m_iNumCoeff)
	{
		SAFE_DELA(m_pCoeff);
		SAFE_DELA(m_pCoeffIndex);

		m_iNumCoeff		=	iNumCoeff;
		m_pCoeff		=	new	double[m_iNumCoeff+1];
		m_pCoeffIndex	=	new	int[m_iNumCoeff+1];

		assert(m_pCoeff);
		assert(m_pCoeffIndex);
	}

	if(iNumVariable)
	{
		for(int i=0;i<iNumCoeff+1;++i)	m_pCoeff[i]	=	1;
		for(int iIndex=1,i=iNumVariable-iNumCoeff+2;i<=iNumVariable;++i)	m_pCoeffIndex[iIndex++]	=	i;		
	}

#ifdef _DEBUG
	m_pCoeffIndex[0]			=	-99;
	m_pCoeffIndex[m_iNumCoeff]	=	-99;
	m_pCoeff[0]					=	-99;
	m_pCoeff[m_iNumCoeff]		=	-99;
#endif
}

double CILPSolver::GetSolution(int iIndex)
{
	return	m_pSolution[iIndex];
}

void CILPSolver::SetCoeffIndex(int iIndex, int iValue)
{
	m_pCoeffIndex[iIndex]	=	iValue;
}

void CILPSolver::SetCoeff(int iIndex, int iValue)
{
	m_pCoeff[iIndex]	=	iValue;
}

void CILPSolver::SetCoeff(int iIndex, double dValue)
{
	m_pCoeff[iIndex]	=	dValue;
}

int CILPSolver::GetNumVariable()
{
	return	lpx_get_num_cols(m_pLPX);
}

int CILPSolver::GetNumConstraint()
{
	return	lpx_get_num_rows(m_pLPX);
}

void CILPSolver::RoundUp(double dThreshold)
{
	for(int i=1;i<=m_pSolution[0];++i)
	{
		if(m_pSolution[i]>=dThreshold)	m_pSolution[i]	=	1;
		else							m_pSolution[i]	=	0;
	}				
}

