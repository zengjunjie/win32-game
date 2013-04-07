#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>

#define KEY_LEFT			0x0001
#define KEY_RIGHT			0x0002
#define KEY_DIRECTION		0x0003
#define KEY_FIRE			0x0004

#define PLAYER_LEFT_KEY		SDLK_a
#define PLAYER_RIGHT_KEY	SDLK_d
#define PLAYER_FIRE_KEY		SDLK_SPACE

// Ԥ��֡��(50֡/��)
#define	FRAMES_PER_SEC	50
// ����ƶ��ٶ�
#define PLAYER_SPEED	4
// ͬʱ���ڵ�����ӵ���Ŀ
#define MAX_BULLETS	3
// �ӵ�����ٶ�
#define SHOT_SPEED	6
// ������
#define MIN_SHOT_TIME	200
// �������������
#define MAX_ALIENS	30
// �������ƶ��ٶ�
#define ALIEN_SPEED	5
// �����߲������
#define MIN_ALIEN_TIME	1000
// ��ը����ʱ��
#define EXPLODE_TIME	100

// ����ṹ��
struct Sprite
{
	Uint32			alive;		// �Ƿ���
	int				direct;		// �ƶ�����
	SDL_Rect		rect;		// �����ڵ�ͼ�еİ�Χ����
	SDL_Surface *	image;		// ͼ��
};

// ���ڱ���
SDL_Surface *gScreen = NULL;
// ����ͼ��
SDL_Surface *gBkImage = NULL;
// ���ͼ��
SDL_Surface *gPlayerImage = NULL;
// �ӵ�ͼ��
SDL_Surface *gBulletImage = NULL;
// ������ͼ��
SDL_Surface *gAlienImage = NULL;
// ��ըͼ��
SDL_Surface *gExplodeImage = NULL;

// ��Ҿ���
Sprite gPlayer;
// �ӵ���������
Sprite gBullets[MAX_BULLETS];
// �����߾�������
Sprite gAliens[MAX_ALIENS];
// ��ը����
Sprite gExplodes[MAX_ALIENS];

// ��Чö��
enum {
	MUSIC_WAV,
	SHOT_WAV,
	EXPLODE_WAV,
	NUM_WAVES
};
// ��Ч����
Mix_Chunk *gSounds[NUM_WAVES] = {NULL};

// �û�����
Uint16 gPlayerInput = 0;

// ����ͼ���ļ�
SDL_Surface * loadImage(const char *fname, bool transparent)
{
	SDL_Surface *image, *surface;
	
	image = IMG_Load(fname);
	if (image == NULL) 
	{
		printf("Couldn't load image %s: %s\n", fname, IMG_GetError());
		return NULL;
	}
	if (transparent) 
	{
		// �ٶ�����ͼ����32bpp, ����͸��ɫ�����Ͻ�����ָ��
		SDL_SetColorKey(image, (SDL_SRCCOLORKEY|SDL_RLEACCEL),
			*(Uint32 *)image->pixels);
	}
	// ��ͼ��ת��Ϊ��ʾ���ݸ�ʽ(���ڿ�����ʾ)
	surface = SDL_DisplayFormat(image);
	SDL_FreeSurface(image);
	return(surface);
}

// �ȴ�һ֡���
void waitForFrame()
{
	static Uint32 nextTick = 0;
	Uint32 thisTick;
	
	// ��ȡ��ǰʱ��
	thisTick = SDL_GetTicks(); 
	// �����ǰʱ��С����һ֡ʱ��ȴ�
	if (thisTick < nextTick) 
	{
		SDL_Delay(nextTick-thisTick);
	}
	// ������һ֡ʱ��
	nextTick = thisTick + (1000/FRAMES_PER_SEC);
}

