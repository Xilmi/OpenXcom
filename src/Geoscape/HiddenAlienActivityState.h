#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "../Engine/State.h"

namespace OpenXcom
{

class Ufo;
class TextButton;
class Window;
class Text;
class TextList;
class GeoscapeState;

/**
 * Displays hidden alien activity info.
 */
class HiddenAlienActivityState : public State
{
private:

	GeoscapeState* _state;
	TextButton* _btnOk;
	TextButton* _btnCancel;
	Window *_window;
	Text *_txtInfo, *_txtHeaderRegions, *_txtSightingsRegions, *_txtHeaderCountries, *_txtSightingsCountries;
	TextList *_lstHiddenAlienActivityRegions, *_lstHiddenAlienActivityCountries;

public:

	HiddenAlienActivityState(GeoscapeState* state, const std::map<OpenXcom::Region*, int> &displayHiddenAlienActivityRegions, const std::map<OpenXcom::Country*, int> &displayHiddenAlienActivityCountries);
	~HiddenAlienActivityState();
	void btnOkClick(Action* action);
	void btnCancelClick(Action* action);

};

}
