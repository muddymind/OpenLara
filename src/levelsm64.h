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

#include <time.h>
#include "core.h"
#include "controller.h"
#include "marioMacros.h"
#include "format.h"
#include "objects.h"

struct LevelSM64 : SM64::ILevelSM64
{
	/**
	 * @brief the minimum x,y,z distance between faces of the clipper blocks 
	 */
	#define CLIPPER_DISPLACEMENT 40

	struct FaceLimits
	{
		int roomId, faceId; //only for debugging necessities
		int limits[3][2];
		bool positive;
	};

	struct RoomLimits
	{
		int limits[3][2];
		struct FaceLimits *xfaces, *zfaces;
		int xfacesCount, zfacesCount;
	};

	struct RoomLimits **roomLimits;

	struct ClipsDetected
	{
		FaceLimits *face1;
		FaceLimits *face2;
	};

    TR::Level *level=NULL;
	Controller *controller=NULL;

	struct FaceLimits *totalXfaces[2048];
	int totalXfacesCount=0;

	struct FaceLimits *totalZfaces[2048];
	int totalZfacesCount=0;

	struct ClipsDetected clips[MAX_CLIPPER_BLOCKS];
	int clipsCount=0;

	SM64Surface clipBlockers[MAX_CLIPPER_BLOCKS_FACES];
	int clipBlockersCount = 0;

    LevelSM64() : SM64::ILevelSM64()
    {
		roomLimits=NULL;
    }
    virtual ~LevelSM64()
    {
		for(int i=0; i<level->roomsCount; i++)
		{
			if(roomLimits[i]->xfaces!=NULL)
			{
				free(roomLimits[i]->xfaces);
			}
			if(roomLimits[i]->zfaces!=NULL)
			{
				free(roomLimits[i]->zfaces);
			}
			free(roomLimits[i]);
		}
		free(roomLimits);
        sm64_level_unload();
    }

