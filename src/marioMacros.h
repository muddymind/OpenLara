#ifndef H_MARIOMACROS
#define H_MARIOMACROS

#define MARIO_SCALE 4.f
#define IMARIO_SCALE 4

extern "C" {
	#include <libsm64/src/libsm64.h>
	#include <libsm64/src/decomp/include/surface_terrains.h>
}

#define ADD_ROOM_FACE_ABSOLUTE(surfaces, surface_ind, room, d, f, slippery) \
	surfaces[surface_ind++] = {(int16_t)(slippery ? SURFACE_SLIPPERY : SURFACE_NOT_SLIPPERY), 0, TERRAIN_STONE, { \
		{(room.info.x + d->vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[2]].pos.z)/IMARIO_SCALE}, \
		{(room.info.x + d->vertices[f.vertices[1]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[1]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[1]].pos.z)/IMARIO_SCALE}, \
		{(room.info.x + d->vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[0]].pos.z)/IMARIO_SCALE}, \
	}}; \
	if (!f.triangle) \
	{ \
		surfaces[surface_ind++] = {(int16_t)(slippery ? SURFACE_SLIPPERY : SURFACE_NOT_SLIPPERY), 0, TERRAIN_STONE, { \
			{(room.info.x + d->vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[0]].pos.z)/IMARIO_SCALE}, \
			{(room.info.x + d->vertices[f.vertices[3]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[3]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[3]].pos.z)/IMARIO_SCALE}, \
			{(room.info.x + d->vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d->vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d->vertices[f.vertices[2]].pos.z)/IMARIO_SCALE}, \
		}}; \
	}


#define ADD_MESH_FACE_RELATIVE(surfaces, surface_ind, d, f) \
	surfaces[surface_ind++] = {(int16_t)(SURFACE_DEFAULT), 0, TERRAIN_STONE, { \
		{(d->vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[2]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[2]].coord.z)/IMARIO_SCALE}, \
		{(d->vertices[f.vertices[1]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[1]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[1]].coord.z)/IMARIO_SCALE}, \
		{(d->vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[0]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[0]].coord.z)/IMARIO_SCALE}, \
	}}; \
	if (!f.triangle) \
	{ \
		surfaces[surface_ind++] = {(int16_t)((int16_t)(SURFACE_DEFAULT)), 0, TERRAIN_STONE, { \
			{(d->vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[0]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[0]].coord.z)/IMARIO_SCALE}, \
			{(d->vertices[f.vertices[3]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[3]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[3]].coord.z)/IMARIO_SCALE}, \
			{(d->vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -d->vertices[f.vertices[2]].coord.y/IMARIO_SCALE, -(d->vertices[f.vertices[2]].coord.z)/IMARIO_SCALE}, \
		}}; \
	}


#define X_GEOMETRY_TRANSFORMATION(x, displacement) (x+displacement)/IMARIO_SCALE
#define Y_GEOMETRY_TRANSFORMATION(y) -y/IMARIO_SCALE
#define Z_GEOMETRY_TRANSFORMATION(z, displacement) -(z+displacement)/IMARIO_SCALE


#define ADD_CUBE_GEOMETRY(container, container_index, xDisplacement, zDisplacement, minx, maxx, miny, maxy, minz, maxz) \
	ADD_CUBE_GEOMETRY_NON_TRANSFORMED( \
		container, container_index, \
		X_GEOMETRY_TRANSFORMATION(minx, xDisplacement), X_GEOMETRY_TRANSFORMATION(maxx, xDisplacement), \
		Y_GEOMETRY_TRANSFORMATION(miny), Y_GEOMETRY_TRANSFORMATION(maxy), \
		Z_GEOMETRY_TRANSFORMATION(minz, zDisplacement), Z_GEOMETRY_TRANSFORMATION(maxz, zDisplacement) \
	)


#define ADD_TRIANGLE_FACE_GEOMETRY(container, container_index, xDisplacement, zDisplacement, v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z) \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED( \
		container, container_index, \
		X_GEOMETRY_TRANSFORMATION(v0x, xDisplacement), Y_GEOMETRY_TRANSFORMATION(v0y), Z_GEOMETRY_TRANSFORMATION(v0z, zDisplacement), \
		X_GEOMETRY_TRANSFORMATION(v1x, xDisplacement), Y_GEOMETRY_TRANSFORMATION(v1y), Z_GEOMETRY_TRANSFORMATION(v1z, zDisplacement), \
		X_GEOMETRY_TRANSFORMATION(v2x, xDisplacement), Y_GEOMETRY_TRANSFORMATION(v2y), Z_GEOMETRY_TRANSFORMATION(v2z, zDisplacement) \
	)


#define ADD_CUBE_GEOMETRY_NON_TRANSFORMED(container, container_index, minx, maxx, miny, maxy, minz, maxz) \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, maxx, maxy, minz, minx, maxy, minz, minx, miny, minz) /*face1*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, minx, miny, minz, maxx, miny, minz, maxx, maxy, minz) /*face2*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, maxx, maxy, minz, maxx, miny, minz, maxx, miny, maxz) /*face3*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, maxx, miny, maxz, maxx, maxy, maxz, maxx, maxy, minz) /*face4*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, maxx, maxy, maxz, maxx, miny, maxz, minx, miny, maxz) /*face5*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, minx, miny, maxz, minx, maxy, maxz, maxx, maxy, maxz) /*face6*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, minx, miny, maxz, minx, miny, minz, minx, maxy, maxz) /*face7*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, minx, miny, minz, minx, maxy, minz, minx, maxy, maxz) /*face8*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, maxx, miny, maxz, maxx, miny, minz, minx, miny, minz) /*face9*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, minx, miny, minz, minx, miny, maxz, maxx, miny, maxz) /*face10*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, maxx, maxy, maxz, minx, maxy, maxz, minx, maxy, minz) /*face11*/ \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, minx, maxy, minz, maxx, maxy, minz, maxx, maxy, maxz) /*face12*/

