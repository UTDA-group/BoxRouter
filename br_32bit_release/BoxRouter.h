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
#ifndef __BOX_ROUTER_H__
#define __BOX_ROUTER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include "Util.h"

#ifdef WIN32
#pragma warning( disable : 4786 )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4996 )
#pragma warning( disable : 4819 )
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <hash_map>
#include <list>
#include <stack>
#include <hash_set>
using namespace std;
#define	KEY							__int64
#define STRICMP						stricmp
#define STRNICMP					strnicmp
//#define	__USE_HASH__
#else
#ifdef STLPORT
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <hash_map>
#include <list>
#include <stack>
#include <hash_set>
using namespace std;
#else
#include <vector.h>
#include <map.h>
#include <multimap.h>
#include <set.h>
#include <queue.h>
#include <deque.h>
#include <algorithm>
#include <hash_map.h>
#include <list.h>
#include <stack.h>
#endif
#define	KEY						long long
#define STRICMP						strcasecmp
#define STRNICMP					strncasecmp
#undef NULL
#define NULL						(0x00)
//#define	__USE_HASH__

#ifndef STLPORT
// additional handler for KEY (64bit) [6/26/2006 thyeros]
namespace __gnu_cxx
{	
	template<> struct hash<KEY> {size_t operator()(KEY __x) const { return __x; }};
	//	template<> struct hash<ADDRESS> {size_t operator()(ADDRESS __x) const { return __x; }};
}
#endif
#endif

//#define __64BIT__
#ifdef __64BIT__
#define ADDRESS						KEY
#else
#define ADDRESS						int
#endif

//#ifdef __USE_HASH__
//#define	MAP							hash_map
//#define SET							hash_set
//#else
//#define	MAP							map
//#define SET							set
//#endif

#ifndef FALSE
#define	FALSE						(0)
#endif

#ifndef TRUE
#define	TRUE						(1)
#endif

#define	MAX_BUFFER_STR				(512)
#define MAX_BUFFER					(1024*8)
#define MAX_ADJ_GRID				(4)
#define MAX_NUMBER					(0x7f7f7f7f)
#define MAX_PENALTY					(10000)

#define DIR_DIAGONAL				(0x0000000)
#define	DIR_TOP						(0x0000001)
#define	DIR_RIGHT					(0x0000002)
#define	DIR_BOTTOM					(0x0000004)
#define	DIR_LEFT					(0x0000008)
#define	DIR_HORIZONTAL				(DIR_TOP|DIR_BOTTOM)
#define	DIR_VERTICAL				(DIR_LEFT|DIR_RIGHT)
#define DIR_PERPENDICULAR			(0x0000010)

#define LAYER_DEVICE				(0x0000000)
#define LAYER_METAL1				(0x0000001)
#define LAYER_METAL2				(0x0000002)
#define LAYER_METAL3				(0x0000003)
#define LAYER_METAL4				(0x0000004)
#define LAYER_METAL5				(0x0000005)
#define LAYER_METAL6				(0x0000006)
#define LAYER_METAL7				(0x0000007)
#define LAYER_METAL8				(0x0000008)

//////////////////////////////////////////////////////////////////////////
//thyeros- function mode [6/16/2006]

#define	PRINT_MODE_TEXT				(0x0000000)
#define	PRINT_MODE_MATLAB			(0x0000001)
#define	PRINT_MODE_GPLOT			(0x0000002)
#define PRINT_MODE_SEGMENT			(0x0000010)
#define PRINT_MODE_CONGET			(0x0000020)
#define PRINT_MODE_RESULT			(0x0000040)
#define	PRINT_MODE_MATLAB3D			(0x0000080)
#define PRINT_MODE_GNUPLOT			(0x0000100)
#define PRINT_MODE_GNUPLOT3D		(0x0000200)
#define	PRINT_MODE_OFDUMP			(0x0000400)
#define PRINT_MODE_CGDUMP			(0x0000800)
#define PRINT_MODE_PIN				(0x0001000)
#define PRINT_MODE_DUMP				(0x0002000)
#define PRINT_MODE_CGMAP			(0x0004000)
//#define PRINT_MODE_HISTO			(0x1000000)
//#define PRINT_MODE_DEBUG			(0x8000000)
//#define PRINT_MODE_CONGET_HISTO		(PRINT_MODE_CONGET|PRINT_MODE_HISTO)

#define DISPLAY_MODE_INFO			(0x0000000)
#define DISPLAY_MODE_ERRO			(0x0000001)
#define DISPLAY_MODE_PARM			(0x0000002)
#define DISPLAY_MODE_WARN			(0x0000003)
#define DISPLAY_MODE_EXEC			(0x0000004)
#define DISPLAY_MODE_NONE			(0x0000005)

