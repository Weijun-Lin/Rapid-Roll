#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
#include <cstring>
#include <cstdio>
#include <conio.h>

using namespace std;
#define RAD(x) (M_PI)/180*(x)

using namespace std;

const int WIDTH = 300;
const int HEIGHT = 600;
const int FPS = 60;			// 帧率
const int scoreBits = 6;	// 分数 最大6位

SDL_Window *window;			// 窗口
SDL_Renderer *render;		// 渲染器
TTF_Font *font;				// 字体

bool init();

// 类声明
class BackGround;			// 背景图 以及游戏的一些环境参数
class Ball;					// 球类
class Board;				// 板/障碍物
class BoardManager;			// 板管理器
class Game;					// 游戏流程总控 （开始动画，主循环，结束动画）以及音效，图片资源的加载
class Life;					// 获取额外生命系统
class BoardThorn;			// 含刺的障碍物

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

Mix_Chunk* Game::takeOff = NULL;
Mix_Chunk* Game::bump = NULL;
Mix_Chunk* Game::gameOver = NULL;
bool Game::isBump = false;

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

// 类外初始化
int BackGround::boardHeight = 8;
int BackGround::life = 3;
int BackGround::score = 0;
bool BackGround::isDie = false;
const float BackGround::ballSpeedH = 0.3;
const float BackGround::ballSpeedV = 0.2;
float BackGround::boardSpeed = -0.2;
const char* BackGround::windowName = "Rapid Roll";
const char* BackGround::fontPath = "./src/TTF/font.ttf";
const char* BackGround::bumpPath = "./src/Audio/bump.mp3";
const char* BackGround::takeOffPath = "./src/Audio/takeOff.mp3";
const char* BackGround::bgmPath = "./src/Audio/bgm.mp3";
const char* BackGround::gameOverPath = "./src/Audio/gameOver.mp3";
const char* BackGround::catImgPath = "./src/Image/catBlack.bmp";

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

class BoardThorn :public Board {
public:
	BoardThorn(SDL_Point pos_, int width_, int height_, float speedV_ = 0, SDL_Color color_ = { 0,0,0,0xff });
	void draw();
};

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

int main(int argc, char* args[])
{
	if (!init()) return -1;
	Game game;
	bool isLegal = game.welcome();	// 判断是否正常结束流程 如： 中途直接关闭窗口
	if (!isLegal) {
		game.quit();
		return -1;
	}
	game.run();
	game.quit();
	return 0;
}

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

// --------------------- Ball ---------------------------------
Ball::Ball(int size_, SDL_Point pos_, float speedV_, float speedH_) :
	size(size_), pos(pos_), speedV(speedV_), speedH(speedH_)
{}

bool Ball::isBump(Board &board_)
{
	if ((pos.y + size / 2 >= board_.pos.y - board_.height / 2 && pos.y + size / 2 <= board_.pos.y + board_.height / 2) &&
		(pos.x <= board_.pos.x + board_.width / 2 + size / 2 && pos.x >= board_.pos.x - board_.width / 2 - size / 2)) {
		return true;
	}
	return false;
}

