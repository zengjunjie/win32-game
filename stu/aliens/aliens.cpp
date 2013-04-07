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

// 预定帧速(50帧/秒)
#define	FRAMES_PER_SEC	50
// 玩家移动速度
#define PLAYER_SPEED	4
// 同时存在的最大子弹数目
#define MAX_BULLETS	3
// 子弹射击速度
#define SHOT_SPEED	6
// 射击间隔
#define MIN_SHOT_TIME	200
// 最大入侵者数量
#define MAX_ALIENS	30
// 入侵者移动速度
#define ALIEN_SPEED	5
// 入侵者产生间隔
#define MIN_ALIEN_TIME	1000
// 爆炸持续时间
#define EXPLODE_TIME	100

// 精灵结构体
struct Sprite
{
	Uint32			alive;		// 是否存活
	int				direct;		// 移动方向
	SDL_Rect		rect;		// 精灵在地图中的包围矩形
	SDL_Surface *	image;		// 图像
};

// 窗口表面
SDL_Surface *gScreen = NULL;
// 背景图像
SDL_Surface *gBkImage = NULL;
// 玩家图像
SDL_Surface *gPlayerImage = NULL;
// 子弹图像
SDL_Surface *gBulletImage = NULL;
// 入侵者图像
SDL_Surface *gAlienImage = NULL;
// 爆炸图像
SDL_Surface *gExplodeImage = NULL;

// 玩家精灵
Sprite gPlayer;
// 子弹精灵数组
Sprite gBullets[MAX_BULLETS];
// 入侵者精灵数组
Sprite gAliens[MAX_ALIENS];
// 爆炸精灵
Sprite gExplodes[MAX_ALIENS];

// 音效枚举
enum {
	MUSIC_WAV,
	SHOT_WAV,
	EXPLODE_WAV,
	NUM_WAVES
};
// 音效数组
Mix_Chunk *gSounds[NUM_WAVES] = {NULL};

// 用户输入
Uint16 gPlayerInput = 0;

// 载入图像文件
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
		// 假定载入图像是32bpp, 而且透明色由左上角像素指定
		SDL_SetColorKey(image, (SDL_SRCCOLORKEY|SDL_RLEACCEL),
			*(Uint32 *)image->pixels);
	}
	// 将图像转换为显示兼容格式(便于快速显示)
	surface = SDL_DisplayFormat(image);
	SDL_FreeSurface(image);
	return(surface);
}

// 等待一帧间隔
void waitForFrame()
{
	static Uint32 nextTick = 0;
	Uint32 thisTick;
	
	// 获取当前时间
	thisTick = SDL_GetTicks(); 
	// 如果当前时间小于下一帧时间等待
	if (thisTick < nextTick) 
	{
		SDL_Delay(nextTick-thisTick);
	}
	// 设置下一帧时刻
	nextTick = thisTick + (1000/FRAMES_PER_SEC);
}

// 载入游戏资源
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

// 释放游戏资源
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

// 初始化游戏
void initGame()
{
	int i;
	// 初始化随机种子
	srand( (unsigned) time(NULL));

	// 初始化玩家
	gPlayer.alive = 1;
	gPlayer.image = gPlayerImage;
	gPlayer.rect.w = gPlayerImage->w;
	gPlayer.rect.h = gPlayerImage->h;
	gPlayer.rect.x = (gScreen->w - gPlayer.rect.w) / 2;
	gPlayer.rect.y = gScreen->h - gPlayer.rect.h;

	// 初始化子弹
	for(i=0; i<MAX_BULLETS; i++)
	{
		gBullets[i].alive = 0;
		gBullets[i].image = gBulletImage;
		gBullets[i].rect.x = gBullets[i].rect.y = 0;
		gBullets[i].rect.w = gBulletImage->w;
		gBullets[i].rect.h = gBulletImage->h;
	}

	// 初始化入侵者
	for(i=0; i<MAX_ALIENS; i++)
	{
		gAliens[i].alive = 0;
		gAliens[i].image = gAlienImage;
		gAliens[i].rect.x = gAliens[i].rect.y = 0;
		gAliens[i].rect.w = gAlienImage->w;
		gAliens[i].rect.h = gAlienImage->h;
	}

	// 初始化爆炸
	for(i=0; i<MAX_ALIENS; i++)
	{
		gExplodes[i].alive = 0;
		gExplodes[i].image = gExplodeImage;
		gExplodes[i].rect.x = gExplodes[i].rect.y = 0;
		gExplodes[i].rect.w = gExplodeImage->w;
		gExplodes[i].rect.h = gExplodeImage->h;
	}
}

// 绘制游戏场景
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

	// 更新窗口
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
	
		//绘制子弹
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
	//初始化爆炸
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

// 碰撞检测
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

// 处理游戏
void processGame()
{
	int i,j;
	Uint32 tick;
	static Uint32 fireTick  = 0;
	static Uint32 alienTick = 0;

	//出现方向
	//srand(time(NULL));
	int dir = rand()%2;

	//获取当前时间
	tick = SDL_GetTicks();

	if(gPlayerInput & KEY_LEFT) gPlayer.direct= -1;
	else if(gPlayerInput & KEY_RIGHT) gPlayer.direct = 1;
	else gPlayer.direct = 0;
		gPlayer.rect.x +=gPlayer.direct * PLAYER_SPEED;
	if(gPlayer.rect.x <0) gPlayer.rect.x = 0;
	else if(gPlayer.rect.x +gPlayer.rect.w >= gScreen->w) {
		gPlayer.rect.x = gScreen->w-gPlayer.rect.w-1;
	}
	//处理发射子弹

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
//判断碰撞
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
	// 初始化SDL库
	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) 
	{
		printf("初始化SDL失败: %s\n",SDL_GetError());
		exit(2);
	}
	atexit(SDL_Quit);

	// 设置显示输出
	gScreen = SDL_SetVideoMode(640, 480, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
	if ( gScreen == NULL ) 
	{
		printf("不能够设置显示模式640x480: %s\n",
			SDL_GetError());
		exit(2);
	}

	if (Mix_OpenAudio(11025, AUDIO_U8, 1, 512) < 0) 
	{
		printf("警告: 不能够设置11025 Hz 8-bit声频\n- 原因: %s\n",
			SDL_GetError());
	}

	// 设置SDL窗口标题
    SDL_WM_SetCaption ("Aliens", NULL);

	// 载入游戏资源
	if (!loadResourse())
	{
		printf("载入游戏资源失败!\n");
		exit(3);
	}
	
	// 初始化游戏
	initGame();
	
    //游戏循环 done=1时退出，done=0时继续
	int done = 0;
    while (!done)
    {
		// 等待一帧
		waitForFrame();
		
		// 处理游戏
		processGame();

		// 绘制游戏
		drawGame();

		// 播放背景音乐
		if (!Mix_Playing(MUSIC_WAV)) 
		{
			Mix_PlayChannel(MUSIC_WAV, gSounds[MUSIC_WAV], 0);
		}

		// 处理事件
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