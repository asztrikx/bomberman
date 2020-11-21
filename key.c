#include "debugmalloc.h"
#include "key.h"
#include "type/object.h"
#include "type/character.h"
#include "type/geometry.h"
#include "type/world/server.h"
#include "type/key.h"
#include "geometry.h"
#include "config.h"

//KeyMovementCollisionDetectObject is a helper function of KeyMovement
static bool KeyMovementCollisionDetectObject(void* this, Object* that){
	return
		that->type == ObjectTypeWall ||
		that->type == ObjectTypeBox || (
			that->type == ObjectTypeBomb && (
				that->owner != (Character*)this ||
				that->bombOut
			)
		);
}

//KeyMovementCollisionDetectCharacter is a helper function of KeyMovement
static bool KeyMovementCollisionDetectCharacter(void* this, Character* that){
	//CharacterTypeUser is solid for CharacterTypeUser
	//CharacterTypeEnemy is not solid for CharacterTypeUser
	//vice versa with CharacterTypeEnemy
	//so only same type character is solid
	return that->type == ((Character*)this)->type;
}

//KeyMovement moves character based on it's pressed keys
void KeyMovement(Character* character, WorldServer* worldServer){
	Position positionNew = character->position;
	if(character->keyS[KeyUp]){
		positionNew.y -= character->velocity;
	}
	if(character->keyS[KeyLeft]){
		positionNew.x -= character->velocity;
	}
	if(character->keyS[KeyDown]){
		positionNew.y += character->velocity;
	}
	if(character->keyS[KeyRight]){
		positionNew.x += character->velocity;
	}

	//collision
	positionNew = CollisionLinePositionGet(
		worldServer,
		character->position,
		positionNew,
		character,
		KeyMovementCollisionDetectObject,
		KeyMovementCollisionDetectCharacter
	);

	//enemy new one way direction
	if(
		character->type == CharacterTypeEnemy &&
		PositionSame(character->position, positionNew)
	){
		for(int i=0; i<KeyLength; i++){
			character->keyS[i] = false;
		}
		
		character->keyS[rand() % KeyLength] = true;
	}
	character->position = positionNew;

	//moved out from a bomb with !bombOut
	//in one move it is not possible that it moved out from bomb then moved back again
	for(ListItem* item = worldServer->objectList->head; item != NULL; item = item->next){
		Object* object = item->data;
		if(
			object->type == ObjectTypeBomb &&
			object->owner == character &&
			!object->bombOut &&
			!CollisionPoint(character->position, object->position)
		){
			object->bombOut = true;
		}
	}
}

//KeyBombPlace places a bomb to the nearest square in the grid relative to the character
void KeyBombPlace(Character* character, WorldServer* worldServer, long long tickCount){
	//bomb available
	if(character->bombCount == 0){
		return;
	}

	Position positionNew = character->position;

	//position
	positionNew.y -= positionNew.y % squaresize;
	positionNew.x -= positionNew.x % squaresize;
	if (character->position.y % squaresize > squaresize / 2){
		positionNew.y += squaresize;
	}
	if (character->position.x % squaresize > squaresize / 2){
		positionNew.x += squaresize;
	}

	//collision
	List* collisionObjectS = CollisionPointAllObjectGet(worldServer->objectList, positionNew, NULL, NULL);
	List* collisionCharacterS = CollisionPointAllCharacterGet(worldServer->characterList, positionNew, character, NULL);

	if(
		collisionCharacterS->length != 0 ||
		collisionObjectS->length != 0
	){
		ListDelete(collisionObjectS, NULL);
		ListDelete(collisionCharacterS, NULL);
		return;
	}

	ListDelete(collisionObjectS, NULL);
	ListDelete(collisionCharacterS, NULL);

	//bomb insert
	Object* object = ObjectNew();
	object->created = tickCount;
	object->destroy = tickCount + 2 * tickSecond;
	object->position = positionNew;
	object->type = ObjectTypeBomb;
	object->velocity = 0;
	object->bombOut = false;
	object->owner = character;
	ListInsert(&(worldServer->objectList), object);

	//bomb decrease
	character->bombCount--;
}

//KeyMovementRandom sets randomly one key to be active
void KeyMovementRandom(Character* character){
	for(int i=0; i<KeyLength; i++){
		character->keyS[i] = false;
	}
	character->keyS[rand() % KeyLength] = true;
}
