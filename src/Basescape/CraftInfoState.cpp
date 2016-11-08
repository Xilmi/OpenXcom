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
#include "CraftInfoState.h"
#include <cmath>
#include <sstream>
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextEdit.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Action.h"
#include "../Engine/Palette.h"
#include "../Savegame/Craft.h"
#include "../Mod/RuleCraft.h"
#include "../Savegame/CraftWeapon.h"
#include "../Mod/RuleCraftWeapon.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "CraftSoldiersState.h"
#include "CraftWeaponsState.h"
#include "CraftEquipmentState.h"
#include "CraftArmorState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Craft Info screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param craftId ID of the selected craft.
 */
CraftInfoState::CraftInfoState(Base *base, size_t craftId) : _base(base), _craftId(craftId)
{
	// Create objects
	if (_game->getSavedGame()->getMonthsPassed() != -1)
	{
		_window = new Window(this, 320, 200, 0, 0, POPUP_BOTH);
	}
	else
	{
		_window = new Window(this, 320, 200, 0, 0, POPUP_NONE);
	}

	_craft = _base->getCrafts()->at(_craftId);
	_weaponNum = _craft->getRules()->getWeapons();
	if (_weaponNum > RuleCraft::WeaponMax)
		_weaponNum = RuleCraft::WeaponMax;

	const int top = _weaponNum > 2 ? 42 : 64;
	const int top_row = 41;
	const int bottom = 125;
	const int bottom_row = 17;
	_btnOk = new TextButton(288, 16, 16, 176);
	for(int i = 0; i < _weaponNum; ++i)
	{
		const int x = i % 2 ? 282 : 14;
		const int y = top + (i / 2) * top_row;
		_btnW[i] = new TextButton(24, 32, x, y);
	}
	_btnCrew = new TextButton(64, 16, 16, bottom);
	_btnEquip = new TextButton(64, 16, 16, bottom + bottom_row);
	_btnArmor = new TextButton(64, 16, 16, bottom + 2 * bottom_row);
	_edtCraft = new TextEdit(this, 140, 16, 80, 8);
	_txtDamage = new Text(100, 17, 14, 24);
	_txtFuel = new Text(82, 17, 228, 24);
	for(int i = 0; i < _weaponNum; ++i)
	{
		const int x = i % 2 ? 204 : 46;
		const int y = top + (i / 2) * top_row;
		const int d = i % 2 ? 20 : 0;
		_txtWName[i] = new Text(95, 16, x - d, y);
		_txtWAmmo[i] = new Text(75, 24, x, y + 16);
	}
	_sprite = new Surface(32, 40, 144, 56);
	for(int i = 0; i < _weaponNum; ++i)
	{
		const int x = i % 2 ? 184 : 121;
		const int y = top + 16 + (i / 2) * top_row;
		_weapon[i] = new Surface(15, 17, x, y);
	}
	_crew = new Surface(220, 18, 85, bottom - 1);
	_equip = new Surface(220, 18, 85, bottom + bottom_row);

	// Set palette
	setInterface("craftInfo");

	add(_window, "window", "craftInfo");
	add(_btnOk, "button", "craftInfo");
	for(int i = 0; i < _weaponNum; ++i)
	{
		add(_btnW[i], "button", "craftInfo");
	}
	add(_btnCrew, "button", "craftInfo");
	add(_btnEquip, "button", "craftInfo");
	add(_btnArmor, "button", "craftInfo");
	add(_edtCraft, "text1", "craftInfo");
	add(_txtDamage, "text1", "craftInfo");
	add(_txtFuel, "text1", "craftInfo");
	for(int i = 0; i < _weaponNum; ++i)
	{
		add(_txtWName[i], "text2", "craftInfo");
		add(_txtWAmmo[i], "text2", "craftInfo");
		add(_weapon[i]);
	}
	add(_sprite);
	add(_crew);
	add(_equip);

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface("BACK14.SCR"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&CraftInfoState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&CraftInfoState::btnOkClick, Options::keyCancel);

	for(int i = 0; i < _weaponNum; ++i)
	{
		const wchar_t num[] = { wchar_t(L'1' + i), 0 };
		_btnW[i]->setText(num);
		_btnW[i]->onMouseClick((ActionHandler)&CraftInfoState::btnWClick);
	}

	_btnCrew->setText(tr("STR_CREW"));
	_btnCrew->onMouseClick((ActionHandler)&CraftInfoState::btnCrewClick);

	_btnEquip->setText(tr("STR_EQUIPMENT_UC"));
	_btnEquip->onMouseClick((ActionHandler)&CraftInfoState::btnEquipClick);

	_btnArmor->setText(tr("STR_ARMOR"));
	_btnArmor->onMouseClick((ActionHandler)&CraftInfoState::btnArmorClick);

	_edtCraft->setBig();
	_edtCraft->setAlign(ALIGN_CENTER);
	_edtCraft->onChange((ActionHandler)&CraftInfoState::edtCraftChange);

	_txtDamage->setColor(Palette::blockOffset(13)+10);
	_txtDamage->setSecondaryColor(Palette::blockOffset(13));

	_txtFuel->setColor(Palette::blockOffset(13)+10);
	_txtFuel->setSecondaryColor(Palette::blockOffset(13));

	for(int i =0; i < _weaponNum; ++i)
	{
		_txtWName[i]->setWordWrap(true);
	}
}

/**
 *
 */
CraftInfoState::~CraftInfoState()
{

}

/**
 * The craft info can change
 * after going into other screens.
 */
