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

	Mesh* TRmarioMesh;

	Mario(IGame *game, int entity) : Lara(game, entity)
	{
		isMario = true;
		marioId = -1;
		marioTicks = 0;

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

		fprintf(file, "};\nconst size_t surfaces_count = sizeof( surfaces ) / sizeof( surfaces[0] );\n");
		fprintf(file, "const int32_t spawn[] = {%d, %d, %d};\n", pos.x/IMARIO_SCALE, -pos.y/IMARIO_SCALE, -pos.z/IMARIO_SCALE);
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

		if (marioId != -1) sm64_mario_delete(marioId);
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
		if (Input::state[pid][cLook] && marioState.action & (1 << 26)) // ACT_FLAG_ALLOW_FIRST_PERSON
		{
			sm64_set_mario_action(marioId, 0x0C000227); // ACT_FIRST_PERSON
			if (!lookSnd) sm64_play_sound_global(SOUND_MENU_CAMERA_ZOOM_IN);
			lookSnd = true;
			camera->mode = Camera::MODE_LOOK;
		}
		else if (!Input::state[pid][cLook] || !(marioState.action & (1 << 26)))
		{
			if (lookSnd)
			{
				sm64_play_sound_global(SOUND_MENU_CAMERA_ZOOM_OUT);
				lookSnd = false;
			}
		}

		marioInputs.buttonA = Input::state[pid][cJump];
		marioInputs.buttonB = Input::state[pid][cAction];
		marioInputs.buttonZ = Input::state[pid][cDuck];
		marioInputs.stickX = spd ? spd * cosf(dir) : 0;
		marioInputs.stickY = spd ? spd * sinf(dir) : 0;

		return 0;
	}

	Stand getStand()
	{
		if (marioId < 0) return STAND_GROUND;

		if ((marioState.action & 0x000001C0) == (3 << 6)) // ((marioState.action & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED) (check if mario is in the water)
			return (marioState.position[1] >= (sm64_get_mario_water_level(marioId) - 140)*IMARIO_SCALE) ? STAND_ONWATER : STAND_UNDERWATER;
		return STAND_GROUND;
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
				if (p != pos) {
					if (updateZone())
						updateLights();
					else
						pos = p;
				}

				float hp = health / float(LARA_MAX_HEALTH);
				if (hp > 0.f && hp <= 0.2f)
				{
					// mario panting
					sm64_mario_set_health(marioId, 0x200);
				}
				else if (hp > 0.2f)
				{
					// keep mario's health full
					sm64_mario_set_health(marioId, 0x880);
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
					pos.x = marioState.position[0];
					pos.y = -marioState.position[1];
					pos.z = -marioState.position[2];
					angle.y = -marioState.faceAngle + M_PI;
				}
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
