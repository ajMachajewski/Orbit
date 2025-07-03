#pragma once
#include "Game/Button.hpp"
#include <vector>


//----------------------------------------------------------------------------------------------------------
class Menu
{
public:
	Menu( std::string const& name );

	void Update();
	void Render() const;

public:
	std::vector<Button> m_buttons;

private:
	Button* m_selectedButton;
	std::string m_name;
};