#define ENUM_MODE_L					(0x0000001)
#define ENUM_MODE_Z					(0x0000002)
#define ENUM_MODE_LZ				(ENUM_MODE_L|ENUM_MODE_Z)

#define	GET_MODE_PROP				(0x0000000)
#define	GET_MODE_STATE				(0x0000001)
//#define GET_MODE_UNIT_UM			(0x0000002)
//#define	GET_MODE_UNIT_NM			(0x0000004)
//#define GET_MODE_NEW				(0x0000008)
//#define GET_MODE_CUR				(0x0000010)
#define	GET_MODE_AVG				(0x0000020)
#define	GET_MODE_MAX				(0x0000040)
#define	GET_MODE_MIN				(0x0000080)
//#define	GET_MODE_STD				(0x0000100)
//#define GET_MODE_ALL				(0x0000200)
//#define GET_MODE_SPECIAL			(0x0000400)
//#define GET_MODE_SIGNAL				(0x0000800)
#define GET_MODE_ACAP				(0x0001000)	// available capacity [6/23/2006 thyeros]
#define GET_MODE_OCAP				(0x0002000)	// occupied capacity [6/23/2006 thyeros]
#define GET_MODE_BCAP				(0x0004000)	// blockage capacity [3/5/2007 thyeros]
#define	GET_MODE_LENGTH				(0x0008000)
//#define	GET_MODE_UB					(0x0010000)	// upper bound
//#define	GET_MODE_LB					(0x0020000)	// lower bound
//#define	GET_MODE_DENSITY			(0x0080000)
#define	GET_MODE_LAYER				(0x0100000)
//#define	GET_MODE_COV				(0x0200000)
#define GET_MODE_SUM				(0x0400000)
//#define GET_MODE_TARGET				(0x0800000)
//#define	GET_MODE_PONESIG			(GET_MODE_AVG|GET_MODE_STD|0x00000000)	// + one sigma
//#define	GET_MODE_MONESIG			(GET_MODE_AVG|GET_MODE_STD|0x10000000)	// - one sigma
//#define	GET_MODE_PTWOSIG			(GET_MODE_AVG|GET_MODE_STD|0x20000000)	// + two sigma
//#define	GET_MODE_MTWOSIG			(GET_MODE_AVG|GET_MODE_STD|0x40000000)	// - two sigma

//#define GET_DR_MIN_VIA_SPACING		(0x0000000)
//#define GET_DR_MIN_WIR_SPACING		(0x0000001)
//#define GET_DR_MIN_WIR_WIDTH		(0x0000002)

#define DELWIRE_MODE_FRMAP			(0x0000000)
#define DELWIRE_MODE_NOMAP			(0x0000001)

#define	UPBOUND_MODE_ADD			(0x0000001)
#define UPBOUND_MODE_DEL			(0x0000002)

//////////////////////////////////////////////////////////////////////////
//thyeros- object state [6/16/2006]

#define	STATE_INVALID				(0x0000000)

#define STATE_POINT_PIN				(0x0000001)

#define STATE_BODR_ANY				(0x0000020)

#define	STATE_WIRE_ROUTED			(0x0000001)
#define STATE_WIRE_UNROUTED			(0x0000002)
#define	STATE_WIRE_ANY				(STATE_WIRE_ROUTED|STATE_WIRE_UNROUTED)
#define STATE_WIRE_ASSGNED			(0x0000004)
#define STATE_WIRE_NOTASSGNED		(0x0000008)
#define STATE_WIRE_REROUTED			(0x0000040)

#define	STATE_NET_ROUTED			(0x0000001)
#define STATE_NET_UNROUTED			(0x0000002)
#define	STATE_NET_ANY				(STATE_NET_ROUTED|STATE_NET_UNROUTED)
//#define STATE_NET_REROUTED			(0x0000004)
//#define STATE_NET_UNREROUTED		(0x0000008)
#define STATE_NET_BBOXDIRTY			(0x0000010)
#define STATE_NET_BBOXCLEAN			(0x0000020)
#define STATE_NET_PREROUTED			(0x0000040)

#define STATE_DESN_OVERFLOW			(0x0000001)
#define	STATE_DESN_ROUTED			(0x0000002)
#define	STATE_DESN_REROUTED			(0x0000004)
#define	STATE_DESN_ROUTING			(0x0000008)
#define	STATE_DESN_REROUTING		(0x0000010)
#define STATE_DESN_SUCCESS			(0x0000020)
#define	STATE_DESN_STUCK			(0x0000040)
#define	STATE_DESN_COMPLETED		(0x0000080)

//////////////////////////////////////////////////////////////////////////
// maze routing state [6/23/2006 thyeros]

#define MAZE_GRID_NULL				(0x00)
#define MAZE_GRID_SOURCE			(0x01)
#define MAZE_GRID_TARGET			(0x02)
#define MAZE_GRID_BRIDGE			(0x04)
#define MAZE_GRID_FLOODED			(0x08)
#define MAZE_GRID_BOUND				(0x80)