#define ADD_RECTANGLE_HORIZONTAL_GEOMETRY_NON_TRANSFORMED(container, container_index, minx, maxx, minz, maxz) \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, minx, 0, maxz, minx, 0, minz, maxx, 0, maxz) \
	ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, maxx, 0, minz, maxx, 0, maxz, minx, 0, minz)

#define ADD_TRIANGLE_FACE_GEOMETRY_NON_TRANSFORMED(container, container_index, v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z) \
	container[container_index++] = {(int16_t)(SURFACE_DEFAULT), 0, TERRAIN_STONE, { \
		{ v2x , v2y, v2z }, \
		{ v1x , v1y, v1z }, \
		{ v0x , v0y, v0z } \
	}};


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
 /*face1*/\  
	boundingBox->faces[0].triangle=1; \
	boundingBox->faces[0].vertices[0]=2; \
	boundingBox->faces[0].vertices[1]=1; \
	boundingBox->faces[0].vertices[2]=0; \
 /*face2*/\
	boundingBox->faces[1].triangle=1; \
	boundingBox->faces[1].vertices[0]=0; \
	boundingBox->faces[1].vertices[1]=3; \
	boundingBox->faces[1].vertices[2]=2; \
/*face3*/\
	boundingBox->faces[2].triangle=1; \
	boundingBox->faces[2].vertices[0]=2; \
	boundingBox->faces[2].vertices[1]=3; \
	boundingBox->faces[2].vertices[2]=7; \
/*face4*/\
	boundingBox->faces[3].triangle=1; \
	boundingBox->faces[3].vertices[0]=7; \
	boundingBox->faces[3].vertices[1]=6; \
	boundingBox->faces[3].vertices[2]=2; \
/*face5*/\
	boundingBox->faces[4].triangle=1; \
	boundingBox->faces[4].vertices[0]=6; \
	boundingBox->faces[4].vertices[1]=7; \
	boundingBox->faces[4].vertices[2]=4; \
/*face6*/\
	boundingBox->faces[5].triangle=1; \
	boundingBox->faces[5].vertices[0]=4; \
	boundingBox->faces[5].vertices[1]=5; \
	boundingBox->faces[5].vertices[2]=6; \
/*face7*/\
	boundingBox->faces[6].triangle=1; \
	boundingBox->faces[6].vertices[0]=4; \
	boundingBox->faces[6].vertices[1]=0; \
	boundingBox->faces[6].vertices[2]=5; \
/*face8*/\
	boundingBox->faces[7].triangle=1; \
	boundingBox->faces[7].vertices[0]=0; \
	boundingBox->faces[7].vertices[1]=1; \
	boundingBox->faces[7].vertices[2]=5; \
/*face9*/\
	boundingBox->faces[8].triangle=1; \
	boundingBox->faces[8].vertices[0]=7; \
	boundingBox->faces[8].vertices[1]=3; \
	boundingBox->faces[8].vertices[2]=0; \
/*face10*/\
	boundingBox->faces[9].triangle=1; \
	boundingBox->faces[9].vertices[0]=0; \
	boundingBox->faces[9].vertices[1]=4; \
	boundingBox->faces[9].vertices[2]=7; \
/*face11*/\
	boundingBox->faces[10].triangle=1; \
	boundingBox->faces[10].vertices[0]=6; \
	boundingBox->faces[10].vertices[1]=5; \
	boundingBox->faces[10].vertices[2]=1; \
/*face12*/\
	boundingBox->faces[11].triangle=1; \
	boundingBox->faces[11].vertices[0]=1; \
	boundingBox->faces[11].vertices[1]=2; \
	boundingBox->faces[11].vertices[2]=6; 


#define CONVERT_DEBUG_FACE_COORDINATES(src) \
	vec3(src.v1[0]*IMARIO_SCALE, -src.v1[1]*IMARIO_SCALE, -src.v1[2]*IMARIO_SCALE), \
	vec3(src.v2[0]*IMARIO_SCALE, -src.v2[1]*IMARIO_SCALE, -src.v2[2]*IMARIO_SCALE), \
	vec3(src.v3[0]*IMARIO_SCALE, -src.v3[1]*IMARIO_SCALE, -src.v3[2]*IMARIO_SCALE)


#define TEST_FACE_OVERLAP(face1, face2, mainAxis, otherAxis) \
	face1->positive != face2->positive && face1->mainAxis[0] == face2->mainAxis[0] && \
	!(\
		((face1->otherAxis[0] == face2->otherAxis[0] && face1->otherAxis[1] == face2->otherAxis[1]) ? \
			face1->limits[0][0] > face2->limits[0][1] || face1->limits[0][1] < face2->limits[0][0] : \
			face1->limits[0][0] >= face2->limits[0][1] || face1->limits[0][1] <= face2->limits[0][0] ) \
		|| face1->otherAxis[0] >= face2->otherAxis[1] || face1->otherAxis[1] <= face2->otherAxis[0] \
	)\

#endif // H_MARIOMACROS
