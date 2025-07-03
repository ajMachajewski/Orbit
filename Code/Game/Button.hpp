#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <string>


//----------------------------------------------------------------------------------------------------------
constexpr float BUTTON_PRESS_TIME = 0.125f;
constexpr const char* BUTTON_PRESS_EVENT_NAME = "OnButtonPress";


//----------------------------------------------------------------------------------------------------------
enum CardinalDirection : int
{
	DIRECTION_INVALID = -1,

	EAST,
	NORTH,
	WEST,
	SOUTH,

	NUM_CARDINAL_DIRECTIONS
};


//----------------------------------------------------------------------------------------------------------
class Button
{
public:
	Button( AABB2 const& bounds, std::string const& eventName, std::string const& label = "" );

	void Update( Vec2 const& cursorPosition );
	void Render() const;

	bool TryClick();
	void OnPress();

private:
	bool CheckCursorOverlap( Vec2 const& cursorPosition );
	void UpdatePressedState();

public:
	Button* m_neighbors[NUM_CARDINAL_DIRECTIONS] = { nullptr };

private:
	std::string m_eventName;
	std::string m_label;
	AABB2 m_bounds;
	Rgba8 m_defaultColor = Rgba8::WHITE;
	Rgba8 m_selectedColor = Rgba8::RED;
	Rgba8 m_pressedColor = Rgba8::BLUE;
	float m_timePressed = -100.f;
	bool m_pressed = true;
	bool m_hovered = false;

public:
	bool m_selected = false;
};