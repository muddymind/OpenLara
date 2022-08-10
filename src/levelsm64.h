#ifndef H_LEVELSM64
#define H_LEVELSM64

extern "C" {
	#include <libsm64/src/libsm64.h>
	#include <libsm64/src/decomp/include/external_types.h>
	#include <libsm64/src/decomp/include/surface_terrains.h>
	#include <libsm64/src/decomp/include/PR/ultratypes.h>
	#include <libsm64/src/decomp/include/audio_defines.h>
	#include <libsm64/src/decomp/include/seq_ids.h>
	#include <libsm64/src/decomp/include/mario_animation_ids.h>
}


#include "core.h"
// #include "game.h"
// #include "lara.h"
// #include "objects.h"
// #include "sprite.h"
// #include "enemy.h"
// #include "inventory.h"
// #include "mesh.h"
#include "format.h"
// #include "level.h"
#include "controller.h"

#include "marioMacros.h"

struct MarioControllerObj
{
	MarioControllerObj() : ID(0), entity(NULL), spawned(false) {}

	uint32_t ID;
	struct SM64ObjectTransform transform;
	TR::Entity* entity;
	vec3 offset;
	bool spawned;
};

struct LevelSM64
{
    TR::Level *level=NULL;
	IController *controller=NULL;
    struct MarioControllerObj dynamicObjects[4096];
	int dynamicObjectsCount=0;

    LevelSM64()
    {
    }
    ~LevelSM64()
    {
        sm64_level_unload();
    }

    struct SM64Surface* marioLoadRoomSurfaces(int roomId, int *room_surfaces_count)
	{
		*room_surfaces_count = 0;
		size_t level_surface_i = 0;

		TR::Room &room = level->rooms[roomId];
		TR::Room::Data &d = room.data;

		// Count the number of static surface triangles
		for (int j = 0; j < d.fCount; j++)
		{
			TR::Face &f = d.faces[j];
			if (f.water) continue;

			*room_surfaces_count += (f.triangle) ? 1 : 2;
		}

		struct SM64Surface *collision_surfaces = (struct SM64Surface *)malloc(sizeof(struct SM64Surface) * (*room_surfaces_count));

		// Generate all static surface triangles
		for (int cface = 0; cface < d.fCount; cface++)
		{
			TR::Face &f = d.faces[cface];
				if (f.water) continue;

			int16 x[2] = {d.vertices[f.vertices[0]].pos.x, 0};
			int16 z[2] = {d.vertices[f.vertices[0]].pos.z, 0};
			for (int j=1; j<3; j++)
			{
				if (x[0] != d.vertices[f.vertices[j]].pos.x) x[1] = d.vertices[f.vertices[j]].pos.x;
				if (z[0] != d.vertices[f.vertices[j]].pos.z) z[1] = d.vertices[f.vertices[j]].pos.z;
			}

			float midX = (x[0] + x[1]) / 2.f;
			float topY = max(d.vertices[f.vertices[0]].pos.y, max(d.vertices[f.vertices[1]].pos.y, d.vertices[f.vertices[2]].pos.y));
			float midZ = (z[0] + z[1]) / 2.f;

			TR::Level::FloorInfo info;

			bool slippery = false;
			if(controller) {
				controller->getFloorInfo(roomId, vec3(room.info.x + midX, topY, room.info.z + midZ), info);
				slippery = (abs(info.slantX) > 2 || abs(info.slantZ) > 2);
			}

			ADD_FACE(collision_surfaces, level_surface_i, room, d, f, slippery);
		}

		return collision_surfaces;
	}