void Ball::move(BoardManager & board_, int type, bool isMusicOn)
{
	if (type == 1) pos.x -= speedH * 1000 / FPS;
	else if (type == 2) pos.x += speedH * 1000 / FPS;
	for (int i = 0; i < board_.boards.size(); i++) {
		if (isBump(board_.boards[i])) {
			if (!Game::isBump) {		// 第一次碰撞之后 播放音效
				if (isMusicOn) Mix_PlayChannel(-1, Game::takeOff, 0);
				Game::isBump = true;
			}
			pos.y = board_.boards[i].pos.y - board_.boards[i].height / 2 - size / 2;
			return;
		}
	}
	if (isBump(board_.boardThorn)) {
		if (isMusicOn) Mix_PlayChannel(-1, Game::bump, 0);
		pos.y = (board_.boards.end() - 1)->pos.y - BackGround::boardHeight - size;
		pos.x = (board_.boards.end() - 1)->pos.x;
		BackGround::life--;
		if (isMusicOn) Mix_PlayChannel(-1, Game::bump, 0);
	}
	Game::isBump = false;
	if (pos.x - size / 2 <= 0) pos.x = size / 2;
	else if (pos.x + size / 2 >= WIDTH) pos.x = WIDTH - size / 2;
	pos.y += speedV * 1000 / FPS;
	BackGround::score += speedV * 1000 / FPS;
	if (pos.y >= HEIGHT - BackGround::up || pos.y <= BackGround::up + BackGround::tranigleSize) {
		pos.y = (board_.boards.end() - 1)->pos.y - BackGround::boardHeight - size;
		pos.x = (board_.boards.end() - 1)->pos.x;
		BackGround::life--;
		if (isMusicOn) Mix_PlayChannel(-1, Game::bump, 0);
	}
}

void Ball::draw(SDL_Color color) {
	SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
	int posX = pos.x;
	int posY = pos.y;
	SDL_Rect rects[3];
	rects[0] = { posX - size / 2,posY - size / 6,size,size / 3 };
	rects[1] = { posX - size / 6,posY - size / 2,size / 3,size };
	rects[2] = { posX - size / 3,posY - size / 3,2 * size / 3,2 * size / 3 };
	SDL_RenderFillRects(render, rects, 3);
}

// ---------------------- BackGound -----------------------------
BackGround::BackGround(SDL_Color bkColor_) :
	bkColor(bkColor_), scoreTextPos({ WIDTH - scoreBits * 15 - 10,0,scoreBits * 15,15 })
{}

void BackGround::drawTranigle(int st_, int edgePos_, bool flag_)
{
	int detY = (flag_ == true) ? 1 : -1;
	int st = st_, ed = st + tranigleSize;
	while (st <= ed) {
		SDL_RenderDrawLine(render, st, edgePos_, ed, edgePos_);
		SDL_RenderDrawLine(render, st, edgePos_ + 1, ed, edgePos_ + 1);
		edgePos_ += detY * 2;
		st++, ed--;
	}
}

void BackGround::drawFence()
{
	int st = 0;
	while (st < WIDTH) {
		drawTranigle(st, up, true);
		drawTranigle(st, down, false);
		st += tranigleSize;
	}
}

void BackGround::draw()
{
	SDL_SetRenderDrawColor(render, bkColor.r, bkColor.g, bkColor.b, bkColor.a);
	SDL_RenderClear(render);
	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	drawFence();
	// 画生命
	for (int i = 0; i < life; i++) {
		SDL_Point pos = { i,i };
		Ball ball(4 * up / 5, { i*up + up / 2,up / 2 });
		ball.draw();
	}
	// 画分数
	char scoreStr[10] = { 0 };
	sprintf(scoreStr, "%06d", score);
	SDL_Texture *tex = SDL_CreateTextureFromSurface(render, TTF_RenderText_Blended(font, scoreStr, { 0,0,0,0xff }));
	SDL_RenderCopy(render, tex, NULL, &scoreTextPos);
	SDL_DestroyTexture(tex);
}

// ------------------------------------- Board ------------------------------------
Board::Board(SDL_Point pos_, int width_, int height_, float speedV_, SDL_Color color_) :
	pos(pos_), speedV(speedV_), width(width_), height(height_), color(color_)
{}

void Board::move()
{
	pos.y += speedV * 1000 / FPS;
}

void Board::draw()
{
	SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(render, &SDL_Rect({ pos.x - width / 2,pos.y - height / 2,width,height }));
}

// ----------------------------- BoardManager ----------------------------------
BoardManager::BoardManager() :
	boardThorn({ -20,-20 }, BackGround::boardHeight, BackGround::boardHeight), isThornExist(false),GAP(100)
{}

