
#include "Game.h"
#include "jpfappdefines.h"
#include "jpf.h"
#include <assert.h>
#include "jpf_private.h"
#include <iostream>

const sf::Time Game::TimePerFrame = sf::seconds(1.f / 20.f);

struct JPFUSR {
	int n;
};

struct JPFUSR usrs[11] = {
	{0},
	{1},
	{2},
	{3},
	{4},
	{5},
	{6},
	{7},
	{8},
	{9},
	{10}
};

static struct Userinput {
	int users[11];
	int cur;
	int keys[NROF_KEYEVT][11];
	Userinput() : cur(-1) {};
} userinput;

static struct Camera camera[11];


Game::Game()
	: mWindow(sf::VideoMode(SCR_W, SCR_H), "Your Awesome Game!")
{
	//jpf_on_new_user(50);
}

void Game::Run()
{
	jpf_init();
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;

	while (mWindow.isOpen())
	{
		timeSinceLastUpdate += clock.restart();
		while (timeSinceLastUpdate > TimePerFrame)
		{
			timeSinceLastUpdate -= TimePerFrame;
			processEvents();
			update(TimePerFrame);
		}
		render();
		sf::Time delayTime = sf::milliseconds(1.0);
		sf::sleep(delayTime);
	}
}

void Game::processEvents()
{
	sf::Event event{};

	while (mWindow.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::KeyPressed:
			handlePlayerInput(event.key, true);
			break;
		case sf::Event::KeyReleased:
			handlePlayerInput(event.key, false);
			break;
		case sf::Event::Closed:
			mWindow.close();
			break;
		default:
			break;
		}

	}
}

void Game::update(sf::Time deltaTime)
{
	jpf_on_tick();
	struct Camera* c = &camera[userinput.cur];
	if (-1 != userinput.cur) {
		jpfpriv_tick_camera(c);
		//std::cout << userinput.cur << std::endl;
	}
}

void Game::render()
{
	mWindow.clear(sf::Color::White);
	jpf_on_draw((jpfhandle_t)&mWindow);


	mWindow.display();
}




void Game::handlePlayerInput(sf::Event::KeyEvent ke, bool isPressed)
{
	sf::Keyboard::Key key = ke.code;
	int n = -1;
	switch(key) {
	case sf::Keyboard::Num0:
		n = 10;
		break;
	case sf::Keyboard::Num1:
		n = 1;
		break;
	case sf::Keyboard::Num2:
		n = 2;
		break;
	case sf::Keyboard::Num3:
		n = 3;
		break;
	case sf::Keyboard::Num4:
		n = 4;
		break;
	case sf::Keyboard::Num5:
		n = 5;
		break;
	case sf::Keyboard::Num6:
		n = 6;
		break;
	case sf::Keyboard::Num7:
		n = 7;
		break;
	case sf::Keyboard::Num8:
		n = 8;
		break;
	case sf::Keyboard::Num9:
		n = 9;
		break;
	}
	if (n != -1) {
		userinput.cur = n;

		if (!ke.shift) {
			if (!userinput.users[n]) {
				jpf_on_new_user(&usrs[n]);
				userinput.users[n] = 1;
			}
		}
		else {
			if (userinput.users[n]) {
				jpf_on_remove_user(&usrs[n]);
				userinput.users[n] = 0;
				userinput.cur = -1;
			}
		}
	}

	if (-1 == userinput.cur)return;

	 if (key == sf::Keyboard::W)
	 {
		 userinput.keys[KEY_W][userinput.cur] = isPressed;
	 }
	 if (key == sf::Keyboard::A)
	 {
		 userinput.keys[KEY_A][userinput.cur] = isPressed;
	 }
	 if (key == sf::Keyboard::S)
	 {
		 userinput.keys[KEY_S][userinput.cur] = isPressed;
	 }
	 if (key == sf::Keyboard::D)
	 {
		 userinput.keys[KEY_D][userinput.cur] = isPressed;
	 }
	 if (key == sf::Keyboard::Space)
	 {
		 userinput.keys[KEY_SPACE][userinput.cur] = isPressed;
	 }
}

int is_key(jpfusr_t usr, enum keyevt key)
{
	return userinput.keys[key][usr->n];
}


struct spritelistelem
{
	// Cannot be allcoated static?
	sf::Texture texture;
	sf::Sprite sprite;
};

// Global sprite list
static struct {
	struct {
		char src[40];
		int inuse;		
	}arr[100];
	spritelistelem * spritelist[100];
	int cnt;
}sprites;

int jpf_create_sprite(char* src)
{
	assert(sprites.cnt < 100); // TODO...
	for (int i = 0; i < sprites.cnt; i++) {
		if (sprites.arr[i].inuse && !strcmp(src, sprites.arr[i].src)) {
			//sprites.arr[i].inuse = 1;
			return i;
		}
	}
	strcpy_s(sprites.arr[sprites.cnt].src, 40, src);
	sprites.arr[sprites.cnt].inuse = 1;
	sprites.spritelist[sprites.cnt] = new spritelistelem;
	if (!sprites.spritelist[sprites.cnt]->texture.loadFromFile(src))
	{
		printf("Error loading pic!\n");
		exit(1);
	}
	sprites.spritelist[sprites.cnt]->sprite.setTexture(sprites.spritelist[sprites.cnt]->texture);

	return sprites.cnt++;
}

void jpf_release_sprite(int spid)
{

}
void jpf_draw_sprite(jpfhandle_t h, int spid, int x, int y, int rot)
{
	sf::RenderWindow * winp = (sf::RenderWindow*)h;
	sprites.spritelist[spid]->sprite.setRotation(rot + 90);
	int xs, ys;
	struct Camera* c = &camera[userinput.cur];
	if (-1 == userinput.cur)return;
	xs = jpfpriv_x_w2s(x, c);
	ys = jpfpriv_y_w2s(y, c);
	sprites.spritelist[spid]->sprite.setPosition(xs, ys);
	sprites.spritelist[spid]->sprite.setOrigin(16, 16);
	winp->draw(sprites.spritelist[spid]->sprite);
}

void jpf_camera_follow(jpfusr_t usr, int x, int y)
{
	struct Camera* c = &camera[userinput.cur];
	if (-1 != userinput.cur && userinput.cur == usr->n) {
		jpfpriv_jpfpriv_set_camera(c, x, y);
		//std::cout << userinput.cur << std::endl;
	}
	
	//std::cout << userinput.cur << " " << x << " " << y << " : " << c->tgt_x << " " << c->x << " " << c->tgt_y << " " << c->y << std::endl;
	//std::cout << y << " : "  << c->tgt_y << " " << c->y << std::endl;
	
}