void CraftInfoState::init()
{
	State::init();

	_edtCraft->setText(_craft->getName(_game->getLanguage()));

	_sprite->clear();
	SurfaceSet *texture = _game->getMod()->getSurfaceSet("BASEBITS.PCK");
	texture->getFrame(_craft->getRules()->getSprite() + 33)->setX(0);
	texture->getFrame(_craft->getRules()->getSprite() + 33)->setY(0);
	texture->getFrame(_craft->getRules()->getSprite() + 33)->blit(_sprite);

	std::wostringstream ss;
	ss << tr("STR_DAMAGE_UC_").arg(Text::formatPercentage(_craft->getDamagePercentage()));
	if (_craft->getStatus() == "STR_REPAIRS" && _craft->getDamage() > 0)
	{
		int damageHours = (int)ceil((double)_craft->getDamage() / _craft->getRules()->getRepairRate());
		ss << formatTime(damageHours);
	}
	_txtDamage->setText(ss.str());

	std::wostringstream ss2;
	ss2 << tr("STR_FUEL").arg(Text::formatPercentage(_craft->getFuelPercentage()));
	if (_craft->getStatus() == "STR_REFUELLING" && _craft->getFuelMax() - _craft->getFuel() > 0)
	{
		int fuelHours = (int)ceil((double)(_craft->getFuelMax() - _craft->getFuel()) / _craft->getRules()->getRefuelRate() / 2.0);
		ss2 << formatTime(fuelHours);
	}
	_txtFuel->setText(ss2.str());

	if (_craft->getRules()->getSoldiers() > 0)
	{
		_crew->clear();
		_equip->clear();

		Surface *frame1 = texture->getFrame(38);
		frame1->setY(0);
		for (int i = 0, x = 0; i < _craft->getNumSoldiers(); ++i, x += 10)
		{
			frame1->setX(x);
			frame1->blit(_crew);
		}

		Surface *frame2 = texture->getFrame(40);
		frame2->setY(0);
		int x = 0;
		for (int i = 0; i < _craft->getNumVehicles(); ++i, x += 10)
		{
			frame2->setX(x);
			frame2->blit(_equip);
		}
		Surface *frame3 = texture->getFrame(39);
		for (int i = 0; i < _craft->getNumEquipment(); i += 4, x += 10)
		{
			frame3->setX(x);
			frame3->blit(_equip);
		}
	}
	else
	{
		_crew->setVisible(false);
		_equip->setVisible(false);
		_btnCrew->setVisible(false);
		_btnEquip->setVisible(false);
		_btnArmor->setVisible(false);
	}

	for(int i = 0; i < _weaponNum; ++i)
	{
		CraftWeapon *w1 = _craft->getWeapons()->at(i);

		_weapon[i]->clear();
		if (w1 != 0)
		{
			Surface *frame = texture->getFrame(w1->getRules()->getSprite() + 48);
			frame->setX(0);
			frame->setY(0);
			frame->blit(_weapon[i]);

			std::wostringstream ss;
			ss << L'\x01' << tr(w1->getRules()->getType());
			_txtWName[i]->setText(ss.str());
			ss.str(L"");
			if (w1->getRules()->getAmmoMax())
			{
				ss << tr("STR_AMMO_").arg(w1->getAmmo()) << L"\n\x01";
				ss << tr("STR_MAX").arg(w1->getRules()->getAmmoMax());
				if (_craft->getStatus() == "STR_REARMING" && w1->getAmmo() < w1->getRules()->getAmmoMax())
				{
					int rearmHours = (int)ceil((double)(w1->getRules()->getAmmoMax() - w1->getAmmo()) / w1->getRules()->getRearmRate());
					ss << formatTime(rearmHours);
				}
			}
			_txtWAmmo[i]->setText(ss.str());
		}
		else
		{
			_txtWName[i]->setText(L"");
			_txtWAmmo[i]->setText(L"");
		}
	}

}

/**
 * Turns an amount of time into a
 * day/hour string.
 * @param total Amount in hours.
 */
std::wstring CraftInfoState::formatTime(int total)
{
	std::wostringstream ss;
	int days = total / 24;
	int hours = total % 24;
	ss << L"\n(";
	if (days > 0)
	{
		ss << tr("STR_DAY", days) << L"/";
	}
	if (hours > 0)
	{
		ss << tr("STR_HOUR", hours);
	}
	ss << L")";
	return ss.str();
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CraftInfoState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Goes to the Select Armament window
 * for the weapons.
 * @param action Pointer to an action.
 */
void CraftInfoState::btnWClick(Action * act)
{
	for(int i = 0; i < _weaponNum; ++i)
	{
		if (act->getSender() == _btnW[i])
		{
			_game->pushState(new CraftWeaponsState(_base, _craftId, i));
			return;
		}
	}
}

/**
 * Goes to the Select Squad screen.
 * @param action Pointer to an action.
 */
void CraftInfoState::btnCrewClick(Action *)
{
	_game->pushState(new CraftSoldiersState(_base, _craftId));
}

/**
 * Goes to the Select Equipment screen.
 * @param action Pointer to an action.
 */
void CraftInfoState::btnEquipClick(Action *)
{
	_game->pushState(new CraftEquipmentState(_base, _craftId));
}

/**
 * Goes to the Select Armor screen.
 * @param action Pointer to an action.
 */
void CraftInfoState::btnArmorClick(Action *)
{
	_game->pushState(new CraftArmorState(_base, _craftId));
}

/**
 * Changes the Craft name.
 * @param action Pointer to an action.
 */
void CraftInfoState::edtCraftChange(Action *action)
{
	if (_edtCraft->getText() == _craft->getDefaultName(_game->getLanguage()))
	{
		_craft->setName(L"");
	}
	else
	{
		_craft->setName(_edtCraft->getText());
	}
	if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
		action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
	{
		_edtCraft->setText(_craft->getName(_game->getLanguage()));
	}
}

}
