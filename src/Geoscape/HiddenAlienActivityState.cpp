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
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Unicode.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Interface/Window.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleCraft.h"
#include "../Mod/RuleUfo.h"
#include "../Mod/RuleRegion.h"
#include "../Mod/RuleCountry.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Ufo.h"
#include "../Savegame/Region.h"
#include "../Savegame/Country.h"
#include "GeoscapeState.h"
#include "Globe.h"
#include "InterceptState.h"
#include "HiddenAlienActivityState.h"
#include <sstream>

namespace OpenXcom
{

/**
 * Initializes all the elements in the hidden alien activity window.
 * @param game Pointer to the core game.
 * @param ufo Pointer to the UFO to get info from.
 * @param state Pointer to the Geoscape.
 * @param detected Was the UFO detected?
 * @param hyperwave Was it a hyperwave radar?
 */
HiddenAlienActivityState::HiddenAlienActivityState
(
	GeoscapeState* state,
	const std::map<OpenXcom::Region*, int> &displayHiddenAlienActivityRegions,
	const std::map<OpenXcom::Country*, int> &displayHiddenAlienActivityCountries
)
	:
	_state(state)
{
	_screen = false;

	// create objects

	_window = new Window(this, 224, 180, 16, 10, POPUP_BOTH);
	_txtInfo = new Text(200, 16, 28, 20);
	_txtHeaderRegions = new Text(140, 8, 28, 40);
	_txtSightingsRegions = new Text(40, 8, 28 + 140, 40);
	_lstHiddenAlienActivityRegions = new TextList(180, 40, 28, 50);
	_txtHeaderCountries = new Text(140, 8, 28, 98);
	_txtSightingsCountries = new Text(40, 8, 28 + 140, 98);
	_lstHiddenAlienActivityCountries = new TextList(180, 40, 28, 108);
	_btnOk = new TextButton(200, 12, 28, 152);
	_btnCancel = new TextButton(200, 12, 28, 168);

	// set palette

	setInterface("hiddenAlienActivity", false);

	// add elements

	add(_window, "window", "hiddenAlienActivity");
	add(_txtInfo, "text", "hiddenAlienActivity");
	add(_txtHeaderRegions, "text", "hiddenAlienActivity");
	add(_txtSightingsRegions, "text", "hiddenAlienActivity");
	add(_lstHiddenAlienActivityRegions, "list", "hiddenAlienActivity");
	add(_txtHeaderCountries, "text", "hiddenAlienActivity");
	add(_txtSightingsCountries, "text", "hiddenAlienActivity");
	add(_lstHiddenAlienActivityCountries, "list", "hiddenAlienActivity");
	add(_btnOk, "button", "hiddenAlienActivity");
	add(_btnCancel, "button", "hiddenAlienActivity");

	// set up objects

	setWindowBackground(_window, "hiddenAlienActivity");

	centerAllSurfaces();

	_txtInfo->setBig();
	_txtInfo->setText(tr("STR_HIDDEN_ALIEN_ACTIVITY"));
	_txtInfo->setAlign(ALIGN_CENTER);

	_txtHeaderRegions->setText(tr("STR_UFO_ACTIVITY_IN_AREAS"));
	_txtSightingsRegions->setText(tr("STR_UFO_SIGHTINGS"));
	_txtSightingsRegions->setAlign(ALIGN_RIGHT);

	_lstHiddenAlienActivityRegions->setColumns(2, 140, 40);
	_lstHiddenAlienActivityRegions->setAlign(ALIGN_RIGHT, 1);

	_txtHeaderCountries->setText(tr("STR_UFO_ACTIVITY_IN_COUNTRIES"));
	_txtSightingsCountries->setText(tr("STR_UFO_SIGHTINGS"));
	_txtSightingsCountries->setAlign(ALIGN_RIGHT);

	_lstHiddenAlienActivityCountries->setColumns(2, 140, 40);
	_lstHiddenAlienActivityCountries->setAlign(ALIGN_RIGHT, 1);

	_btnOk->setText(tr("STR_OK_5_SECONDS"));
	_btnOk->onMouseClick((ActionHandler)&HiddenAlienActivityState::btnOkClick);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&HiddenAlienActivityState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&HiddenAlienActivityState::btnCancelClick, Options::keyCancel);

	if (Options::displayHiddenAlienActivity == 1)
	{
		_txtSightingsRegions->setVisible(false);
		_txtSightingsCountries->setVisible(false);
	}

	// populate alien activity lists

	for (const std::pair<Region* const, int> &displayHiddenAlienActivityRegionEntry : displayHiddenAlienActivityRegions)
	{
		Region* region = displayHiddenAlienActivityRegionEntry.first;
		int activity = displayHiddenAlienActivityRegionEntry.second;

		std::ostringstream ossName;
		std::ostringstream ossValue;
		if (activity == 0)
		{
			ossName << Unicode::TOK_COLOR_FLIP;
			ossValue << Unicode::TOK_COLOR_FLIP;
		}
		ossName << tr(region->getRules()->getType());
		ossValue << std::to_string(activity);

		if (Options::displayHiddenAlienActivity == 2)
		{
			_lstHiddenAlienActivityRegions->addRow(2, ossName.str().c_str(), ossValue.str().c_str());
		}
		else
		{
			_lstHiddenAlienActivityRegions->addRow(1, ossName.str().c_str());
		}

	}

	for (const std::pair<Country* const, int> &displayHiddenAlienActivitycountryEntry : displayHiddenAlienActivityCountries)
	{
		Country* country = displayHiddenAlienActivitycountryEntry.first;
		int activity = displayHiddenAlienActivitycountryEntry.second;

		std::ostringstream ossName;
		std::ostringstream ossValue;
		if (activity == 0)
		{
			ossName << Unicode::TOK_COLOR_FLIP;
			ossValue << Unicode::TOK_COLOR_FLIP;
		}
		ossName << tr(country->getRules()->getType());
		ossValue << std::to_string(activity);

		if (Options::displayHiddenAlienActivity == 2)
		{
			_lstHiddenAlienActivityCountries->addRow(2, ossName.str().c_str(), ossValue.str().c_str());
		}
		else
		{
			_lstHiddenAlienActivityCountries->addRow(1, ossName.str().c_str());
		}

	}

}

/**
 *
 */
HiddenAlienActivityState::~HiddenAlienActivityState()
{
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void HiddenAlienActivityState::btnOkClick(Action*)
{
	_state->timerReset();
	_game->popState();
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void HiddenAlienActivityState::btnCancelClick(Action*)
{
	_game->popState();
}

}

