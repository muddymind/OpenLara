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
	/**
	 * @brief the minimum x,y,z distance between faces of the clipper blocks 
	 */
	#define CLIPPER_DISPLACEMENT 40

	enum MESH_LOADING_RESULT{
		MESH_LOADING_DISCARD,
		MESH_LOADING_NORMAL,
		MESH_LOADING_BOUNDING_BOX
	};

	struct ClipsDetected
	{
		int room1;
		int face1;
		int room2;
		int face2;
	};

	struct FacesToEvaluate
	{
		int roomIdx;
		int faceIdx;
		vec3 average;
	};

    TR::Level *level=NULL;
	IController *controller=NULL;
    struct MarioControllerObj dynamicObjects[4096];
	int dynamicObjectsCount=0;

	struct FacesToEvaluate xfaces[18192];
	int xfacesCount=0;

	struct FacesToEvaluate zfaces[18192];
	int zfacesCount=0;

	struct ClipsDetected clips[10];
	int clipsCount=0;

	SM64Surface clipBlockers[MAX_CLIPPER_BLOCKS_FACES];
	int clipBlockersCount = 0;

    LevelSM64()
    {
    }
    ~LevelSM64()
    {
        sm64_level_unload();
    }

	    void loadSM64Level(TR::Level *newLevel, IController *player, int initRoom=0)
    {
		level=newLevel;
		controller = player;
        #ifdef DEBUG_RENDER	
        printf("Loading level %d\n", level->id);
        #endif
        sm64_level_init(level->roomsCount);

		//createLevelClippers(initRoom);

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

    struct SM64Surface* marioLoadRoomSurfaces(int roomId, int *room_surfaces_count)
	{
		*room_surfaces_count = 0;
		size_t level_surface_i = 0;

		TR::Room &room = level->rooms[roomId];
		TR::Room::Data *d = &room.data;

		// Count the number of static surface triangles
		for (int j = 0; j < d->fCount; j++)
		{
			TR::Face &f = d->faces[j];
			if (f.water) continue;

			*room_surfaces_count += (f.triangle) ? 1 : 2;
		}

		struct SM64Surface *collision_surfaces = (struct SM64Surface *)malloc(sizeof(struct SM64Surface) * (*room_surfaces_count));

		// Generate all static surface triangles
		for (int cface = 0; cface < d->fCount; cface++)
		{
			TR::Face &f = d->faces[cface];
				if (f.water) continue;

			int16 x[2] = {d->vertices[f.vertices[0]].pos.x, 0};
			int16 z[2] = {d->vertices[f.vertices[0]].pos.z, 0};
			for (int j=1; j<3; j++)
			{
				if (x[0] != d->vertices[f.vertices[j]].pos.x) x[1] = d->vertices[f.vertices[j]].pos.x;
				if (z[0] != d->vertices[f.vertices[j]].pos.z) z[1] = d->vertices[f.vertices[j]].pos.z;
			}

			float midX = (x[0] + x[1]) / 2.f;
			float topY = max(d->vertices[f.vertices[0]].pos.y, max(d->vertices[f.vertices[1]].pos.y, d->vertices[f.vertices[2]].pos.y));
			float midZ = (z[0] + z[1]) / 2.f;

			TR::Level::FloorInfo info;

			bool slippery = false;
			if(controller) {
				controller->getFloorInfo(roomId, vec3(room.info.x + midX, topY, room.info.z + midZ), info); 
				slippery = (abs(info.slantX) > 2 || abs(info.slantZ) > 2);
			}

			ADD_ROOM_FACE_ABSOLUTE(collision_surfaces, level_surface_i, room, d, f, slippery, roomId, cface);
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

		CREATE_BOUNDING_BOX(boundingBox, box.min.x, box.max.x, box.min.y, box.max.y, box.min.z, box.max.z);

		return boundingBox;
	}

	TR::Mesh *generateRawMeshBoundingBox(int minx, int maxx, int miny, int maxy, int minz, int maxz)
	{
		
		TR::Mesh *boundingBox = (TR::Mesh *) malloc(sizeof(TR::Mesh));
		boundingBox->fCount = 12;
		boundingBox->faces = (TR::Face *)malloc(sizeof(TR::Face)*boundingBox->fCount);

		boundingBox->vCount = 8;
		boundingBox->vertices = (TR::Mesh::Vertex *)malloc(sizeof(TR::Mesh::Vertex)*boundingBox->vCount);

		CREATE_BOUNDING_BOX(boundingBox, minx, maxx, miny, maxy, minz, maxz);

		return boundingBox;
	}

	enum MESH_LOADING_RESULT evaluateRoomMeshLoadingOverride(int meshIndex, TR::StaticMesh *sm)
	{
		switch(level->id)
		{
		case TR::LevelID::LVL_TR1_GYM:
			switch (meshIndex)
			{
				case 0:	// plant - Mario can get stuck
				case 1:	// plant - Mario can get stuck
				case 2: // plant - Mario can get stuck
				case 3: // plant - Mario can get stuck
					return MESH_LOADING_DISCARD;
				case 7: //harp - Mario can get stuck
				case 8: //piano	- Martio can get stuck
				case 11: //pedestal - Mario can get stuck between it and the columns
				case 13: //handcart - Mario can get stuck
					return MESH_LOADING_BOUNDING_BOX;
				case 9: //table at library is non collision for some reason
					return MESH_LOADING_NORMAL;
			}
			break;
		case TR::LevelID::LVL_TR1_1: //Caves
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_2: //City of Vilcabamba
			switch (meshIndex)
			{
				case 3: // barn animal food holder - Mario can get stuck on it.
				case 7:	// Stone Statues - Mario can clip through them and fall into the void.
					return MESH_LOADING_BOUNDING_BOX;
				case 6: // platform with small statue - wihtout it mario falls into the void.
					return MESH_LOADING_NORMAL;
			}
			break;
		case TR::LevelID::LVL_TR1_3A: //The Lost Valley
			switch (meshIndex)
			{
				case 6: // old tree bark - mario gets stuck
				return MESH_LOADING_DISCARD;
			}
			break;
		case TR::LevelID::LVL_TR1_3B:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_CUT_1:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_4:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_5:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_6:// Palace Midas
			switch (meshIndex)
			{
				case 14: // bushes - mario gets stuck
				return MESH_LOADING_DISCARD;
			}
			break;
		case TR::LevelID::LVL_TR1_7A:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_7B:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_CUT_2:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_8A:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_8B: //Obelisk of Khamoon
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_8C:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_10A:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_CUT_3:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_10B: //Atlantis
			switch (meshIndex)
			{
				case 0:	// weird flesh 1 surface walls - Mario can clip
				case 1:	// weird flesh 1 surface walls - Mario can clip
					return MESH_LOADING_BOUNDING_BOX;
			}
			break;
		case TR::LevelID::LVL_TR1_CUT_4:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_10C:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_EGYPT:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_CAT:
			switch (meshIndex)
			{
				
			}
			break;
		}

		if (sm->flags != 2 || !level->meshOffsets[sm->mesh]) 
		{
			return MESH_LOADING_DISCARD;
		}

		// 1 face geometry must use a bounding box or Mario will clip through
		if( sm->vbox.minX==sm->vbox.maxX || sm->vbox.minY==sm->vbox.maxY || sm->vbox.minZ==sm->vbox.maxZ){
			return MESH_LOADING_BOUNDING_BOX;
		}

		return MESH_LOADING_NORMAL;
	}
	
	struct SM64SurfaceObject* marioLoadRoomMeshes(int roomId, int *room_meshes_count)
	{
		TR::Room &room = level->rooms[roomId];
		TR::Room::Data &d = room.data;

		*room_meshes_count = 0;

		struct SM64SurfaceObject *meshes = (struct SM64SurfaceObject*)malloc(sizeof(struct SM64SurfaceObject)*room.meshesCount);

		for (int j = 0; j < room.meshesCount; j++)
		{
			TR::Room::Mesh *m  = &(room.meshes[j]);
			TR::StaticMesh *sm = &(level->staticMeshes[m->meshIndex]);
			
			enum MESH_LOADING_RESULT action = evaluateRoomMeshLoadingOverride(m->meshIndex, sm);

			TR::Mesh *mesh=NULL;

			switch(action){
				case MESH_LOADING_DISCARD:
					continue;
				case MESH_LOADING_BOUNDING_BOX:
					mesh = generateMeshBoundingBox(sm);
					break;
				case MESH_LOADING_NORMAL:
				default:
					mesh = &(level->meshes[level->meshOffsets[sm->mesh]]);
					break;
			}

			// define the surface object
			struct SM64SurfaceObject obj;
			obj.surfaceCount = 0;
			obj.transform.position[0] = m->x / MARIO_SCALE;
			obj.transform.position[1] = -m->y / MARIO_SCALE;
			obj.transform.position[2] = -m->z / MARIO_SCALE;
			for (int k=0; k<3; k++) obj.transform.eulerRotation[k] = (k == 1) ? float(m->rotation) / M_PI * 180.f : 0;

			// increment the surface count for this
			for (int k = 0; k < mesh->fCount; k++)
				obj.surfaceCount += (mesh->faces[k].triangle) ? 1 : 2;

			obj.surfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * obj.surfaceCount);
			size_t surface_ind = 0;

			// add the faces
			for (int k = 0; k < mesh->fCount; k++)
			{
				TR::Face &f = mesh->faces[k];
				
				ADD_MESH_FACE_RELATIVE(obj.surfaces, surface_ind, mesh, f);
			}

			if( action == MESH_LOADING_BOUNDING_BOX)
			{
				free(mesh->faces);
				free(mesh->vertices);
				free(mesh);
			}
			mesh=NULL;

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
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{128, 0, 128}, {-128, 0, -128}, {-128, 0, 128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{-128, 0, -128}, {128, 0, 128}, {128, 0, -128}}}; // top right, bottom left, bottom right

				// left (Z+)
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{-128, 0, 128}, {128, 256, 128}, {-128, 256, 128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{128, 256, 128}, {-128, 0, 128}, {128, 0, 128}}}; // top right, bottom left, bottom right

				// right (Z-)
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{128, 0, -128}, {-128, 256, -128}, {128, 256, -128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{-128, 256, -128}, {128, 0, -128}, {-128, 0, -128}}}; // top right, bottom left, bottom right

				// back (X+)
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{128, 0, -128}, {128, 256, -128}, {128, 256, 128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{128, 256, 128}, {128, 0, 128}, {128, 0, -128}}}; // top right, bottom left, bottom right

				// front (X-)
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{-128, 0, 128}, {-128, 256, 128}, {-128, 256, -128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{-128, 256, -128}, {-128, 0, -128}, {-128, 0, 128}}}; // top right, bottom left, bottom right

				// top of block
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{128, 256, 128}, {-128, 256, -128}, {-128, 256, 128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{-128, 256, -128}, {128, 256, 128}, {128, 256, -128}}}; // top right, bottom left, bottom right
			}
			else if (e->type == TR::Entity::TRAP_FLOOR)
			{
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{128, 0, 128}, {-128, 0, -128}, {-128, 0, 128}}}; // bottom left, top right, top left
				obj.surfaces[surface_ind++] = {SURFACE_DEFAULT,0,TERRAIN_STONE, -1, -1, {{-128, 0, -128}, {128, 0, 128}, {128, 0, -128}}}; // top right, bottom left, bottom right
			}
			else
			{
				for (int c = 0; c < model->mCount; c++)
				{
					int index = level->meshOffsets[model->mStart + c];
					if (index || model->mStart + c <= 0)
					{
						TR::Mesh *d = &(level->meshes[index]);
						for (int j = 0; j < d->fCount; j++)
						{
							TR::Face &f = d->faces[j];

							if (!offset.y) offset.y = topPointSign * max(offset.y, (float)d->vertices[f.vertices[0]].coord.y, max((float)d->vertices[f.vertices[1]].coord.y, (float)d->vertices[f.vertices[2]].coord.y, (f.triangle) ? 0.f : (float)d->vertices[f.vertices[3]].coord.y));

							ADD_MESH_FACE_RELATIVE(obj.surfaces, surface_ind, d, f);
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

    void getFaceMiddlePoint(int roomIdx, int faceIdx, struct FacesToEvaluate *face)
	{
		TR::Room &room = level->rooms[roomIdx];
		TR::Room::Data &d = room.data;
		TR::Face &f = d.faces[faceIdx];

		face->roomIdx = roomIdx;
		face->faceIdx = faceIdx;
		face->average = vec3(
			(d.vertices[f.vertices[0]].pos.x+d.vertices[f.vertices[1]].pos.x+d.vertices[f.vertices[2]].pos.x+d.vertices[f.vertices[3]].pos.x+(4*room.info.x))/4.0f,
			(d.vertices[f.vertices[0]].pos.y+d.vertices[f.vertices[1]].pos.y+d.vertices[f.vertices[2]].pos.y+d.vertices[f.vertices[3]].pos.y)/4.0f,
			(d.vertices[f.vertices[0]].pos.z+d.vertices[f.vertices[1]].pos.z+d.vertices[f.vertices[2]].pos.z+d.vertices[f.vertices[3]].pos.z+(4*room.info.z))/4.0f
		);
	}

	bool evalXface(int roomIdx, int faceIdx)
	{
		TR::Room &room = level->rooms[roomIdx];
		TR::Room::Data &d = room.data;
		TR::Face &f = d.faces[faceIdx];

		if(f.water || f.triangle)
		{
			return false;	
		}

		int16 x = d.vertices[f.vertices[0]].pos.x;
		if(d.vertices[f.vertices[1]].pos.x == x && d.vertices[f.vertices[2]].pos.x == x && d.vertices[f.vertices[3]].pos.x==x)
		{
			getFaceMiddlePoint(roomIdx, faceIdx, &(xfaces[xfacesCount++]));
			return true;
		}
		return false;
	}

	void printface(int roomIdx, int faceIdx)
	{
		TR::Room &room = level->rooms[roomIdx];
		TR::Room::Data &d = room.data;
		TR::Face &f = d.faces[faceIdx];

		if(f.water || f.triangle)
		{
			return;
		}

		printf("face r:%d i:%d\n", roomIdx, faceIdx);
		printf("(%d, %d, %d)\n", d.vertices[f.vertices[0]].pos.x+room.info.x, d.vertices[f.vertices[0]].pos.y, d.vertices[f.vertices[0]].pos.z+room.info.z);
		printf("(%d, %d, %d)\n", d.vertices[f.vertices[1]].pos.x+room.info.x, d.vertices[f.vertices[1]].pos.y, d.vertices[f.vertices[1]].pos.z+room.info.z);
		printf("(%d, %d, %d)\n", d.vertices[f.vertices[2]].pos.x+room.info.x, d.vertices[f.vertices[2]].pos.y, d.vertices[f.vertices[2]].pos.z+room.info.z);
		printf("(%d, %d, %d)\n", d.vertices[f.vertices[3]].pos.x+room.info.x, d.vertices[f.vertices[3]].pos.y, d.vertices[f.vertices[3]].pos.z+room.info.z);
	}

	bool evalZface(int roomIdx, int faceIdx)
	{
		TR::Room &room = level->rooms[roomIdx];
		TR::Room::Data &d = room.data;
		TR::Face &f = d.faces[faceIdx];

		if(f.water || f.triangle)
		{
			return false;	
		}

		int16 z = d.vertices[f.vertices[0]].pos.z;
		if(d.vertices[f.vertices[1]].pos.z == z && d.vertices[f.vertices[2]].pos.z == z && d.vertices[f.vertices[3]].pos.z==z)
		{
			getFaceMiddlePoint(roomIdx, faceIdx, &(zfaces[zfacesCount++]));
			return true;
		}
		return false;
	}

	float crudeDistance(vec3 v1, vec3 v2)
	{
		return abs(v1.x-v2.x)+abs(v1.y-v2.y)+abs(v1.z-v2.z);
	}

	void evaluateClippingSurfaces(int *roomsList, int roomsCount)
	{
		xfacesCount=0;
		zfacesCount=0;
		for(int roomId=0; roomId<roomsCount; roomId++)
		{
			for(int faceId=0; faceId<level->rooms[roomsList[roomId]].data.fCount; faceId++)
			{
				if(!evalXface(roomsList[roomId], faceId))
				{
					evalZface(roomsList[roomId], faceId);
				}
			}
		}

		//printf("xfaces: %d\tzfaces: %d\n", xfacesCount, zfacesCount);

		//printf("Evaluating clips...\n");
		clipsCount = 0;
		for(int i=0; i<xfacesCount-1; i++)
		{
			for(int j=i+1; j<xfacesCount; j++)
			{
				if(crudeDistance(xfaces[i].average, xfaces[j].average)<50)
				{
					clips[clipsCount].room1=xfaces[i].roomIdx;
					clips[clipsCount].face1=xfaces[i].faceIdx;
					clips[clipsCount].room2=xfaces[j].roomIdx;
					clips[clipsCount].face2=xfaces[j].faceIdx;
					clipsCount++;
					//printf("found xclip between room %d face %d with room %d with face %d with crude distance of %f\n", xfaces[i].roomIdx, xfaces[i].faceIdx, xfaces[j].roomIdx, xfaces[j].faceIdx, crudeDistance(xfaces[i].average, xfaces[j].average));
					//printface(xfaces[i].roomIdx, xfaces[i].faceIdx);
				}
			}
		}
		for(int i=0; i<zfacesCount-1; i++)
		{
			for(int j=i+1; j<zfacesCount; j++)
			{
				if(crudeDistance(zfaces[i].average, zfaces[j].average)<50)
				{
					clips[clipsCount].room1=zfaces[i].roomIdx;
					clips[clipsCount].face1=zfaces[i].faceIdx;
					clips[clipsCount].room2=zfaces[j].roomIdx;
					clips[clipsCount].face2=zfaces[j].faceIdx;
					clipsCount++;
					//printf("found zclip between room %d face %d with room %d with face %d with crude distance of %f\n", zfaces[i].roomIdx, zfaces[i].faceIdx, zfaces[j].roomIdx, zfaces[j].faceIdx, crudeDistance(zfaces[i].average, zfaces[j].average));
					//printface(zfaces[i].roomIdx, zfaces[i].faceIdx);
				}
			}
		}
	}

	void createClipBlockers()
	{
		clipBlockersCount=0;

		for(int i=0; i<clipsCount; i++)
		{
			TR::Room &room = level->rooms[clips[i].room1];
			TR::Room::Data &d = room.data;
			TR::Face &f = d.faces[clips[i].face1];

			//printface(clips[i].room1, clips[i].face1);

			int minx = d.vertices[f.vertices[0]].pos.x;
			int maxx = d.vertices[f.vertices[0]].pos.x;
			for(int i=1; i<4; i++){
				if(d.vertices[f.vertices[i]].pos.x > maxx) maxx = d.vertices[f.vertices[i]].pos.x;
				if(d.vertices[f.vertices[i]].pos.x < minx) minx = d.vertices[f.vertices[i]].pos.x;				
			}
			if(minx == maxx)
			{
				minx-=CLIPPER_DISPLACEMENT;
				maxx+=CLIPPER_DISPLACEMENT;
			}

			int miny = d.vertices[f.vertices[0]].pos.y;
			int maxy = d.vertices[f.vertices[0]].pos.y;
			for(int i=1; i<4; i++){
				if(d.vertices[f.vertices[i]].pos.y > maxy) maxy = d.vertices[f.vertices[i]].pos.y;
				if(d.vertices[f.vertices[i]].pos.y < miny) miny = d.vertices[f.vertices[i]].pos.y;
			}
			if(miny == maxy)
			{
				miny-=CLIPPER_DISPLACEMENT;
				maxy+=CLIPPER_DISPLACEMENT;
			}

			int minz = d.vertices[f.vertices[0]].pos.z;
			int maxz = d.vertices[f.vertices[0]].pos.z;
			for(int i=1; i<4; i++){
				if(d.vertices[f.vertices[i]].pos.z > maxz) maxz = d.vertices[f.vertices[i]].pos.z;
				if(d.vertices[f.vertices[i]].pos.z < minz) minz = d.vertices[f.vertices[i]].pos.z;				
			}
			if(minz == maxz)
			{
				minz-=CLIPPER_DISPLACEMENT;
				maxz+=CLIPPER_DISPLACEMENT;
			}

			TR::Mesh *mesh = generateRawMeshBoundingBox(minx, maxx, miny, maxy, minz, maxz);

			for(int i=0; i<mesh->fCount; i++)
			{		
				ADD_MESH_FACE_ABSOLUTE(clipBlockers,clipBlockersCount, room, mesh, mesh->faces[i] );
			}
			
			free(mesh->faces);
			free(mesh);
		}
	}

	void getCurrentAndAdjacentRoomsWithClips(int marioId, int currentRoomIndex, int to, int maxDepth, bool evaluateClips = false) 
	{
		int nearRooms[256];
		int nearRoomsCount=0;

		getCurrentAndAdjacentRooms(nearRooms, &nearRoomsCount, currentRoomIndex, to, 2);

		if(evaluateClips)
		{
			evaluateClippingSurfaces(nearRooms, nearRoomsCount);
			createClipBlockers();
			sm64_level_update_player_loaded_Rooms_with_clippers(marioId, nearRooms, nearRoomsCount, clipBlockers, clipBlockersCount);
		}
		else
		{
			sm64_level_update_loaded_rooms_list(marioId, nearRooms, nearRoomsCount);
		}

		return;			
	}

	void getCurrentAndAdjacentRooms(int *roomsList, int *roomsCount, int currentRoomIndex, int to, int maxDepth, int count=0) {
        if (count>maxDepth || *roomsCount == 256) {
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

		for (int i = 0; i < room.portalsCount; i++) {
			getCurrentAndAdjacentRooms(roomsList, roomsCount, currentRoomIndex, room.portals[i].roomIndex, maxDepth, count);
		}
    }

	int createMarioInstance(int roomIndex, vec3(pos))
	{
		int nearRooms[256];
		int nearRoomsCount=0;
		getCurrentAndAdjacentRooms(nearRooms, &nearRoomsCount, roomIndex, roomIndex, 2);
		return sm64_mario_create(pos.x/MARIO_SCALE, -pos.y/MARIO_SCALE, -pos.z/MARIO_SCALE, 0, 0, 0, 0, nearRooms, nearRoomsCount);
	}


};

#endif