bool BoardManager::isOut(Board & board_)
{
	if (board_.pos.y + board_.height / 2 <= BackGround::up) return true;
	return false;
}

void BoardManager::clearInlegal()
{
	for (int i = boards.size() - 1; i >= 0; i--) {
		if (isOut(boards[i])) boards.erase(boards.begin() + i);
	}
	if (isOut(boardThorn)) {
		boardThorn.pos = { -20,-20 };
		isThornExist = false;
	}
}


void BoardManager::move()
{
	for (int i = 0; i < boards.size(); i++) {
		boards[i].move();
	}
	boardThorn.move();
}

void BoardManager::draw()
{
	for (int i = 0; i < boards.size(); i++) {
		boards[i].draw();
	}
	boardThorn.draw();
}

void BoardManager::creatABoard(int * last, int now)
{
	clearInlegal();
	int gap = now - *last;
	if (gap >= GAP - 1000 / FPS * 2 && gap <= GAP + 1000 / FPS * 2) {
		srand(now);
		int st = rand() % WIDTH;
		if (isThornExist) boards.push_back(Board({ st,HEIGHT }, BackGround::boardWidth, BackGround::boardHeight, BackGround::boardSpeed));
		else {
			boardThorn = BoardThorn({ st,HEIGHT }, BackGround::boardWidth, BackGround::boardHeight, BackGround::boardSpeed);
			isThornExist = true;
		}
		GAP = rand() % 300 + 250;
		*last = now;
	}
}

// ------------------------------------------------ Game ----------------------------------
Game::Game()
{
	bgm = Mix_LoadMUS(BackGround::bgmPath);
	int flag = Mix_PlayMusic(bgm, -1);
	takeOff = Mix_LoadWAV(BackGround::takeOffPath);
	bump = Mix_LoadWAV(BackGround::bumpPath);
	// 绘画Title
	title = SDL_CreateTextureFromSurface(render, TTF_RenderText_Blended(font, BackGround::windowName, { 0,0,0,0xff }));
	startGame = SDL_CreateTextureFromSurface(render, TTF_RenderText_Blended(font, "Start Game", { 0,0,0,0xff }));
	authorInfo = SDL_CreateTextureFromSurface(render, TTF_RenderText_Blended(font, "Produced By Joke-Lin", { 0,0,0,0xff }));
	rePlay = SDL_CreateTextureFromSurface(render, TTF_RenderText_Blended(font, "RePlay", { 0,0,0,0xff }));
	gameOverTitle = SDL_CreateTextureFromSurface(render, TTF_RenderText_Blended(font, "Game Over", { 0,0,0,0xff }));
	score = NULL;
}

void Game::drawStartGame(SDL_Color color)
{
	SDL_SetRenderDrawColor(render, 0, 0, 0, 0xff);
	SDL_RenderFillRect(render, &SDL_Rect({ 50,360,200,100 }));
	SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(render, &SDL_Rect({ 60,370, 180, 80 }));
	SDL_RenderCopy(render, startGame, NULL, &SDL_Rect({ 80,390, 140, 40 }));
}

void Game::drawGameOver(SDL_Color color)
{
	SDL_SetRenderDrawColor(render, 0, 0, 0, 0xff);
	SDL_RenderFillRect(render, &SDL_Rect({ 50,360,200,100 }));
	SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(render, &SDL_Rect({ 60,370, 180, 80 }));
	SDL_RenderCopy(render, rePlay, NULL, &SDL_Rect({ 80,390, 140, 40 }));
}

bool Game::isInStart(int x, int y)
{
	if (x >= 60 && x <= 60 + 180 && y >= 370 && y <= 370 + 80) return true;
	return false;
}


