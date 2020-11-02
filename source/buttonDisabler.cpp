#include "include/SM64DS_2.h"

uint16_t realButtonsPressed, realButtonsHeld;

void hook_0202bbe4()
{
	realButtonsPressed = INPUT_ARR[0].buttonsPressed;
	realButtonsHeld = INPUT_ARR[0].buttonsHeld;

	if (LEVEL_ID == 1)
	{
		constexpr uint16_t disabledButtons = Input::X | Input::L | Input::CAM_LEFT | Input::CAM_RIGHT | Input::SELECT;

		INPUT_ARR[0].buttonsPressed &= ~disabledButtons;
		INPUT_ARR[0].buttonsHeld &= ~disabledButtons;
	}
}