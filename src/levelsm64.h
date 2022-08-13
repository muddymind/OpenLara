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
#include "format.h"
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

	struct FacesToEvaluate
	{
		int roomIdx;
		int faceIdx;
		bool positive;

		int32 x[2];
		int32 y[2];
		int32 z[2];
	};

	struct ClipsDetected
	{
		struct FacesToEvaluate *face1;
		struct FacesToEvaluate *face2;
	};

    TR::Level *level=NULL;
	IController *controller=NULL;
    struct MarioControllerObj dynamicObjects[4096];
	int dynamicObjectsCount=0;

	struct FacesToEvaluate xfaces[4096];
	int xfacesCount=0;

	struct FacesToEvaluate zfaces[4096];
	int zfacesCount=0;

	struct ClipsDetected clips[MAX_CLIPPER_BLOCKS];
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

			ADD_ROOM_FACE_ABSOLUTE(collision_surfaces, level_surface_i, room, d, f, slippery);
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
				case 4: // Meat holder - Thin geometry and mario can get stuck
				case 7:	// Stone Statues - Mario can clip through them and fall into the void.
				case 8: // Meat holder - Thin geometry and mario can get stuck
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
		case TR::LevelID::LVL_TR1_3B: //Tomb of Qualopec
			switch (meshIndex)
			{

			}
			break;
		case TR::LevelID::LVL_TR1_CUT_1: // cut scene after Larson's fight
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_4: //St Francis' Folly
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
				case 6: //railings at the balcony at the final level are 1 face thick and mario clips
					return MESH_LOADING_BOUNDING_BOX;
				case 14: // bushes - mario gets stuck
				case 19: // small tree - mario gets stuck
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
				ADD_CUBE_GEOMETRY_NON_TRANSFORMED(obj.surfaces, surface_ind, -128, 128, 0, 256, -128, 128);
			}
			else if (e->type == TR::Entity::TRAP_FLOOR)
			{
				ADD_RECTANGLE_HORIZONTAL_GEOMETRY_NON_TRANSFORMED(obj.surfaces, surface_ind, -128, 128, -128, 128);
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

	void printface(int roomIdx, int faceIdx)
	{
		TR::Room &room = level->rooms[roomIdx];
		TR::Room::Data &d = room.data;
		TR::Face &f = d.faces[faceIdx];

		if(f.water || f.triangle)
		{
			return;
		}

		printf("\nface r:%d i:%d\n", roomIdx, faceIdx);
		printf("(%d, %d, %d)\n", d.vertices[f.vertices[0]].pos.x+room.info.x, d.vertices[f.vertices[0]].pos.y, d.vertices[f.vertices[0]].pos.z+room.info.z);
		printf("(%d, %d, %d)\n", d.vertices[f.vertices[1]].pos.x+room.info.x, d.vertices[f.vertices[1]].pos.y, d.vertices[f.vertices[1]].pos.z+room.info.z);
		printf("(%d, %d, %d)\n", d.vertices[f.vertices[2]].pos.x+room.info.x, d.vertices[f.vertices[2]].pos.y, d.vertices[f.vertices[2]].pos.z+room.info.z);
		printf("(%d, %d, %d)\n", d.vertices[f.vertices[3]].pos.x+room.info.x, d.vertices[f.vertices[3]].pos.y, d.vertices[f.vertices[3]].pos.z+room.info.z);
		printf("n(%d, %d, %d)\n\n", f.normal.x, f.normal.y, f.normal.z);
	}

	bool evalXface(int roomIdx, int faceIdx)
	{
		TR::Room &room = level->rooms[roomIdx];
		TR::Room::Data &d = room.data;
		TR::Face &f = d.faces[faceIdx];

		// We only want vertical surfaces
		if(f.normal.y != 0 || f.normal.z!=0)
		{
			return false;
		}

		if(f.water || f.triangle)
		{
			return false;	
		}

		int32 x = d.vertices[f.vertices[0]].pos.x+room.info.x;

		int32 miny = d.vertices[f.vertices[0]].pos.y;
		int32 maxy = d.vertices[f.vertices[0]].pos.y;
		int32 minz = d.vertices[f.vertices[0]].pos.z;
		int32 maxz = d.vertices[f.vertices[0]].pos.z;

		for(int i=1; i<4; i++)
		{
			if(d.vertices[f.vertices[i]].pos.y<miny) miny=d.vertices[f.vertices[i]].pos.y;
			if(d.vertices[f.vertices[i]].pos.y>maxy) maxy=d.vertices[f.vertices[i]].pos.y;
			if(d.vertices[f.vertices[i]].pos.z<minz) minz=d.vertices[f.vertices[i]].pos.z;
			if(d.vertices[f.vertices[i]].pos.z>maxz) maxz=d.vertices[f.vertices[i]].pos.z;
		}

		xfaces[xfacesCount++] = {
			roomIdx,
			faceIdx,
			f.normal.x > 0 ? true : false,
			{x, x},
			{miny, maxy},
			{minz+room.info.z, maxz+room.info.z},
		};

		return true;

	}

	bool evalZface(int roomIdx, int faceIdx)
	{
		TR::Room &room = level->rooms[roomIdx];
		TR::Room::Data &d = room.data;
		TR::Face &f = d.faces[faceIdx];

		// We only want vertical surfaces
		if(f.normal.y != 0 || f.normal.x!=0)
		{
			return false;
		}

		if(f.water || f.triangle)
		{
			return false;	
		}

		int32 z = d.vertices[f.vertices[0]].pos.z+room.info.z;

		int32 miny = d.vertices[f.vertices[0]].pos.y;
		int32 maxy = d.vertices[f.vertices[0]].pos.y;
		int32 minx = d.vertices[f.vertices[0]].pos.x;
		int32 maxx = d.vertices[f.vertices[0]].pos.x;

		for(int i=1; i<4; i++)
		{
			if(d.vertices[f.vertices[i]].pos.y<miny) miny=d.vertices[f.vertices[i]].pos.y;
			if(d.vertices[f.vertices[i]].pos.y>maxy) maxy=d.vertices[f.vertices[i]].pos.y;
			if(d.vertices[f.vertices[i]].pos.x<minx) minx=d.vertices[f.vertices[i]].pos.x;
			if(d.vertices[f.vertices[i]].pos.x>maxx) maxx=d.vertices[f.vertices[i]].pos.x;
		}

		zfaces[zfacesCount++] = {
			roomIdx,
			faceIdx,
			f.normal.z > 0 ? true : false,
			{minx+room.info.x, maxx+room.info.x},
			{miny, maxy},
			{z, z}
		};

		return true;

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

		#ifdef DEBUG_RENDER	
		printf("%d xfaces to evaluate\n%d zfaces to evaluate\n", xfacesCount, zfacesCount);
		#endif

		clipsCount = 0;
		for(int i=0; i<xfacesCount-1; i++)
		{
			for(int j=i+1; j<xfacesCount; j++)
			{
				if( TEST_FACE_OVERLAP(xfaces[i], xfaces[j], x, z) )
				{
					bool found=false;
					for(int w=0; w<clipsCount; w++)
					{
						struct ClipsDetected *clip = &(clips[w]);
						//We need to check if this is really a new clip that hasn't been found before
						if(clip->face1 == &xfaces[i] || clip->face1 == &xfaces[j])
						{
							found=true;
							break;
						}
					}
					if(!found)
					{
						clips[clipsCount++]={&xfaces[i], &xfaces[j]};
					}
				}
			}
		}
		for(int i=0; i<zfacesCount-1; i++)
		{
			for(int j=i+1; j<zfacesCount; j++)
			{

				if( TEST_FACE_OVERLAP(zfaces[i], zfaces[j], z, x) )
				{
					bool found=false;
					for(int w=0; w<clipsCount; w++)
					{
						struct ClipsDetected *clip = &(clips[w]);
						//We need to check if this is really a new clip that hasn't been found before
						if(clip->face1 == &zfaces[i] || clip->face1 == &zfaces[j])
						{
							found=true;
							break;
						}
					}
					if(!found)
					{
						clips[clipsCount++]={&zfaces[i], &zfaces[j]};
					}
				}
			}
		}
		#ifdef DEBUG_RENDER	
		printf("clips found: %d\n", clipsCount);
		#endif
	}

	void createClipBlockers()
	{
		clipBlockersCount=0;

		for(int i=0; i<clipsCount; i++)
		{
			FacesToEvaluate *e = clips[i].face1;

			if(e->x[0] == e->x[1])
			{
				e->x[0]-=CLIPPER_DISPLACEMENT;
				e->x[1]+=CLIPPER_DISPLACEMENT;
			}

			if(e->z[0] == e->z[1])
			{
				e->z[0]-=CLIPPER_DISPLACEMENT;
				e->z[1]+=CLIPPER_DISPLACEMENT;
			}

			ADD_CUBE_GEOMETRY(clipBlockers, clipBlockersCount, 0, 0, e->x[0], e->x[1], e->y[0], e->y[1], e->z[0], e->z[1]);
		}
	}

	void getCurrentAndAdjacentRoomsWithClips(int marioId, int currentRoomIndex, int to, int maxDepth, bool evaluateClips = false) 
	{
		int nearRooms[256];
		int nearRoomsCount=0;

		getCurrentAndAdjacentRooms(nearRooms, &nearRoomsCount, currentRoomIndex, to, maxDepth);

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
		case TR::LevelID::LVL_TR1_6: //Palace Midas
			switch (currentRoomIndex)
			{
			case 25: //room with huge pillar that goes down
				if(to==63)
					return;
				break;
			case 63: //corridor that goes down the huge pillar
				if(to==25)
					return;
				break;
			}
			break;
		case TR::LVL_TR1_10B: //Atlantis
			switch (currentRoomIndex)
			{
			case 47: //room with blocks and switches to trapdoors
				if(to==84)
					return;
				break;
			}
			break;
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