	TR::Mesh *generateMeshBoundingBox(TR::StaticMesh *sm)
	{
		Box box;
		sm->getBox(true, box);
		
		TR::Mesh *boundingBox = (TR::Mesh *) malloc(sizeof(TR::Mesh));
		boundingBox->fCount = 12;
		boundingBox->faces = (TR::Face *)malloc(sizeof(TR::Face)*boundingBox->fCount);

		boundingBox->vCount = 8;
		boundingBox->vertices = (TR::Mesh::Vertex *)malloc(sizeof(TR::Mesh::Vertex)*boundingBox->vCount);

		boundingBox->vertices[0].coord.x=box.min.x;
		boundingBox->vertices[0].coord.y=box.min.y;
		boundingBox->vertices[0].coord.z=box.min.z;

		boundingBox->vertices[1].coord.x=box.min.x;
		boundingBox->vertices[1].coord.y=box.max.y;
		boundingBox->vertices[1].coord.z=box.min.z;

		boundingBox->vertices[2].coord.x=box.max.x;
		boundingBox->vertices[2].coord.y=box.max.y;
		boundingBox->vertices[2].coord.z=box.min.z;

		boundingBox->vertices[3].coord.x=box.max.x;
		boundingBox->vertices[3].coord.y=box.min.y;
		boundingBox->vertices[3].coord.z=box.min.z;

		boundingBox->vertices[4].coord.x=box.min.x;
		boundingBox->vertices[4].coord.y=box.min.y;
		boundingBox->vertices[4].coord.z=box.max.z;

		boundingBox->vertices[5].coord.x=box.min.x;
		boundingBox->vertices[5].coord.y=box.max.y;
		boundingBox->vertices[5].coord.z=box.max.z;

		boundingBox->vertices[6].coord.x=box.max.x;
		boundingBox->vertices[6].coord.y=box.max.y;
		boundingBox->vertices[6].coord.z=box.max.z;
		
		boundingBox->vertices[7].coord.x=box.max.x;
		boundingBox->vertices[7].coord.y=box.min.y;
		boundingBox->vertices[7].coord.z=box.max.z;

		boundingBox->faces[0].triangle=1;
		boundingBox->faces[0].vertices[0]=2;
		boundingBox->faces[0].vertices[1]=1;
		boundingBox->faces[0].vertices[2]=0;

		boundingBox->faces[1].triangle=1;
		boundingBox->faces[1].vertices[0]=0;
		boundingBox->faces[1].vertices[1]=3;
		boundingBox->faces[1].vertices[2]=2;

		boundingBox->faces[2].triangle=1;
		boundingBox->faces[2].vertices[0]=2;
		boundingBox->faces[2].vertices[1]=3;
		boundingBox->faces[2].vertices[2]=7;

		boundingBox->faces[3].triangle=1;
		boundingBox->faces[3].vertices[0]=7;
		boundingBox->faces[3].vertices[1]=6;
		boundingBox->faces[3].vertices[2]=2;

		boundingBox->faces[4].triangle=1;
		boundingBox->faces[4].vertices[0]=6;
		boundingBox->faces[4].vertices[1]=7;
		boundingBox->faces[4].vertices[2]=4;

		boundingBox->faces[5].triangle=1;
		boundingBox->faces[5].vertices[0]=4;
		boundingBox->faces[5].vertices[1]=5;
		boundingBox->faces[5].vertices[2]=6;

		boundingBox->faces[6].triangle=1;
		boundingBox->faces[6].vertices[0]=4;
		boundingBox->faces[6].vertices[1]=0;
		boundingBox->faces[6].vertices[2]=5;

		boundingBox->faces[7].triangle=1;
		boundingBox->faces[7].vertices[0]=0;
		boundingBox->faces[7].vertices[1]=1;
		boundingBox->faces[7].vertices[2]=5;

		boundingBox->faces[8].triangle=1;
		boundingBox->faces[8].vertices[0]=7;
		boundingBox->faces[8].vertices[1]=3;
		boundingBox->faces[8].vertices[2]=0;

		boundingBox->faces[9].triangle=1;
		boundingBox->faces[9].vertices[0]=0;
		boundingBox->faces[9].vertices[1]=4;
		boundingBox->faces[9].vertices[2]=7;

		boundingBox->faces[10].triangle=1;
		boundingBox->faces[10].vertices[0]=6;
		boundingBox->faces[10].vertices[1]=5;
		boundingBox->faces[10].vertices[2]=1;

		boundingBox->faces[11].triangle=1;
		boundingBox->faces[11].vertices[0]=1;
		boundingBox->faces[11].vertices[1]=2;
		boundingBox->faces[11].vertices[2]=6;

		return boundingBox;
	}

