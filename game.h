// https://giovanni.codes/sfml-template-to-start-with-your-game/

#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>

class Game
{
public:
	Game();
	void Run();

private:
	void processEvents();
	void update(sf::Time deltaTime);
	void render();
	void handlePlayerInput(sf::Keyboard::Key key, bool isPressed);

	sf::RenderWindow mWindow;

	static const sf::Time TimePerFrame;

	void * winhandle;
};

#endif //GAME_H
