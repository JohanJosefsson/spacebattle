
#include "Game.h"
#include "jpf.h"
const sf::Time Game::TimePerFrame = sf::seconds(1.f / 20.f);

Game::Game()
	: mWindow(sf::VideoMode(MAX_X, MAX_Y), "Your Awesome Game!")
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
			handlePlayerInput(event.key.code, true);
			break;
		case sf::Event::KeyReleased:
			handlePlayerInput(event.key.code, false);
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
	jpf_on_tick((jpfhandle_t)&mWindow);
}

void Game::render()
{
	mWindow.clear(sf::Color::White);
	jpf_on_draw((jpfhandle_t)&mWindow);


	mWindow.display();
}




static struct Userinput{
	int users[11];
	int cur;
	int keys[NROF_KEYEVT][11];
	Userinput() : cur(-1) {};
} userinput;


void Game::handlePlayerInput(sf::Keyboard::Key key, bool isPressed)
{

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

		if (!userinput.users[n]) {
			jpf_on_new_user(n);
			userinput.users[n] = 1;
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
	return userinput.keys[key][usr];
}


struct spritelistelem
{
	// Cannot be allcoated static?
	sf::Texture texture;
	sf::Sprite sprite;
};

static struct {
	int cur;
	struct spritelistelem * spritelist[100];
} g_sprites;

int jpf_create_sprite(char * src)
{

	//g_sprites.spritelist[g_sprites.cur] = (spritelistelem*)malloc(sizeof(struct spritelistelem));
	g_sprites.spritelist[g_sprites.cur] = new spritelistelem;
	
	if (!g_sprites.spritelist[g_sprites.cur]->texture.loadFromFile(src))
	{
		printf("Error loading pic!\n");
		exit(1);
	}
	g_sprites.spritelist[g_sprites.cur]->sprite.setTexture(g_sprites.spritelist[g_sprites.cur]->texture);
	
	return g_sprites.cur++;
}
void jpf_release_sprite(int spid)
{

}
void jpf_draw_sprite(jpfhandle_t h, int spid, int x, int y, int rot)
{
	sf::RenderWindow * winp = (sf::RenderWindow*)h;
	g_sprites.spritelist[spid]->sprite.setRotation(rot + 90);
	g_sprites.spritelist[spid]->sprite.setPosition(x, y);
	g_sprites.spritelist[spid]->sprite.setOrigin(16, 16);
	winp->draw(g_sprites.spritelist[spid]->sprite);
}