// ������Ϸ��Դ
bool loadResourse()
{
	gBkImage = loadImage("res\\background.bmp", false);
	if (!gBkImage) return false;

	gPlayerImage = loadImage("res\\player.bmp", true);
	if (!gPlayerImage) return false;

	gBulletImage = loadImage("res\\bullet.bmp", false);
	if (!gBulletImage) return false;

	gAlienImage = loadImage("res\\alien.bmp", true);
	if (!gAlienImage) return false;

	gExplodeImage = loadImage("res\\explode.bmp", true);
	if (!gExplodeImage) return false;

	gSounds[MUSIC_WAV] = Mix_LoadWAV("res\\music.wav");
	gSounds[SHOT_WAV] = Mix_LoadWAV("res\\shot.wav");
	gSounds[EXPLODE_WAV] = Mix_LoadWAV("res\\explode.wav");

	return true;
}

// �ͷ���Ϸ��Դ
void releaseResourse()
{
	int i;
	SDL_FreeSurface(gBkImage);
	SDL_FreeSurface(gPlayerImage);
	SDL_FreeSurface(gBulletImage);
	SDL_FreeSurface(gAlienImage);
	SDL_FreeSurface(gExplodeImage);

	for(i=0; i<NUM_WAVES; ++i) 
	{
		Mix_FreeChunk(gSounds[i]);
	}
}

// ��ʼ����Ϸ
void initGame()
{
	int i;
	// ��ʼ���������
	srand( (unsigned) time(NULL));

	// ��ʼ�����
	gPlayer.alive = 1;
	gPlayer.image = gPlayerImage;
	gPlayer.rect.w = gPlayerImage->w;
	gPlayer.rect.h = gPlayerImage->h;
	gPlayer.rect.x = (gScreen->w - gPlayer.rect.w) / 2;
	gPlayer.rect.y = gScreen->h - gPlayer.rect.h;

	// ��ʼ���ӵ�
	for(i=0; i<MAX_BULLETS; i++)
	{
		gBullets[i].alive = 0;
		gBullets[i].image = gBulletImage;
		gBullets[i].rect.x = gBullets[i].rect.y = 0;
		gBullets[i].rect.w = gBulletImage->w;
		gBullets[i].rect.h = gBulletImage->h;
	}

	// ��ʼ��������
	for(i=0; i<MAX_ALIENS; i++)
	{
		gAliens[i].alive = 0;
		gAliens[i].image = gAlienImage;
		gAliens[i].rect.x = gAliens[i].rect.y = 0;
		gAliens[i].rect.w = gAlienImage->w;
		gAliens[i].rect.h = gAlienImage->h;
	}

	// ��ʼ����ը
	for(i=0; i<MAX_ALIENS; i++)
	{
		gExplodes[i].alive = 0;
		gExplodes[i].image = gExplodeImage;
		gExplodes[i].rect.x = gExplodes[i].rect.y = 0;
		gExplodes[i].rect.w = gExplodeImage->w;
		gExplodes[i].rect.h = gExplodeImage->h;
	}
}

// ������Ϸ����
void drawGame()
{
	Uint32 timeTick;
	static Uint32 showTick = 0;
	timeTick = SDL_GetTicks();
	
	int i;
	SDL_Rect r;
	r.x = 0; r.y = 0;
	r.w = gBkImage->w;
	r.h = gBkImage->h;

	while(r.x < gScreen->w) {

	// ���´���
		SDL_BlitSurface(gBkImage, NULL, gScreen, &r);
		
		r.x += r.w;
	}
	if(gPlayer.alive) 
	{
		SDL_BlitSurface(gPlayer.image, NULL, gScreen, &gPlayer.rect);
	}
	
	for( i =0; i<MAX_ALIENS; i++) 
	{
			if(gAliens[i].alive)
			SDL_BlitSurface(gAliens[i].image, NULL, gScreen, &gAliens[i].rect);
	}
	
		//�����ӵ�
	for(i=0; i<MAX_BULLETS; i++) {
		if(gBullets[i].alive) {
			SDL_BlitSurface(gBullets[i].image, NULL, gScreen, &gBullets[i].rect);
		}
	}

	for( i=0; i<MAX_BULLETS; i++) {
		if(!gBullets[i].alive) continue;
			gBullets[i].rect.y -= SHOT_SPEED;
			if(gBullets[i].rect.y +gBullets[i].rect.h <0) {
				gBullets[i].alive = 0;
			}
	}
	//��ʼ����ը
	for( i=0; i<MAX_ALIENS; i++) 
	{
			if(gExplodes[i].alive)
			{
				SDL_BlitSurface(gExplodes[i].image, NULL, gScreen,&gExplodes[i].rect);
				if(timeTick - showTick > EXPLODE_TIME)
				{
					gExplodes[i].alive = 0;
					showTick = timeTick;
				}
				
			}
	}
	SDL_UpdateRect(gScreen, 0, 0, gScreen->w, gScreen->h);
}