bool Game::welcome()
{
	Ball ball(100, { 150,200 }); // 欢迎界面的球
	Board board({ 150,280 }, 200, 10); // 欢迎界面的横线
	bool isquit = false;
	bool isMouseHit = false;
	int mouseX, mouseY;
	while (!isquit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) return false;
			else if (event.type == SDL_MOUSEBUTTONUP) isMouseHit = true;
			else isMouseHit = false;
		}
		SDL_GetMouseState(&mouseX, &mouseY);
		SDL_SetRenderDrawColor(render, 0xff, 0xff, 0xff, 0xff);
		SDL_RenderClear(render);
		SDL_RenderCopy(render, title, NULL, &SDL_Rect({ 0, 10, 300, 100 }));
		if (isInStart(mouseX, mouseY)) {
			drawStartGame({ 125,125,125,0xff });
			if (isMouseHit) isquit = true; // 按键退出
		}
		else drawStartGame({ 0xff,0xff,0xff,0xff });

		SDL_RenderCopy(render, authorInfo, NULL, &SDL_Rect({ 75, 500, 150, 30 }));
		ball.draw();
		board.draw();
		SDL_RenderPresent(render);
	}
	return true;
}

void Game::run()
{
	bool isquit = false;
	BackGround background({ 0xff,0xff,0xff,0xff }); // 背景绘制
	Ball ball(BackGround::ballSize, { 100,100 }, BackGround::ballSpeedV, BackGround::ballSpeedH); // 球
	BoardManager boards; // 阶梯
	Life lifeControl;

	int last = SDL_GetTicks();	// 上一次的时间
	int cmd = 0;				// 按键命令
	bool isStart = false;		// 判断是否游戏开始 -- 进入时要暂停一下 也可实现暂停功能
	int scores[] = { 1000,2000,3000,4000,5000,6000,7000,8000,9000 };
	int level = 0;

	while (!isquit) {
		if (BackGround::life == -1)
		{
			if (GameOver()) {
				BackGround::score = 0;
				BackGround::life = 3;
				BackGround::boardSpeed = -0.2;
				level = 0;
				isStart = false;
				boards = BoardManager();
				last = SDL_GetTicks();
				ball = Ball(BackGround::ballSize, { 100,100 }, BackGround::ballSpeedV, BackGround::ballSpeedH);
				lifeControl.reSet();
			}
			else return;
		}
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) isquit = true;
		}
		// 获取键盘状态
		const Uint8* keyStates = SDL_GetKeyboardState(NULL);
		if (keyStates[SDL_SCANCODE_A]) cmd = 1, isStart = true;
		else if (keyStates[SDL_SCANCODE_D]) cmd = 2, isStart = true;
		else if (keyStates[SDL_SCANCODE_P]) isStart = false, cmd = 0;
		else cmd = 0;
		// 重画每一帧
		if (level < 9 && BackGround::score > scores[level]) {
			if (BackGround::boardSpeed < -0.35) BackGround::boardHeight = 12;
			BackGround::boardSpeed -= 0.04;
			level++;
		}
		background.draw();
		if (isStart) {
			boards.move();
			ball.move(boards, cmd);
			boards.creatABoard(&last, SDL_GetTicks());
			lifeControl.creatALife(SDL_GetTicks(), boards);
			if (lifeControl.isEatALife(ball)) {
				if (BackGround::life < BackGround::maxLife) BackGround::life++;
			}
		}
		else last = SDL_GetTicks();
		boards.draw();
		ball.draw();
		SDL_RenderPresent(render);
	}
}

void Game::quit()
{
	SDL_DestroyTexture(title);
	SDL_DestroyTexture(startGame);
	SDL_DestroyTexture(authorInfo);
	if(score != NULL) SDL_DestroyTexture(score);
	SDL_DestroyTexture(rePlay);
	SDL_DestroyTexture(gameOverTitle);
	Mix_FreeChunk(bump);
	Mix_FreeChunk(takeOff);
	Mix_FreeChunk(gameOver);
	Mix_FreeMusic(bgm);

	Mix_Quit();
	SDL_DestroyRenderer(render);
	SDL_Quit();
}

