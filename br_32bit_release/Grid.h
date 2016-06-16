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
// Grid.h: interface for the CGrid class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRID_H__836598E7_1724_4B7D_A043_7251988FE56B__INCLUDED_)
#define AFX_GRID_H__836598E7_1724_4B7D_A043_7251988FE56B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Boundary.h"

class CLayer;
class CGrid : public CBoundary  
{
	friend class CLayer;
	//friend class CDesign;
	
public:
	CGrid();
	virtual ~CGrid();

	virtual void		Initialize(int iX, int iY, int iZ, CLayer* pParent);
//	int					IsBlocked();
	int					GetMaze();
	void				SetMaze(int iMaze);
	CGrid*				GetMazeGrid();
	//void				SetMazeGrid(CGrid* pGrid);
	//void				SetMazeCost(int iCost);
	void				SetMazeCostGrid(int iCost,CGrid* pGrid);
	void				SetMazeCostGrid(int iCost, int iPCost, CGrid* pGrid);

	int					GetMazeCost();
	CGrid*				GetAdjGrid(int iIndex);
//	int					GetNumPin();

	virtual void		Print(FILE* pFile, int iMode);
	CLayer*				GetParent();
	int					m_iHeapPos;
	int					m_iMazePCost;

	int					GetHeapCost();
protected:
	unsigned char*		m_pMaze;
	//int				m_iMazeCost;
	CGrid*				m_pMazeGrid;
	CGrid*				m_pAdjGrid[MAX_ADJ_GRID];
};

// utilize m_iState, as Grid and Boundary don't need any state [2/15/2007 thyeros]

inline int		CGrid::GetHeapCost(){	return m_iMazePCost;}
inline int		CGrid::GetMazeCost(){	return	m_iState;}
inline CGrid*	CGrid::GetMazeGrid(){	return	m_pMazeGrid;}
//inline void		CGrid::SetMazeGrid(CGrid *pGrid){	m_pMazeGrid	=	pGrid;}
//inline void		CGrid::SetMazeCost(int iCost){	m_iState	=	iCost;}
inline int		CGrid::GetMaze() {	return	*m_pMaze;}
inline void		CGrid::SetMazeCostGrid(int iCost, CGrid* pGrid)
{	
	m_iState	=	iCost;
	m_pMazeGrid	=	pGrid;
}

inline void		CGrid::SetMazeCostGrid(int iCost, int iPCost, CGrid* pGrid)
{	
	m_iState		=	iCost;
	m_iMazePCost	=	iCost+iPCost;
	m_pMazeGrid		=	pGrid;
}

inline void		CGrid::SetMaze(int iMaze)
{
	switch(iMaze){
	case MAZE_GRID_FLOODED:
		*m_pMaze	|=	MAZE_GRID_FLOODED;
		m_iHeapPos	=	-1;
		break;
	case MAZE_GRID_NULL:
		*m_pMaze	=	MAZE_GRID_NULL;
		m_iState	=	MAX_NUMBER;
		m_pMazeGrid	=	NULL;
		m_iHeapPos	=	-1;
		break;
	case MAZE_GRID_SOURCE:
		*m_pMaze	|=	MAZE_GRID_SOURCE;
		*m_pMaze	&=	~(MAZE_GRID_TARGET|MAZE_GRID_BRIDGE);
//		*m_pMaze	&=	~MAZE_GRID_BRIDGE;
		m_pMazeGrid	=	NULL;
		//m_iState	=	0;
		//m_iMazePCost=	0;
		m_iHeapPos	=	-1;
		break;
	case MAZE_GRID_TARGET:
		*m_pMaze	|=	MAZE_GRID_TARGET;
		*m_pMaze	&=	~(MAZE_GRID_SOURCE|MAZE_GRID_BRIDGE);
		//*m_pMaze	&=	~MAZE_GRID_SOURCE;
		//*m_pMaze	&=	~MAZE_GRID_BRIDGE;
//		m_iState	=	MAX_NUMBER;
		m_pMazeGrid	=	NULL;
		m_iHeapPos	=	-1;
		break;
	case MAZE_GRID_BRIDGE:
		*m_pMaze	|=	MAZE_GRID_BRIDGE;
		*m_pMaze	&=	~(MAZE_GRID_TARGET|MAZE_GRID_SOURCE);
		m_pMazeGrid	=	NULL;
		//*m_pMaze	&=	~MAZE_GRID_SOURCE;
		//*m_pMaze	&=	~MAZE_GRID_TARGET;
		m_iState	=	MAX_NUMBER;
		m_iHeapPos	=	-1;
		break;
	case MAZE_GRID_BOUND:
		if(*m_pMaze==MAZE_GRID_NULL)	*m_pMaze	|=	MAZE_GRID_BOUND;
		break;
	default:
		assert(FALSE);
		break;
	}
}

//class CGridOpMaze {
//public:
//	bool operator () (const CGrid* pL, const CGrid* pR) const
//	{ 
//		int	iCostL	=	((CGrid*)pL)->GetMazeCost();
//		int	iCostR	=	((CGrid*)pR)->GetMazeCost();
//
//		if(iCostL==iCostR)	return ((CGrid*)pL)->GetKey()>=((CGrid*)pR)->GetKey();
//		return	iCostL>=iCostR;
//
//		return	((CGrid*)pL)->GetMazeCost()>=((CGrid*)pR)->GetMazeCost();
//	}
//};

#endif // !defined(AFX_GRID_H__836598E7_1724_4B7D_A043_7251988FE56B__INCLUDED_)