// ��ײ���
bool collide(Sprite *sprite1, Sprite *sprite2)
{
	if ((sprite1->rect.y >= (sprite2->rect.y+sprite2->rect.h)) ||
		(sprite1->rect.x >= (sprite2->rect.x+sprite2->rect.w)) ||
		(sprite2->rect.y >= (sprite1->rect.y+sprite1->rect.h)) ||
		(sprite2->rect.x >= (sprite1->rect.x+sprite1->rect.w)) ) 
	{
		return false;
	}
	else return true;
}

// ������Ϸ
void processGame()
{
	int i,j;
	Uint32 tick;
	static Uint32 fireTick  = 0;
	static Uint32 alienTick = 0;

	//���ַ���
	//srand(time(NULL));
	int dir = rand()%2;

	//��ȡ��ǰʱ��
	tick = SDL_GetTicks();

	if(gPlayerInput & KEY_LEFT) gPlayer.direct= -1;
	else if(gPlayerInput & KEY_RIGHT) gPlayer.direct = 1;
	else gPlayer.direct = 0;
		gPlayer.rect.x +=gPlayer.direct * PLAYER_SPEED;
	if(gPlayer.rect.x <0) gPlayer.rect.x = 0;
	else if(gPlayer.rect.x +gPlayer.rect.w >= gScreen->w) {
		gPlayer.rect.x = gScreen->w-gPlayer.rect.w-1;
	}
	//�������ӵ�

	if(gPlayerInput & KEY_FIRE && tick-fireTick>MIN_SHOT_TIME) 
	{
		for(i=0; i<MAX_BULLETS; i++) 
		{
			if(gBullets[i].alive) continue;
				gBullets[i].alive = 1;
				
				gBullets[i].rect.x = gPlayer.rect.x +gPlayer.rect.w/2 -gBullets[i].rect.w/2;
				gBullets[i].rect.y = gPlayer.rect.y - gBullets[i].rect.h;
				SDL_BlitSurface(gExplodes[0].image, NULL, gScreen, &gExplodes[0].rect);
				
				fireTick = tick;
				Mix_PlayChannel(SHOT_WAV, gSounds[SHOT_WAV], 0);
			break;
		}
		/*for(i=0; i<MAX_BULLETS; i++) {
		if(!gBullets[i].alive) continue;
			gBullets[i].rect.y -= SHOT_SPEED;
			if(gBullets[i].rect.y +gBullets[i].rect.h <0) {
				gBullets[i].alive = 0;
			}*/
	}
	if(tick - alienTick > MIN_ALIEN_TIME)
		{
		for(i=0;i<MAX_ALIENS;i++)
			{
				if(gAliens[i].alive) continue;
					gAliens[i].alive = 1;
				if(dir==1) {
					gAliens[i].direct = dir;
					gAliens[i].rect.x = 0;
					gAliens[i].rect.y = 0;
				}
				else
				{
					gAliens[i].direct = -1;
					gAliens[i].rect.x = gScreen->w - gAliens[i].rect.w ;
					gAliens[i].rect.y = 0;
				}
			
				alienTick = tick;
				
				break;
			}
		}
	for(i=0; i<MAX_ALIENS; i++)
	{
			if(!gAliens[i].alive) continue;

			gAliens[i].rect.x +=gAliens[i].direct * ALIEN_SPEED;
			if(gAliens[i].rect.x  <= 0)
			{
				gAliens[i].rect.y += gAliens[i].rect.h;
				gAliens[i].direct = 1;
			}
			if(gAliens[i].rect.x + gAliens[i].rect.w >=gScreen->w)
			{
				gAliens[i].rect.x = gScreen->w-gAliens[i].rect.w;
				gAliens[i].rect.y += gAliens[i].rect.h;
				gAliens[i].direct = -1;
			}
	}
//�ж���ײ
	for(i=0; i<MAX_BULLETS; i++)
	{
		for(j=0; j<MAX_ALIENS; j++)
		{
				if(collide(&gBullets[i], &gAliens[j]))
				{
						
						gBullets[i].alive = 0;
						gAliens[j].alive = 0;
						gExplodes[j].rect.x = gAliens[j].rect.x;
						gExplodes[j].rect.y = gAliens[j].rect.y;
						//gExplodes[j].alive = 1;
						//Mix_PlayChannel(SHOT_WAV,gSounds[EXPLODE_WAV],0);
					//SDL_BlitSurface(gExplodes[j].image, NULL, gScreen,&gExplodes[j].rect);

				}
			else if(collide(&gPlayer, &gAliens[j]))
			{	
				gAliens[j].alive = 0;
				gExplodes[j].alive = 1;
				gExplodes[j].rect.y = gAliens[j].rect.y;
				gExplodes[j].rect.x = gAliens[j].rect.x;
				Mix_PlayChannel(SHOT_WAV,gSounds[EXPLODE_WAV],0);
				Sleep(300);	
				gPlayer.alive = 0;
			}
		}
	}
		
			/*for(j=0; j<MAX_ALIENS; j++)
			{
				if(SDL_GetTicks()-tick  > MIN_ALIEN_TIME || gExplodes[j].alive)
				{
					gExplodes[j].alive = 0;
					showTick = tick;
				}
			}*/
		
}

