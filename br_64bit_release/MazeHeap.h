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
#if !defined(AFX_MAZEHEAP_H__DFE2F4_754A_45CA_A69B_9FDD394510__INCLUDED_)
#define AFX_MAZEHEAP_H__DFE2F4_754A_45CA_A69B_9FDD394510__INCLUDED_


#include "BoxRouter.h"
#include "Grid.h"


class CMazeHeap
{
public:

	CMazeHeap(){m_iSize		=	0;};

#define MAX_QUEUE	(163840)
//#define MAX_QUEUE	(20480)
	static CGrid*	m_Queue[MAX_QUEUE];
	int				m_iSize;

	int	GetCount()
	{
#ifdef _DEBUG
		for(int i=1;i<m_iSize;i++)
		{
			int	iCurPos		=	i;
			int	iParPos		=	(iCurPos-1)/2;

			assert(m_Queue[iCurPos]->m_iHeapPos==iCurPos);
			assert(m_Queue[iCurPos]->GetHeapCost()>=m_Queue[iParPos]->GetHeapCost());
		}
#endif
		return	m_iSize;
	}

	inline CGrid* Top(){	return	m_Queue[0];}

	inline void Swab(int iPos1, int iPos2)
	{
		//if(0x0091dafc==(int)m_Queue[iPos1])
		//{
		//	int a=0;
		//}
		//if(0x0091dafc==(int)m_Queue[iPos2])
		//{
		//	int a=0;
		//}
		if(iPos1==iPos2)
		{
			assert(m_Queue[iPos1]==m_Queue[iPos2]);
			return;
		}

		m_Queue[iPos1]->m_iHeapPos	=	iPos2;
		m_Queue[iPos2]->m_iHeapPos	=	iPos1;

		CGrid*	pTemp	=	m_Queue[iPos1];
		m_Queue[iPos1]	=	m_Queue[iPos2];
		m_Queue[iPos2]	=	pTemp;

		assert(m_Queue[iPos1]->m_iHeapPos==iPos1);
		assert(m_Queue[iPos2]->m_iHeapPos==iPos2);
	}

	void Pop()
	{
		Swab(0,--m_iSize);
		m_Queue[m_iSize]->m_iHeapPos	=	-1;

		assert(m_iSize>=0);
		if(m_iSize==0)	return;

		int iCurPos		=	0;
		int iCurCost	=	m_Queue[0]->GetHeapCost();

		// heap-down [2/19/2007 thyeros]
		while(TRUE)
		{
			int	iLChildPos	=	iCurPos+iCurPos+1;
			int	iRChildPos	=	iLChildPos+1;

			// both are valid children [2/19/2007 thyeros]
			if(iLChildPos<m_iSize&&iRChildPos<m_iSize)
			{
				int	iLChildCost	=	m_Queue[iLChildPos]->GetHeapCost();
				int	iRChildCost	=	m_Queue[iRChildPos]->GetHeapCost();
				
				if(iLChildCost<iRChildCost)
				{
					if(iCurCost<=iLChildCost)	break;
					Swab(iCurPos,iLChildPos);
					iCurPos	=	iLChildPos;
				}
				else
				{
					if(iCurCost<=iRChildCost)	break;
					Swab(iCurPos,iRChildPos);
					iCurPos	=	iRChildPos;
				}
			}
			else if(iLChildPos<m_iSize)
			{
				if(iCurCost<=m_Queue[iLChildPos]->GetHeapCost())	break;
				Swab(iCurPos,iLChildPos);
				break;
			}
			else 
			{
				break;
			}
		}
	}

	void Push(CGrid* pGrid)
	{
		assert(m_iSize<MAX_QUEUE);
		if(pGrid->m_iHeapPos<0)
		{
			int	iCurPos			=	m_iSize;
			pGrid->m_iHeapPos	=	iCurPos;

			m_Queue[m_iSize++]	=	pGrid;

			int	iCurCost	=	pGrid->GetHeapCost();

			if(iCurPos==0)	return;

			while(TRUE)
			{
				int	iParPos		=	(iCurPos-1)/2;

				if(iCurCost>=m_Queue[iParPos]->GetHeapCost())	break;			
				Swab(iCurPos,iParPos);
				iCurPos	=	iParPos;
			}
		}
		else
		{
			assert(pGrid==m_Queue[pGrid->m_iHeapPos]);
			int	iCurPos		=	pGrid->m_iHeapPos;
			int	iParPos		=	(iCurPos-1)/2;

			int	iCurCost	=	pGrid->GetHeapCost();

			if(iCurPos&&iCurCost<m_Queue[iParPos]->GetHeapCost())
			{
				while(TRUE)
				{
					int	iParPos		=	(iCurPos-1)/2;

					if(iCurCost>=m_Queue[iParPos]->GetHeapCost())	break;
					Swab(iCurPos,iParPos);
					iCurPos	=	iParPos;
				}
			}
			else
			{
				if(m_iSize==0)	return;

				while (TRUE)
				{
					int	iLChildPos	=	iCurPos+iCurPos+1;
					int	iRChildPos	=	iLChildPos+1;

					// both are valid children [2/19/2007 thyeros]
					if(iLChildPos<m_iSize&&iRChildPos<m_iSize)
					{
						int	iLChildCost	=	m_Queue[iLChildPos]->GetHeapCost();
						int	iRChildCost	=	m_Queue[iRChildPos]->GetHeapCost();

						if(iLChildCost<iRChildCost)
						{
							if(iCurCost<=iLChildCost)	break;
							Swab(iCurPos,iLChildPos);
							iCurPos	=	iLChildPos;
						}
						else
						{
							if(iCurCost<=iRChildCost)	break;
							Swab(iCurPos,iRChildPos);
							iCurPos	=	iRChildPos;
						}
					}
					else if(iLChildPos<m_iSize)
					{
						if(iCurCost<=m_Queue[iLChildPos]->GetHeapCost())	break;
						Swab(iCurPos,iLChildPos);
						break;
					}
					else 
					{
						break;
					}
				}
			}
		}

		assert(pGrid==m_Queue[pGrid->m_iHeapPos]);
	}
};

CGrid*	CMazeHeap::m_Queue[];
#endif