	virtual void loadSM64Level(TR::Level *newLevel, void *player, int initRoom=0)
    {
		level=newLevel;
		controller = (Controller *)player;
        #ifdef DEBUG_RENDER	
        printf("Loading level %d\n", level->id);
        #endif
        sm64_level_init(level->roomsCount);

		roomLimits= (struct RoomLimits **)malloc(sizeof(struct RoomLimits *)*level->roomsCount);

        for(int roomId=0; roomId< level->roomsCount; roomId++)
        {
			precacheRoomAndFacesLimits(roomId);

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
		TR::Room::Info *info = &room.info;

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

			// now adding the faces to SM64
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

	// used for speedup clip discovery calculations
	void precacheRoomAndFacesLimits(int roomId)
	{
		struct RoomLimits *rl = (struct RoomLimits *)malloc(sizeof(struct RoomLimits));
		roomLimits[roomId] = rl;

		TR::Room &room = level->rooms[roomId];
		TR::Room::Data *d = &room.data;
		TR::Room::Info *info = &room.info;
		
		// Initialize room boundaries values
		rl->limits[0][0]=info->x;
		rl->limits[0][1]=info->x;
		rl->limits[1][0]=info->yBottom;
		rl->limits[1][1]=info->yTop;
		rl->limits[2][0]=info->z;
		rl->limits[2][1]=info->z;

		rl->xfacesCount=0;
		rl->zfacesCount=0;

		//get how many face pointers we are going to have to allocate
		for (int cface = 0; cface < d->fCount; cface++)
		{
			TR::Face &f = d->faces[cface];
			if (f.water || f.normal.y != 0 || f.triangle) continue;

			if (f.normal.x == 0) // xface
			{
				rl->xfacesCount++;
			}
			else if (f.normal.z == 0) // zface
			{
				rl->zfacesCount++;
			}
		}
		
		rl->xfaces = rl->xfacesCount > 0 ? (struct FaceLimits *)malloc(sizeof(struct FaceLimits)*rl->xfacesCount) : NULL;
		rl->zfaces = rl->zfacesCount > 0 ? (struct FaceLimits *)malloc(sizeof(struct FaceLimits)*rl->zfacesCount) : NULL;

		int xindex=0;
		int zindex=0;

		for (int cface = 0; cface < d->fCount; cface++)
		{
			if(roomId==12 && cface==404)
			{
				printface(12, 404);
			}
			FaceLimits * fl=NULL;

			TR::Face &f = d->faces[cface];
			
			//we don't care about water, non vertical faces or triangles
			if (f.water || f.normal.y != 0 || f.triangle) continue;

			if (f.normal.x == 0) // xface
			{
				fl=&(rl->xfaces[xindex++]);
				fl->positive = f.normal.z > 0;
			}
			else if (f.normal.z == 0) // zface
			{
				fl=&(rl->zfaces[zindex++]);
				fl->positive = f.normal.x > 0;
			}
			else
			{
				continue;
			}

			fl->roomId = roomId;
			fl->faceId = cface;

			// pre-calculate face limits
			fl->limits[0][0]=d->vertices[f.vertices[0]].pos.x;
			fl->limits[0][1]=d->vertices[f.vertices[0]].pos.x;
			fl->limits[1][0]=d->vertices[f.vertices[0]].pos.y;
			fl->limits[1][1]=d->vertices[f.vertices[0]].pos.y;
			fl->limits[2][0]=d->vertices[f.vertices[0]].pos.z;
			fl->limits[2][1]=d->vertices[f.vertices[0]].pos.z;

			for(int i=1; i< 4; i++)
			{
				if (fl->limits[0][0] > d->vertices[f.vertices[i]].pos.x) fl->limits[0][0] = d->vertices[f.vertices[i]].pos.x;
				if (fl->limits[0][1] < d->vertices[f.vertices[i]].pos.x) fl->limits[0][1] = d->vertices[f.vertices[i]].pos.x;
				if (fl->limits[1][0] > d->vertices[f.vertices[i]].pos.y) fl->limits[1][0] = d->vertices[f.vertices[i]].pos.y;
				if (fl->limits[1][1] < d->vertices[f.vertices[i]].pos.y) fl->limits[1][1] = d->vertices[f.vertices[i]].pos.y;
				if (fl->limits[2][0] > d->vertices[f.vertices[i]].pos.z) fl->limits[2][0] = d->vertices[f.vertices[i]].pos.z;
				if (fl->limits[2][1] < d->vertices[f.vertices[i]].pos.z) fl->limits[2][1] = d->vertices[f.vertices[i]].pos.z;
			}

			// add room displacement to the faces
			fl->limits[0][0]+=info->x;
			fl->limits[0][1]+=info->x;
			fl->limits[2][0]+=info->z;
			fl->limits[2][1]+=info->z;

			// Recheck room boundaries
			if(rl->limits[0][0] > fl->limits[0][0]) rl->limits[0][0] = fl->limits[0][0];
			if(rl->limits[0][1] < fl->limits[0][1]) rl->limits[0][1] = fl->limits[0][1];
			if(rl->limits[1][0] > fl->limits[1][0]) rl->limits[1][0] = fl->limits[1][0];
			if(rl->limits[1][1] < fl->limits[1][1]) rl->limits[1][1] = fl->limits[1][1];
			if(rl->limits[2][0] > fl->limits[2][0]) rl->limits[2][0] = fl->limits[2][0];
			if(rl->limits[2][1] < fl->limits[2][1]) rl->limits[2][1] = fl->limits[2][1];
		}

		//Add some padding to the room boundaries 
		rl->limits[0][0]-=50;
		rl->limits[0][1]+=50;
		rl->limits[1][0]-=50;
		rl->limits[1][1]+=50;
		rl->limits[2][0]-=50;
		rl->limits[2][1]+=50;
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

	// copied from debug.h to help fixing issues.
	const char *getEntityName(const TR::Level &level, const TR::Entity &entity) 
	{
		const char *TR_TYPE_NAMES[] = { TR_TYPES(DECL_STR) };
		
		if (entity.type < TR::Entity::TR1_TYPE_MAX)
			return TR_TYPE_NAMES[entity.type - TR1_TYPES_START];
		if (entity.type < TR::Entity::TR2_TYPE_MAX)
			return TR_TYPE_NAMES[entity.type - TR2_TYPES_START + (TR::Entity::TR1_TYPE_MAX - TR1_TYPES_START) + 1];
		if (entity.type < TR::Entity::TR3_TYPE_MAX)
			return TR_TYPE_NAMES[entity.type - TR3_TYPES_START + (TR::Entity::TR1_TYPE_MAX - TR1_TYPES_START) + (TR::Entity::TR2_TYPE_MAX - TR2_TYPES_START) + 2];
		if (entity.type < TR::Entity::TR4_TYPE_MAX)
			return TR_TYPE_NAMES[entity.type - TR4_TYPES_START + (TR::Entity::TR1_TYPE_MAX - TR1_TYPES_START) + (TR::Entity::TR2_TYPE_MAX - TR2_TYPES_START) + (TR::Entity::TR3_TYPE_MAX - TR3_TYPES_START) + 3];

		return "UNKNOWN";
	}

	virtual void updateTransformation(TR::Entity *entity, struct SM64ObjectTransform *transform)
	{
		Controller *controller = (Controller*)entity->controller;
		if(controller == NULL)
			return;

		controller->updateJoints();

		transform->position[0] = controller->joints[0].pos[0];
		transform->position[1] = -controller->joints[0].pos[1];
		transform->position[2] = -controller->joints[0].pos[2];

		vec3(0.0f,0.0f,0.0f).copyToArray(transform->eulerRotation);

		if(entity->type == TR::Entity::TRAP_FLOOR)
		{
			//This is composed by multiple meshes (joints).
			//We grab the x,z position of the entire entity and the z from one of the joints.
			transform->position[0] = controller->pos[0];
			transform->position[1] = -controller->joints[0].pos[1];
			transform->position[2] = -controller->pos[2];
		}
		else if(entity->isBlock())
		{
			// y position from the joint has different slight offsets that differ depending on the level.
			// using the controller y position and offsetting from the middle point to the ground fixes it.
			// Why does the joint position is wrong? Who knows?
			transform->position[1] = -controller->pos[1]+512;

			// for the x and z displacements we just increase Marios' interaction limits.
		}
		else
		{
			vec3 newAngle = ((controller->animation.frameA->getAngle(level->version, 0)*vec3(-1,1,-1))+controller->angle)/ M_PI * 180.f;

			newAngle.copyToArray(transform->eulerRotation);
		}

		for(int i=0; i<3; i++)
		{
			transform->position[i] = transform->position[i]/MARIO_SCALE;
		}
	}

	void create_door(int entityIndex, TR::Model *model)
	{
		TR::Entity *entity = &(level->entities[entityIndex]);

		Controller *controller = (Controller *)entity->controller;
		TR::AnimFrame *frame = controller->animation.frameA;
		struct SM64SurfaceObject obj;
		obj.surfaceCount = 0;
		
		obj.surfaceCount=12;
		obj.surfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * obj.surfaceCount);
		size_t surface_ind = 0;

		switch(entity->type)
		{
			case TR::Entity::TRAP_FLOOR:
				ADD_CUBE_GEOMETRY_NON_TRANSFORMED(obj.surfaces, surface_ind, -128, 128, -10, 10, -128, 128);
				break;
			default:

				int limits[3][2];
				for(int i=0; i<3; i++)
				{
					limits[i][0]=INT16_MAX;
					limits[i][1]=INT16_MIN;
				}

				int index = level->meshOffsets[model->mStart];
				TR::Mesh *d = &(level->meshes[index]);
				for (int i = 0; i < d->fCount; i++)
				{
					TR::Face &f = d->faces[i]; 
					for(int j = 0; j<(f.triangle ? 3 : 4); j++)
					{
						for(int w = 0; w < 3; w++)
						{
							if(d->vertices[f.vertices[j]].coord[w] < limits[w][0]) limits[w][0] = d->vertices[f.vertices[j]].coord[w];
							if(d->vertices[f.vertices[j]].coord[w] > limits[w][1]) limits[w][1] = d->vertices[f.vertices[j]].coord[w];
						}
					}
				}
				if(entity->isTrapdoor())
				{
					// raise the top limit of the trapdoor a bit to avoid Mario crossing the portal to the next room which would cause havoc with the camera.
					limits[1][0]-=10;
				}
				ADD_CUBE_GEOMETRY_ALTERNATE(obj.surfaces, surface_ind, 0, 0, limits);
				break;
		}

		updateTransformation(entity, &obj.transform);

		SM64::MarioControllerObj cObj;
		cObj.spawned = true;
		cObj.ID = sm64_surface_object_create(&obj);
		cObj.transform = obj.transform;
		cObj.entity = entity;

		dynamicObjects[dynamicObjectsCount++] = cObj;
		free(obj.surfaces);
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

			if (e->isDoor() || e->isTrapdoor() || e->type == TR::Entity::DRAWBRIDGE || e->type == TR::Entity::TRAP_FLOOR || e->isBlock())
			{
				create_door(i, model);
				continue;
			}

			struct SM64SurfaceObject obj;
			obj.surfaceCount = 0;
			obj.transform.position[0] = e->x / MARIO_SCALE;
			obj.transform.position[1] = -e->y / MARIO_SCALE;
			obj.transform.position[2] = -e->z / MARIO_SCALE;
			for (int j=0; j<3; j++) obj.transform.eulerRotation[j] = (j == 1) ? float(e->rotation) / M_PI * 180.f : 0;

			size_t surface_ind = 0;
			vec3 offset(0,0,0);
			bool doOffsetY = true;
			int16 topPointSign = 1;
			if (e->type >= 68 && e->type <= 70) topPointSign = -1;
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
			obj.surfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * obj.surfaceCount);

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
			
			obj.transform.position[0] += offset.x/MARIO_SCALE;
			obj.transform.position[1] += offset.y/MARIO_SCALE;
			obj.transform.position[2] += offset.z/MARIO_SCALE;

			// and finally add the object (this returns an uint32_t which is the object's ID)
			SM64::MarioControllerObj cObj;
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

	void printfacelimit(struct FaceLimits *fl)
	{
		printf("\nface r:%d i:%d n:%d\n", fl->roomId, fl->faceId, fl->positive);
		printf("x(%d, %d)\n", fl->limits[0][0], fl->limits[0][1]);
		printf("y(%d, %d)\n", fl->limits[1][0], fl->limits[1][1]);
		printf("z(%d, %d)\n\n", fl->limits[2][0], fl->limits[2][1]);
	}
	
	bool checkIfRoomsIntersect(int roomId1, int roomId2)
	{
		int count=0;

		struct RoomLimits *rl1 = roomLimits[roomId1];
		struct RoomLimits *rl2 = roomLimits[roomId2];

		if(!(rl1->limits[0][1] < rl2->limits[0][0] || rl1->limits[0][0] > rl2->limits[0][1]))
			count++;

		if(!(rl1->limits[1][1] < rl2->limits[1][0] || rl1->limits[1][0] > rl2->limits[1][1]))
			count++;

		if(!(rl1->limits[2][1] < rl2->limits[2][0] || rl1->limits[2][0] > rl2->limits[2][1]))
			count++;

		if(count < 2)
		{
			#ifdef DEBUG_RENDER	
			printf("Clip evaluaiton: room %d and room %d don't intersect.\n", roomId1, roomId2);
			#endif
			return false;
		}
		return true;
	}

	void evaluateSideClippingSurfaces(struct FaceLimits **faces, int facesCount, int mainAxis)
	{
		int otherAxis= mainAxis == 0 ? 2 : 0;

		for(int i=0; i<facesCount-1 && facesCount>1; i++)
		{
			for(int j=i+1; j<facesCount; j++)
			{

				if( faces[i]->positive != faces[j]->positive && faces[i]->limits[mainAxis][0] == faces[j]->limits[mainAxis][0] 
					&& !( ((faces[i]->limits[otherAxis][0] == faces[j]->limits[otherAxis][0] && faces[i]->limits[otherAxis][1] == faces[j]->limits[otherAxis][1]) ? 
						faces[i]->limits[1][0] > faces[j]->limits[1][1] || faces[i]->limits[1][1] < faces[j]->limits[1][0] : 
						faces[i]->limits[1][0] >= faces[j]->limits[1][1] || faces[i]->limits[1][1] <= faces[j]->limits[1][0] ) 
					|| faces[i]->limits[otherAxis][0] >= faces[j]->limits[otherAxis][1] || faces[i]->limits[otherAxis][1] <= faces[j]->limits[otherAxis][0] ) )
				{
					bool found=false;
					for(int w=0; w<clipsCount; w++)
					{
						struct ClipsDetected *clip = &(clips[w]);
						//We need to check if this is really a new clip that hasn't been found before
						if(clip->face1 == faces[i] || clip->face1 == faces[j])
						{
							found=true;
							break;
						}
					}
					if(!found)
					{
						clips[clipsCount++]={faces[i], faces[j]};
					}
				}
			}
		}
	}

	void evaluateClippingSurfaces()
	{

		totalXfacesCount=0;
		totalZfacesCount=0;
		for(int i=0; i<loadedRoomsCount; i++)
		{
			// i=0 is the current room. All others will be tested against the current room.
			if(i>0 && !checkIfRoomsIntersect(loadedRooms[0], loadedRooms[i]))
				continue;

			struct RoomLimits *rl = roomLimits[loadedRooms[i]];
			for(int j=0; j<rl->xfacesCount; j++)
			{
				totalXfaces[totalXfacesCount++]=&(rl->xfaces[j]);
			}
			for(int j=0; j<rl->zfacesCount; j++)
			{
				totalZfaces[totalZfacesCount++]=&(rl->zfaces[j]);
			}			
		}

		clipsCount = 0;
		evaluateSideClippingSurfaces(totalXfaces, totalXfacesCount, 2);
		evaluateSideClippingSurfaces(totalZfaces, totalZfacesCount, 0);

		#ifdef DEBUG_RENDER	
		lastClipsFound = clipsCount;
		#endif
	}

	void createClipBlockers()
	{
		clipBlockersCount=0;

		for(int i=0; i<clipsCount; i++)
		{
			struct FaceLimits *e = clips[i].face1;

			int minmax[3][2];
			for(int j=0; j<3; j++)
				for(int w=0; w<2; w++)
					minmax[j][w]=e->limits[j][w];

			if(minmax[0][0] == minmax[0][1])
			{
				minmax[0][0]-=CLIPPER_DISPLACEMENT;
				minmax[0][1]+=CLIPPER_DISPLACEMENT;
			}

			if(minmax[2][0] == minmax[2][1])
			{
				minmax[2][0]-=CLIPPER_DISPLACEMENT;
				minmax[2][1]+=CLIPPER_DISPLACEMENT;
			}

			ADD_CUBE_GEOMETRY(clipBlockers, clipBlockersCount, 0, 0, minmax[0][0], minmax[0][1], minmax[1][0], minmax[1][1], minmax[2][0], minmax[2][1]);
		}
	}

	virtual void getCurrentAndAdjacentRoomsWithClips(int marioId, int currentRoomIndex, int to, int maxDepth, bool evaluateClips = false) 
	{
		getCurrentAndAdjacentRooms(currentRoomIndex, to, maxDepth);

		if(evaluateClips)
		{
			#ifdef DEBUG_RENDER	
			struct timespec start, stop;
			clock_gettime( CLOCK_REALTIME, &start);
			#endif

			evaluateClippingSurfaces();
			createClipBlockers();

			#ifdef DEBUG_RENDER
			clock_gettime( CLOCK_REALTIME, &stop);
			clipsTimeTaken = (( stop.tv_sec - start.tv_sec ) + ( stop.tv_nsec - start.tv_nsec )/1E9L)*1E3L;
			#endif

			sm64_level_update_player_loaded_Rooms_with_clippers(marioId, loadedRooms, loadedRoomsCount, clipBlockers, clipBlockersCount);
		}
		else
		{
			sm64_level_update_loaded_rooms_list(marioId, loadedRooms, loadedRoomsCount);
		}

		return;			
	}

	void getCurrentAndAdjacentRooms(int currentRoomIndex, int to, int maxDepth, int count=0) {
		if(count==0)
		{
			loadedRoomsCount=0;
		}

        if (count>maxDepth || loadedRoomsCount == 256) {
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
			loadedRooms[loadedRoomsCount++] = to;
		}

		for (int i = 0; i < room.portalsCount; i++) {
			getCurrentAndAdjacentRooms(currentRoomIndex, room.portals[i].roomIndex, maxDepth, count);
		}
    }

	virtual int createMarioInstance(int roomIndex, vec3(pos))
	{
		getCurrentAndAdjacentRooms(roomIndex, roomIndex, 2);
		return sm64_mario_create(pos.x/MARIO_SCALE, -pos.y/MARIO_SCALE, -pos.z/MARIO_SCALE, 0, 0, 0, 0, loadedRooms, loadedRoomsCount);
	}

	virtual void flipMap(int rooms[][2], int count)
	{
		for(int i=0; i<count; i++)
		{
			struct RoomLimits *tmp = roomLimits[rooms[i][0]];
			roomLimits[rooms[i][0]] = roomLimits[rooms[i][1]];
			roomLimits[rooms[i][1]]=tmp;
		}
		sm64_level_rooms_switch(rooms, count);
	}
};

#endif