	struct SM64SurfaceObject* marioLoadRoomMeshes(int roomId, int *room_meshes_count)
	{
		TR::Room &room = level->rooms[roomId];
		TR::Room::Data &d = room.data;

		*room_meshes_count = 0;

		struct SM64SurfaceObject *meshes = (struct SM64SurfaceObject*)malloc(sizeof(struct SM64SurfaceObject)*room.meshesCount);

		for (int j = 0; j < room.meshesCount; j++)
		{
			TR::Room::Mesh &m  = room.meshes[j];
			TR::StaticMesh *sm = &(level->staticMeshes[m.meshIndex]);
			if (sm->flags != 2 || !level->meshOffsets[sm->mesh]) {
				continue;
			}

			// define the surface object
			struct SM64SurfaceObject obj;
			obj.surfaceCount = 0;
			obj.transform.position[0] = m.x / MARIO_SCALE;
			obj.transform.position[1] = -m.y / MARIO_SCALE;
			obj.transform.position[2] = -m.z / MARIO_SCALE;
			for (int k=0; k<3; k++) obj.transform.eulerRotation[k] = (k == 1) ? float(m.rotation) / M_PI * 180.f : 0;

			TR::Mesh *d = &(level->meshes[level->meshOffsets[sm->mesh]]);
			TR::Mesh *boundingBox = NULL;

			switch(level->id)
			{
			case TR::LevelID::LVL_TR1_GYM:
				switch (m.meshIndex)
				{
				case 0:	// plant - Mario can get stuck
				case 1:	// plant - Mario can get stuck
				case 2: // plant - Mario can get stuck
				case 3: // plant - Mario can get stuck
				case 4: // branches - Mario can get stuck
					continue;
				case 7: //harp - Mario can get stuck
				case 8: //piano	- Martio can get stuck
				case 11: //pedestal - Mario can get stuck between it and the columns
				case 13: //handcart - Mario can get stuck
					boundingBox = generateMeshBoundingBox(sm);					
					break;
				}
				break;
			case TR::LevelID::LVL_TR1_1: //Caves
				switch (m.meshIndex)
				{
				case 0:	// branches - Mario stumbles
				case 1:	// branches	- Mario stumbles
				case 7:	// ice stalactite - Mario stumbles
					continue;
				// case 2: //wood fence blocking path to exit - solved by viewbox size
				// 	boundingBox = generateMeshBoundingBox(sm);					
				// 	break;
				}
				break;
			case TR::LevelID::LVL_TR1_2: //City of Vilcabamba
				switch (m.meshIndex)
				{
				case 0:	// branches - Mario stumbles
				case 1:	// branches - Mario stumbles
					continue;
				case 3: // barn animal food holder - Mario can get stuck on it.
				case 7:	// Stone Statues - Mario can clip through them and fall into the void.
					boundingBox = generateMeshBoundingBox(sm);
					break;
				}
				break;
			case TR::LevelID::LVL_TR1_3A: //The Lost Valley
				switch (m.meshIndex)
				{
				case 0:	// branches - Mario stumbles
				case 1:	// branches - Mario stumbles
				case 2: // tree/bushes - Mario can get stuck
				case 3: // tree/bushes - Mario can get stuck
				case 4: // tree/bushes - Mario can get stuck
				case 5: // tree/bushes - Mario can get stuck
				case 6: // dead tree trunk get mario stuck. we will ignore it.
				case 7: // trees - Mario can get stuck
				case 8: // trees - Mario can get stuck
					continue;
				case 12: // skeleton - Mario stumbles
				case 13: // skeleton - Mario stumbles
					boundingBox = generateMeshBoundingBox(sm);
					break;
				}
				break;
			case TR::LevelID::LVL_TR1_8B: //Obelisk of Khamoon
				switch (m.meshIndex)
				{
				// case 10: // Iron Bars with only 1 face - solved by viewbox size										
				// 	boundingBox = generateMeshBoundingBox(sm);
				// 	break;
				}
				break;
			case TR::LevelID::LVL_TR1_10B: //Atlantis
				switch (m.meshIndex)
				{
				case 0:	// weird flesh 1 surface walls
				case 1:	// weird flesh 1 surface walls
					boundingBox = generateMeshBoundingBox(sm);
					break;
				}
				break;
			}
			
			if(boundingBox)
				d=boundingBox;
			else if(d->fCount<2 || sm->vbox.minX==sm->vbox.maxX || sm->vbox.minY==sm->vbox.maxY || sm->vbox.minZ==sm->vbox.maxZ){
				boundingBox = generateMeshBoundingBox(sm);
				d=boundingBox;
			}

			// increment the surface count for this
			for (int k = 0; k < d->fCount; k++)
				obj.surfaceCount += (d->faces[k].triangle) ? 1 : 2;

			obj.surfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * obj.surfaceCount);
			size_t surface_ind = 0;

