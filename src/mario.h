#ifndef H_MARIO
#define H_MARIO

#define MARIO_SCALE 4.f
#define IMARIO_SCALE 4

extern "C" {
	#include <libsm64/src/libsm64.h>
	#include <libsm64/src/decomp/include/surface_terrains.h>
	#include <libsm64/src/decomp/include/PR/ultratypes.h>
	#include <libsm64/src/decomp/include/audio_defines.h>
}

//#define M_PI 3.14159265358979323846f

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

struct Mario : Lara
{
	uint32_t marioId;
	struct SM64MarioInputs marioInputs;
    struct SM64MarioState marioState;
    struct SM64MarioGeometryBuffers marioGeometry;
	struct MarioRenderState marioRenderState;

	float marioTicks;
	bool postInit;
	uint32_t objIDs[4096];
	int objCount;

	Mesh* TRmarioMesh;

	Mario(IGame *game, int entity) : Lara(game, entity)
	{
		isMario = true;
		postInit = false;
		marioId = -1;
		marioTicks = 0;
		objCount = 0;

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

		// load sm64surfaces
		FILE* file = fopen("level.c", "w");
		fprintf(file, "#include \"level.h\"\n#include \"../src/decomp/include/surface_terrains.h\"\nconst struct SM64Surface surfaces[] = {\n");
		size_t surfaces_count = 0;
		size_t surface_ind = 0;
		static bool printed = false;

		for (int i = 0; i < level->roomsCount; i++)
		{
			TR::Room &room = level->rooms[i];
			TR::Room::Data &d = room.data;

			for (int j = 0; j < d.fCount; j++)
			{
				TR::Face &f = d.faces[j];
				if (f.water) continue;

				surfaces_count += (f.triangle) ? 1 : 2;
					fprintf(file, "{SURFACE_DEFAULT,0,TERRAIN_STONE,{{%d,%d,%d},{%d,%d,%d},{%d,%d,%d}}},\n", (room.info.x + d.vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[2]].pos.z)/IMARIO_SCALE, (room.info.x + d.vertices[f.vertices[1]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[1]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[1]].pos.z)/IMARIO_SCALE, (room.info.x + d.vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[0]].pos.z)/IMARIO_SCALE);
				if (!f.triangle)
					fprintf(file, "{SURFACE_DEFAULT,0,TERRAIN_STONE,{{%d,%d,%d},{%d,%d,%d},{%d,%d,%d}}},\n", (room.info.x + d.vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[0]].pos.z)/IMARIO_SCALE, (room.info.x + d.vertices[f.vertices[3]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[3]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[3]].pos.z)/IMARIO_SCALE, (room.info.x + d.vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[2]].pos.z)/IMARIO_SCALE);
				surface_ind++;
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
		printed = true;
		surface_ind = 0;

		struct SM64Surface surfaces[surfaces_count];
		for (int i = 0; i < level->roomsCount; i++)
		{
			TR::Room &room = level->rooms[i];
			TR::Room::Data &d = room.data;

			for (int j = 0; j < d.fCount; j++)
			{
				TR::Face &f = d.faces[j];
				if (f.water) continue;

				surfaces[surface_ind] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, {
					{(room.info.x + d.vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[2]].pos.z)/IMARIO_SCALE},
					{(room.info.x + d.vertices[f.vertices[1]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[1]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[1]].pos.z)/IMARIO_SCALE},
					{(room.info.x + d.vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[0]].pos.z)/IMARIO_SCALE},
				}};

				if (!f.triangle)
				{
					surface_ind++;
					surfaces[surface_ind] = {SURFACE_DEFAULT, 0, TERRAIN_STONE, {
						{(room.info.x + d.vertices[f.vertices[0]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[0]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[0]].pos.z)/IMARIO_SCALE},
						{(room.info.x + d.vertices[f.vertices[3]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[3]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[3]].pos.z)/IMARIO_SCALE},
						{(room.info.x + d.vertices[f.vertices[2]].pos.x)/IMARIO_SCALE, -d.vertices[f.vertices[2]].pos.y/IMARIO_SCALE, -(room.info.z + d.vertices[f.vertices[2]].pos.z)/IMARIO_SCALE},
					}};
				}

				surface_ind++;
			}
		}

		sm64_static_surfaces_load((const struct SM64Surface*)&surfaces, surfaces_count);
		marioId = sm64_mario_create(pos.x/MARIO_SCALE, -pos.y/MARIO_SCALE, -pos.z/MARIO_SCALE, 0, 0, 0, 0);
		printf("%.2f %.2f %.2f\n", pos.x/MARIO_SCALE, -pos.y/MARIO_SCALE, -pos.z/MARIO_SCALE);
		if (marioId >= 0) sm64_set_mario_faceangle(marioId, (int16_t)((-angle.y + M_PI) / M_PI * 32768.0f));

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
			sm64_surface_object_delete(objIDs[i]);

		if (marioId != -1) sm64_mario_delete(marioId);
	}

	void postInitMario()
	{
		if (postInit) return;
		postInit = true;

		// create collisions from entities (bridges, doors)
		for (int i=0; i<level->entitiesCount; i++)
		{
			TR::Entity *e = &level->entities[i];
			if (e->isEnemy() || e->isLara() || e->isSprite() || e->isPuzzleHole() || e->isPickup() || e->type == 169)
				continue;

			if (e->type != 68 && e->type != 69 && e->type != 70)
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
								{(d.vertices[f.vertices[0]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[0]].coord.y)/IMARIO_SCALE, -(d.center.z + d.vertices[f.vertices[0]].coord.z)/IMARIO_SCALE},
								{(d.vertices[f.vertices[3]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[3]].coord.y)/IMARIO_SCALE, -(d.center.z + d.vertices[f.vertices[3]].coord.z)/IMARIO_SCALE},
								{(d.vertices[f.vertices[2]].coord.x)/IMARIO_SCALE, -(d.vertices[f.vertices[2]].coord.y)/IMARIO_SCALE, -(d.center.z + d.vertices[f.vertices[2]].coord.z)/IMARIO_SCALE},
							}};
						}

						surface_ind++;
					}
				}
			}
			obj.transform.position[1] -= topPoint/MARIO_SCALE;

			// and finally add the object (this returns an uint32_t which is the object's ID)
			objIDs[objCount++] = sm64_surface_object_create(&obj);
			free(obj.surfaces);
		}
	}

	vec3 getPos() {return vec3(marioState.position[0], -marioState.position[1], -marioState.position[2]);}

	void render(Frustum *frustum, MeshBuilder *mesh, Shader::Type type, bool caustics)
	{
		//Lara::render(frustum, mesh, type, caustics);

		Core::setCullMode(cmBack);
		glUseProgram(Core::marioShader);

		GAPI::Texture *dtex = Core::active.textures[sDiffuse];
		Core::marioTexture->bind(sDiffuse);

		MeshRange range;
		range.aIndex = 0;
		range.iStart = 0;
		range.vStart = 0;
		range.iCount = marioRenderState.mario.num_vertices;

		TRmarioMesh->render(range);

		Core::setCullMode(cmFront);

		if (dtex) dtex->bind(sDiffuse);
	}

	virtual void hit(float damage, Controller *enemy = NULL, TR::HitType hitType = TR::HIT_DEFAULT)
	{
		if (dozy || level->isCutsceneLevel()) return;

		if (health <= 0.0f && hitType != TR::HIT_FALL) return;

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

		if (enemy)
			sm64_mario_take_damage(marioId, (uint32_t)(ceil(damage/100.f)), 0, enemy->pos.x, enemy->pos.y, enemy->pos.z);

		if (health > 0.0f)
			return;

		game->stopTrack();
		sm64_mario_kill(marioId);
	}

	bool checkInteraction(Controller *controller, const TR::Limits::Limit *limit, bool action)
	{
		return false;
	}

	int getInput()
	{
        int pid = camera->cameraIndex;
		int input = 0;
		bool canMove = (state != STATE_PICK_UP);

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

        if (Input::state[pid][cAction])    input |= ACTION;

		return input;
	}

	Stand getStand()
	{
		if (marioId < 0) return STAND_GROUND;

		if ((marioState.action & 0x000001C0) == (3 << 6)) // ((marioState.action & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED) (check if mario is in the water)
			return (marioState.position[1] >= (sm64_get_mario_water_level(marioId) - 100)*IMARIO_SCALE) ? STAND_ONWATER : STAND_UNDERWATER;
		else if (marioState.action == 0x0800034B) // marioState.action == ACT_LEDGE_GRAB
			return STAND_HANG;
		return STAND_GROUND;
	}

	void setStateFromMario()
	{	
		state = STATE_STOP;
		if (marioState.action == 0x01000889) // marioState.action == ACT_WATER_JUMP
			state = STATE_WATER_OUT;
		else if (stand == STAND_ONWATER)
			state = (marioState.velocity[0] == 0 && marioState.velocity[2] == 0) ? STATE_SURF_TREAD : STATE_SURF_SWIM;
		else if (marioState.action == 0x0800034B) // marioState.action == ACT_LEDGE_GRAB
			state = STATE_HANG;
		else if (!(marioState.action & (1 << 22)) && stand != STAND_UNDERWATER) // !(marioState.action & ACT_FLAG_IDLE)
			state = STATE_RUN;
	}

	virtual void updateState()
	{
		if (state == STATE_PICK_UP)
		{
			int16_t rot[3];
			struct SM64AnimInfo *marioAnim = sm64_mario_get_anim_info(marioId, rot);
			printf("%d %d %d %d\n", marioState.action, 0x0C400201, 0x00000383, marioAnim->animFrame, marioAnim->curAnim->loopEnd-1);

			if (marioAnim->animFrame == marioAnim->curAnim->loopEnd-1) // anim done, pick up
			{
				if (marioState.action == 0x800380) // punching action
				{
					printf("punch done\n");
					sm64_set_mario_action(marioId, 0x00000383); // ACT_PICKING_UP
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
					state = STATE_STOP;
					sm64_set_mario_action(marioId, 0x0C400201); // ACT_IDLE
					printf("now idle!\n");
				}
			}

			return;
		}

		setStateFromMario();
		Lara::updateState();
	}

	virtual bool doPickUp()
	{
		if (marioState.velocity[0] != 0 || marioState.velocity[1] != 0 || marioState.velocity[2] != 0) return false;

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
			state = STATE_PICK_UP;
			return true;
		}

		return false;
	}

	Controller* marioFindTarget()
	{
		float dist = 512.f;

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

		return target;
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
                        inventory->remove(usedItem);
                    }
                    usedItem = TR::Entity::NONE;
                default : ;
            }

            updateRoom();
			if (getRoom().waterLevelSurface != TR::NO_WATER) sm64_set_mario_water_level(marioId, -getRoom().waterLevelSurface/IMARIO_SCALE);
			else if (getRoom().flags.water) sm64_set_mario_water_level(marioId, 32767);
			else sm64_set_mario_water_level(marioId, -32768);

			vec3 p = pos;
			input = getInput();
			stand = getStand();
			updateState();
			Controller::update();

			marioInputs.camLookX = marioState.position[0] - camera->eye.pos.x;
			marioInputs.camLookZ = marioState.position[2] + camera->eye.pos.z;
			//printf("%.2f %.2f - %.2f %.2f - %.2f %.2f\n", marioState.position[0], marioState.position[2], camera->eye.pos.x, camera->eye.pos.z, marioInputs.camLookX, marioInputs.camLookZ);

			if (flags.active) {
				// do mario64 tick here
				marioTicks += Core::deltaTime;

				while (marioTicks > 1./30)
				{
					sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry);
					marioTicks -= 1./30;
					marioRenderState.mario.num_vertices = 3 * marioGeometry.numTrianglesUsed;

					for (int i=0; i<3; i++) marioState.position[i] *= MARIO_SCALE;

					for (int i=0; i<9 * SM64_GEO_MAX_TRIANGLES; i++)
					{
						marioGeometry.position[i] *= MARIO_SCALE;

						if (i%3 != 0) // flip y and z
						{
							marioGeometry.position[i] = -marioGeometry.position[i];
							marioGeometry.normal[i] = -marioGeometry.normal[i];
						}
					}

					TRmarioMesh->update(&marioGeometry);
				}

				//sm64_mario_teleport(marioId, pos.x, pos.y, pos.z);

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

				if (marioState.action & (1 << 23)) // ACT_FLAG_ATTACKING
				{
					// find an enemy close by and hurt it
					Controller* target = marioFindTarget();
					if (target)
					{
						((Character*)target)->hit(5, this);
					}
				}

				if (marioId >= 0)
				{
					angle.y = -marioState.faceAngle + M_PI;
					pos.x = (stand == STAND_HANG) ? marioState.position[0] - (sin(angle.y)*64) : marioState.position[0];
					pos.y = (stand == STAND_HANG) ? -marioState.position[1]+128 : -marioState.position[1];
					pos.z = (stand == STAND_HANG) ? -marioState.position[2] - (cos(angle.y)*64) : -marioState.position[2];
				}
				if (p != pos && updateZone())
					updateLights();

				checkTrigger(this, false);
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
