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

#include "UnitTurnBState.h"
#include "TileEngine.h"
#include "Map.h"
#include "../Savegame/BattleUnit.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Mod/Mod.h"
#include "../Engine/Sound.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

/**
 * Sets up an UnitTurnBState.
 * @param parent Pointer to the Battlescape.
 * @param action Pointer to an action.
 */
UnitTurnBState::UnitTurnBState(BattlescapeGame *parent, BattleAction action, bool chargeTUs) : BattleState(parent, action), _unit(0), _turret(false), _chargeTUs(chargeTUs)
{

}

/**
 * Deletes the UnitTurnBState.
 */
UnitTurnBState::~UnitTurnBState()
{

}

/**
 * Initializes the state.
 */
void UnitTurnBState::init()
{
	_unit = _action.actor;
	if (_unit->isOut())
	{
		_parent->popState();
		return;
	}
	_action.clearTU();
	if (_unit->getFaction() == FACTION_PLAYER)
		_parent->setStateInterval(Options::battleXcomSpeed);
	else
		_parent->setStateInterval(Options::battleAlienSpeed);

	// if the unit has a turret and we are turning during targeting, then only the turret turns
	_turret = _unit->getTurretType() != -1 && (_action.targeting || _action.strafe);

	if (_unit->getPosition() != _action.target)
		_unit->lookAt(_action.target, _turret);

	if (_chargeTUs && _unit->getStatus() != STATUS_TURNING)
	{
		if (_action.type == BA_NONE)
		{
			// try to open a door
			int visibleTilesBefore = _unit->getVisibleTiles()->size();
			int door = _parent->getTileEngine()->unitOpensDoor(_unit, true);
			// when unit sees more tiles than it did before, the door was opened and it shall proceed. When tiles are same or lower it is done.
			if (_unit->isAIControlled() && _unit->getVisibleTiles()->size() > visibleTilesBefore)
			{
				if (Options::traceAI)
				{
					Log(LOG_INFO) << _unit->getId() << " should now want to continue their turn";
				}
				_unit->checkForReactivation(_parent->getSave());
			}
			if (door == 0)
			{
				_parent->getMod()->getSoundByDepth(_parent->getDepth(), Mod::DOOR_OPEN)->play(-1, _parent->getMap()->getSoundAngle(_unit->getPosition())); // normal door
			}
			if (door == 1)
			{
				_parent->getMod()->getSoundByDepth(_parent->getDepth(), Mod::SLIDING_DOOR_OPEN)->play(-1, _parent->getMap()->getSoundAngle(_unit->getPosition())); // ufo door
			}
			if (door == 4)
			{
				_action.result = "STR_NOT_ENOUGH_TIME_UNITS";
			}
		}
		_parent->popState();
	}
}

/**
 * Runs state functionality every cycle.
 */
void UnitTurnBState::think()
{
	const int tu = _chargeTUs ? (_turret ? 1 :_unit->getTurnCost()) : 0;

	if (_chargeTUs && _unit->getFaction() == _parent->getSave()->getSide() && _parent->getPanicHandled() && !_action.targeting && !_parent->checkReservedTU(_unit, tu, 0))
	{
		_unit->abortTurn();
		_parent->popState();
		return;
	}

	if (_unit->spendTimeUnits(tu))
	{
		size_t unitSpotted = _unit->getUnitsSpottedThisTurn().size();
		_unit->turn(_turret);
		_parent->getTileEngine()->calculateFOV(_unit);
		if (_chargeTUs && _unit->getFaction() == _parent->getSave()->getSide() && _parent->getPanicHandled() && _action.type == BA_NONE && _unit->getUnitsSpottedThisTurn().size() > unitSpotted)
		{
			if (Options::traceAI)
			{
				Log(LOG_INFO) << "Found " << _unit->getUnitsSpottedThisTurn().size() - unitSpotted << " new units while turning.Letting my allies know about it.";
			}
			for (BattleUnit *unit : *(_parent->getSave()->getUnits()))
			{
				if (unit->isOut())
					continue;
				if (!unit->getAIModule() || !unit->isBrutal() || unit->getFaction() != _unit->getFaction())
					continue;
				unit->checkForReactivation(_parent->getSave());
			}
			_unit->abortTurn();
			_parent->popState();
		}
		else if (_unit->getStatus() == STATUS_STANDING)
		{
			_parent->popState();

			if (_action.kneel && !_unit->isFloating() && !_unit->isKneeled())
			{
				BattleAction kneel;
				kneel.type = BA_KNEEL;
				kneel.actor = _unit;
				kneel.Time = _unit->getKneelChangeCost();
				if (kneel.spendTU())
				{
					_unit->kneel(!_unit->isKneeled());
					// kneeling or standing up can reveal new terrain or units. I guess.
					_parent->getTileEngine()->calculateFOV(_unit->getPosition(), 1, false); //Update unit FOV for everyone through this position, skip tiles.
					_parent->getTileEngine()->checkReactionFire(_unit, kneel);
				}
			}
		}
	}
	else if (_parent->getPanicHandled())
	{
		_action.result = "STR_NOT_ENOUGH_TIME_UNITS";
		_unit->abortTurn();
		_parent->popState();
	}
}

/**
 * Unit turning cannot be cancelled.
 */
void UnitTurnBState::cancel()
{
}

}