int main(int argc, char *argv[])
{
	// ��ʼ��SDL��
	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) 
	{
		printf("��ʼ��SDLʧ��: %s\n",SDL_GetError());
		exit(2);
	}
	atexit(SDL_Quit);

	// ������ʾ���
	gScreen = SDL_SetVideoMode(640, 480, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if ( gScreen == NULL ) 
	{
		printf("���ܹ�������ʾģʽ640x480: %s\n",
			SDL_GetError());
		exit(2);
	}

	if (Mix_OpenAudio(11025, AUDIO_U8, 1, 512) < 0) 
	{
		printf("����: ���ܹ�����11025 Hz 8-bit��Ƶ\n- ԭ��: %s\n",
			SDL_GetError());
	}

	// ����SDL���ڱ���
    SDL_WM_SetCaption ("Aliens", NULL);

	// ������Ϸ��Դ
	if (!loadResourse())
	{
		printf("������Ϸ��Դʧ��!\n");
		exit(3);
	}
	
	// ��ʼ����Ϸ
	initGame();
	
    //��Ϸѭ�� done=1ʱ�˳���done=0ʱ����
	int done = 0;
    while (!done)
    {
		// �ȴ�һ֡
		waitForFrame();
		
		// ������Ϸ
		processGame();

		// ������Ϸ
		drawGame();

		// ���ű�������
		if (!Mix_Playing(MUSIC_WAV)) 
		{
			Mix_PlayChannel(MUSIC_WAV, gSounds[MUSIC_WAV], 0);
		}

		// �����¼�
		SDL_Event event;	
        while(SDL_PollEvent(&event))	
        {
			if (event.type == SDL_KEYDOWN)
			{
				switch(event.key.keysym.sym)
				{
				case PLAYER_LEFT_KEY:	gPlayerInput |= KEY_LEFT;	break;
				case PLAYER_RIGHT_KEY:	gPlayerInput |= KEY_RIGHT;	break;
				case PLAYER_FIRE_KEY:	gPlayerInput |= KEY_FIRE;	break;
				}
			}
			else if(event.type == SDL_KEYUP)
			{
				switch(event.key.keysym.sym)
				{
				case PLAYER_LEFT_KEY:	gPlayerInput &= ~KEY_LEFT;	break;
				case PLAYER_RIGHT_KEY:	gPlayerInput &= ~KEY_RIGHT;	break;
				case PLAYER_FIRE_KEY:	gPlayerInput &= ~KEY_FIRE;	break;
				}
			}
            else if (event.type == SDL_QUIT)
			{
				done = 1;
                break;	
            }
        }

		if (!gPlayer.alive) done = 1;
    }
	
    Mix_CloseAudio();

	return 0;
}