bool Game::GameOver()	// 差不多就是把开始界面换一个
{
	// 分数纹理
	char scoreStr[10] = { 0 };
	sprintf(scoreStr, "%06d", BackGround::score);
	score = SDL_CreateTextureFromSurface(render, TTF_RenderText_Blended(font, scoreStr, { 0,0,0,0xff }));
	Ball ball(100, { 150,200 }); // 欢迎界面的球
	Board board({ 150,280 }, 200, 10); // 欢迎界面的横线
	bool isquit = false;
	bool isMouseHit = false;
	int mouseX, mouseY;
	while (!isquit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) return false;
			else if (event.type == SDL_MOUSEBUTTONUP) isMouseHit = true;
			else isMouseHit = false;
		}
		SDL_GetMouseState(&mouseX, &mouseY);
		SDL_SetRenderDrawColor(render, 0xff, 0xff, 0xff, 0xff);
		SDL_RenderClear(render);
		SDL_RenderCopy(render, gameOverTitle, NULL, &SDL_Rect({ 0, 10, 300, 100 }));
		if (isInStart(mouseX, mouseY)) {
			drawGameOver({ 125,125,125,0xff });
			if (isMouseHit) isquit = true; // 按键退出
		}
		else drawGameOver({ 0xff,0xff,0xff,0xff });

		SDL_RenderCopy(render, score, NULL, &SDL_Rect({ 75, 290, 150, 70 }));
		SDL_RenderCopy(render, authorInfo, NULL, &SDL_Rect({ 75, 500, 150, 30 }));
		ball.draw();
		board.draw();
		SDL_RenderPresent(render);
	}
	return true;
}

// -------------------------- Life --------------------------------------------
Life::Life() :
	isCreat(false), ball(lifeSize, { -20,-20 })
{}

void Life::reSet()
{
	isCreat = false;
	ball.pos = { -20,-20 };
}

void Life::creatALife(int clocks, BoardManager boards)
{
	if (isCreat) {
		if (ball.pos.y < BackGround::up) isCreat = false;
		ball.move(boards, 0, false); // 此时碰撞不播放音乐
		ball.draw(lifeColor);
		return;
	}
	bool isLegal = false;
	for (int i = 15000; i < 15050; i++) {
		if (clocks%i == 0) {
			isLegal = true;
			break;
		}
	}
	if (isLegal) {
		ball = Ball(lifeSize, { boards.boards[boards.boards.size() - 1].pos.x,boards.boards[boards.boards.size() - 1].pos.y - lifeSize });
		isCreat = true;
	}
}

bool Life::isEatALife(Ball ball_)
{
	if (!isCreat) return false;
	if (ball_.pos.y <= ball.pos.y + lifeSize && ball_.pos.y >= ball.pos.y - lifeSize && ball_.pos.x <= ball.pos.x + lifeSize && ball_.pos.x >= ball.pos.x - lifeSize) {

		isCreat = false;
		ball.pos = { -20,-20 };
		Mix_PlayChannel(-1, Game::takeOff, 0);
		return true;
	}
	return false;
}

// -------------------------- BoardThorn --------------------------------------------

BoardThorn::BoardThorn(SDL_Point pos_, int width_, int height_, float speedV_, SDL_Color color_) :
	Board(pos_, width_, height_, speedV_, color_)
{}

void BoardThorn::draw()
{
	SDL_SetRenderDrawColor(render, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(render, &SDL_Rect({ pos.x - width / 2,pos.y - height / 2,width,height }));
	int beg = pos.x - width / 2;
	int edgePos_ = pos.y - height / 2;
	for (int i = beg; i < beg + width; i += BackGround::tranigleSize) {
		int st = i, ed = i + BackGround::tranigleSize;
		while (st <= ed) {
			SDL_RenderDrawLine(render, st, edgePos_, ed, edgePos_);
			SDL_RenderDrawLine(render, st, edgePos_ + 1, ed, edgePos_ + 1);
			edgePos_ -= 1;
			st++, ed--;
		}
		edgePos_ = pos.y - height / 2;
	}
}