			// add the faces
			for (int k = 0; k < d->fCount; k++)
			{
				TR::Face &f = d->faces[k];

				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, {
					{(d->vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(d->vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(d->vertices[f.vertices[2]].coord.z)/IMARIO_SCALE},
					{(d->vertices[f.vertices[1]].coord.x)/IMARIO_SCALE, -(d->vertices[f.vertices[1]].coord.y)/IMARIO_SCALE, -(d->vertices[f.vertices[1]].coord.z)/IMARIO_SCALE},
					{(d->vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(d->vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(d->vertices[f.vertices[0]].coord.z)/IMARIO_SCALE},
				}};

				if (!f.triangle)
				{
					obj.surfaces[surface_ind++] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, {
						{(d->vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(d->vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(d->vertices[f.vertices[0]].coord.z)/IMARIO_SCALE},
						{(d->vertices[f.vertices[3]].coord.x)/IMARIO_SCALE, -(d->vertices[f.vertices[3]].coord.y)/IMARIO_SCALE, -(d->vertices[f.vertices[3]].coord.z)/IMARIO_SCALE},
						{(d->vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(d->vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(d->vertices[f.vertices[2]].coord.z)/IMARIO_SCALE},
					}};
				}
			}

			if(boundingBox!=NULL)
			{
				free(boundingBox->faces);
				free(boundingBox->vertices);
				free(boundingBox);
				boundingBox=NULL;
			}

			meshes[(*room_meshes_count)++]=obj;
		}

		return meshes;
	}

    void createDynamicObjects()
	{
		// create collisions from entities (bridges, doors)
		for (int i=0; i<level->entitiesCount; i++)
		{
			TR::Entity *e = &level->entities[i];
			if (e->isEnemy() || e->isLara() || e->isSprite() || e->isPuzzleHole() || e->isPickup() || e->type == 169)
				continue;

			if (!(e->type >= 68 && e->type <= 70) && !e->isDoor() && !e->isBlock() && e->type != TR::Entity::MOVING_BLOCK && e->type != TR::Entity::TRAP_FLOOR && e->type != TR::Entity::TRAP_DOOR_1 && e->type != TR::Entity::TRAP_DOOR_2 && e->type != TR::Entity::DRAWBRIDGE)
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
			if (e->isBlock()) obj.surfaceCount += 12; // 6 sides, 2 triangles per side to form a quad
			else if (e->type == TR::Entity::TRAP_FLOOR) obj.surfaceCount += 2; // floor
			else
			{
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
			}

			// then create the surface and add the objects
			obj.surfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * obj.surfaceCount);
			size_t surface_ind = 0;
			vec3 offset(0,0,0);
			bool doOffsetY = true;
			int16 topPointSign = 1;
			if (e->type >= 68 && e->type <= 70) topPointSign = -1;
			else if (e->type == TR::Entity::TRAP_FLOOR)
			{
				offset.y = 512;
				doOffsetY = false;
			}
			else if (e->isDoor())
			{
				offset.x = (512*cos(float(e->rotation)) - 512*sin(float(e->rotation))) * (1);
				offset.z = (-512*sin(float(e->rotation)) - 512*cos(float(e->rotation))) * ((e->type == TR::Entity::DOOR_1 || e->type == TR::Entity::DOOR_2 || (e->type == TR::Entity::DOOR_3 && abs(cos(float(e->rotation))) == 1) || e->type == TR::Entity::DOOR_4 || e->type == TR::Entity::DOOR_5 ||e->type == TR::Entity::DOOR_6 || e->type == TR::Entity::DOOR_7 || e->type == TR::Entity::DOOR_8) ? -1 : 1);
			}
			else if (e->type == TR::Entity::TRAP_DOOR_1)
			{
				offset.x = 512*cos(float(e->rotation)) - 512*sin(float(e->rotation));
				offset.z = 512*sin(float(e->rotation)) + 512*cos(float(e->rotation));
				offset.y = -88;
				doOffsetY = false;
			}
			else if (e->type == TR::Entity::TRAP_DOOR_2)
			{
				offset.x = 512*cos(float(e->rotation)) - 512*sin(float(e->rotation));
				offset.z = -512*sin(float(e->rotation)) + 512*cos(float(e->rotation));
				offset.y = -88;
				doOffsetY = false;
			}
			else if (e->type == TR::Entity::DRAWBRIDGE)
			{
				offset.y = -20;
				doOffsetY = false;
				// before you go "yanderedev code":
				// can't use switch() here because eulerRotation isn't an integer
				if (obj.transform.eulerRotation[1] == 90)
				{
					offset.x = -384-128;
					offset.z = -384-128;
				}
				else if (obj.transform.eulerRotation[1] == 270)
				{
					offset.x = 512;
					offset.z = 512;
				}
				else if (obj.transform.eulerRotation[1] == 180)
				{
					offset.x = -384;
					offset.z = 512;
				}
				else
				{
					offset.x = 384+128;
					offset.z = -384-128;
				}
			}

			if (e->isBlock())
			{
				// bottom of block
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{128, 0, 128}, {-128, 0, -128}, {-128, 0, 128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{-128, 0, -128}, {128, 0, 128}, {128, 0, -128}}}; // top right, bottom left, bottom right

				// left (Z+)
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{-128, 0, 128}, {128, 256, 128}, {-128, 256, 128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{128, 256, 128}, {-128, 0, 128}, {128, 0, 128}}}; // top right, bottom left, bottom right

				// right (Z-)
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{128, 0, -128}, {-128, 256, -128}, {128, 256, -128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{-128, 256, -128}, {128, 0, -128}, {-128, 0, -128}}}; // top right, bottom left, bottom right

				// back (X+)
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{128, 0, -128}, {128, 256, -128}, {128, 256, 128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{128, 256, 128}, {128, 0, 128}, {128, 0, -128}}}; // top right, bottom left, bottom right

				// front (X-)
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{-128, 0, 128}, {-128, 256, 128}, {-128, 256, -128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{-128, 256, -128}, {-128, 0, -128}, {-128, 0, 128}}}; // top right, bottom left, bottom right

				// top of block
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{128, 256, 128}, {-128, 256, -128}, {-128, 256, 128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{-128, 256, -128}, {128, 256, 128}, {128, 256, -128}}}; // top right, bottom left, bottom right
			}
			else if (e->type == TR::Entity::TRAP_FLOOR)
			{
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{128, 0, 128}, {-128, 0, -128}, {-128, 0, 128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE,{{-128, 0, -128}, {128, 0, 128}, {128, 0, -128}}}; // top right, bottom left, bottom right
			}
			else
			{
				for (int c = 0; c < model->mCount; c++)
				{
					int index = level->meshOffsets[model->mStart + c];
					if (index || model->mStart + c <= 0)
					{
						TR::Mesh &d = level->meshes[index];
						for (int j = 0; j < d.fCount; j++)
						{
							TR::Face &f = d.faces[j];

							if (!offset.y) offset.y = topPointSign * max(offset.y, (float)d.vertices[f.vertices[0]].coord.y, max((float)d.vertices[f.vertices[1]].coord.y, (float)d.vertices[f.vertices[2]].coord.y, (f.triangle) ? 0.f : (float)d.vertices[f.vertices[3]].coord.y));

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
			}
			obj.transform.position[0] += offset.x/MARIO_SCALE;
			obj.transform.position[1] += offset.y/MARIO_SCALE;
			obj.transform.position[2] += offset.z/MARIO_SCALE;

			// and finally add the object (this returns an uint32_t which is the object's ID)
			struct MarioControllerObj cObj;
			cObj.spawned = true;
			cObj.ID = sm64_surface_object_create(&obj);
			cObj.transform = obj.transform;
			cObj.entity = e;
			cObj.offset = offset;

			dynamicObjects[dynamicObjectsCount++] = cObj;
			free(obj.surfaces);
		}
	}

    

    void loadSM64Level(TR::Level *newLevel, IController *player)
    {
		level=newLevel;
		controller = player;
        #ifdef DEBUG_RENDER	
        printf("Loading level %d\n", level->id);
        #endif
        sm64_level_init(level->roomsCount);

        for(int roomId=0; roomId< level->roomsCount; roomId++)
        {
            int staticSurfacesCount;
            struct SM64Surface *staticSurfaces = marioLoadRoomSurfaces(roomId, &staticSurfacesCount);

            int roomMeshesCount;
            struct SM64SurfaceObject *roomMeshes = marioLoadRoomMeshes(roomId, &roomMeshesCount);

            sm64_level_load_room(roomId, staticSurfaces, staticSurfacesCount, roomMeshes, roomMeshesCount);

            free(staticSurfaces);
            for(int i=0; i<roomMeshesCount; i++)
            {
                free(roomMeshes[i].surfaces);
            }
            free(roomMeshes);	
        }

        createDynamicObjects();
    }

	void getCurrentAndAdjacentRooms(int *roomsList, int *roomsCount, int currentRoomIndex, int to, int maxDepth, int count=0) {
        if (count>maxDepth) {
            return;
        }

		// if we are starting to search we set all levels visibility to false
		if(count==0)
		{
			for (int i = 0; i < level->roomsCount; i++)
				level->rooms[i].flags.visible = false;
		}

		//Hardcoded exceptions to avoid invisible collisions
		switch (level->id)
		{
		case TR::LVL_TR1_10B: //Atlantis
			switch (currentRoomIndex)
			{
			case 47: //room with blocks and switches to trapdoors
				if(to==84)
					return;
			}
		}

		count++;

        TR::Room &room = level->rooms[to];

		if(!room.flags.visible){
			room.flags.visible = true;
			roomsList[*roomsCount] = to;
			*roomsCount+=1;
		}

		if(*roomsCount == 256)
			return;

		for (int i = 0; i < room.portalsCount; i++) {
			getCurrentAndAdjacentRooms(roomsList, roomsCount, currentRoomIndex, room.portals[i].roomIndex, maxDepth, count);
		}
    }


};

#endif