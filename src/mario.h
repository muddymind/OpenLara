#ifndef H_MARIO
#define H_MARIO

extern "C" {
	#include <libsm64/src/libsm64.h>
	#include <libsm64/src/decomp/include/external_types.h>
	#include <libsm64/src/decomp/include/surface_terrains.h>
	#include <libsm64/src/decomp/include/PR/ultratypes.h>
	#include <libsm64/src/decomp/include/audio_defines.h>
	#include <libsm64/src/decomp/include/seq_ids.h>
	#include <libsm64/src/decomp/include/mario_animation_ids.h>
	#include <libsm64/src/decomp/include/sm64shared.h>
}


#include "core.h"
#include "game.h"
#include "lara.h"
#include "objects.h"
#include "sprite.h"
#include "enemy.h"
#include "inventory.h"
#include "mesh.h"
#include "format.h"
#include "enemy.h"

#include "marioMacros.h"

#define SUBMERGE_DISPLACEMENT_DELAY 1.0f

struct MarioMesh
{
	size_t num_vertices;
	uint16_t *index;
};

struct MarioRenderState
{
	MarioMesh mario;
};

struct MarioStaticMeshObj
{
	MarioStaticMeshObj() : ID(0), staticMesh(NULL) {}

	uint32_t ID;
	struct SM64ObjectTransform transform;
	TR::StaticMesh* staticMesh;
};

struct Mario : Lara
{
	int32_t marioId;
	struct SM64MarioInputs marioInputs;
    struct SM64MarioState marioState;
    struct SM64MarioGeometryBuffers marioGeometry;
	struct MarioRenderState marioRenderState;
	float lastPos[3], currPos[3];
	float lastGeom[9 * SM64_GEO_MAX_TRIANGLES], currGeom[9 * SM64_GEO_MAX_TRIANGLES];

	int customTimer;
	int switchInteraction;
	int marioWaterLevel;
	bool reverseAnim;
	float marioTicks;
	bool postInit;
	int16 previousRoomIndex;
	float submergedTime;

	Mesh* TRmarioMesh;

	Block* movingBlock; // hack
	Character* bowserSwing; // hack

	int switchSnd[3];
	bool switchSndPlayed;

	Mario(IGame *game, int entity) : Lara(game, entity)
	{
		isMario = true;
		postInit = false;
		reverseAnim = false;
		marioId = -1;
		marioTicks = 0;
		movingBlock = NULL;
		bowserSwing = NULL;
		bowserAngle = 0;
		customTimer = 0;
		marioWaterLevel = 0;
		switchInteraction = 0;
		previousRoomIndex = TR::NO_ROOM;
		submergedTime = 0.0f;

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

		#ifdef DEBUG_RENDER	
        printf("pre loading Mario\n");
        #endif

		lastPos[0] = currPos[0] = pos.x;
		lastPos[1] = currPos[1] = -pos.y;
		lastPos[2] = currPos[2] = -pos.z;

		switchSnd[0] = 38; // normal switch
		switchSnd[1] = 61; // underwater switch
		switchSnd[2] = 25; // button
		switchSndPlayed = false;

		animation.setAnim(ANIM_STAND);
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

		for (int i=0; i<SEQ_EVENT_CUTSCENE_LAKITU; i++)
			sm64_stop_background_music(i);

		if (marioId != -1) levelSM64->deleteMarioInstance(marioId);
	}

	virtual void reset(int room, const vec3 &pos, float angle, Stand forceStand = STAND_GROUND)
	{
		Lara::reset(room, pos, angle, forceStand);

		if (marioId != -1) levelSM64->deleteMarioInstance(marioId);

		marioId = levelSM64->createMarioInstance(getRoomIndex(), pos);
		if (marioId >= 0) sm64_set_mario_faceangle(marioId, (int16_t)((-angle + M_PI) / M_PI * 32768.0f));
	}

	vec3 getPos() {return vec3(marioState.position[0], -marioState.position[1], -marioState.position[2]);}
	bool canDrawWeapon() {return false;}
	void marioInteracting(TR::Entity::Type type) // hack
	{
		marioInputs.buttonB = false;
		if (type == TR::Entity::MIDAS_HAND)
			sm64_set_mario_action(marioId, ACT_UNLOCKING_STAR_DOOR);
	}

