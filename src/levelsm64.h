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

	/**
	 * @brief The maximum joints available for a dynamic object 
	 */
	#define MAX_DYNAMIC_OBJECT_JOINTS 10
	#define MAX_DYNAMIC_OBJECTS 1024

	struct MarioControllerObj
    {   
        int jointsCount;
        uint32_t id[MAX_DYNAMIC_OBJECT_JOINTS];
        struct SM64ObjectTransform transforms[MAX_DYNAMIC_OBJECT_JOINTS];
        TR::Entity *entity;
		Controller *controller;
        bool spawned;

        MarioControllerObj() : jointsCount(0), entity(NULL), controller(NULL), spawned(false) 
        {
			jointsCount = 0;
            for(int i=0; i<MAX_DYNAMIC_OBJECT_JOINTS; i++)
            {
                id[i] = -1;
            }
        }

		void Update(TR::Level *level)
		{
			if(!spawned || !entity || controller == NULL) return;

			controller->updateJoints();

			for(int i=0; i<jointsCount; i++)
			{
				UpdateSingleJoint(level, i);
			}

			if(spawned)
			{
				if (entity->type == TR::Entity::TRAP_FLOOR && !controller->isCollider())
				{
					spawned = false;
					sm64_surface_object_delete(id[0]);
				}
			}
		}

		void UpdateSingleJoint(TR::Level *level, int i, bool updateSM64=true)
		{
			if(i>=jointsCount) return;

			struct SM64ObjectTransform *transform = &(transforms[i]);

			transform->position[0] = controller->joints[i].pos[0];
			transform->position[1] = -controller->joints[i].pos[1];
			transform->position[2] = -controller->joints[i].pos[2];

			vec3(0.0f,0.0f,0.0f).copyToArray(transform->eulerRotation);

			if(entity->type == TR::Entity::TRAP_FLOOR)
			{
				//This is composed by multiple meshes (joints).
				//We grab the x,z position of the entire entity and the z from one of the joints.
				transform->position[0] = controller->pos[0];
				transform->position[1] = -controller->joints[i].pos[1];
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
				vec3 newAngle = ((controller->animation.frameA->getAngle(level->version, i)*vec3(-1,1,-1))+controller->angle)/ M_PI * 180.f;

				newAngle.copyToArray(transform->eulerRotation);
			}

			for(int i=0; i<3; i++)
			{
				transform->position[i] = transform->position[i]/MARIO_SCALE;
			}

			if(updateSM64) sm64_surface_object_move(id[i], transform);
		}

		bool useOriginalGeometry()
		{
			return entity->type == TR::Entity::TRAP_BOULDER 
				|| entity->type == TR::Entity::CABIN
				|| entity->type == TR::Entity::BRIDGE_1
				|| entity->type == TR::Entity::BRIDGE_2
				|| entity->type == TR::Entity::BRIDGE_3;
		}

		SM64Surface* CreateCabin(uint32_t *count)
		{
			//We just need to define the roof and floor. The clipper boxes algorithm will do the rest.
			*count = 7*12;

			SM64Surface* result = (SM64Surface*)malloc(sizeof(SM64Surface)*(*count));

			int idx = 0;
			ADD_CUBE_GEOMETRY(result, idx, 0, 0, -1600, 1600, 0, 232, -1100, 1100); //top
			ADD_CUBE_GEOMETRY(result, idx, 0, 0, -1600, 1600, 1215, 1310, -1100, 1100); //bottom
			ADD_CUBE_GEOMETRY(result, idx, 0, 0, -1600, -1450, 232, 1215, -1100, 1100); //right
			ADD_CUBE_GEOMETRY(result, idx, 0, 0, 1412, 1600, 232, 1215, -1100, 1100); //left
			ADD_CUBE_GEOMETRY(result, idx, 0, 0, -1600, 1600, 232, 1215, -1100, -1000); //back
			ADD_CUBE_GEOMETRY(result, idx, 0, 0, -1600, -50, 232, 1215, 1000, 1100); //front right
			ADD_CUBE_GEOMETRY(result, idx, 0, 0, 550, 1600, 232, 1215, 1000, 1100); //front left
			
			return result;
		}

		SM64Surface* GetCustomGeometry(int joint, uint32_t *count)
		{
			switch (entity->type)
			{
				case TR::Entity::CABIN:
					switch (joint)
					{
						case 1:
							return CreateCabin(count);
						
						default:
							return NULL;
					}
				
				default:
					return NULL;
			}
		}

		void debugObject(TR::Level *level, TR::Model *model, int i)
		{
			printf("Cabin joint: %d\n", i);
			int limits[3][2];
			for(int i=0; i<3; i++)
			{
				limits[i][0]=INT16_MAX;
				limits[i][1]=INT16_MIN;
			}

			int index = level->meshOffsets[model->mStart + i];
			if (index || model->mStart + i <= 0)
			{
				TR::Mesh *d = &(level->meshes[index]);
				for (int fi = 0; fi < d->fCount; fi++)
				{
					TR::Face &f = d->faces[fi]; 
					for(int v = 0; v<(f.triangle ? 3 : 4); v++)
					{
						for(int w = 0; w < 3; w++)
						{
							if(d->vertices[f.vertices[v]].coord[w] < limits[w][0]) limits[w][0] = d->vertices[f.vertices[v]].coord[w];
							if(d->vertices[f.vertices[v]].coord[w] > limits[w][1]) limits[w][1] = d->vertices[f.vertices[v]].coord[w];
						}
					}
				}
			}
			printf("x(%d,%d) y(%d,%d) z(%d,%d)\n", limits[0][0], limits[0][1], limits[1][0], limits[1][1], limits[2][0], limits[2][1]);	
		}

		void LoadDynamicObject(TR::Level *level, TR::Entity* e)
		{
			entity = e;
			controller = (Controller*) e->controller;

			if (!controller)
			{
				// prevent a crash on linux
				spawned = false;
				return;
			}

			spawned = true;
			controller->updateJoints();

			TR::Model *model = &level->models[e->modelIndex - 1];
			jointsCount = model->mCount;

			// number of joints override
			switch(e->type)
			{
				case TR::Entity::TRAP_FLOOR:
					jointsCount = 1;
					break;
			}

			TR::AnimFrame *frame = controller->animation.frameA;

			for(int i=0; i<jointsCount; i++)
			{				
				struct SM64SurfaceObject obj;
				obj.surfaceCount = 0;

				obj.surfaces = GetCustomGeometry(i, &(obj.surfaceCount));

				if(obj.surfaces!=NULL)
				{
					UpdateSingleJoint(level, i, false);
					obj.transform = transforms[i];
					id[i] = sm64_surface_object_create(&obj);
					free(obj.surfaces);
					continue;
				}
				
				if(useOriginalGeometry())
				{
					int index = level->meshOffsets[model->mStart + i];
					if (index || model->mStart + i <= 0)
					{
						TR::Mesh &d = level->meshes[index];
						for (int j = 0; j < d.fCount; j++)
							obj.surfaceCount += (d.faces[j].triangle) ? 1 : 2;
					}
				}
				else
				{
					obj.surfaceCount=12;
				}

				// if(entity->type == TR::Entity::CABIN)
				// {
				//		debugObject(level, model, i);
				// }
					
				obj.surfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * obj.surfaceCount);
				size_t surface_ind = 0;

				if(useOriginalGeometry())
				{
					int index = level->meshOffsets[model->mStart + i];
					if (index || model->mStart + i <= 0)
					{
						TR::Mesh *d = &(level->meshes[index]);
						for (int j = 0; j < d->fCount; j++)
						{
							TR::Face &f = d->faces[j];
							ADD_MESH_FACE_RELATIVE(obj.surfaces, surface_ind, d, f);
						}
					}				
				}
				else
				{
					switch(entity->type)
					{
						case TR::Entity::TRAP_FLOOR:
						{
							ADD_CUBE_GEOMETRY_NON_TRANSFORMED(obj.surfaces, surface_ind, -128, 128, -10, 10, -128, 128);
							break;
						}
						default:
						{

							int limits[3][2];
							for(int j=0; j<3; j++)
							{
								limits[j][0]=INT16_MAX;
								limits[j][1]=INT16_MIN;
							}

							int index = level->meshOffsets[model->mStart + i];
							if (index || model->mStart + i <= 0)
							{
								TR::Mesh *d = &(level->meshes[index]);
								for (int fi = 0; fi < d->fCount; fi++)
								{
									TR::Face &f = d->faces[fi]; 
									for(int v = 0; v<(f.triangle ? 3 : 4); v++)
									{
										for(int w = 0; w < 3; w++)
										{
											if(d->vertices[f.vertices[v]].coord[w] < limits[w][0]) limits[w][0] = d->vertices[f.vertices[v]].coord[w];
											if(d->vertices[f.vertices[v]].coord[w] > limits[w][1]) limits[w][1] = d->vertices[f.vertices[v]].coord[w];
										}
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
					}
				}				

				UpdateSingleJoint(level, i, false);
				obj.transform = transforms[i];
				id[i] = sm64_surface_object_create(&obj);
				free(obj.surfaces);
			}
		}

		void LoadSpecialDynamicObject(TR::Level *level, int boxId)
		{
			struct SM64ObjectTransform *transform = &(transforms[0]);
			spawned = false;
	
			struct SM64SurfaceObject obj;
			obj.surfaceCount=12;
			obj.surfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * obj.surfaceCount);
			size_t surface_ind = 0;

			int limits[3][2];

			limits[0][0] = level->boxes[boxId].minX;
			limits[0][1] = level->boxes[boxId].maxX;

			limits[1][0] = level->boxes[boxId].floor;
			limits[1][1] = level->boxes[boxId].floor+150;

			limits[2][0] = level->boxes[boxId].minZ;
			limits[2][1] = level->boxes[boxId].maxZ;

			ADD_CUBE_GEOMETRY_ALTERNATE(obj.surfaces, surface_ind, 0, 0, limits);

			for (int j=0; j<3; j++) transform->position[j] = 0.0f;
			for (int j=0; j<3; j++) transform->eulerRotation[j] = 0.0f;

			obj.transform = *transform;
			id[0] = sm64_surface_object_create(&obj);
			free(obj.surfaces);
		}
    };

	struct MarioControllerObj dynamicObjects[1024];
	int dynamicObjectsCount=0;


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
		DEBUG_TIME_INIT();

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

		DEBUG_TIME_END(loadLevelTimeTaken);
    }

    struct SM64Surface* marioLoadRoomSurfaces(int roomId, int *room_surfaces_count)
	{
		*room_surfaces_count = 0;
		size_t level_surface_i = 0;

		TR::Room &room = level->rooms[roomId];
		TR::Room::Data *d = &room.data;
		TR::Room::Info *info = &room.info;

		bool foundfloors[level->floorsCount];
		for(int i=0; i<level->floorsCount; i++)
		{
			foundfloors[i]=false;
		}

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
				foundfloors[info.floorIndex]=true;
			}

			ADD_ROOM_FACE_ABSOLUTE(collision_surfaces, level_surface_i, room, d, f, slippery, roomId, cface);
		}

		// This next block is to add special dynamic objects for pickups floating in the air.
		if(controller)
		{
			for (int z = 0; z < room.zSectors; z++)
			{
				for (int x = 0; x < room.xSectors; x++)
				{
					TR::Room::Sector *sector = level->rooms[roomId].getSector(x, z);
					if(sector->boxIndex < level->boxesCount && !foundfloors[sector->floorIndex])
					{
						
						TR::Box box = level->boxes[sector->boxIndex];
						TR::Level::FloorInfo finfo;
						controller->getFloorInfo(roomId, vec3((box.minX+box.maxX)/2.0,0.0, (box.minZ+box.maxZ)/2.0), finfo);

						if(finfo.trigger == TR::Level::Trigger::PICKUP)
						{
							#ifdef DEBUG_RENDER	
							printf("created exceptional dynamic object with box %d in room %d for sector xz(%d,%d)\n", sector->boxIndex, roomId, x, z);
							#endif
							createExceptionalDynamicObject(sector->boxIndex);
						}
					}
				}                    
			}
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
		for(int i=0; i<3; i++)
		{
			rl->limits[i][0]=INT32_MAX;
			rl->limits[i][1]=INT32_MIN;
		}

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
			FaceLimits * fl=NULL;

			TR::Face &f = d->faces[cface];

			if( f.normal.y == 0 && !f.water && !f.triangle )
			{
				if (f.normal.x == 0 ) // xface
				{
					fl=&(rl->xfaces[xindex++]);
					fl->positive = f.normal.z > 0;
				}
				else if (f.normal.z == 0 ) // zface
				{
					fl=&(rl->zfaces[zindex++]);
					fl->positive = f.normal.x > 0;
				}
			}

			// If we have a face to add
			if(fl!=NULL)
			{
				fl->roomId = roomId;
				fl->faceId = cface;

				// pre-calculate face limits
				for(int i=0; i<3; i++)
				{
					fl->limits[i][0]=INT16_MAX;
					fl->limits[i][1]=INT16_MIN;
				}

				for(int i=0; i< 4; i++)
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
			}
			
			// Recheck room boundaries
			for(int i=0; i< (f.triangle ? 3 : 4); i++)
			{
				if (rl->limits[0][0] > d->vertices[f.vertices[i]].pos.x+info->x) rl->limits[0][0] = d->vertices[f.vertices[i]].pos.x+info->x;
				if (rl->limits[0][1] < d->vertices[f.vertices[i]].pos.x+info->x) rl->limits[0][1] = d->vertices[f.vertices[i]].pos.x+info->x;
				if (rl->limits[1][0] > d->vertices[f.vertices[i]].pos.y) rl->limits[1][0] = d->vertices[f.vertices[i]].pos.y;
				if (rl->limits[1][1] < d->vertices[f.vertices[i]].pos.y) rl->limits[1][1] = d->vertices[f.vertices[i]].pos.y;
				if (rl->limits[2][0] > d->vertices[f.vertices[i]].pos.z+info->z) rl->limits[2][0] = d->vertices[f.vertices[i]].pos.z+info->z;
				if (rl->limits[2][1] < d->vertices[f.vertices[i]].pos.z+info->z) rl->limits[2][1] = d->vertices[f.vertices[i]].pos.z+info->z;
			}
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

		if( abs(box.max.x-box.min.x) <2*CLIPPER_DISPLACEMENT)
		{
			box.max.x+=CLIPPER_DISPLACEMENT;
			box.min.x-=CLIPPER_DISPLACEMENT;
		}
		if( abs(box.max.y-box.min.y) <2*CLIPPER_DISPLACEMENT)
		{
			box.max.y+=CLIPPER_DISPLACEMENT;
			box.min.y-=CLIPPER_DISPLACEMENT;
		}
		if( abs(box.max.z-box.min.z) <2*CLIPPER_DISPLACEMENT)
		{
			box.max.z+=CLIPPER_DISPLACEMENT;
			box.min.z-=CLIPPER_DISPLACEMENT;
		}

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
		case TR::LevelID::LVL_TR1_1: // Caves
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_2: // City of Vilcabamba
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
		case TR::LevelID::LVL_TR1_3A: // The Lost Valley
			switch (meshIndex)
			{
				case 6: // old tree bark - mario gets stuck
				return MESH_LOADING_DISCARD;
			}
			break;
		case TR::LevelID::LVL_TR1_3B: // Tomb of Qualopec
			switch (meshIndex)
			{

			}
			break;
		case TR::LevelID::LVL_TR1_CUT_1: // cut scene after Larson's fight
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_4: // St Francis' Folly
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_5: // Colosseum
			switch (meshIndex)
			{
				case 6: // chairs. Mario gets stuck.
					return MESH_LOADING_BOUNDING_BOX;
			}
			break;
		case TR::LevelID::LVL_TR1_6:// Palace Midas
			switch (meshIndex)
			{
				case 6: //railings at the balcony at the final level are 1 face thick and mario clips
					return MESH_LOADING_BOUNDING_BOX;
				case 14: // bushes - mario gets stuck
				case 19: // small tree - mario gets stuck
				case 22: // mida's fingers - mario gets stuck
				case 23: // mida's thumb finger - mario gets stuck
					return MESH_LOADING_DISCARD;
			}
			break;
		case TR::LevelID::LVL_TR1_7A: // Cistern
			switch (meshIndex)
			{
				case 1: //railings where Mario can get stuck if he climbs them.
					return MESH_LOADING_BOUNDING_BOX;
			}
			break;
		case TR::LevelID::LVL_TR1_7B: // Tomb of Tihocan
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_CUT_2: // cut scene after Pierres's fight
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_8A: // city of kahmoon
			switch (meshIndex)
			{
				case 9: // Door with very weird mesh - Mario has stumbles against it
					return MESH_LOADING_BOUNDING_BOX;
			}
			break;
		case TR::LevelID::LVL_TR1_8B: // Obelisk of Khamoon
			switch (meshIndex)
			{
				case 3: // Furniture where Mario can get stuck
				case 4: // Furniture where Mario can get stuck
				case 5:	// Furniture where Mario can get stuck
				case 6: // Furniture where Mario can get stuck
				case 9: // Panel with weird mesh that Mario stumbles against
					return MESH_LOADING_BOUNDING_BOX;
			}
			break;
		case TR::LevelID::LVL_TR1_8C:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_10A: // Natla's Mines
			switch (meshIndex)
			{
				case 10: // coal cart - Mario get's stuck
				case 16: // coal cart - Mario get's stuck
				case 20: // cabin door - Mario get's stuck
					return MESH_LOADING_BOUNDING_BOX;
			}
			break;
		case TR::LevelID::LVL_TR1_CUT_3:
			switch (meshIndex)
			{
				
			}
			break;
		case TR::LevelID::LVL_TR1_10B: // Atlantis
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

	void createExceptionalDynamicObject(int boxId)
	{
		dynamicObjects[dynamicObjectsCount++].LoadSpecialDynamicObject(level, boxId);
	}

    void createDynamicObjects()
	{
		// create collisions from entities (bridges, doors)
		for (int i=0; i<level->entitiesCount; i++)
		{
			TR::Entity *e = &level->entities[i];
			if (e->isEnemy() || e->isLara() || e->isSprite() || e->isPuzzleHole() || e->isPickup() || e->type == 169)
				continue;

			if (!(e->type >= 68 && e->type <= 70) && !e->isDoor() && !e->isBlock() && e->type != TR::Entity::MOVING_BLOCK 
				&& e->type != TR::Entity::TRAP_FLOOR && e->type != TR::Entity::TRAP_DOOR_1 && e->type != TR::Entity::TRAP_DOOR_2 
				&& e->type != TR::Entity::DRAWBRIDGE && e->type != TR::Entity::TRAP_BOULDER && e->type != TR::Entity::MOVING_OBJECT
				&& e->type != TR::Entity::CABIN)
				continue;


			TR::Model *model = &level->models[e->modelIndex - 1];
			if (!model)
				continue;

			dynamicObjects[dynamicObjectsCount++].LoadDynamicObject(level, e);
		}
	}

	virtual void updateDynamicObjects()
	{
		DEBUG_TIME_INIT();

		for (int i=0; i< dynamicObjectsCount ; i++)
		{
			dynamicObjects[i].Update(level);
		}

		DEBUG_TIME_END_AVERAGED(updateDynamicTimeTaken);
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

		for(int i=0; i<3; i++)
		{
			if(!(rl1->limits[i][1] < rl2->limits[i][0] || rl1->limits[i][0] > rl2->limits[i][1]))
			{
				count++;
			}
		}

		if(count < 3)
		{
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

	void evaluateClippingSurfaces(SM64::MarioPlayer *player)
	{
		totalXfacesCount=0;
		totalZfacesCount=0;
		for(int i=0; i<player->loadedRoomsCount; i++)
		{
			// i=0 is the current room. All others will be tested against the current room.
			if(i>0 && !checkIfRoomsIntersect(player->loadedRooms[0], player->loadedRooms[i]))
				continue;

			struct RoomLimits *rl = roomLimits[player->loadedRooms[i]];
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
		player->lastClipsFound = clipsCount;
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

	void discardDistantRooms(SM64::MarioPlayer *player, vec3 position)
	{
		// let's consider a 2 cell lenght displacement for marios position
		// Mainly to avoid any clipping issues into static meshes geometry when entering a room
		const int displacement = 2048;

		int prevLoadedRooms[player->loadedRoomsCount];
		int prevLoadedRoomsCount = player->loadedRoomsCount;

		for(int i=0; i<player->loadedRoomsCount; i++)
		{
			prevLoadedRooms[i]=player->loadedRooms[i];
		}

		player->loadedRoomsCount=0;
		player->discardedRoomsCount=0;
		for(int i=0; i<prevLoadedRoomsCount; i++)
		{
			struct RoomLimits *rlimits = roomLimits[prevLoadedRooms[i]];
			int count=0;
			for(int j=0; j<3; j++)
			{
				// Check if mario is not out of bounds in each axis even with displacement
				if(!(position[j]+displacement < rlimits->limits[j][0] || position[j]-displacement > rlimits->limits[j][1]))
				{
					count++;
				}
			}
			// if mario is in bounds in all axis then we consider this room
			if(count==3)
			{
				player->loadedRooms[player->loadedRoomsCount++] = prevLoadedRooms[i];
			}
			else
			{
				player->discardedRooms[player->discardedRoomsCount++] = prevLoadedRooms[i];
			}
		}
	}

	/**
	 * This will help discard overlapped rooms 
	 */
	void discardDistantPortals(SM64::MarioPlayer *player, vec3 position)
	{
		for(int i=0; i<player->crossedPortalsCount; i++)
		{
			SM64::CrossedPortal *cportal = &(player->crossedPortals[i]);
			// bool found=false;

			// for(int j=0; j<player->loadedRoomsCount; j++)
			// {
			// 	if(player->loadedRooms[j]==cportal->to)
			// 	{
			// 		found=true;
			// 		break;
			// 	}
			// }
			// // 1st we invalid portals that lead to rooms we already discarded
			// if(!found)
			// {
			// 	cportal->valid=false;
			// }
			// else
			// {
				// Since this portal is going to be evaluated we calculate it's limits and if it is in range
				for(int j=0; j<3; j++)
				{
					cportal->limits[j][0]=INT_MAX;
					cportal->limits[j][1]=INT_MIN;
				}

				TR::Room::Portal *portal = cportal->portal;
				TR::Room::Info *ri = &(level->rooms[cportal->from].info);

				//we only need to get the values from opposite corners to have all limits
				for(int j=0; j<2; j++)
				{
					if(cportal->limits[0][0] > portal->vertices[j*2].x+ri->x) cportal->limits[0][0] = portal->vertices[j*2].x+ri->x;
					if(cportal->limits[0][1] < portal->vertices[j*2].x+ri->x) cportal->limits[0][1] = portal->vertices[j*2].x+ri->x;

					if(cportal->limits[1][0] > portal->vertices[j*2].y) cportal->limits[1][0] = portal->vertices[j*2].y;
					if(cportal->limits[1][1] < portal->vertices[j*2].y) cportal->limits[1][1] = portal->vertices[j*2].y;

					if(cportal->limits[2][0] > portal->vertices[j*2].z+ri->z) cportal->limits[2][0] = portal->vertices[j*2].z+ri->z;
					if(cportal->limits[2][1] < portal->vertices[j*2].z+ri->z) cportal->limits[2][1] = portal->vertices[j*2].z+ri->z;
				}

				vec3 p=position;

				//middlepoint for mario's height
				p.y-=MARIO_MIDDLE_Y; 

				if( 
					(portal->normal.x!=0 && (abs(cportal->limits[0][0]-p.x) > PORTAL_DISPLACEMENT || // x portal
					p.y + PORTAL_DISPLACEMENT < cportal->limits[1][0] || p.y - PORTAL_DISPLACEMENT > cportal->limits[1][1] ||
					p.z + PORTAL_DISPLACEMENT < cportal->limits[2][0] || p.z - PORTAL_DISPLACEMENT > cportal->limits[2][1]))
					|| 
					(portal->normal.y!=0 && (abs(cportal->limits[1][0]-p.y) > PORTAL_DISPLACEMENT || // y portal
					p.x + PORTAL_DISPLACEMENT < cportal->limits[0][0] || p.x - PORTAL_DISPLACEMENT > cportal->limits[0][1] ||
					p.z + PORTAL_DISPLACEMENT < cportal->limits[2][0] || p.z - PORTAL_DISPLACEMENT > cportal->limits[2][1]))
					||
					(portal->normal.z!=0 && (abs(cportal->limits[2][0]-p.z) > PORTAL_DISPLACEMENT || // z portal
					p.x + PORTAL_DISPLACEMENT < cportal->limits[0][0] || p.x - PORTAL_DISPLACEMENT > cportal->limits[0][1] ||
					p.y + PORTAL_DISPLACEMENT < cportal->limits[1][0] || p.y - PORTAL_DISPLACEMENT > cportal->limits[1][1])
					)
				)
				{
					player->crossedPortals[i].valid=false;
					continue;
				}
			//}
		}

		// remove all rooms that don't have a valid portal to them (except the 1st room which is always valid)
		int prevLoadedRooms[player->loadedRoomsCount];
		int prevLoadedRoomsCount = player->loadedRoomsCount;

		for(int i=0; i<player->loadedRoomsCount; i++)
		{
			prevLoadedRooms[i]=player->loadedRooms[i];
		}

		player->loadedRoomsCount=1;
		player->discardedPortalsCount=0;
		for(int i=1; i<prevLoadedRoomsCount; i++)
		{
			int roomId = prevLoadedRooms[i];
			bool found=false;
			for(int j=0; j<player->crossedPortalsCount; j++)
			{
				if(player->crossedPortals[j].to == roomId && player->crossedPortals[j].valid)
				{
					found=true;
					break;
				}
			}
			if(found)
			{
				player->loadedRooms[player->loadedRoomsCount++]=roomId;
			}
			else
			{
				player->discardedPortals[player->discardedPortalsCount++]=roomId;
			}
		}
	}

	virtual void getCurrentAndAdjacentRoomsWithClips(int marioId, vec3 position, int currentRoomIndex, int to, int maxDepth, bool evaluateClips = false) 
	{
		SM64::MarioPlayer *player = getMarioPlayer(marioId);

		DEBUG_TIME_INIT();

		player->crossedPortalsCount=0;
		getCurrentAndAdjacentRooms(player, player->loadedRooms, &(player->loadedRoomsCount), -1, currentRoomIndex, to, maxDepth);

		// If Mario is too far away from the room then there's no need to have them loaded
		//discardDistantRooms(player, position);

		// If Mario is too far from the portal that leads to a room then there's no need to have that room loaded
		discardDistantPortals(player, position);

		if(evaluateClips)
		{
			evaluateClippingSurfaces(player);
			createClipBlockers();

			sm64_level_update_player_loaded_Rooms_with_clippers(marioId, player->loadedRooms, player->loadedRoomsCount, clipBlockers, clipBlockersCount);
		}
		else
		{
			sm64_level_update_loaded_rooms_list(marioId, player->loadedRooms, player->loadedRoomsCount);
		}

		DEBUG_TIME_END_AVERAGED(player->clipsTimeTaken);

		return;			
	}

	void getCurrentAndAdjacentRooms(SM64::MarioPlayer *player, int *loadedRooms, int *loadedRoomsCount, int currentRoomIndex, int from, int to, int maxDepth, int count=0) {
		if(count==0)
		{
			*loadedRoomsCount=0;
		}

        if (count>maxDepth || *loadedRoomsCount == 256) {
            return;
        }

		// if we are starting to search we set all levels visibility to false
		if(count==0)
		{
			for (int i = 0; i < level->roomsCount; i++)
				level->rooms[i].flags.visible = false;
		}
		

		count++;

        TR::Room &room = level->rooms[to];

		if(!room.flags.visible){
			room.flags.visible = true;
			loadedRooms[(*loadedRoomsCount)++] = to;
		}

		for (int i = 0; i < room.portalsCount && count<=maxDepth; i++) {

			// Let's avoid going backwards.
			if(to == room.portals[i].roomIndex)
				continue;

			if(player!=NULL)
				player->crossedPortals[player->crossedPortalsCount++]={to, room.portals[i].roomIndex, true, &(room.portals[i])};
			getCurrentAndAdjacentRooms(player, loadedRooms, loadedRoomsCount, currentRoomIndex, to, room.portals[i].roomIndex, maxDepth, count);
		}
    }

	virtual int createMarioInstance(int roomIndex, vec3 pos, float animationScale)
	{
		SM64::MarioPlayer *player = getMarioPlayer(-1);
		getCurrentAndAdjacentRooms(NULL, player->loadedRooms, &(player->loadedRoomsCount), -1, roomIndex, roomIndex, 2);
		int marioId = sm64_mario_create(pos.x/MARIO_SCALE, -pos.y/MARIO_SCALE, -pos.z/MARIO_SCALE, 0, 0, 0, 0, player->loadedRooms, player->loadedRoomsCount, animationScale);
		player->marioId=marioId;
		return marioId;
	}

	virtual void deleteMarioInstance(int marioId)
	{
		sm64_mario_delete(marioId);
		removeMarioPlayer(marioId);
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

	virtual void marioTick(int32_t marioId, const struct SM64MarioInputs *inputs, struct SM64MarioState *outState, struct SM64MarioGeometryBuffers *outBuffers)
	{
		DEBUG_TIME_INIT();

		sm64_mario_tick(marioId, inputs, outState, outBuffers);

		DEBUG_TIME_END_AVERAGED(getMarioPlayer(marioId)->marioTickTimeTaken);
	}

	SM64::MarioPlayer *getMarioPlayer(int marioId)
	{
		for(int i=0; i<MAX_MARIO_PLAYERS; i++)
		{
			if(marioPlayers[i].marioId==marioId)
			{
				return &(marioPlayers[i]);
			}
		}
		printf("Tried to get an invalid MarioId player!\n");
		return NULL;
	}

	void removeMarioPlayer(int marioId)
	{
		getMarioPlayer(marioId)->marioId = -1;
	}

	SM64::MarioPlayer *createMarioPlayer()
	{
		SM64::MarioPlayer *player = getMarioPlayer(-1);
		
		player->lastClipsFound=0;
		player->clipsTimeTaken=0.0;
		player->marioTickTimeTaken=0.0;
		player->loadedRoomsCount;

		return player;
	}

	virtual vec3 getMarioPosition(int marioId)
	{
		float *rpos = sm64_get_mario_position(marioId);

		if(rpos == NULL)
			return vec3(0.0f,0.0f,0.0f);

		vec3 pos;
		pos.x = rpos[0] * MARIO_SCALE;
		pos.y =-rpos[1] * MARIO_SCALE;
		pos.z =-rpos[2] * MARIO_SCALE;

		return pos;
	}
};

#endif
