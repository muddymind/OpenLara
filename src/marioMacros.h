#ifndef H_MARIOMACROS
#define H_MARIOMACROS

#define MARIO_SCALE 4.f
#define IMARIO_SCALE 4

extern "C" {
	#include <libsm64/src/libsm64.h>
	#include <libsm64/src/decomp/include/surface_terrains.h>
}

#define ADD_FACE(surfaces, surface_ind, room, d, f) \
	surfaces[surface_ind++] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, { \
		{(room.info.x + d.vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[2]].pos.z)/IMARIO_SCALE}, \
		{(room.info.x + d.vertices[f.vertices[1]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[1]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[1]].pos.z)/IMARIO_SCALE}, \
		{(room.info.x + d.vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[0]].pos.z)/IMARIO_SCALE}, \
	}}; \
	if (!f.triangle) \
	{ \
		surfaces[surface_ind++] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, { \
			{(room.info.x + d.vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[0]].pos.z)/IMARIO_SCALE}, \
			{(room.info.x + d.vertices[f.vertices[3]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[3]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[3]].pos.z)/IMARIO_SCALE}, \
			{(room.info.x + d.vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[2]].pos.z)/IMARIO_SCALE}, \
		}}; \
	}

#define ADD_FACE_COORD_ROOM_MESH(surfaces, surface_ind, m, d, f) \
	surfaces[surface_ind++] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, { \
		{(m.x + d.vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(m.y + d.vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(m.z + d.vertices[f.vertices[2]].coord.z)/IMARIO_SCALE}, \
		{(m.x + d.vertices[f.vertices[1]].coord.x)/IMARIO_SCALE, -(m.y + d.vertices[f.vertices[1]].coord.y)/IMARIO_SCALE, -(m.z + d.vertices[f.vertices[1]].coord.z)/IMARIO_SCALE}, \
		{(m.x + d.vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(m.y + d.vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(m.z + d.vertices[f.vertices[0]].coord.z)/IMARIO_SCALE}, \
	}}; \
	if (!f.triangle) \
	{ \
		surfaces[surface_ind++] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, { \
			{(m.x + d.vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(m.y + d.vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(m.z + d.vertices[f.vertices[0]].coord.z)/IMARIO_SCALE}, \
			{(m.x + d.vertices[f.vertices[3]].coord.x)/IMARIO_SCALE, -(m.y + d.vertices[f.vertices[3]].coord.y)/IMARIO_SCALE, -(m.z + d.vertices[f.vertices[3]].coord.z)/IMARIO_SCALE}, \
			{(m.x + d.vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(m.y + d.vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(m.z + d.vertices[f.vertices[2]].coord.z)/IMARIO_SCALE}, \
		}}; \
	}

#define COUNT_ROOM_SECTORS(level, surfaces_count, room) \
	COUNT_ROOM_SECTORS_UP(level, surfaces_count, room);\
	COUNT_ROOM_SECTORS_DOWN(level, surfaces_count, room);

#define ADD_ROOM_SECTORS(level, surfaces, surface_ind, room) \
	ADD_ROOM_SECTORS_UP(level, surfaces, surface_ind, room);\
	ADD_ROOM_SECTORS_DOWN(level, surfaces, surface_ind, room);

void COUNT_ROOM_SECTORS_UP(TR::Level* level, size_t& surfaces_count, TR::Room& room)
{
	for (int i=0; i<room.xSectors * room.zSectors; i++)
	{
		if (room.sectors[i].roomAbove == TR::NO_ROOM) continue;
		
		TR::Room &roomUp = level->rooms[room.sectors[i].roomAbove];
		TR::Room::Data &dUp = roomUp.data;
		
		for (int j = 0; j < dUp.fCount; j++)
		{
			TR::Face &f = dUp.faces[j];
			if (f.water) continue;
			
			surfaces_count += (f.triangle) ? 1 : 2;
		}
		
		COUNT_ROOM_SECTORS_UP(level, surfaces_count, roomUp);
		break;
	}
}

void COUNT_ROOM_SECTORS_DOWN(TR::Level* level, size_t& surfaces_count, TR::Room& room)
{
	for (int i=0; i<room.xSectors * room.zSectors; i++)
	{
		if (room.sectors[i].roomBelow == TR::NO_ROOM) continue;
		
		TR::Room &roomDown = level->rooms[room.sectors[i].roomBelow];
		TR::Room::Data &dDown = roomDown.data;
		
		for (int j = 0; j < dDown.fCount; j++)
		{
			TR::Face &f = dDown.faces[j];
			if (f.water) continue;
			
			surfaces_count += (f.triangle) ? 1 : 2;
		}
		
		COUNT_ROOM_SECTORS_DOWN(level, surfaces_count, roomDown);
		break;
	}
}

void ADD_ROOM_SECTORS_UP(TR::Level* level, struct SM64Surface* surfaces, size_t& surface_ind, TR::Room& room)
{
	for (int i=0; i<room.xSectors * room.zSectors; i++)
	{
		if (room.sectors[i].roomAbove == TR::NO_ROOM) continue;
		
		TR::Room &roomUp = level->rooms[room.sectors[i].roomAbove];
		TR::Room::Data &dUp = roomUp.data;
		
		for (int j = 0; j < dUp.fCount; j++)
		{
			TR::Face &f = dUp.faces[j];
			if (f.water) continue;
			
			ADD_FACE(surfaces, surface_ind, roomUp, dUp, f);
		}
		
		ADD_ROOM_SECTORS_UP(level, surfaces, surface_ind, roomUp);
		break;
	}
}

void ADD_ROOM_SECTORS_DOWN(TR::Level* level, struct SM64Surface* surfaces, size_t& surface_ind, TR::Room& room)
{
	for (int i=0; i<room.xSectors * room.zSectors; i++)
	{
		if (room.sectors[i].roomBelow == TR::NO_ROOM) continue;
		
		TR::Room &roomDown = level->rooms[room.sectors[i].roomBelow];
		TR::Room::Data &dDown = roomDown.data;
		
		for (int j = 0; j < dDown.fCount; j++)
		{
			TR::Face &f = dDown.faces[j];
			if (f.water) continue;
			
			ADD_FACE(surfaces, surface_ind, roomDown, dDown, f);
		}
		
		ADD_ROOM_SECTORS_DOWN(level, surfaces, surface_ind, roomDown);
		break;
	}
}

#endif // H_MARIOMACROS
