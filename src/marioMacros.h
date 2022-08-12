#ifndef H_MARIOMACROS
#define H_MARIOMACROS

#define MARIO_SCALE 4.f
#define IMARIO_SCALE 4

extern "C" {
	#include <libsm64/src/libsm64.h>
	#include <libsm64/src/decomp/include/surface_terrains.h>
}

#define ADD_ROOM_FACE_ABSOLUTE(surfaces, surface_ind, room, d, f, slippery, roomIndex, faceIndex) \
	surfaces[surface_ind++] = {(int16_t)(slippery ? SURFACE_SLIPPERY : SURFACE_NOT_SLIPPERY), 0, TERRAIN_STONE, roomIndex, faceIndex, { \
		{(room.info.x + d->vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[2]].pos.z)/IMARIO_SCALE}, \
		{(room.info.x + d->vertices[f.vertices[1]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[1]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[1]].pos.z)/IMARIO_SCALE}, \
		{(room.info.x + d->vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[0]].pos.z)/IMARIO_SCALE}, \
	}}; \
	if (!f.triangle) \
	{ \
		surfaces[surface_ind++] = {(int16_t)(slippery ? SURFACE_SLIPPERY : SURFACE_NOT_SLIPPERY), 0, TERRAIN_STONE, roomIndex, faceIndex, { \
			{(room.info.x + d->vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[0]].pos.z)/IMARIO_SCALE}, \
			{(room.info.x + d->vertices[f.vertices[3]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[3]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[3]].pos.z)/IMARIO_SCALE}, \
			{(room.info.x + d->vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[2]].pos.z)/IMARIO_SCALE}, \
		}}; \
	}


#define ADD_MESH_FACE_RELATIVE(surfaces, surface_ind, d, f) \
	surfaces[surface_ind++] = {(int16_t)(SURFACE_DEFAULT), 0, TERRAIN_STONE, -1, -1, { \
		{(d->vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[2]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[2]].coord.z)/IMARIO_SCALE}, \
		{(d->vertices[f.vertices[1]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[1]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[1]].coord.z)/IMARIO_SCALE}, \
		{(d->vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[0]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[0]].coord.z)/IMARIO_SCALE}, \
	}}; \
	if (!f.triangle) \
	{ \
		surfaces[surface_ind++] = {(int16_t)((int16_t)(SURFACE_DEFAULT)), 0, TERRAIN_STONE, -1, -1, { \
			{(d->vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[0]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[0]].coord.z)/IMARIO_SCALE}, \
			{(d->vertices[f.vertices[3]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[3]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[3]].coord.z)/IMARIO_SCALE}, \
			{(d->vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[2]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[2]].coord.z)/IMARIO_SCALE}, \
		}}; \
	}

#define ADD_MESH_FACE_ABSOLUTE(surfaces, surface_ind, room, d, f) \
	surfaces[surface_ind++] = {(int16_t)(SURFACE_DEFAULT), 0, TERRAIN_STONE, -1, -1, { \
		{(room.info.x + d->vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[2]].coord.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[2]].coord.z)/IMARIO_SCALE}, \
		{(room.info.x + d->vertices[f.vertices[1]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[1]].coord.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[1]].coord.z)/IMARIO_SCALE}, \
		{(room.info.x + d->vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[0]].coord.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[0]].coord.z)/IMARIO_SCALE}, \
	}}; \
	if (!f.triangle) \
	{ \
		surfaces[surface_ind++] = {(int16_t)((int16_t)(SURFACE_DEFAULT)), 0, TERRAIN_STONE, -1, -1, { \
			{(room.info.x + d->vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[0]].coord.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[0]].coord.z)/IMARIO_SCALE}, \
			{(room.info.x + d->vertices[f.vertices[3]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[3]].coord.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[3]].coord.z)/IMARIO_SCALE}, \
			{(room.info.x + d->vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[2]].coord.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[2]].coord.z)/IMARIO_SCALE}, \
		}}; \
	}

