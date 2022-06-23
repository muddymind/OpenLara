#ifndef H_MARIO
#define H_MARIO

#define MARIO_SCALE 4.f
#define IMARIO_SCALE 4

extern "C" {
	#include <libsm64/src/libsm64.h>
	#include <libsm64/src/decomp/include/surface_terrains.h>
	#include <libsm64/src/decomp/include/PR/ultratypes.h>
	#include <libsm64/src/decomp/include/audio_defines.h>
	#include <libsm64/src/decomp/include/mario_animation_ids.h>
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

#include "core.h"
#include "game.h"
#include "lara.h"
#include "objects.h"
#include "sprite.h"
#include "enemy.h"
#include "inventory.h"
#include "mesh.h"

struct MarioMesh
{
	size_t num_vertices;
	uint16_t *index;
};

struct MarioRenderState
{
	MarioMesh mario;
};

struct MarioControllerObj
{
	MarioControllerObj() : ID(0), entity(NULL), topPoint(0) {}

	uint32_t ID;
	struct SM64ObjectTransform transform;
	TR::Entity* entity;
	int16 topPoint;
};

struct Mario : Lara
{
	uint32_t marioId;
	struct SM64MarioInputs marioInputs;
    struct SM64MarioState marioState;
    struct SM64MarioGeometryBuffers marioGeometry;
	struct MarioRenderState marioRenderState;
	float lastPos[3], currPos[3];
	float lastGeom[9 * SM64_GEO_MAX_TRIANGLES], currGeom[9 * SM64_GEO_MAX_TRIANGLES];

	float marioTicks;
	bool postInit;
	struct MarioControllerObj objs[4096];
	int objCount;

	Mesh* TRmarioMesh;

	Block* movingBlock; // hack

	Mario(IGame *game, int entity) : Lara(game, entity)
	{
		isMario = true;
		postInit = false;
		marioId = -1;
		marioTicks = 0;
		objCount = 0;
		movingBlock = NULL;

		for (int i=0; i<3; i++)
		{
			lastPos[i] = 0;
			currPos[i] = 0;
		}
		for (int i=0; i<9 * SM64_GEO_MAX_TRIANGLES; i++)
		{
			lastGeom[i] = 0;
			currGeom[i] = 0;
		}

		marioGeometry.position = (float*)malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
		marioGeometry.color    = (float*)malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
		marioGeometry.normal   = (float*)malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
		marioGeometry.uv       = (float*)malloc( sizeof(float) * 6 * SM64_GEO_MAX_TRIANGLES );

		marioRenderState.mario.index = (uint16_t*)malloc( 3 * SM64_GEO_MAX_TRIANGLES * sizeof(uint16_t) );
		for( int i = 0; i < 3 * SM64_GEO_MAX_TRIANGLES; ++i )
			marioRenderState.mario.index[i] = i;

		marioRenderState.mario.num_vertices = 3 * SM64_GEO_MAX_TRIANGLES;

		TRmarioMesh = new Mesh((Index*)marioRenderState.mario.index, marioRenderState.mario.num_vertices, NULL, 0, 1, true, true);
		TRmarioMesh->initMario(&marioGeometry);

		// export a level.c file for use in the libsm64 test program
		
		FILE* file = fopen("level.c", "w");
		fprintf(file, "#include \"level.h\"\n#include \"../src/decomp/include/surface_terrains.h\"\nconst struct SM64Surface surfaces[] = {\n");

		for (int i = 0; i < level->roomsCount; i++)
		{
			TR::Room &room = level->rooms[i];
			TR::Room::Data &d = room.data;

			for (int j = 0; j < d.fCount; j++)
			{
				TR::Face &f = d.faces[j];
				if (f.water) continue;

					fprintf(file, "{SURFACE_DEFAULT,0,TERRAIN_STONE,{{%d,%d,%d},{%d,%d,%d},{%d,%d,%d}}},\n", (room.info.x + d.vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[2]].pos.z)/IMARIO_SCALE, (room.info.x + d.vertices[f.vertices[1]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[1]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[1]].pos.z)/IMARIO_SCALE, (room.info.x + d.vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[0]].pos.z)/IMARIO_SCALE);
				if (!f.triangle)
					fprintf(file, "{SURFACE_DEFAULT,0,TERRAIN_STONE,{{%d,%d,%d},{%d,%d,%d},{%d,%d,%d}}},\n", (room.info.x + d.vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[0]].pos.z)/IMARIO_SCALE, (room.info.x + d.vertices[f.vertices[3]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[3]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[3]].pos.z)/IMARIO_SCALE, (room.info.x + d.vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[2]].pos.z)/IMARIO_SCALE);
			}
		}

		for (int i=0; i<level->entitiesCount; i++)
		{
			TR::Entity *e = &level->entities[i];
			if (e->isEnemy() || e->isLara() || e->isSprite() || e->isPuzzleHole() || e->isPickup() || e->type == 169)
				continue;

			TR::Model *model = &level->models[e->modelIndex - 1];

			for (int c = 0; c < model->mCount; c++)
			{
				int index = level->meshOffsets[model->mStart + c];
				if (index || model->mStart + c <= 0)
				{
					TR::Mesh &d = level->meshes[index];
					for (int j = 0; j < d.fCount; j++)
					{
						TR::Face &f = d.faces[j];

							fprintf(file, "{SURFACE_DEFAULT,0,TERRAIN_STONE,{{%d,%d,%d},{%d,%d,%d},{%d,%d,%d}}},\n", (e->x + d.vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(e->y + d.vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(e->z + d.vertices[f.vertices[2]].coord.z)/IMARIO_SCALE, (e->x + d.vertices[f.vertices[1]].coord.x)/IMARIO_SCALE, -(e->y + d.vertices[f.vertices[1]].coord.y)/IMARIO_SCALE, -(e->z + d.vertices[f.vertices[1]].coord.z)/IMARIO_SCALE, (e->x + d.vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(e->y + d.vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(e->z + d.vertices[f.vertices[0]].coord.z)/IMARIO_SCALE);
						if (!f.triangle)
							fprintf(file, "{SURFACE_DEFAULT,0,TERRAIN_STONE,{{%d,%d,%d},{%d,%d,%d},{%d,%d,%d}}},\n", (e->x + d.vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(e->y + d.vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(e->z + d.vertices[f.vertices[0]].coord.z)/IMARIO_SCALE, (e->x + d.vertices[f.vertices[3]].coord.x)/IMARIO_SCALE, -(e->y + d.vertices[f.vertices[3]].coord.y)/IMARIO_SCALE, -(e->z + d.vertices[f.vertices[3]].coord.z)/IMARIO_SCALE, (e->x + d.vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(e->y + d.vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(e->z + d.vertices[f.vertices[2]].coord.z)/IMARIO_SCALE);
					}
				}
			}
		}

		fprintf(file, "};\nconst size_t surfaces_count = sizeof( surfaces ) / sizeof( surfaces[0] );\n");
		fprintf(file, "const int32_t spawn[] = {%d, %d, %d};\n", (int)(pos.x/MARIO_SCALE), (int)(-pos.y/MARIO_SCALE), (int)(-pos.z/MARIO_SCALE));
		fclose(file);

		marioUpdateRoom();
		marioId = sm64_mario_create(pos.x/MARIO_SCALE, -pos.y/MARIO_SCALE, -pos.z/MARIO_SCALE, 0, 0, 0, 0);
		printf("%.2f %.2f %.2f\n", pos.x/MARIO_SCALE, -pos.y/MARIO_SCALE, -pos.z/MARIO_SCALE);
		if (marioId >= 0) sm64_set_mario_faceangle(marioId, (int16_t)((-angle.y + M_PI) / M_PI * 32768.0f));

		animation.setAnim(ANIM_STAND);

		postInitMario();
	}
	
	virtual ~Mario()
	{
		free(marioGeometry.position);
		free(marioGeometry.color);
		free(marioGeometry.normal);
		free(marioGeometry.uv);
		TRmarioMesh->iBuffer = NULL;
		free(marioRenderState.mario.index);
		delete TRmarioMesh;

		for (int i=0; i<objCount; i++)
			sm64_surface_object_delete(objs[i].ID);

		if (marioId != -1) sm64_mario_delete(marioId);
	}

	void postInitMario()
	{
		if (postInit) return;
		postInit = true;

		// create collisions from static meshes (objects like the fence at lara's home, the chairs/table, the music instruments etc.)
		for (int i = 0; i < level->roomsCount; i++)
		{
			TR::Room *room = &level->rooms[i];
			for (int j = 0; j < room->meshesCount; j++)
			{
				TR::Room::Mesh &m  = room->meshes[j];
				TR::StaticMesh *sm = &level->staticMeshes[m.meshIndex];
				if (sm->flags < 2 || !level->meshOffsets[sm->mesh]) continue;

				// define the surface object
				struct SM64SurfaceObject obj;
				obj.surfaceCount = 0;
				obj.transform.position[0] = m.x / MARIO_SCALE;
				obj.transform.position[1] = -m.y / MARIO_SCALE;
				obj.transform.position[2] = -m.z / MARIO_SCALE;
				for (int k=0; k<3; k++) obj.transform.eulerRotation[k] = (k == 1) ? float(m.rotation) / M_PI * 180.f : 0;

				TR::Mesh &d = level->meshes[level->meshOffsets[sm->mesh]];
				if (d.fCount <= 4) continue;

				// increment the surface count for this
				for (int k = 0; k < d.fCount; k++)
					obj.surfaceCount += (d.faces[k].triangle) ? 1 : 2;

				obj.surfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * obj.surfaceCount);
				size_t surface_ind = 0;

				// add the faces
				for (int k = 0; k < d.fCount; k++)
				{
					TR::Face &f = d.faces[k];

					obj.surfaces[surface_ind++] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, {
						{(d.vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[2]].coord.z)/IMARIO_SCALE},
						{(d.vertices[f.vertices[1]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[1]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[1]].coord.z)/IMARIO_SCALE},
						{(d.vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[0]].coord.z)/IMARIO_SCALE},
					}};

					if (!f.triangle)
					{
						obj.surfaces[surface_ind++] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, {
							{(d.vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[0]].coord.z)/IMARIO_SCALE},
							{(d.vertices[f.vertices[3]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[3]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[3]].coord.z)/IMARIO_SCALE},
							{(d.vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[2]].coord.z)/IMARIO_SCALE},
						}};
					}
				}

				// create the final object and put it in the objs[] array
				struct MarioControllerObj cObj;
				cObj.ID = sm64_surface_object_create(&obj);
				cObj.transform = obj.transform;
				cObj.entity = NULL;

				objs[objCount++] = cObj;
				free(obj.surfaces);
			}
		}

		// create collisions from entities (bridges, doors)
		for (int i=0; i<level->entitiesCount; i++)
		{
			TR::Entity *e = &level->entities[i];
			if (e->isEnemy() || e->isLara() || e->isSprite() || e->isPuzzleHole() || e->isPickup() || e->type == 169)
				continue;

			if (!(e->type >= 68 && e->type <= 70) && !e->isBlock())
				continue;

			TR::Model *model = &level->models[e->modelIndex - 1];
			if (!model)
				continue;

			struct SM64SurfaceObject obj;
			obj.surfaceCount = 0;
			obj.transform.position[0] = e->x / MARIO_SCALE;
			obj.transform.position[1] = -e->y / MARIO_SCALE;
			obj.transform.position[2] = -e->z / MARIO_SCALE;
			for (int j=0; j<3; j++) obj.transform.eulerRotation[j] = (j == 1) ? float(e->rotation) / M_PI * 180.f : 0;

			// some code taken from extension.h below

			// first increment the surface count
			for (int c = 0; c < model->mCount; c++)
			{
				int index = level->meshOffsets[model->mStart + c];
				if (index || model->mStart + c <= 0)
				{
					TR::Mesh &d = level->meshes[index];
					for (int j = 0; j < d.fCount; j++)
						obj.surfaceCount += (d.faces[j].triangle) ? 1 : 2;
				}
			}

			// then create the surface and add the objects
			obj.surfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * obj.surfaceCount);
			size_t surface_ind = 0;
			int16 topPoint = 0;
			int16 topPointSign = 1;

			for (int c = 0; c < model->mCount; c++)
			{
				int index = level->meshOffsets[model->mStart + c];
				if (index || model->mStart + c <= 0)
				{
					TR::Mesh &d = level->meshes[index];
					for (int j = 0; j < d.fCount; j++)
					{
						TR::Face &f = d.faces[j];

						topPoint = max(topPoint, d.vertices[f.vertices[0]].coord.y, max(d.vertices[f.vertices[1]].coord.y, d.vertices[f.vertices[2]].coord.y, (f.triangle) ? (int16)0 : d.vertices[f.vertices[3]].coord.y));

						obj.surfaces[surface_ind] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, {
							{(d.vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[2]].coord.z)/IMARIO_SCALE},
							{(d.vertices[f.vertices[1]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[1]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[1]].coord.z)/IMARIO_SCALE},
							{(d.vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[0]].coord.z)/IMARIO_SCALE},
						}};

						if (!f.triangle)
						{
							surface_ind++;
							obj.surfaces[surface_ind] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, {
								{(d.vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[0]].coord.z)/IMARIO_SCALE},
								{(d.vertices[f.vertices[3]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[3]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[3]].coord.z)/IMARIO_SCALE},
								{(d.vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(d.vertices[f.vertices[2]].coord.z)/IMARIO_SCALE},
							}};
						}

						surface_ind++;
					}
				}
			}
			if (e->type >= 68 && e->type <= 70) topPointSign = -1;
			//else obj.transform.position[1] += topPoint/MARIO_SCALE;
			obj.transform.position[1] += topPoint/MARIO_SCALE * topPointSign;

			// and finally add the object (this returns an uint32_t which is the object's ID)
			struct MarioControllerObj cObj;
			cObj.ID = sm64_surface_object_create(&obj);
			cObj.transform = obj.transform;
			cObj.entity = e;
			cObj.topPoint = topPoint * topPointSign;

			objs[objCount++] = cObj;
			free(obj.surfaces);
		}
	}

	vec3 getPos() {return vec3(marioState.position[0], -marioState.position[1], -marioState.position[2]);}
	void marioInteracting(TR::Entity::Type type) // hack
	{
		marioInputs.buttonB = false;
		if (type == TR::Entity::MIDAS_HAND)
			sm64_set_mario_action(marioId, 0x0000132F); // ACT_UNLOCKING_STAR_DOOR
	}

	void render(Frustum *frustum, MeshBuilder *mesh, Shader::Type type, bool caustics)
	{
		//Lara::render(frustum, mesh, type, caustics);

		Core::setCullMode(cmBack);
		glUseProgram(marioState.flags & 0x00000004 ? Core::metalMarioShader : Core::marioShader);

		GAPI::Texture *dtex = Core::active.textures[sDiffuse];

		if (marioState.flags & 0x00000004)
		{
			game->setRoomParams(getRoomIndex(), Shader::MIRROR, 1.2f, 1.0f, 0.2f, 1.0f, false);
			//environment->bind(sDiffuse);
			//Core::marioTexture->bind(sDiffuse);
			//Core::setBlendMode(BlendMode::bmAdd);
			visibleMask ^= 0xFFFFFFFF;
		}
		else
			Core::marioTexture->bind(sDiffuse);

		MeshRange range;
		range.aIndex = 0;
		range.iStart = 0;
		range.vStart = 0;
		range.iCount = marioRenderState.mario.num_vertices;

		TRmarioMesh->render(range);

		Core::setCullMode(cmFront);
		if (marioState.flags & 0x00000004)
		{
			visibleMask ^= 0xFFFFFFFF;
		}

		if (dtex) dtex->bind(sDiffuse);
	}

	virtual void hit(float damage, Controller *enemy = NULL, TR::HitType hitType = TR::HIT_DEFAULT)
	{
		if (dozy || level->isCutsceneLevel()) return;
		if (!burn && (marioState.action & 1 << 12 || marioState.action & 1 << 17)) return; // ACT_FLAG_INTANGIBLE || ACT_FLAG_INVULNERABLE
		if (health <= 0.0f && hitType != TR::HIT_FALL) return;

		if (hitType == TR::HIT_MIDAS)
		{
			if (!(marioState.flags & 0x00000004))
			{
				// it's a secret... ;)
				bakeEnvironment(environment);
				sm64_mario_interact_cap(marioId, 0x00000004, 0, true);
			}
			return;
		}

		damageTime = LARA_DAMAGE_TIME;

		Character::hit(damage, enemy, hitType);

		hitTimer = LARA_VIBRATE_HIT_TIME;

		switch (hitType) {
			case TR::HIT_DART      : addBlood(enemy->pos, vec3(0));
			case TR::HIT_BLADE     : addBloodBlade(); break;
			case TR::HIT_SPIKES    : addBloodSpikes(); break;
			case TR::HIT_SWORD     : addBloodBlade(); break;
			case TR::HIT_SLAM      : addBloodSlam(enemy); break;
			case TR::HIT_LIGHTNING : lightning = (Lightning*)enemy; break;
			default                : ;
		}

		if (enemy && 
		    (
		     (!(marioState.action & 1 << 23) && (enemy->getEntity().type != TR::Entity::ENEMY_BEAR && enemy->getEntity().type != TR::Entity::ENEMY_REX)) || // 1<<23 == ACT_FLAG_ATTACKING (mario is not attacking and the enemy is not one of these, or...)
		     (!(marioState.action & 1 << 11) && (enemy->getEntity().type == TR::Entity::ENEMY_BEAR || enemy->getEntity().type == TR::Entity::ENEMY_REX)) // 1<<11 == ACT_FLAG_AIR (mario is not in the air, and the enemy is one of these)
			)
		   )
		{
			sm64_mario_take_damage(marioId, (uint32_t)(ceil(damage/100.f)), 0, enemy->pos.x, enemy->pos.y, enemy->pos.z);
		}

		if (health > 0.0f)
			return;

		game->stopTrack();
		sm64_mario_kill(marioId);
	}

	int getInput()
	{
        int pid = camera->cameraIndex;
		int input = 0;
		bool canMove = (state != STATE_PICK_UP && state != STATE_USE_KEY && state != STATE_USE_PUZZLE && state != STATE_PUSH_BLOCK && state != STATE_PULL_BLOCK && state != STATE_PUSH_PULL_READY && state != STATE_SWITCH_DOWN && state != STATE_SWITCH_UP && state != STATE_MIDAS_USE);

		float dir;
		float spd = 0;
		if (Input::state[pid][cUp] && Input::state[pid][cRight])
		{
			dir = -M_PI * 0.25f;
			spd = (Input::state[pid][cWalk]) ? 0.65f : 1;
		}
		else if (Input::state[pid][cUp] && Input::state[pid][cLeft])
		{
			dir = -M_PI * 0.75f;
			spd = (Input::state[pid][cWalk]) ? 0.65f : 1;
		}
		else if (Input::state[pid][cDown] && Input::state[pid][cRight])
		{
			dir = M_PI * 0.25f;
			spd = (Input::state[pid][cWalk]) ? 0.65f : 1;
		}
		else if (Input::state[pid][cDown] && Input::state[pid][cLeft])
		{
			dir = M_PI * 0.75f;
			spd = (Input::state[pid][cWalk]) ? 0.65f : 1;
		}
		else if (Input::state[pid][cUp])
		{
			dir = -M_PI * 0.5f;
			spd = (Input::state[pid][cWalk]) ? 0.65f : 1;
		}
		else if (Input::state[pid][cDown])
		{
			dir = M_PI * 0.5f;
			spd = (Input::state[pid][cWalk]) ? 0.65f : 1;
		}
		else if (Input::state[pid][cLeft])
		{
			dir = M_PI;
			spd = (Input::state[pid][cWalk]) ? 0.65f : 1;
		}
		else if (Input::state[pid][cRight])
		{
			dir = 0;
			spd = (Input::state[pid][cWalk]) ? 0.65f : 1;
		}

		static bool lookSnd = false;
		if (canMove && Input::state[pid][cLook] && marioState.action & (1 << 26)) // ACT_FLAG_ALLOW_FIRST_PERSON
		{
			sm64_set_mario_action(marioId, 0x0C000227); // ACT_FIRST_PERSON
			if (!lookSnd) sm64_play_sound_global(SOUND_MENU_CAMERA_ZOOM_IN);
			lookSnd = true;
			camera->mode = Camera::MODE_LOOK;
		}
		else if ((canMove && !Input::state[pid][cLook]) || !(marioState.action & (1 << 26)))
		{
			if (lookSnd)
			{
				sm64_play_sound_global(SOUND_MENU_CAMERA_ZOOM_OUT);
				lookSnd = false;
			}
		}

		marioInputs.buttonA = canMove && Input::state[pid][cJump];
		marioInputs.buttonB = canMove && Input::state[pid][cAction];
		marioInputs.buttonZ = canMove && Input::state[pid][cDuck];
		marioInputs.stickX = canMove && spd ? spd * cosf(dir) : 0;
		marioInputs.stickY = canMove && spd ? spd * sinf(dir) : 0;

        if (Input::state[pid][cUp])        input |= FORTH;
        if (Input::state[pid][cDown])      input |= BACK;
        if (Input::state[pid][cAction])    input |= ACTION;

		return input;
	}

	Stand getStand()
	{
		if (marioId < 0) return STAND_GROUND;

		TR::Level::FloorInfo info;
		getFloorInfo(getRoomIndex(), pos, info);

		if ((marioState.action & 0x000001C0) == (3 << 6)) // ((marioState.action & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED) (check if mario is in the water)
			return (marioState.position[1] >= (sm64_get_mario_water_level(marioId) - 100)*IMARIO_SCALE) ? STAND_ONWATER : STAND_UNDERWATER;
		else if (marioState.action == 0x0800034B) // marioState.action == ACT_LEDGE_GRAB
			return STAND_HANG;
		return (floor(pos.y) >= info.floor-2 && floor(pos.y) <= info.floor) ? STAND_GROUND : STAND_AIR;
	}

	void setStateFromMario()
	{
		if (state == STATE_PICK_UP || state == STATE_USE_KEY || state == STATE_USE_PUZZLE || state == STATE_PUSH_BLOCK || state == STATE_PULL_BLOCK || state == STATE_SWITCH_DOWN || state == STATE_SWITCH_UP || state == STATE_MIDAS_USE) return;

		if (state != STATE_PUSH_PULL_READY && input & ACTION && getBlock())
		{
			marioInputs.buttonB = false;
			state = STATE_PUSH_PULL_READY;
			return;
		}
		else if (state == STATE_PUSH_PULL_READY && input & ACTION)
		{
			if (input & (FORTH | BACK))
			{
				int pushState = (input & FORTH) ? STATE_PUSH_BLOCK : STATE_PULL_BLOCK;
                Block *block = getBlock();
                if (block && (pushState == STATE_PUSH_BLOCK || block->doMarioMove(input & FORTH != 0)))
				{
					movingBlock = block;
					sm64_set_mario_action(marioId, (pushState == STATE_PUSH_BLOCK) ? 0x00800380 : 0x00000390);
					state = pushState;
				}
			}
			return;
		}

		state = STATE_STOP;
		if (marioState.action == 0x01000889) // marioState.action == ACT_WATER_JUMP
			state = STATE_WATER_OUT;
		else if (stand == STAND_ONWATER)
			state = (marioState.velocity[0] == 0 && marioState.velocity[2] == 0) ? STATE_SURF_TREAD : STATE_SURF_SWIM;
		else if (stand == STAND_UNDERWATER)
			state = STATE_TREAD;
		else if (marioState.action == 0x0800034B) // marioState.action == ACT_LEDGE_GRAB
			state = STATE_HANG;
		else if (stand == STAND_AIR)
			state = (marioState.velocity[0] == 0 && marioState.velocity[2] == 0) ? STATE_UP_JUMP : STATE_FORWARD_JUMP;
		else if (!(marioState.action & (1 << 22)) && marioState.action != 0x800380 && stand != STAND_UNDERWATER) // !(marioState.action & ACT_FLAG_IDLE)
			state = STATE_RUN;
	}

	virtual void updateState()
	{
		setStateFromMario();
		if (state != STATE_WATER_OUT) Lara::updateState();
	}

	virtual bool doPickUp()
	{
		if ((marioState.velocity[0] != 0 || marioState.velocity[1] != 0 || marioState.velocity[2] != 0) && state != STATE_TREAD) return false;

		int room = getRoomIndex();

		pickupListCount = 0;

		for (int i = 0; i < level->entitiesCount; i++)
		{
			TR::Entity &entity = level->entities[i];
			if (!entity.controller || !entity.isPickup())
				continue;

			Controller *controller = (Controller*)entity.controller;

			if (controller->getRoomIndex() != room || controller->flags.invisible)
				continue;

			if (entity.type == TR::Entity::CRYSTAL) {
				if (Input::lastState[camera->cameraIndex] == cAction) {
					vec3 dir = controller->pos - pos;
					if (dir.length2() < SQR(350.0f) && getDir().dot(dir.normal()) > COS30) {
						pickupListCount = 0;
						game->invShow(camera->cameraIndex, Inventory::PAGE_SAVEGAME, i);
						return true;
					}
				}
			} else {
				if (!canPickup(controller))
					continue;

				ASSERT(pickupListCount < COUNT(pickupList));
				pickupList[pickupListCount++] = controller;
			}
		}

		if (pickupListCount > 0)
		{
			sm64_set_mario_action(marioId, stand != STAND_UNDERWATER ? 0x00800380 : 0x300024E1); // ACT_PUNCHING or ACT_WATER_PUNCH
			state = STATE_PICK_UP;
			return true;
		}

		return false;
	}

	Controller* marioFindTarget()
	{
		/*float dist = 512+32;

		Controller* target = NULL;

		vec3 from = pos - vec3(0, 650, 0);

		Controller *c = Controller::first;
		do {
			if (!c->getEntity().isEnemy())
				continue;

			Character *enemy = (Character*)c;
			if (!enemy->isActiveTarget())
				continue;

			Box box = enemy->getBoundingBox();
			vec3 p = box.center();
			p.y = box.min.y + (box.max.y - box.min.y) / 3.0f;
			
			vec3 v = p - pos;
			float d = v.length();

			if (d > dist || !checkOcclusion(from, p, d))
				continue;

			target = enemy;

		} while ((c = c->next));
*/
		Controller* target = NULL;

		Controller *c = Controller::first;
		do {
			if (!c->getEntity().isEnemy())
				continue;

			Character *enemy = (Character*)c;
			if (!enemy->isActiveTarget())
				continue;

			if (collide(enemy))
				target = enemy;

		} while ((c = c->next));

		return target;
	}

	virtual void applyFlow(TR::Camera &sink)
	{
		if (stand != STAND_UNDERWATER) return;
		Lara::applyFlow(sink);
		sm64_add_mario_position(marioId, flowVelocity.x/MARIO_SCALE, -flowVelocity.y/MARIO_SCALE, -flowVelocity.z/MARIO_SCALE);
	}

	virtual bool checkInteraction(Controller *controller, const TR::Limits::Limit *limit, bool action)
	{
		bool result = Lara::checkInteraction(controller, limit, action);
		if (result) marioInputs.buttonB = false;
		return result;
	}

	virtual void checkTrigger(Controller *controller, bool heavy)
	{
		TR::Level::FloorInfo info;
		getFloorInfo(controller->getRoomIndex(), controller->pos, info);

		if (getEntity().isLara() && info.lava && info.floor == pos.y) {
			hit(LARA_MAX_HEALTH + 1, NULL, TR::HIT_LAVA);
			return;
		}

		if (!info.trigCmdCount) return; // has no trigger

		if (camera->mode != Camera::MODE_HEAVY) {
			refreshCamera(info);
		}

		TR::Limits::Limit *limit = NULL;
		bool switchIsDown = false;
		float timer = info.trigInfo.timer == 1 ? EPS : float(info.trigInfo.timer);
		int cmdIndex = 0;
		int actionState = state;

		switch (info.trigger) {
			case TR::Level::Trigger::ACTIVATE : break;

			case TR::Level::Trigger::SWITCH : {
				Switch *controller = (Switch*)level->entities[info.trigCmd[cmdIndex++].args].controller;

				if (controller->flags.state != TR::Entity::asActive) {
					limit = state == STATE_STOP ? (controller->getEntity().type == TR::Entity::SWITCH_BUTTON ? &TR::Limits::SWITCH_BUTTON : &TR::Limits::SWITCH_MARIO) : &TR::Limits::SWITCH_UNDERWATER;
					if (checkInteraction(controller, limit, Input::state[camera->cameraIndex][cAction])) {
						actionState = (controller->state == Switch::STATE_DOWN && stand == STAND_GROUND) ? STATE_SWITCH_UP : STATE_SWITCH_DOWN;

						int animIndex;
						switch (controller->getEntity().type) {
							case TR::Entity::SWITCH_BUTTON : animIndex = ANIM_PUSH_BUTTON; break;
							case TR::Entity::SWITCH_BIG    : animIndex = controller->state == Switch::STATE_DOWN ? ANIM_SWITCH_BIG_UP : ANIM_SWITCH_BIG_DOWN; break;
							default : animIndex = -1;
						}

						state = actionState;
						controller->activate();
					}
				}

				if (!controller->setTimer(timer))
					return;

				switchIsDown = controller->state == Switch::STATE_DOWN;
				state = STATE_STOP;
				break;
			}

			case TR::Level::Trigger::KEY : {
				TR::Entity &entity = level->entities[info.trigCmd[cmdIndex++].args];
				KeyHole *controller = (KeyHole*)entity.controller;

				if (controller->flags.state == TR::Entity::asNone) {
					if (controller->flags.active == TR::ACTIVE || state != STATE_STOP)
					{
						//printf("is active: %s, state != state_stop: %s\n", controller->flags.active == TR::ACTIVE ? "y":"n", state != STATE_STOP ? "y":"n");
						return;
					}

					actionState = entity.isPuzzleHole() ? STATE_USE_PUZZLE : STATE_USE_KEY;
					//if (!animation.canSetState(actionState))
						//return;

					limit = actionState == STATE_USE_PUZZLE ? &TR::Limits::PUZZLE_HOLE_MARIO : &TR::Limits::KEY_HOLE_MARIO;
					//printf("check return 1 limit %d: %s, %s (%d %d)\n", limit, isPressed(ACTION) ? "y":"n", usedItem != TR::Entity::NONE ? "y":"n", usedItem, TR::Entity::NONE);
					if (!checkInteraction(controller, limit, isPressed(ACTION) || usedItem != TR::Entity::NONE))
					{
						//printf("returned 1\n");
						return;
					}

					//printf("past 1\n");
					if (usedItem == TR::Entity::NONE) {
						if (isPressed(ACTION) && !game->invChooseKey(camera->cameraIndex, entity.type))
							sm64_play_sound_global(SOUND_MENU_CAMERA_BUZZ);
							//game->playSound(TR::SND_NO, pos, Sound::PAN); // no compatible items in inventory
						//printf("return 2\n");
						return;
					}
					//printf("past 2\n");

					if (TR::Level::convToInv(TR::Entity::getItemForHole(entity.type)) != usedItem) { // check compatibility if user select other
						//game->playSound(TR::SND_NO, pos, Sound::PAN); // uncompatible item
						//printf("return 3\n");
						sm64_play_sound_global(SOUND_MENU_CAMERA_BUZZ);
						return;
					}
					//printf("past 3\n");

					keyHole = controller;

					if (game->invUse(camera->cameraIndex, usedItem)) {
						/*
						keyItem = game->addEntity(usedItem, getRoomIndex(), pos, 0);
						keyItem->lockMatrix = true;
						keyItem->pos     = keyHole->pos + vec3(0, -590, 484).rotateY(-keyHole->angle.y);
						keyItem->angle.x = PI * 0.5f;
						keyItem->angle.y = keyHole->angle.y;
						*/
					}

					//printf("yup\n");
					state = actionState;
					sm64_set_mario_action(marioId, 0x0000132E); // ACT_UNLOCKING_KEY_DOOR
				}

				if (controller->flags.state != TR::Entity::asInactive)
				{
					//printf("as inactive\n");
					return;
				}

				break;
			}

			case TR::Level::Trigger::PICKUP : {
				Controller *controller = (Controller*)level->entities[info.trigCmd[cmdIndex++].args].controller;
				if (!controller->flags.invisible)
					return;
				break;
			}

			case TR::Level::Trigger::COMBAT :
				if (wpnReady() && !emptyHands())
					return;
				break;

			case TR::Level::Trigger::PAD :
			case TR::Level::Trigger::ANTIPAD :
				if (pos.y != info.floor) return;
				break;

			case TR::Level::Trigger::HEAVY :
				if (!heavy) return;
				break;
			case TR::Level::Trigger::DUMMY :
				return;
		}

		bool needFlip = false;
		TR::Effect::Type effect = TR::Effect::NONE;

		while (cmdIndex < info.trigCmdCount) {
			TR::FloorData::TriggerCommand &cmd = info.trigCmd[cmdIndex++];

			switch (cmd.action) {
				case TR::Action::ACTIVATE : {
					if (cmd.args >= level->entitiesBaseCount) {
						break;
					}
					TR::Entity &e = level->entities[cmd.args];
					Controller *controller = (Controller*)e.controller;
					ASSERT(controller);
					TR::Entity::Flags &flags = controller->flags;

					if (flags.once)
						break;
					controller->timer = timer;

					if (info.trigger == TR::Level::Trigger::SWITCH)
						flags.active ^= info.trigInfo.mask;
					else if (info.trigger == TR::Level::Trigger::ANTIPAD)
						flags.active &= ~info.trigInfo.mask;
					else
						flags.active |= info.trigInfo.mask;

					if (flags.active != TR::ACTIVE)
						break;

					flags.once |= info.trigInfo.once;
					
					controller->activate();
					break;
				}
				case TR::Action::CAMERA_SWITCH : {
					TR::FloorData::TriggerCommand &cam = info.trigCmd[cmdIndex++];

					if (!level->cameras[cmd.args].flags.once) {
						camera->viewIndex = cmd.args;

						if (!(info.trigger == TR::Level::Trigger::COMBAT) &&
							!(info.trigger == TR::Level::Trigger::SWITCH && info.trigInfo.timer && !switchIsDown) &&
							 (info.trigger == TR::Level::Trigger::SWITCH || camera->viewIndex != camera->viewIndexLast))
						{
							camera->smooth = cam.speed > 0;
							camera->mode   = heavy ? Camera::MODE_HEAVY : Camera::MODE_STATIC;
							camera->timer  = cam.timer == 1 ? EPS : float(cam.timer);
							camera->speed  = cam.speed * 8;

							level->cameras[camera->viewIndex].flags.once |= cam.once;
						}
					}
					break;
				}
				case TR::Action::FLOW :
					applyFlow(level->cameras[cmd.args]);
					break;
				case TR::Action::FLIP : {
					SaveState::ByteFlags &flip = level->state.flipmaps[cmd.args];

					if (flip.once)
						break;

					if (info.trigger == TR::Level::Trigger::SWITCH)
						flip.active ^= info.trigInfo.mask;
					else
						flip.active |= info.trigInfo.mask;

					if (flip.active == TR::ACTIVE)
						flip.once |= info.trigInfo.once;

					if ((flip.active == TR::ACTIVE) ^ level->state.flags.flipped)
						 needFlip = true;

					break;
				}
				case TR::Action::FLIP_ON :
					if (level->state.flipmaps[cmd.args].active == TR::ACTIVE && !level->state.flags.flipped)
						needFlip = true;
					break;
				case TR::Action::FLIP_OFF :
					if (level->state.flipmaps[cmd.args].active == TR::ACTIVE && level->state.flags.flipped)
						needFlip = true;
					break;
				case TR::Action::CAMERA_TARGET :
					if (camera->mode == Camera::MODE_STATIC || camera->mode == Camera::MODE_HEAVY) {
						camera->viewTarget = (Controller*)level->entities[cmd.args].controller;
					}
					break;
				case TR::Action::END :
					game->loadNextLevel();
					break;
				case TR::Action::SOUNDTRACK : {
					int track = doTutorial(cmd.args);

					if (track == 0) break;

				// check trigger
					SaveState::ByteFlags &flags = level->state.tracks[track];

					if (flags.once)
						break;

					if (info.trigger == TR::Level::Trigger::SWITCH)
						flags.active ^= info.trigInfo.mask;
					else if (info.trigger == TR::Level::Trigger::ANTIPAD)
						flags.active &= ~info.trigInfo.mask;
					else
						flags.active |= info.trigInfo.mask;

					if ( (flags.active == TR::ACTIVE) || (((level->version & (TR::VER_TR2 | TR::VER_TR3))) && flags.active) ) {
						flags.once |= info.trigInfo.once;
						game->playTrack(track);
					} else
						game->stopTrack();

					break;
				}
				case TR::Action::EFFECT :
					effect = TR::Effect::Type(cmd.args);
					break;
				case TR::Action::SECRET :
					if (!(saveStats.secrets & (1 << cmd.args))) {
						saveStats.secrets |= 1 << cmd.args;
						if (!game->playSound(TR::SND_SECRET, pos))
							game->playTrack(TR::TRACK_TR1_SECRET, true);
					}
					break;
				case TR::Action::CLEAR_BODIES :
					break;
				case TR::Action::FLYBY :
					cmdIndex++; // TODO
					break;
				case TR::Action::CUTSCENE :
					cmdIndex++; // TODO
					break;
			}
		}

		if (needFlip) {
			game->flipMap();
			game->setEffect(this, effect);
		}
	}

	virtual void doCustomCommand(int curFrame, int prevFrame)
	{
		int16_t rot[3];
		struct SM64AnimInfo *marioAnim = sm64_mario_get_anim_info(marioId, rot);
		if (!marioAnim || !marioAnim->curAnim) return;
		//if (marioAnim && marioAnim->curAnim && marioState.action) printf("%d %x %d %d\n", state, marioState.action, marioAnim->animFrame, marioAnim->curAnim->loopEnd-1);

		switch (state)
		{
			case STATE_MIDAS_USE:
				if (marioAnim->animID == MARIO_ANIM_CREDITS_RAISE_HAND && marioAnim->animFrame == marioAnim->curAnim->loopEnd-1)
				{
					timer += Core::deltaTime;
					if (timer >= 1.0f / 30.0f)
					{
						timer -= 1.0f / 30.0f;
						vec3 sprPos(pos.x - (cos(angle.y)*128) + (-64 + (rand() % 128)), pos.y - 512-64 + (-64 + (rand() % 128)), pos.z - (sin(angle.y)*128) + (-64 + (rand() % 128)));
						game->addEntity(TR::Entity::SPARKLES, getRoomIndex(), sprPos);
					}
				}
				else if (marioAnim->animID == MARIO_ANIM_CREDITS_LOWER_HAND)
				{
					timer += Core::deltaTime;
					if (timer >= 1.0f / 30.0f)
					{
						timer -= 1.0f / 30.0f;
						for (int i=0; i<3; i++)
						{
							vec3 sprPos(pos.x - (cos(angle.y) * (128 + (-96 + (i*96))) + (-128 + (rand() % 256))),
										pos.y - 768 + (marioAnim->animFrame*12),
										pos.z - (sin(angle.y) * (128 + (-96 + (i*96))) + (-128 + (rand() % 256))));

							game->addEntity(TR::Entity::SPARKLES, getRoomIndex(), sprPos);
						}
					}
					if (marioAnim->animFrame == marioAnim->curAnim->loopEnd-1)
					{
						sm64_set_mario_action(marioId, 0x0C400201); // ACT_IDLE
						state = STATE_STOP;
					}
				}
				break;

			case STATE_PUSH_BLOCK:
				{
					static bool moved = false;
					if (!movingBlock)
						state = STATE_STOP;
					else if (marioState.flags & 0x00100000 && !movingBlock->marioAnim && !moved) // MARIO_PUNCHING
					{
						movingBlock->doMarioMove(true);
						moved = true;
					}
					else if (!movingBlock->marioAnim && moved)
					{
						movingBlock = NULL;
						state = STATE_STOP;
						moved = false;
					}
				}
				break;
				
			case STATE_PULL_BLOCK:
				if (!movingBlock) state = STATE_STOP;
				else if (!movingBlock->marioAnim)
				{
					if (marioState.action != 0x00000392) sm64_set_mario_action(marioId, 0x00000392);
					else if (marioAnim->animFrame == marioAnim->curAnim->loopEnd-1)
					{
						movingBlock = NULL;
						state = STATE_STOP;
					}
				}
				break;

			case STATE_PICK_UP:
				{
					int end = (marioState.action == 0x00001319) ? 60 : marioAnim->curAnim->loopEnd-1;
					if (marioAnim->animFrame == end) // anim done, pick up
					{
						if (marioState.action == 0x800380 || marioState.action == 0x300024E1) // punching action || ACT_WATER_PUNCH
						{
							if (stand != STAND_UNDERWATER && stand != STAND_ONWATER) sm64_set_mario_action(marioId, 0x00000383); // ACT_PICKING_UP
							else
								state = STATE_STOP;

							camera->setup(true);
							for (int i = 0; i < pickupListCount; i++)
							{
								Controller *item = pickupList[i];

								if (item->getEntity().type == TR::Entity::SCION_PICKUP_HOLDER)
									continue;
								item->deactivate();
								item->flags.invisible = true;
								game->invAdd(item->getEntity().type, 1);

								vec4 p = game->projectPoint(vec4(item->pos, 1.0f));

								#ifdef _OS_WP8
									swap(p.x, p.y);
								#endif

								if (p.w != 0.0f) {
									p.x = ( p.x / p.w * 0.5f + 0.5f) * UI::width;
									p.y = (-p.y / p.w * 0.5f + 0.5f) * UI::height;
									if (game->getLara(1)) {
										p.x *= 0.5f;
									}
								} else
									p = vec4(UI::width * 0.5f, UI::height * 0.5f, 0.0f, 0.0f);

								UI::addPickup(item->getEntity().type, camera->cameraIndex, vec2(p.x, p.y));
								saveStats.pickups++;
							}
							pickupListCount = 0;
						}
						else if (marioState.action == 0x00000383) // ACT_PICKING_UP
						{
							if (pickupList[0] && pickupList[0]->getEntity().type == TR::Entity::SCION_PICKUP_QUALOPEC)
							{
								sm64_set_mario_action(marioId, 0x00001319); // ACT_CREDITS_CUTSCENE
								sm64_set_mario_animation(marioId, MARIO_ANIM_CREDITS_RAISE_HAND);
							}
							else
							{
								state = STATE_STOP;
								sm64_set_mario_action(marioId, 0x0C400201); // ACT_IDLE
							}
						}
						else if (marioState.action == 0x00001319) // ACT_CREDITS_CUTSCENE
						{
							state = STATE_STOP;
							sm64_set_mario_action(marioId, 0x0C400201); // ACT_IDLE
						}
					}
				}
				break;

			case STATE_USE_KEY:
			case STATE_USE_PUZZLE:
			{
				if (keyHole)
				{
					int end = (state == STATE_USE_KEY) ? 15 : 57;
					if (marioState.action == 0x0000132E && marioAnim->animFrame == marioAnim->curAnim->loopEnd-end)
					{
						sm64_set_mario_action(marioId, 0x0C400201); // ACT_IDLE
						state = STATE_STOP;
						keyHole->activate();
						keyHole = NULL;
					}
				}
				break;
			}
			break;
		}
	}

	void marioUpdateRoom()
	{
		// load sm64surfaces
		size_t surfaces_count = 0;
		size_t surface_ind = 0;
		TR::Room &room = getRoom();

		// get meshes from this room
		TR::Room::Data &d = room.data;
		for (int i = 0; i < d.fCount; i++)
		{
			TR::Face &f = d.faces[i];
			if (f.water) continue;

			surfaces_count += (f.triangle) ? 1 : 2;
		}

		COUNT_ROOM_SECTORS(level, surfaces_count, room);

		// get meshes from this room's portals to adjacent rooms
		for (int i = 0; i < room.portalsCount; i++)
		{
			TR::Room &room2 = level->rooms[room.portals[i].roomIndex];
			TR::Room::Data &d2 = room2.data;

			for (int j = 0; j < d2.fCount; j++)
			{
				TR::Face &f = d2.faces[j];
				if (f.water) continue;

				surfaces_count += (f.triangle) ? 1 : 2;
			}

			COUNT_ROOM_SECTORS(level, surfaces_count, room2);

			/*
			for (int j = 0; j < room2.portalsCount; j++)
			{
				if (room2.portals[j].roomIndex == getRoomIndex()) continue;
				TR::Room &room3 = level->rooms[room2.portals[j].roomIndex];
				TR::Room::Data &d3 = room3.data;

				for (int k = 0; k < d3.fCount; k++)
				{
					TR::Face &f = d3.faces[k];
					if (f.water) continue;

					surfaces_count += (f.triangle) ? 1 : 2;
				}

				COUNT_ROOM_SECTORS(level, surfaces_count, room3);

				/*
				for (int k = 0; k < room3.portalsCount; k++)
				{
					if (room3.portals[k].roomIndex == room2.portals[j].roomIndex) continue;
					TR::Room &room4 = level->rooms[room3.portals[k].roomIndex];
					TR::Room::Data &d4 = room4.data;

					for (int l = 0; l < d4.fCount; l++)
					{
						TR::Face &f = d4.faces[l];
						if (f.water) continue;

						surfaces_count += (f.triangle) ? 1 : 2;
					}
				}
			}
			*/
		}

		struct SM64Surface surfaces[surfaces_count];
		for (int i = 0; i < d.fCount; i++)
		{
			TR::Face &f = d.faces[i];
			if (f.water) continue;

			ADD_FACE(surfaces, surface_ind, room, d, f);
		}

		ADD_ROOM_SECTORS(level, surfaces, surface_ind, room);

		for (int i = 0; i < room.portalsCount; i++)
		{
			TR::Room &room2 = level->rooms[room.portals[i].roomIndex];
			TR::Room::Data &d2 = room2.data;

			for (int j = 0; j < d2.fCount; j++)
			{
				TR::Face &f = d2.faces[j];
				if (f.water) continue;

				ADD_FACE(surfaces, surface_ind, room2, d2, f);
			}

			ADD_ROOM_SECTORS(level, surfaces, surface_ind, room2);

			/*
			for (int j = 0; j < room2.portalsCount; j++)
			{
				if (room2.portals[j].roomIndex == getRoomIndex()) continue;
				TR::Room &room3 = level->rooms[room2.portals[j].roomIndex];
				TR::Room::Data &d3 = room3.data;

				for (int k = 0; k < d3.fCount; k++)
				{
					TR::Face &f = d3.faces[k];
					if (f.water) continue;

					ADD_FACE(surfaces, surface_ind, room3, d3, f);
				}

				ADD_ROOM_SECTORS(level, surfaces, surface_ind, room3);

				for (int k = 0; k < room3.portalsCount; k++)
				{
					if (room3.portals[k].roomIndex == room2.portals[j].roomIndex) continue;
					TR::Room &room4 = level->rooms[room3.portals[k].roomIndex];
					TR::Room::Data &d4 = room4.data;

					for (int l = 0; l < d4.fCount; l++)
					{
						TR::Face &f = d4.faces[l];
						if (f.water) continue;

						ADD_FACE(surfaces, surface_ind, room4, d4, f);
					}
				}
			}
			*/
		}

		sm64_static_surfaces_load((const struct SM64Surface*)&surfaces, surfaces_count);
	}

	void update()
	{
		if (level->isCutsceneLevel()) {
			updateAnimation(true);

			updateLights();

			if (fixRoomIndex()) {
				for (int i = 0; i < COUNT(braid); i++) {
					if (braid[i]) {
						braid[i]->update();
					}
				}
			}
		} else {
			switch (usedItem) {
				case TR::Entity::INV_MEDIKIT_SMALL :
				case TR::Entity::INV_MEDIKIT_BIG   :
					if (health < LARA_MAX_HEALTH) {
						damageTime = LARA_DAMAGE_TIME;
						health = min(LARA_MAX_HEALTH, health + (usedItem == TR::Entity::INV_MEDIKIT_SMALL ? LARA_MAX_HEALTH / 2 : LARA_MAX_HEALTH));
						//game->playSound(TR::SND_HEALTH, pos, Sound::PAN);
						sm64_play_sound_global(SOUND_MENU_POWER_METER);
						inventory->remove(usedItem);
					}
					usedItem = TR::Entity::NONE;
				default : ;
			}

			int oldRoom = getRoomIndex();
			updateRoom();
			if (getRoomIndex() != oldRoom) marioUpdateRoom();

			if (getRoom().waterLevelSurface != TR::NO_WATER) sm64_set_mario_water_level(marioId, -getRoom().waterLevelSurface/IMARIO_SCALE);
			else if (getRoom().flags.water) sm64_set_mario_water_level(marioId, 32767);
			else sm64_set_mario_water_level(marioId, -32768);

			if (animation.index != ANIM_STAND) animation.setAnim(ANIM_STAND);

			vec3 p = pos;
			input = getInput();
			stand = getStand();
			updateState();
			Controller::update();

			for (int i=0; i<objCount; i++)
			{
				struct MarioControllerObj *obj = &objs[i];
				if (!obj->entity || (obj->entity->type >= 68 && obj->entity->type <= 70)) continue;

				if (obj->entity->controller)
				{
					Controller *c = (Controller*)obj->entity->controller;
					if ((c->pos.x != obj->transform.position[0]*MARIO_SCALE || c->pos.y - obj->topPoint != obj->transform.position[1]*MARIO_SCALE || c->pos.z != -obj->transform.position[2]*MARIO_SCALE) &&
					    (c->pos.x != obj->transform.position[0]*MARIO_SCALE || c->pos.y - obj->topPoint != -obj->transform.position[1]*MARIO_SCALE || c->pos.z != -obj->transform.position[2]*MARIO_SCALE))
					{
						printf("moving %d: %.2f %.2f %.2f - %.2f %.2f %.2f\n", i, c->pos.x, c->pos.y - obj->topPoint, c->pos.z, obj->transform.position[0]*MARIO_SCALE, obj->transform.position[1]*MARIO_SCALE, -obj->transform.position[2]*MARIO_SCALE);
						obj->transform.position[0] = c->pos.x/MARIO_SCALE;
						obj->transform.position[1] = -(c->pos.y - obj->topPoint)/MARIO_SCALE;
						obj->transform.position[2] = -c->pos.z/MARIO_SCALE;
						sm64_surface_object_move(obj->ID, &obj->transform);
					}
				}
			}

			if (burn)
			{
				if (marioState.action != 0x010208B4 && marioState.action != 0x010208B5 && marioState.action != 0x010208B7 && marioState.action != 0x00020449)
				{
					sm64_set_mario_action_arg(marioId, 0x010208B4, 1);
					sm64_play_sound_global(SOUND_MARIO_ON_FIRE);
				}
				else if (marioState.burnTimer <= 0)
					burn = false;
			}

			marioInputs.camLookX = marioState.position[0] - (camera->spectator ? camera->specPosSmooth.x : camera->eye.pos.x);
			marioInputs.camLookZ = marioState.position[2] + (camera->spectator ? camera->specPosSmooth.z : camera->eye.pos.z);

			if (flags.active) {
				// do mario64 tick here
				marioTicks += Core::deltaTime;
				checkTrigger(this, false);

				while (marioTicks > 1./30)
				{
					for (int i=0; i<3; i++) lastPos[i] = marioState.position[i];
					for (int i=0; i<3 * marioRenderState.mario.num_vertices; i++) lastGeom[i] = marioGeometry.position[i];

					sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry);
					marioTicks -= 1./30;
					marioRenderState.mario.num_vertices = 3 * marioGeometry.numTrianglesUsed;

					for (int i=0; i<3; i++) currPos[i] = marioState.position[i] * MARIO_SCALE;
					for (int i=0; i<3 * marioRenderState.mario.num_vertices; i++)
					{
						currGeom[i] = marioGeometry.position[i] * MARIO_SCALE;

						if (i%3 != 0) // flip y and z
						{
							currGeom[i] = -currGeom[i];
							marioGeometry.normal[i] = -marioGeometry.normal[i];
						}
					}
				}

				for (int i=0; i<3; i++) marioState.position[i] = lerp(lastPos[i], currPos[i], marioTicks/(1./30));
				for (int i=0; i<3 * marioRenderState.mario.num_vertices; i++) marioGeometry.position[i] = lerp(lastGeom[i], currGeom[i], marioTicks/(1./30));
				TRmarioMesh->update(&marioGeometry);

				float hp = health / float(LARA_MAX_HEALTH);
				float ox = oxygen / float(LARA_MAX_OXYGEN);
				if (hp > 0.f)
				{
					switch(stand)
					{
						case STAND_UNDERWATER:
							sm64_mario_set_health(marioId, (ox <= 0.2f) ? 0x200 : 0x880); // if low on oxygen, play sound
							break;
						
						case STAND_ONWATER:
							sm64_mario_set_health(marioId, 0x880); // don't play low oxygen sound on surface
							break;
						
						default:
							sm64_mario_set_health(marioId, (hp <= 0.2f) ? 0x200 : 0x880); // mario panting animation if low on health
							break;
					}
				}

				// find an enemy close by
				Controller* target = marioFindTarget();
				if (target)
				{
					Sphere spheres[MAX_JOINTS];
					int count = target->getSpheres(spheres);
					for (int i=0; i<count; i++)
					{
						if (collide(spheres[i]) && (sm64_mario_attack(marioId, spheres[i].center.x, spheres[i].center.y, spheres[i].center.z, target->getBoundingBox().max.y - target->getBoundingBox().min.y) ||
						    sm64_mario_attack(marioId, spheres[i].center.x, -spheres[i].center.y, -spheres[i].center.z, target->getBoundingBox().max.y - target->getBoundingBox().min.y)))
						{
							float damage = 5.f;
							if (marioState.action == 0x018008AC) // ACT_JUMP_KICK
								damage += 5;
							else if (marioState.action == 0x008008A9) // ACT_GROUND_POUND
							{
								damage += 15;
								sm64_set_mario_action(marioId, 0x01000882); // ACT_TRIPLE_JUMP
								sm64_play_sound_global(SOUND_ACTION_HIT);
							}

							if (marioState.flags & 0x00000004)
								damage += 25;

							printf("attacked sphere %d/%d: %.2f %.2f %.2f - %.2f %.2f %.2f\n", i, count, marioState.position[0], -marioState.position[1], -marioState.position[2], spheres[i].center.x, spheres[i].center.y, spheres[i].center.z);
							((Character*)target)->hit(damage, this);
							break;
						}
					}
				}
				//if (marioState.action & (1 << 23)) // ACT_FLAG_ATTACKING
				//{
				//}

				if (marioId >= 0)
				{
					angle.y = -marioState.faceAngle + M_PI;
					pos.x = (stand == STAND_HANG) ? marioState.position[0] - (sin(angle.y)*64) : marioState.position[0];
					pos.y = (stand == STAND_HANG) ? -marioState.position[1]+128 : -marioState.position[1];
					pos.z = (stand == STAND_HANG) ? -marioState.position[2] - (cos(angle.y)*64) : -marioState.position[2];
				}
				if (p != pos && updateZone())
					updateLights();
			}
        }
        
        camera->update();

        if (hitTimer > 0.0f) {
            hitTimer -= Core::deltaTime;
            if (hitTimer > 0.0f)
                Input::setJoyVibration(camera->cameraIndex, 0.5f, 0.5f);
            else
                Input::setJoyVibration(camera->cameraIndex, 0, 0);
        }

        if (level->isCutsceneLevel())
            return;

        if (damageTime > 0.0f)
            damageTime = max(0.0f, damageTime - Core::deltaTime);

        if (stand == STAND_UNDERWATER && !dozy) {
            if (oxygen > 0.0f)
                oxygen -= Core::deltaTime;
            else
                hit(Core::deltaTime * 150.0f);
        } else
            if (oxygen < LARA_MAX_OXYGEN && health > 0.0f)
                oxygen = min(LARA_MAX_OXYGEN, oxygen + Core::deltaTime * 10.0f);

        usedItem = TR::Entity::NONE;

        if (camera->mode != Camera::MODE_CUTSCENE && camera->mode != Camera::MODE_STATIC) {
            camera->mode = (emptyHands() || health <= 0.0f) ? Camera::MODE_FOLLOW : Camera::MODE_COMBAT;
            if (input & LOOK)
                camera->mode = Camera::MODE_LOOK;
        }

        if (keyItem) {
            keyItem->flags.invisible = animation.frameIndex < (state == STATE_USE_KEY ? 70 : 30);
            keyItem->lockMatrix = true;
            mat4 &m = keyItem->matrix;
            Basis b;

            if (state == STATE_USE_KEY) {
                b = getJoint(10);
                b.rot = b.rot * quat(vec3(0, 1, 0), PI * 0.5f);
                b.translate(vec3(0, 120, 0));
            } else {
                vec3 rot(0.0f);
                
                // TODO: hardcode item-hand alignment 8)
                rot.x = -PI * 0.5f;

                if (animation.frameIndex < 55)
                    b = getJoint(13);
                else
                    b = getJoint(10);

                b.translate(vec3(0, 48, 0));

                if (rot.x != 0.0f) b.rot = b.rot * quat(vec3(1, 0, 0), rot.x);
                if (rot.y != 0.0f) b.rot = b.rot * quat(vec3(0, 1, 0), rot.y);
                if (rot.z != 0.0f) b.rot = b.rot * quat(vec3(0, 0, 1), rot.z);
            }

            keyItem->joints[0] = b;

            m.identity();
            m.setRot(b.rot);
            m.setPos(b.pos);
            //m = getMatrix();
        }
	}

	void move()
	{
		//Lara::move();
	}

	void updatePosition()
	{
		//Lara::updatePosition();
		//printf("L %.2f %.2f %.2f\n", pos.x, pos.y, pos.z);
		//printf("M %.2f %.2f %.2f\n", marioState.position[0], marioState.position[1], marioState.position[2]);
	}

	void updateVelocity()
	{
		//Lara::updateVelocity();
	}
};

#endif
