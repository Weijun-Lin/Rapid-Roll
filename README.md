# Rapid-Roll
> A classical game in Nokia, which is implemented by SDL2 & C/C++

## 游戏介绍

> 一个经典的诺基亚小游戏 --------- 彩球滑梯

- 通过 A/D 控制小球左右移动，接触到尖刺失去一条生命；吃到红色小球增加一条生命；生命上限为5条
- 难度随分数的增加越来越大，分数达到10000难度封顶 （通过加大障碍物移动速度增加难度）
- 开始和结束界面均设置按钮，开始界面为开始游戏，结束界面为重新开始，不单独设置退出，直接点击关闭窗口即可退出
- P键即可暂停游戏，在按 A/D  继续游戏

**Demo:**

1、**开始界面**

![](/Demo/beg.png)

2、**运行界面**

![](/Demo/run.png)

3、**游戏结束界面**

![](/Demo/gameover.png)

## 文件目录

> 源代码只有一个main.cpp在Rapid-Roll工程目录下

├─EXE　　　　　　　　　　　　　　　　# 项目最后编译结果，exe所在目录  
│  └─src	　　　　　　　　　　　　　　　# 资源文件   
│      ├─Audio　　　　　　　　　　　　　# 音效文件  
│      └─TTF　　　　　　　　　　　　　　# 字体文件  
└─Rapid-Roll　　　　　　　　　　　　　# 工程目录 Visual Studio 结构  
    ├─Debug　　　　　　　　　　　　　　# Debug文件夹  
    │  └─src  
    │      ├─Audio  
    │      └─TTF  
    └─Rapid-Roll　　　　　　　　　　　　# 工程文件  
        ├─Debug  
        │  └─Rapid-Roll.tlog  
        ├─Release  
        │  └─Rapid-Roll.tlog  
            ├─Audio  
            └─TTF  

## 项目环境

> SDL2 + Visual Studio 2017 开发，代码编码格式为 GB2312

- 外部链接库：**SDL2.lib** &  **SDL2_ttf.lib** & **SDL2_mixer.lib** 
- 包含目录：SDL2相关库的include所在目录
- 库目录：SDL2相关库libs所在目录
- 常规：取消SDL检查
- 链接器 
  - 附加依赖项：
    - SDL2main.lib
    - SDL2.lib
    - SDL2_ttf.lib
    - SDL2_mixer.lib
  - 子系统：
    - 窗口 (/SUBSYSTEM:WINDOWS)
  - 高级：
    - 入口点：mainCRTStartup

## 代码解释

```#define SDL_MAIN_HANDLED```重定义main函数入口，SDL将main重定义，参考 ```SDL_main(...) ```

```c+
class BackGround;			// 背景图 以及游戏的一些环境参数
class Ball;					// 球类
class Board;				// 板/障碍物
class BoardManager;			// 板管理器：控制障碍物的出现以及运动
class Game;					// 游戏流程总控(开始动画，主循环，结束动画)以及音效，图片资源的加载
class Life;					// 获取额外生命系统
class BoardThorn;			// 含刺的障碍物
```

游戏未特别设置帧率,采用```SDL_RENDERER_PRESENTVSYNC```默认为 60 fps

```SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);```

### init(): 初始化环境

```c++
bool init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}
	if (TTF_Init() == -1) {
		cout << TTF_GetError() << endl;
		return false;
	}

	if (Mix_Init(MIX_INIT_MP3) != 0) {
		cout << Mix_GetError() << endl;
		return false;
	}

	window = SDL_CreateWindow(BackGround::windowName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	font = TTF_OpenFont(BackGround::fontPath, 100);
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);

	if (window == NULL || render == NULL || font == NULL) {
		cout << SDL_GetError() << endl;
		return false;
	}
	return true;
}
```

### Game类：游戏总控制，开始，运行，结束