#define CREATE_BOUNDING_BOX(boundingBox, minx, maxx, miny, maxy, minz, maxz) \
	boundingBox->vertices[0].coord.x=minx; \
	boundingBox->vertices[0].coord.y=miny; \
	boundingBox->vertices[0].coord.z=minz; \
 \
	boundingBox->vertices[1].coord.x=minx; \
	boundingBox->vertices[1].coord.y=maxy; \
	boundingBox->vertices[1].coord.z=minz; \
 \
	boundingBox->vertices[2].coord.x=maxx; \
	boundingBox->vertices[2].coord.y=maxy; \
	boundingBox->vertices[2].coord.z=minz; \
 \
	boundingBox->vertices[3].coord.x=maxx; \
	boundingBox->vertices[3].coord.y=miny; \
	boundingBox->vertices[3].coord.z=minz; \
 \
	boundingBox->vertices[4].coord.x=minx; \
	boundingBox->vertices[4].coord.y=miny; \
	boundingBox->vertices[4].coord.z=maxz; \
 \
	boundingBox->vertices[5].coord.x=minx; \
	boundingBox->vertices[5].coord.y=maxy; \
	boundingBox->vertices[5].coord.z=maxz; \
 \
	boundingBox->vertices[6].coord.x=maxx; \
	boundingBox->vertices[6].coord.y=maxy; \
	boundingBox->vertices[6].coord.z=maxz; \
 \
	boundingBox->vertices[7].coord.x=maxx; \
	boundingBox->vertices[7].coord.y=miny; \
	boundingBox->vertices[7].coord.z=maxz; \
 \
	boundingBox->faces[0].triangle=1; \
	boundingBox->faces[0].vertices[0]=2; \
	boundingBox->faces[0].vertices[1]=1; \
	boundingBox->faces[0].vertices[2]=0; \
 \
	boundingBox->faces[1].triangle=1; \
	boundingBox->faces[1].vertices[0]=0; \
	boundingBox->faces[1].vertices[1]=3; \
	boundingBox->faces[1].vertices[2]=2; \
\
	boundingBox->faces[2].triangle=1; \
	boundingBox->faces[2].vertices[0]=2; \
	boundingBox->faces[2].vertices[1]=3; \
	boundingBox->faces[2].vertices[2]=7; \
\
	boundingBox->faces[3].triangle=1; \
	boundingBox->faces[3].vertices[0]=7; \
	boundingBox->faces[3].vertices[1]=6; \
	boundingBox->faces[3].vertices[2]=2; \
\
	boundingBox->faces[4].triangle=1; \
	boundingBox->faces[4].vertices[0]=6; \
	boundingBox->faces[4].vertices[1]=7; \
	boundingBox->faces[4].vertices[2]=4; \
\
	boundingBox->faces[5].triangle=1; \
	boundingBox->faces[5].vertices[0]=4; \
	boundingBox->faces[5].vertices[1]=5; \
	boundingBox->faces[5].vertices[2]=6; \
\
	boundingBox->faces[6].triangle=1; \
	boundingBox->faces[6].vertices[0]=4; \
	boundingBox->faces[6].vertices[1]=0; \
	boundingBox->faces[6].vertices[2]=5; \
\
	boundingBox->faces[7].triangle=1; \
	boundingBox->faces[7].vertices[0]=0; \
	boundingBox->faces[7].vertices[1]=1; \
	boundingBox->faces[7].vertices[2]=5; \
\
	boundingBox->faces[8].triangle=1; \
	boundingBox->faces[8].vertices[0]=7; \
	boundingBox->faces[8].vertices[1]=3; \
	boundingBox->faces[8].vertices[2]=0; \
\
	boundingBox->faces[9].triangle=1; \
	boundingBox->faces[9].vertices[0]=0; \
	boundingBox->faces[9].vertices[1]=4; \
	boundingBox->faces[9].vertices[2]=7; \
\
	boundingBox->faces[10].triangle=1; \
	boundingBox->faces[10].vertices[0]=6; \
	boundingBox->faces[10].vertices[1]=5; \
	boundingBox->faces[10].vertices[2]=1; \
\
	boundingBox->faces[11].triangle=1; \
	boundingBox->faces[11].vertices[0]=1; \
	boundingBox->faces[11].vertices[1]=2; \
	boundingBox->faces[11].vertices[2]=6; 

#define CONVERT_DEBUG_FACE_COORDINATES(src) \
	vec3(src.v1[0]*IMARIO_SCALE, -src.v1[1]*IMARIO_SCALE, -src.v1[2]*IMARIO_SCALE), \
	vec3(src.v2[0]*IMARIO_SCALE, -src.v2[1]*IMARIO_SCALE, -src.v2[2]*IMARIO_SCALE), \
	vec3(src.v3[0]*IMARIO_SCALE, -src.v3[1]*IMARIO_SCALE, -src.v3[2]*IMARIO_SCALE)


#endif // H_MARIOMACROS
