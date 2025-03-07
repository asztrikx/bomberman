#include "debugmalloc.h"
#include "SDL.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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

//SDLResourceListLoadObject loads resources related to objects into TextureSSObject
static void SDLResourceListLoadObject(void){
	ObjectType objectTypeS[] = {
		ObjectTypeBomb,
		ObjectTypeBombFire,
		ObjectTypeBox,
		ObjectTypeWall,
		ObjectTypeExit,
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
			case ObjectTypeExit:
				folder = "object/exit";
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
				if(index == 0){
					SDL_Log("SDLResourceListLoadObject: missing resource: %s", path);
					exit(1);
				}
				break;
			}

			ArrayInsert(TextureSSObject[objectTypeS[i]], texture, sizeof(SDL_Texture*));
			index++;
		}
	}
}

//SDLResourceListLoadCharacter loads resources related to characters TextureSSCharacter
static void SDLResourceListLoadCharacter(void){
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
				if(index == 0){
					SDL_Log("SDLResourceListLoadObject: missing resource: %s", path);
					exit(1);
				}
				break;
			}

			ArrayInsert(TextureSSCharacter[characterTypeS[i]], texture, sizeof(SDL_Texture*));
			index++;
		}
	}
}

//SDLInit loads SDL modules
void SDLInit(void){
	//init
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		SDL_Log("SDLInit: SDL_Init: %s", SDL_GetError());
		exit(1);
	}

	//window
	SDLWindow = SDL_CreateWindow(
		"Bomberman",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_FULLSCREEN_DESKTOP
	);
	if (SDLWindow == NULL) {
		SDL_Log("SDLInit: SDL_CreateWindow: %s", SDL_GetError());
		exit(1);
	}

	//adapt fullscreen
	SDL_DisplayMode sdl_DisplayMode;
	SDL_GetCurrentDisplayMode(0, &sdl_DisplayMode);
	windowHeight = sdl_DisplayMode.h;
	windowWidth = sdl_DisplayMode.w;

	//render
	SDLRenderer = SDL_CreateRenderer(SDLWindow, -1, SDL_RENDERER_SOFTWARE);
	if (SDLRenderer == NULL) {
		SDL_Log("SDLInit: SDL_CreateRenderer: %s", SDL_GetError());
		exit(1);
	}
	SDL_RenderClear(SDLRenderer);

	SDLResourceListLoadObject();
	SDLResourceListLoadCharacter();
}

//SDLTextureDelete is a helper function of ArrayDelete
static void SDLTextureDelete(void* texture){
	SDL_DestroyTexture(texture);
}

//SDLDestroy unloads SDL modules
void SDLDestroy(void){
	for(int i=0; i<TextureSSLengthObject; i++){
		ArrayDelete(TextureSSObject[i], SDLTextureDelete);
	}
	for(int i=0; i<TextureSSLengthCharacter; i++){
		ArrayDelete(TextureSSCharacter[i], SDLTextureDelete);
	}
	free(TextureSSObject);
	free(TextureSSCharacter);

	SDL_Quit();
}
