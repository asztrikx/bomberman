#include "debugmalloc.h"
#include "SDL.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "config.h"
#include "type/array.h"
#include "type/object.h"
#include "type/character.h"

SDL_Window* SDLWindow;
SDL_Renderer* SDLRenderer;

Array** TextureSSObject;
int TextureSSLengthObject;
Array** TextureSSCharacter;
int TextureSSLengthCharacter;
int* DelaySObject;
int* DelaySCharacter;

void sdlResourceListLoadObject(void){
	ObjectType objectTypeS[] = {
		ObjectTypeBomb,
		ObjectTypeBombFire,
		ObjectTypeBox,
		ObjectTypeWall,
	};
	TextureSSLengthObject = sizeof(objectTypeS) / sizeof(ObjectType);

	TextureSSObject = (Array**) malloc(TextureSSLengthObject * sizeof(Array*));

	for (int i=0; i < TextureSSLengthObject; i++){
		TextureSSObject[objectTypeS[i]] = ArrayNew(sizeof(SDL_Texture*));

		char* folder;
		switch (objectTypeS[i]){
			case ObjectTypeBomb:
				folder = "object/bomb";
				break;
			case ObjectTypeBombFire:
				folder = "object/bombFire";
				break;
			case ObjectTypeBox:
				folder = "object/box";
				break;
			case ObjectTypeWall:
				folder = "object/wall";
				break;
		}

		//insert
		int index = 0;
		while(true){
			//path with index
			char path[200];
			sprintf(path, "resource/%s/%d.png", folder, index);

			//texture save
			SDL_Texture* texture = IMG_LoadTexture(SDLRenderer, path);
			if(texture == NULL){
				break;
			}

			ArrayInsert(TextureSSObject[objectTypeS[i]], texture, sizeof(SDL_Texture*));
			index++;
		}
	}
}

void sdlResourceListLoadCharacter(void){
	CharacterType characterTypeS[] = {
		CharacterTypeUser,
		CharacterTypeYou,
		CharacterTypeEnemy,
	};
	TextureSSLengthCharacter = sizeof(characterTypeS) / sizeof(CharacterType);

	TextureSSCharacter = (Array**) malloc(TextureSSLengthCharacter * sizeof(Array*));

	for (int i=0; i < TextureSSLengthCharacter; i++){
		TextureSSCharacter[characterTypeS[i]] = ArrayNew(sizeof(SDL_Texture*));

		char* folder;
		switch (characterTypeS[i]){
			case CharacterTypeUser:
			case CharacterTypeYou:
				folder = "character/user";
				break;
			case CharacterTypeEnemy:
				folder = "character/enemy";
				break;
		}

		//insert
		int index = 0;
		while(true){
			//path with index
			char path[200];
			sprintf(path, "resource/%s/%d.png", folder, index);

			//texture save
			SDL_Texture* texture = IMG_LoadTexture(SDLRenderer, path);
			if(texture == NULL){
				break;
			}

			ArrayInsert(TextureSSCharacter[characterTypeS[i]], texture, sizeof(SDL_Texture*));
			index++;
		}
	}
}

void SDLInit(void){
	//init
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		SDL_Log("sdl init: %s", SDL_GetError());
		exit(1);
	}

	/*if (TTF_Init() < 0) {
		SDL_Log("sdl init: %s", SDL_GetError());
		exit(1);
	}*/

	//window
	SDLWindow = SDL_CreateWindow("alt", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0);
	if (SDLWindow == NULL) {
		SDL_Log("sdl window: %s", SDL_GetError());
		exit(1);
	}

	//render
	SDLRenderer = SDL_CreateRenderer(SDLWindow, -1, SDL_RENDERER_SOFTWARE);
	if (SDLRenderer == NULL) {
		SDL_Log("sdl render: %s", SDL_GetError());
		exit(1);
	}
	SDL_RenderClear(SDLRenderer);

	sdlResourceListLoadObject();
	sdlResourceListLoadCharacter();
}

void sdlTextureDelete(void* texture){
	SDL_DestroyTexture(texture);
}

void SDLDestroy(void){
	for(int i=0; i<TextureSSLengthObject; i++){
		ArrayDelete(TextureSSObject[i], sdlTextureDelete);
	}
	for(int i=0; i<TextureSSLengthCharacter; i++){
		ArrayDelete(TextureSSCharacter[i], sdlTextureDelete);
	}
	free(TextureSSObject);
	free(TextureSSCharacter);

	//TTF_Quit();
	SDL_Quit();
}