	void render(Frustum *frustum, MeshBuilder *mesh, Shader::Type type, bool caustics)
	{
		//Lara::render(frustum, mesh, type, caustics);

		Core::setCullMode(cmBack);
		Core::currMarioShader =
			(marioState.flags & MARIO_METAL_CAP) ? Core::metalMarioShader :
			((marioState.action & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED) ? Core::waterMarioShader :
			Core::marioShader;

		glUseProgram(Core::currMarioShader);

		GAPI::Texture *dtex = Core::active.textures[sDiffuse];

		if (marioState.flags & MARIO_METAL_CAP)
		{
			game->setRoomParams(getRoomIndex(), Shader::MIRROR, 1.2f, 1.0f, 0.2f, 1.0f, false);
			if (environment) environment->bind(sDiffuse);
		}
		else
			Core::marioTexture->bind(sDiffuse);

		MeshRange range;
		range.aIndex = 0;
		range.iStart = 0;
		range.vStart = 0;
		range.iCount = marioRenderState.mario.num_vertices;

		TRmarioMesh->render(range);

		if (Core::active.shader) glUseProgram(Core::active.shader->ID);

		Core::setCullMode(cmFront);

		if (dtex) dtex->bind(sDiffuse);
	}

	virtual void hit(float damage, Controller *enemy = NULL, TR::HitType hitType = TR::HIT_DEFAULT)
	{
		if (dozy || level->isCutsceneLevel()) return;
		if (!burn && !marioState.fallDamage && marioState.action != ACT_LAVA_BOOST && hitType != TR::HIT_SPIKES && (marioState.action & ACT_FLAG_INTANGIBLE || marioState.action & ACT_FLAG_INVULNERABLE)) return;
		if (health <= 0.0f && hitType != TR::HIT_FALL && hitType != TR::HIT_LAVA) return;

		if (hitType == TR::HIT_MIDAS)
		{
			if (!(marioState.flags & MARIO_METAL_CAP))
			{
				// it's a secret... ;)
				bakeEnvironment(environment);
				sm64_mario_interact_cap(marioId, MARIO_METAL_CAP, 0, true);
			}
			return;
		}
		else if (hitType == TR::HIT_LAVA)
		{
			if (damageTime > 0) return;
			sm64_set_mario_action(marioId, ACT_LAVA_BOOST);
			if (health <= 0) return;
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

		if (enemy && (hitType != TR::HIT_DART || marioState.action != ACT_LEDGE_GRAB) && // don't knockback mario with darts while holding onto a ledge
		    (
		     (!(marioState.action & ACT_FLAG_ATTACKING) && (enemy->getEntity().type != TR::Entity::ENEMY_BEAR && enemy->getEntity().type != TR::Entity::ENEMY_REX)) || // (mario is not attacking and the enemy is not one of these, or...)
		     (!(marioState.action & ACT_FLAG_AIR) && (enemy->getEntity().type == TR::Entity::ENEMY_BEAR || enemy->getEntity().type == TR::Entity::ENEMY_REX)) // (mario is not in the air, and the enemy is one of these)
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

	void setDozy(bool enable)
	{
		dozy = enable;
		updateWaterLevel();
	}

	int getInput()
	{
		int pid = camera->cameraIndex;
		int input = 0;
		bool canMove = (state != STATE_PICK_UP && state != STATE_USE_KEY && state != STATE_USE_PUZZLE && state != STATE_PUSH_BLOCK && state != STATE_PULL_BLOCK && state != STATE_PUSH_PULL_READY && state != STATE_SWITCH_DOWN && state != STATE_SWITCH_UP && state != STATE_MIDAS_USE);

		if (!dozy && Input::down[ikO])
		{
			setDozy(true);
			return input;
		}
		else if (dozy && Input::state[pid][cWalk])
		{
			setDozy(false);
			return input;
		}

		if (!camera->spectator)
		{
			Input::Joystick &joy = Input::joy[Core::settings.controls[pid].joyIndex];
			bool horizontal = (fabsf(joy.L.x) > INPUT_JOY_DZ_STICK/2.f);
			bool vertical = (fabsf(joy.L.y) > INPUT_JOY_DZ_STICK/2.f);

			if (horizontal) input |= (joy.L.x < 0.0f) ? LEFT : RIGHT;
			if (vertical) input |= (joy.L.y < 0.0f) ? FORTH : BACK;
			marioInputs.stickX = (horizontal && canMove) ? joy.L.x : 0;
			marioInputs.stickY = (vertical && canMove) ? joy.L.y : 0;
		}
		else
			marioInputs.stickX = marioInputs.stickY = 0;

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
		if (canMove && Input::state[pid][cLook] && marioState.action & ACT_FLAG_ALLOW_FIRST_PERSON)
		{
			sm64_set_mario_action(marioId, ACT_FIRST_PERSON);
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
		if (!marioInputs.stickX) marioInputs.stickX = canMove && spd ? spd * cosf(dir) : 0;
		if (!marioInputs.stickY) marioInputs.stickY = canMove && spd ? spd * sinf(dir) : 0;

		if (Input::state[pid][cUp])        input |= FORTH;
		if (Input::state[pid][cDown])      input |= BACK;
		if (Input::state[pid][cAction])    input |= ACTION;

		return input;
	}

	void increaseSubmergionTime()
	{
		if(marioState.flags & MARIO_METAL_CAP)
		{
			submergedTime = 0.0f;
		}

		submergedTime += Core::deltaTime;
		if( submergedTime > SUBMERGE_DISPLACEMENT_DELAY)
		{
			submergedTime = SUBMERGE_DISPLACEMENT_DELAY;
		}
		
	}

	void decreaseSubmergionTime()
	{
		if(marioState.flags & MARIO_METAL_CAP)
		{
			submergedTime = 0.0f;
		}

		submergedTime -= Core::deltaTime;
		if( submergedTime < 0.0f)
		{
			submergedTime = 0.0f;
		}
	}

	Stand getStand()
	{
		if (marioId < 0) return STAND_GROUND;

		if (dozy)
		{
			increaseSubmergionTime();
			return STAND_UNDERWATER;
		}

		if ((marioState.action & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED) // (check if mario is in the water)
		{
			if (marioState.position[1] >= (sm64_get_mario_water_level(marioId) - 100)*IMARIO_SCALE) 
			{
				decreaseSubmergionTime();			
				return STAND_ONWATER;
			}
			else
			{
				increaseSubmergionTime();
				return STAND_UNDERWATER;
			}
		}

		decreaseSubmergionTime();
		if (marioState.action == ACT_LEDGE_GRAB)
			return STAND_HANG;
		return (!(marioState.action & ACT_FLAG_AIR)) ? STAND_GROUND : STAND_AIR;
	}

	void setStateFromMario()
	{
		if (state == STATE_UNUSED_5 || state == STATE_PICK_UP || state == STATE_USE_KEY || state == STATE_USE_PUZZLE || state == STATE_PUSH_BLOCK || state == STATE_PULL_BLOCK || state == STATE_SWITCH_DOWN || state == STATE_SWITCH_UP || state == STATE_MIDAS_USE) return;

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
					sm64_set_mario_action(marioId, (pushState == STATE_PUSH_BLOCK) ? ACT_PUNCHING : ACT_PICKING_UP_BOWSER);
					state = pushState;
				}
			}
			return;
		}

		state = STATE_STOP;
		if (marioState.action == ACT_WATER_JUMP)
			state = STATE_WATER_OUT;
		else if (stand == STAND_ONWATER)
			state = (marioState.velocity[0] == 0 && marioState.velocity[2] == 0) ? STATE_SURF_TREAD : STATE_SURF_SWIM;
		else if (stand == STAND_UNDERWATER)
			state = STATE_TREAD;
		else if (marioState.action == ACT_LEDGE_GRAB)
			state = STATE_HANG;
		else if (stand == STAND_AIR)
			state = (marioState.velocity[0] == 0 && marioState.velocity[2] == 0) ? STATE_UP_JUMP : STATE_FORWARD_JUMP;
		else if (!(marioState.action & ACT_FLAG_IDLE) && marioState.action != ACT_PUNCHING && stand != STAND_UNDERWATER)
			state = STATE_RUN;
	}

	virtual void updateState()
	{
		setStateFromMario();
		if (state != STATE_WATER_OUT) Lara::updateState();
	}

	virtual bool doPickUp()
	{
		if (!(marioState.action & ACT_FLAG_STATIONARY) &&
		    marioState.action != ACT_BRAKING &&
		    marioState.action != ACT_MOVE_PUNCHING &&
		    marioState.action != ACT_DECELERATING &&
		    marioState.action != ACT_BRAKING &&
		    state != STATE_TREAD)
		    return false;
		if (state == STATE_PICK_UP) return false;

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
					if (dir.length2() < SQR(400.0f) /*&& getDir().dot(dir.normal()) > COS30*/) {
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
			sm64_set_mario_action(marioId, (pickupList[0]->getEntity().type == TR::Entity::SCION_PICKUP_HOLDER) ? ACT_UNLOCKING_KEY_DOOR : (stand != STAND_UNDERWATER) ? ACT_PUNCHING : ACT_WATER_PUNCH);
			timer = 0;
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
		if (stand != STAND_UNDERWATER && stand != STAND_ONWATER) return;
		Lara::applyFlow(sink);
		sm64_add_mario_position(marioId, flowVelocity.x/MARIO_SCALE, -flowVelocity.y/MARIO_SCALE, -flowVelocity.z/MARIO_SCALE);
	}

	virtual bool checkInteraction(Controller *controller, const TR::Limits::Limit *limit, bool action)
	{
		bool result = Lara::checkInteraction(controller, limit, action);
		if (result) marioInputs.buttonB = false;
		return result;
	}

	virtual void checkTrigger(Controller *controller, bool heavy, TR::Camera &waterFlow, bool *flowing)
	{
		TR::Level::FloorInfo info;
		getFloorInfo(controller->getRoomIndex(), controller->pos, info);

		if (info.lava && pos.y >= info.floor-144.f) {
			hit(LARA_MAX_HEALTH/3., NULL, TR::HIT_LAVA);
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

						if (stand != STAND_UNDERWATER)
						{
							sm64_set_mario_action(marioId, ACT_UNLOCKING_STAR_DOOR);
							if ((actionState == STATE_SWITCH_DOWN && controller->getEntity().type == TR::Entity::SWITCH) || controller->getEntity().type == TR::Entity::SWITCH_BUTTON) reverseAnim = true;
						}
						state = actionState;
						switchSndPlayed = false;
						switchInteraction = controller->getEntity().type;
						controller->activate();
					}
				}

				if (!controller->setTimer(timer))
					return;

				switchIsDown = controller->state == Switch::STATE_DOWN;
				state = STATE_STOP;
				customTimer = 0;
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
					sm64_set_mario_action(marioId, ACT_UNLOCKING_KEY_DOOR);
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
				if (abs(pos.y - info.floor)>2.0f) return;
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
					waterFlow = level->cameras[cmd.args];
					*flowing = true;
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

						if (level->version & TR::VER_TR1 && level->id == 21 && track == 6) // natla final boss!
							sm64_play_music(0, ((4 << 8) | SEQ_LEVEL_BOSS_KOOPA_FINAL), 0); // SEQUENCE_ARGS(4, seqID)
						else
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
			int roomsSwitched[level->roomsCount][2];
			int roomsSwitchedCount=0;
			game->flipMap(roomsSwitched, roomsSwitchedCount);
			game->setEffect(this, effect);
			levelSM64->flipMap(roomsSwitched, roomsSwitchedCount);
		}

	}

	virtual void doCustomCommand(int curFrame, int prevFrame)
	{
		int16_t rot[3];
		struct SM64AnimInfo *marioAnim = sm64_mario_get_anim_info(marioId, rot);
		if (!marioAnim || !marioAnim->curAnim) return;
		//if (marioState.action) printf("%d %x %d %d %d %s\n", state, marioState.action, marioAnim->animFrame, marioAnim->curAnim->loopEnd-1, customTimer, reverseAnim?"true":"false");

		switch (state)
		{
			case STATE_UNUSED_5: // swinging giant mutant
				bowserSwing->pos.x = pos.x + (sin(-marioState.faceAngle + M_PI)*512);
				bowserSwing->pos.y = pos.y - 144;
				bowserSwing->pos.z = pos.z + (cos(-marioState.faceAngle + M_PI)*512);
				bowserSwing->angle.y = -marioState.faceAngle + M_PI;
				static float angleVel = fabs(marioState.angleVel[1]);
				if (!bowserSwing || marioState.action == ACT_RELEASING_BOWSER)
				{
					state = STATE_STOP;
					bowserSwing->activate();
					bowserSwing->animation.setState(9); // GiantMutant::STATE_FALL
					bowserSwing->animation.setAnim(2); // idle animation
					bowserSwing->state = 9; // GiantMutant::STATE_FALL
					bowserSwing->stand = Character::STAND_AIR;
					bowserSwing->velocity.x = sin(-marioState.faceAngle + M_PI) * angleVel*850;
					bowserSwing->velocity.y = -angleVel*500;
					bowserSwing->velocity.z = cos(-marioState.faceAngle + M_PI) * angleVel*850;
					bowserSwing->hit(angleVel*300, this);
					bowserSwing = NULL;
				}
				else angleVel = fabs(marioState.angleVel[1]);

				break;

			case STATE_SWITCH_DOWN:
				if (switchInteraction == TR::Entity::SWITCH_BUTTON)
				{
					if (!switchSndPlayed)
					{
						if (!reverseAnim) reverseAnim = true;
						if (customTimer == 0) customTimer++;
						if (customTimer < 15) sm64_set_mario_anim_frame(marioId, 94);
					}

					if (marioAnim->animFrame == 80 && !switchSndPlayed)
					{
						game->playSound(switchSnd[2], pos, Sound::PAN);
						switchSndPlayed = true;
					}
					else if (marioAnim->animFrame == 61)
					{
						sm64_set_mario_action(marioId, ACT_IDLE);
						reverseAnim = false;
						customTimer = 0;
					}
				}
				else if (marioState.action == ACT_UNLOCKING_STAR_DOOR)
				{
					if (!switchSndPlayed)
					{
						if (!reverseAnim) reverseAnim = true;
						if (customTimer == 0) customTimer++;
						if (customTimer < 19) sm64_set_mario_anim_frame(marioId, 94);
					}

					if (marioAnim->animFrame == 92 && !switchSndPlayed)
					{
						game->playSound(switchSnd[0], pos, Sound::PAN);
						switchSndPlayed = true;
					}
					else if (marioAnim->animFrame == 68)
					{
						sm64_set_mario_action(marioId, ACT_IDLE);
						reverseAnim = false;
						customTimer = 0;
					}
				}
				else if (stand == STAND_UNDERWATER)
				{
					if (customTimer == 0) customTimer++;
					else if (customTimer == 62 && !switchSndPlayed)
					{
						game->playSound(switchSnd[1], pos, Sound::PAN);
						switchSndPlayed = true;
						sm64_set_mario_action(marioId, ACT_WATER_PUNCH);
					}
					sm64_set_mario_velocity(marioId, 0, 0, 0);
					if (marioState.action != ACT_WATER_PUNCH) sm64_add_mario_position(marioId, 0, ((marioWaterLevel - 80) - (marioState.position[1]/MARIO_SCALE) < 400.0f) ? -0.625f : 1, 0);
				}
				break;

			case STATE_SWITCH_UP:
				if (switchInteraction == TR::Entity::SWITCH_BUTTON)
				{
					if (!switchSndPlayed)
					{
						if (!reverseAnim) reverseAnim = true;
						if (customTimer == 0) customTimer++;
						if (customTimer < 15) sm64_set_mario_anim_frame(marioId, 94);
					}

					if (marioAnim->animFrame == 80 && !switchSndPlayed)
					{
						game->playSound(switchSnd[2], pos, Sound::PAN);
						switchSndPlayed = true;
					}
					else if (marioAnim->animFrame == 61)
					{
						sm64_set_mario_action(marioId, ACT_IDLE); // ACT_IDLE
						reverseAnim = false;
						customTimer = 0;
					}
				}
				else if (switchInteraction != TR::Entity::SWITCH_BUTTON)
				{
					if (marioAnim->animFrame == 80 && !switchSndPlayed)
					{
						game->playSound((switchInteraction == TR::Entity::SWITCH_BUTTON) ? switchSnd[2] : switchSnd[0], pos, Sound::PAN);
						switchSndPlayed = true;
					}
					else if (marioAnim->animFrame == marioAnim->curAnim->loopEnd-1)
						sm64_set_mario_action(marioId, ACT_IDLE); // ACT_IDLE
				}
				break;

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
						sm64_set_mario_action(marioId, ACT_IDLE);
						state = STATE_STOP;
					}
				}
				break;

			case STATE_PUSH_BLOCK:
				{
					static bool moved = false;
					if (!movingBlock)
						state = STATE_STOP;
					else if (marioState.flags & MARIO_PUNCHING && !movingBlock->marioAnim && !moved) // MARIO_PUNCHING
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
					if (marioState.action != ACT_RELEASING_BOWSER) sm64_set_mario_action(marioId, ACT_RELEASING_BOWSER);
					else if (marioAnim->animFrame == marioAnim->curAnim->loopEnd-1)
					{
						movingBlock = NULL;
						state = STATE_STOP;
					}
				}
				break;

			case STATE_PICK_UP:
				{
					if (pickupList[0]->getEntity().type == TR::Entity::SCION_PICKUP_HOLDER)
					{
						switch(marioState.action)
						{
							case ACT_UNLOCKING_KEY_DOOR:
								if (marioAnim->animFrame == 20)
								{
									marioAnim->animFrame--;
									if (customTimer == 0) customTimer++;
									else if (customTimer >= 25)
									{
										sm64_set_mario_action(marioId, ACT_UNLOCKING_STAR_DOOR);
										customTimer = 0;
									}
								}
								break;

							case ACT_UNLOCKING_STAR_DOOR:
								if (marioAnim->animFrame == marioAnim->curAnim->loopEnd-1)
									sm64_set_mario_action(marioId, ACT_THROWING);
								break;

							case ACT_THROWING:
								if (marioAnim->animFrame == 7) // mario touches the scion
									game->loadNextLevel();
								break;
						}
					}

					int end = (marioState.action == ACT_CREDITS_CUTSCENE) ? 60 : marioAnim->curAnim->loopEnd-1;
					if (marioAnim->animFrame == end) // anim done, pick up
					{
						if (marioState.action == ACT_PUNCHING || marioState.action == ACT_MOVE_PUNCHING || marioState.action == ACT_WATER_PUNCH)
						{
							if (stand != STAND_UNDERWATER && stand != STAND_ONWATER) sm64_set_mario_action(marioId, ACT_PICKING_UP);  
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
						else if (marioState.action == ACT_PICKING_UP) // ACT_PICKING_UP
						{
							if (pickupList[0] && pickupList[0]->getEntity().type == TR::Entity::SCION_PICKUP_QUALOPEC)
							{
								sm64_set_mario_action(marioId, ACT_CREDITS_CUTSCENE); // ACT_CREDITS_CUTSCENE
								sm64_set_mario_animation(marioId, MARIO_ANIM_CREDITS_RAISE_HAND);
							}
							else
							{
								state = STATE_STOP;
								sm64_set_mario_action(marioId, ACT_IDLE); // ACT_IDLE
							}
						}
						else if (marioState.action == ACT_CREDITS_CUTSCENE) // ACT_CREDITS_CUTSCENE
						{
							state = STATE_STOP;
							sm64_set_mario_action(marioId, ACT_IDLE); // ACT_IDLE
							if (level->id == 16 && level->version & TR::VER_TR1) // hack for ending at Sanctuary of the Scion
								game->loadNextLevel();
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
					if (marioState.action == ACT_UNLOCKING_KEY_DOOR && marioAnim->animFrame == marioAnim->curAnim->loopEnd-end)
					{
						sm64_set_mario_action(marioId, ACT_IDLE);
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


	bool arrayContainsValue(int val, int *array, int arraySize){
		for(int i=0; i<arraySize; i++)
		{
			if(array[i]==val)
				return true;
		}
		return false;
	}

	bool areArraysEqual(int *a, int *b, int size)
	{
		for(int i=0; i<size; i++)
		{
			if(!arrayContainsValue(a[i], b, size))
				return false;
		}
		return true;
	}

	#ifdef DEBUG_RENDER	
	int prevWaterLevel = -32767;
	#endif

	int getRoomAboveWaterLevel(int16 roomId, vec3 position)
	{
		int sx = (pos.x - level->rooms[roomId].info.x) / 1024;
		int sz = (pos.z - level->rooms[roomId].info.z) / 1024;

		TR::Room::Sector *sector = level->rooms[roomId].getSector(sx, sz);
		if(sector->roomAbove!=TR::NO_ROOM)
		{
			TR::Room *roomAbove = &(level->rooms[sector->roomAbove]);
			if(roomAbove->flags.water && roomAbove->waterLevelSurface != TR::NO_WATER)
			{
				return -roomAbove->waterLevelSurface/IMARIO_SCALE;
			}
			else if(roomAbove->flags.water)
			{
				return 32767;
			}
			else
			{
				return -32767;
			}
		}
		return -32767;
	}

	int getRoomBellowWaterLevel(int16 roomId, vec3 position)
	{
		int sx = (pos.x - level->rooms[roomId].info.x) / 1024;
		int sz = (pos.z - level->rooms[roomId].info.z) / 1024;

		TR::Room::Sector *sector = level->rooms[roomId].getSector(sx, sz);
		if(sector->roomBelow!=TR::NO_ROOM)
		{
			TR::Room *roomBelow = &(level->rooms[sector->roomBelow]);
			if(roomBelow->flags.water && roomBelow->waterLevelSurface != TR::NO_WATER)
			{
				return -roomBelow->waterLevelSurface/IMARIO_SCALE;
			}
			else if(roomBelow->flags.water)
			{
				return 32767;
			}
			else
			{
				return -32767;
			}
		}
		return -32767;
	}
	
	void updateWaterLevel()
	{
		// Set the water level to SM64
		if (marioId >= 0)
		{
			marioWaterLevel = -32768;

			if(dozy)
			{
				marioWaterLevel = 32767;
			}
			else if(getRoom().flags.water)
			{
				if(getRoom().waterLevelSurface != TR::NO_WATER)
				{
					int currentRoomWaterLevel = -getRoom().waterLevelSurface/IMARIO_SCALE;
					int aboveRoomWaterLevel = getRoomAboveWaterLevel(roomIndex, pos);

					if(currentRoomWaterLevel>aboveRoomWaterLevel)
					{
						marioWaterLevel = currentRoomWaterLevel;
					}
					else
					{
						marioWaterLevel = aboveRoomWaterLevel;
					}
				}
				else
				{
					marioWaterLevel = 32767;
				}
			}
			else
			{
				marioWaterLevel = getRoomBellowWaterLevel(roomIndex, pos);
			}

			#ifdef DEBUG_RENDER	
			if(prevWaterLevel != marioWaterLevel)
			{
				printf("MarioId: %d - new water level: %d\n", marioId, marioWaterLevel);
				prevWaterLevel = marioWaterLevel;
			}
			#endif
			sm64_set_mario_water_level(marioId, marioWaterLevel);
		}
	}

	void update()
	{
		updateWaterLevel();

		if(marioId==-1)
		{
			#ifdef DEBUG_RENDER	
			printf("actual mario loading\n");
			#endif
			marioId = levelSM64->createMarioInstance(getRoomIndex(), pos);
			if (marioId >= 0) sm64_set_mario_faceangle(marioId, (int16_t)((-angle.y + M_PI) / M_PI * 32768.0f));
		}
		
		levelSM64->getCurrentAndAdjacentRoomsWithClips(marioId, pos, getRoomIndex(), getRoomIndex(), 2, true);

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

			updateRoom();

			if (animation.index != ANIM_STAND && state != STATE_PICK_UP) animation.setAnim(ANIM_STAND);

			vec3 p = pos;
			input = getInput();
			stand = getStand();
			updateState();
			Controller::update();

			if (burn)
			{
				if (marioState.action != ACT_BURNING_JUMP && marioState.action != ACT_BURNING_FALL && marioState.action != ACT_LAVA_BOOST && marioState.action != ACT_BURNING_GROUND)
				{
					sm64_set_mario_action_arg(marioId, ACT_BURNING_JUMP, 1);
					sm64_play_sound_global(SOUND_MARIO_ON_FIRE);
				}
				else if (marioState.burnTimer <= 2)
					burn = false;
			}

			marioInputs.camLookX = marioState.position[0] - (camera->spectator ? camera->specPosSmooth.x : camera->eye.pos.x);
			marioInputs.camLookZ = marioState.position[2] + (camera->spectator ? camera->specPosSmooth.z : camera->eye.pos.z);

			if (flags.active) {
				// do mario64 tick here
				marioTicks += Core::deltaTime;
				TR::Camera waterFlow;
				bool flowing = false;
				checkTrigger(this, false, waterFlow, &flowing);

				while (marioTicks > 1./30)
				{
					if (flowing) applyFlow(waterFlow);
					if (customTimer) customTimer++;
					if (reverseAnim)
					{
						int16_t rot[3];
						struct SM64AnimInfo *marioAnim = sm64_mario_get_anim_info(marioId, rot);
						sm64_set_mario_anim_frame(marioId, marioAnim->animFrame-2);
						if (marioAnim->animFrame <= 0) reverseAnim = false;
					}

					for (int i=0; i<3; i++) lastPos[i] = marioState.position[i];
					for (int i=0; i<3 * marioRenderState.mario.num_vertices; i++) lastGeom[i] = marioGeometry.position[i];

					levelSM64->marioTick(marioId, &marioInputs, &marioState, &marioGeometry);
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

					if(submergedTime != 0.0f)
					{
						float displacement = submergedTime * 280 / SUBMERGE_DISPLACEMENT_DELAY;
						for (int i=0; i<3 * marioRenderState.mario.num_vertices; i+=3) 
						{
							currGeom[i] -=cos(angle.x)*displacement*sin(angle.y);
							currGeom[i+1] +=displacement;
							currGeom[i+2] +=sin(angle.x)*displacement*cos(angle.y);
						}
					}

					if (marioState.fallDamage)
						hit(marioState.fallDamage / 8 * 250);
				}

				for (int i=0; i<3; i++) marioState.position[i] = lerp(lastPos[i], currPos[i], marioTicks/(1./30));
				for (int i=0; i<3 * marioRenderState.mario.num_vertices; i++) marioGeometry.position[i] = lerp(lastGeom[i], currGeom[i], marioTicks/(1./30));
				TRmarioMesh->update(&marioGeometry);

				float hp = health / LARA_MAX_HEALTH;
				float ox = oxygen / LARA_MAX_OXYGEN;
				if (hp > 0.f)
				{
					switch(stand)
					{
						case STAND_UNDERWATER:
							sm64_mario_set_health(marioId, (ox <= 0.2f) ? MARIO_LOW_HEALTH : MARIO_FULL_HEALTH); // if low on oxygen, play sound
							break;
						
						case STAND_ONWATER:
							sm64_mario_set_health(marioId, MARIO_FULL_HEALTH); // don't play low oxygen sound on surface
							break;
						
						default:
							sm64_mario_set_health(marioId, (hp <= 0.2f) ? MARIO_LOW_HEALTH : MARIO_FULL_HEALTH); // mario panting animation if low on health
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
						if (collide(spheres[i]) &&
						    (sm64_mario_attack(marioId, spheres[i].center.x, spheres[i].center.y, -spheres[i].center.z, 0) ||
						     sm64_mario_attack(marioId, spheres[i].center.x, -spheres[i].center.y, -spheres[i].center.z, 0)))
						{
							if (stand == STAND_GROUND && i == 0 && target->getEntity().type == TR::Entity::ENEMY_GIANT_MUTANT 
								&& (marioState.action == ACT_MOVE_PUNCHING || marioState.action == ACT_DIVE || marioState.action == ACT_DIVE_SLIDE)) 
							{
								// mario picks up the giant mutant and swings it
								bowserSwing = (Character*)target;
								bowserAngle = angle.y;
								game->playSound(TR::SND_HIT_MUTANT, target->pos, Sound::PAN);
								target->deactivate();
								sm64_set_mario_action(marioId, ACT_PICKING_UP_BOWSER);
								state = STATE_UNUSED_5; // swing the mutant
								break;
							}

							float damage = 5.f;
							if (marioState.action == ACT_JUMP_KICK)
								damage += 5;
							else if (marioState.action == ACT_GROUND_POUND)
							{
								damage += 15;
								sm64_set_mario_action(marioId, ACT_TRIPLE_JUMP);
								sm64_play_sound_global(SOUND_ACTION_HIT);
							}

							if (marioState.flags & MARIO_METAL_CAP)
								damage += 25;

							printf("attacked sphere %d/%d: %.2f %.2f %.2f - %.2f %.2f %.2f\n", i, count, marioState.position[0], -marioState.position[1], -marioState.position[2], spheres[i].center.x, spheres[i].center.y, spheres[i].center.z);
							((Character*)target)->hit(damage, this);
							break;
						}
					}
				}

				if (marioId >= 0)
				{
					angle.x = (stand == STAND_UNDERWATER) ? marioState.pitchAngle : 0;
					angle.y = (state == STATE_UNUSED_5) ? bowserAngle : -marioState.faceAngle + M_PI;

					// boss fight
					if( state == STATE_UNUSED_5 ) 
					{
						pos.x = marioState.position[0] + (sin(-marioState.faceAngle + M_PI)*32);
						pos.y=-marioState.position[1];
						pos.z = -marioState.position[2] + (cos(-marioState.faceAngle + M_PI)*32);
					}
					else if ( marioState.action == ACT_LEDGE_GRAB ) 
					{
						
						pos.x= marioState.position[0] - (sin(angle.y)*140);
						pos.y=-marioState.position[1]+722;
						pos.z=-marioState.position[2] - (cos(angle.y)*140);
					}
					else if (marioState.action == ACT_LEDGE_CLIMB_SLOW_1 || marioState.action == ACT_LEDGE_CLIMB_FAST) 
					{
						pos.x = marioState.position[0] - (sin(angle.y)*128); 
						pos.y=-marioState.position[1]-128;
						pos.z=-marioState.position[2] - (cos(angle.y)*128);
					}
					else
					{
						pos.x = marioState.position[0];
						if(marioState.action == ACT_LEDGE_CLIMB_SLOW_2) 
							pos.y=-marioState.position[1]-128;
						else
							pos.y =-marioState.position[1];
						pos.z =-marioState.position[2];
					}

					if(submergedTime != 0.0f)
					{
						float displacement = submergedTime * 280 / SUBMERGE_DISPLACEMENT_DELAY;
						pos.y=-marioState.position[1]+displacement;
					}

					velocity.x = marioState.velocity[0] * 2;
					velocity.y = -marioState.velocity[1] * 2;
					velocity.z = -marioState.velocity[2] * 2;
					speed = sqrtf((velocity.x*velocity.x) + (velocity.z*velocity.z));

					if (marioState.particleFlags & (1 << 11)) // PARTICLE_FIRE
						game->addEntity(TR::Entity::SMOKE, getRoomIndex(), pos);
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

        if (stand == STAND_UNDERWATER && !dozy && !(marioState.flags & MARIO_METAL_CAP)) {
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
