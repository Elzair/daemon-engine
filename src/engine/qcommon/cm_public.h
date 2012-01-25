/*
===========================================================================

OpenWolf GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the OpenWolf GPL Source Code (OpenWolf Source Code).  

OpenWolf Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenWolf Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenWolf Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the OpenWolf Source Code is also subject to certain additional terms. 
You should have received a copy of these additional terms immediately following the 
terms and conditions of the GNU General Public License which accompanied the OpenWolf 
Source Code.  If not, please request a copy in writing from id Software at the address 
below.

If you have questions concerning this license or the applicable additional terms, you 
may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, 
Maryland 20850 USA.

===========================================================================
*/


#include "qfiles.h"

#include "../../../src/engine/qcommon/q_shared.h"
#include "../../../src/engine/renderer/tr_types.h"

void            CM_LoadMap(const char *name, qboolean clientload, int *checksum);
void            CM_ClearMap(void);

clipHandle_t    CM_InlineModel(int index);	// 0 = world, 1 + are bmodels
clipHandle_t    CM_TempBoxModel(const vec3_t mins, const vec3_t maxs, int capsule);

void            CM_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs);

int             CM_NumClusters(void);
int             CM_NumInlineModels(void);
char           *CM_EntityString(void);

// returns an ORed contents mask
int             CM_PointContents(const vec3_t p, clipHandle_t model);
int             CM_TransformedPointContents(const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles);

void            CM_BoxTrace(trace_t * results, const vec3_t start, const vec3_t end,
							vec3_t mins, vec3_t maxs, clipHandle_t model, int brushmask, traceType_t type);
void			CM_TransformedBoxTrace(trace_t * results, const vec3_t start, const vec3_t end,
							const vec3_t mins, const vec3_t maxs,
							clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles, traceType_t type);
void			CM_BiSphereTrace( trace_t *results, const vec3_t start,
										const vec3_t end, float startRad, float endRad,
										clipHandle_t model, int mask );
void			CM_TransformedBiSphereTrace( trace_t *results, const vec3_t start,
										const vec3_t end, float startRad, float endRad,
										clipHandle_t model, int mask,
										const vec3_t origin );

byte           *CM_ClusterPVS(int cluster);

int             CM_PointLeafnum(const vec3_t p);

// only returns non-solid leafs
// overflow if return listsize and if *lastLeaf != list[listsize-1]
int             CM_BoxLeafnums(const vec3_t mins, const vec3_t maxs, int *list, int listsize, int *lastLeaf);

int             CM_LeafCluster(int leafnum);
int             CM_LeafArea(int leafnum);

void            CM_AdjustAreaPortalState(int area1, int area2, qboolean open);
qboolean        CM_AreasConnected(int area1, int area2);

int             CM_WriteAreaBits(byte * buffer, int area);

// cm_tag.c
int             CM_LerpTag(orientation_t * tag, const refEntity_t * refent, const char *tagName, int startIndex);


// cm_marks.c
int             CM_MarkFragments(int numPoints, const vec3_t * points, const vec3_t projection,
								 int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t * fragmentBuffer);

// cm_patch.c
void            CM_DrawDebugSurface(void (*drawPoly) (int color, int numPoints, float *points));

// Physics
#ifdef USE_PHYSICS
void    		CMod_PhysicsInit ( void );
void    		CMod_PhysicsShutdown();
void    		CMod_PhysicsClearBodies();
void    		CMod_PhysicsAddBSPModel(int index, int firstSurface, int numSurfaces);
void    		CMod_PhysicsAddFaceToModel(int modelIndex, int surface, vec3_t vec[3]);
int     		CMod_PhysicsBSPSurfaceIsModel ( int surfaceID );
void    		CMod_PhysicsUpdate();
void    		CMod_PhysicsEndBSPCollisionTree();
void    		CMod_PhysicsBeginBSPCollisionTree();
void    		CMod_PhysicsAddBSPFace(vec3_t vec[3]);
#endif