```c++
class Game {
private:
	SDL_Texture *title;					// 标题纹理
	SDL_Texture *startGame;				// 开始按钮纹理
	SDL_Texture *authorInfo;			// 开发者信息纹理
	SDL_Texture *score;					// 分数纹理 --- 在GameOver中初始化
	SDL_Texture *rePlay;				// 重新开始纹理
	SDL_Texture *gameOverTitle;			// 结束标题
	void drawStartGame(SDL_Color color);// 绘制开始游戏按钮，鼠标在上改变颜色
	void drawGameOver(SDL_Color color); // 同上
	bool isInStart(int x, int y);		// 判断是否点击开始键
	bool GameOver();					// 和welcome差不多
public:
	Game();
	bool welcome(); // 欢迎界面
	void run();		// 游戏运行
	void quit();
	Mix_Music *bgm;						// 背景音乐
	static Mix_Chunk *takeOff;			// 球碰到障碍物音效
	static Mix_Chunk *bump;				// 球破裂（损失生命）音效
	static Mix_Chunk *gameOver;			// 游戏结束音效
	static bool isBump;					// 判断是否撞到
};
```

### BackGround类：运行时背景绘制，游戏运行环境参数

```c++
class BackGround {
private:
	SDL_Color bkColor;
	SDL_Rect scoreTextPos;
	void drawTranigle(int st_, int edgePos_, bool flag_);
	void drawFence();
public:
	const static int tranigleSize = 10;		// 障碍组成的小三角形的底边大小
	const static int up = 12;				// 上尖刺底边
	const static int down = HEIGHT;			// 下尖刺底边
	const static int ballSize = 15;			// 球大小
	const static int boardWidth = 80;		// 障碍物宽
	const static int maxLife = 5;			// 最多大生命值
	const static float ballSpeedV;			
	const static float ballSpeedH;
	const static char *windowName;
	const static char *fontPath;			// 以下，导入外部资源的路径
	const static char *bumpPath;
	const static char *takeOffPath;
	const static char *bgmPath;
	const static char *gameOverPath;
	const static char *catImgPath;

	static int boardHeight;
	static float boardSpeed;
	static int life;
	static int score;
	static bool isDie;
	BackGround(SDL_Color bkColor_);
	void draw();
};
```

### Ball类：移动，碰撞判断

```c++
// 由于SDL没有画圆函数所以由两个矩形和一个正方形模拟
class Ball {
private:
	int size;		// 球的大小
	SDL_Point pos;  // 位置
	float speedV;
	float speedH;

public:
	friend Life;
	Ball(int size_, SDL_Point pos_, float speedV_ = 0, float speedH_ = 0);
	bool isBump(Board &board_);		// 判断是否与板相撞
	void move(BoardManager &board_, int type, bool isMusicOn = true);
	void draw(SDL_Color color = { 0,0,0,0xff });
};
```

### Board类：障碍物

```c++
class Board {
private:
	float speedV;
	int width;
	int height;
	SDL_Point pos;	// 中心点位置
	SDL_Color color;// 颜色
public:
	friend Ball;
	friend BoardManager;
	friend Life;
	friend BoardThorn;
	Board(SDL_Point pos_, int width_, int height_, float speedV_ = 0, SDL_Color color_ = { 0,0,0,0xff });
	void move();
	virtual void draw();
};
```

### BoardThorn类：尖刺障碍物，继承与Board

```c++
class BoardThorn :public Board {
public:
	BoardThorn(SDL_Point pos_, int width_, int height_, float speedV_ = 0, SDL_Color color_ = { 0,0,0,0xff });
	void draw();
};
```

### BoardManager类：管理障碍物的出现及消失

```c++
class BoardManager {
private:
	vector<Board> boards;		// 管理所有板
	bool isOut(Board &board_);	// 判断是否出界
	void clearInlegal();		// 清除非法板（即出界）
	int GAP;					// 下一个板生成的时间间隔
	BoardThorn boardThorn;
	bool isThornExist;
public:
	friend Ball;
	friend Life;
	BoardManager();
	void move();
	void draw();
	void creatABoard(int* last, int now); // 随机产生一个板
};
```

### Life类：生命球

```c++
class Life {
	const int lifeSize = 10;
	const SDL_Color lifeColor = { 0xff,0,0,0xff };
	Ball ball;
	bool isCreat;
public:
	Life();
	void reSet();	// 配合重新开始游戏
	void creatALife(int clocks, BoardManager boards); // 生成一个生命，生命球移动也在此，每一次主循环均要调用
	bool isEatALife(Ball ball_);	// 判断是否吃到
};
```