//////////////////////////////////////////////////////////////////////////
//thyeros- object property [6/16/2006]

#define	PROP_INVALID				(0x0000000)

#define PROP_BODR_ANY				(0x0000008)

#define	PROP_WIRE_HORIZONTAL		(DIR_HORIZONTAL)
#define	PROP_WIRE_VERTICAL			(DIR_VERTICAL)
#define	PROP_WIRE_FLAT				(PROP_WIRE_HORIZONTAL|PROP_WIRE_VERTICAL)
#define	PROP_WIRE_BEND				(0x0000100)
#define	PROP_WIRE_POINT				(0x0000200)
#define	PROP_WIRE_ANY				(PROP_WIRE_FLAT|PROP_WIRE_BEND|PROP_WIRE_POINT)
#define PROP_WIRE_SPECIAL			(0x0000800)
#define PROP_WIRE_PERPENDICULAR		(DIR_PERPENDICULAR)

#define PROP_SEG_HORIZONTAL			(PROP_WIRE_HORIZONTAL)
#define PROP_SEG_VERTICAL			(PROP_WIRE_VERTICAL)
#define	PROP_SEG_FLAT				(PROP_WIRE_FLAT)
#define PROP_SEG_PERPENDICULAR		(PROP_WIRE_PERPENDICULAR)

#define	PROP_NET_LOCAL				(0x0000001)
#define	PROP_NET_TWOPIN				(0x0000002)
#define	PROP_NET_GLOBAL				(0x0000004)
#define PROP_NET_SIGNAL				(PROP_NET_LOCAL|PROP_NET_GLOBAL)
//#define	PROP_NET_SPECIAL			(0x0000008)
#define PROP_NET_FLAT				(0x1000000)
#define	PROP_NET_ANY				(PROP_NET_LOCAL|PROP_NET_GLOBAL)

#define	PROP_PARAM_PRER				(0x0000001)
#define	PROP_PARAM_BOXR				(0x0000002)
#define PROP_PARAM_BOXR_ILPMIN		(0x0000004)
#define PROP_PARAM_BOXR_ILPMAX		(0x0000008)
#define PROP_PARAM_BOXR_ILPHYD		(0x0000010)
#define PROP_PARAM_BOXR_ILP			(PROP_PARAM_BOXR_ILPMIN|PROP_PARAM_BOXR_ILPMAX|PROP_PARAM_BOXR_ILPHYD)
#define PROP_PARAM_BOXR_ILP_RNDUP	(0x0000020)
#define PROP_PARAM_BOXR_MAZE		(0x0000040)
#define PROP_PARAM_BOXR_PATTERN		(0x0000080)
#define	PROP_PARAM_RER				(0x0000100)
#define PROP_PARAM_DISP_OFF			(0x0000200)
#define	PROP_PARAM_MULTILAYER		(0x0000400)
#define PROP_PARAM_OUTPUT_RPIN		(0x0000800)
#define PROP_PARAM_SOLVER_GLPK		(0x0001000)
#define PROP_PARAM_SOLVER_MOSEK		(0x0002000)
#define	PROP_PARAM_RELAYERING		(0x0004000)

//////////////////////////////////////////////////////////////////////////
// thyeros-fuction return [6/23/2006 thyeros]

#define	RET_WIRE_NONE				(0x0000000)
#define RET_WIRE_S					(0x0000001)
#define RET_WIRE_E					(0x0000002)
#define RET_WIRE_SE					(RET_WIRE_S|RET_WIRE_E)

//////////////////////////////////////////////////////////////////////////
//thyeros- macro [6/16/2006]

#define MAX(a, b)					(((a) > (b)) ? (a) : (b))
#define MIN(a, b)					(((a) < (b)) ? (a) : (b))
#define SAFE_DEL(p)					{if(p) {delete p; p=NULL;}}
#define SAFE_DELA(p)				{if(p) {delete[] p; p=NULL;}}
#define SAFE_DELAC(q,p)				{if(q&&p) {delete[] p; p=NULL;}}
#define SAFE_FCLOSE(p)				{if(p) {fclose(p); p=NULL;}}
//#define ZERO(a)						{if(fabs(a)<1e-8) {a=0;}}
#define ZERO(a)						((fabs(a)<1e-8)? (0):(a))
#define EQUL(a, b)					(fabs((a)-(b))<1e-8? 1:0)
#define SQRE(a)						((a)*(a))
#define TRPL(a)						((a)*(a)*(a))

#ifdef _DEBUG
#define TRACE						::trace
extern void trace(char* pFMT, ...);
#else
#define TRACE										
#endif

#endif//__BOX_ROUTER_H__

