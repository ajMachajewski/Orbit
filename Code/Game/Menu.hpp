#pragma once
#include "Game/Button.hpp"
#include <vector>


//----------------------------------------------------------------------------------------------------------
class Texture;


//----------------------------------------------------------------------------------------------------------
class Menu
{
public:
	Menu( std::string const& name, Texture* background = nullptr );
	Menu( std::string const& name, std::string const& backgroundImageFilepath );

	void Update();
	void Render( AABB2 const& screenBounds ) const;

	void Reset();

public:
	std::vector<Button> m_buttons;

private:
	Texture* m_backgroundTexture = nullptr;
	Button* m_selectedButton = nullptr;
	std::string m_name;
};