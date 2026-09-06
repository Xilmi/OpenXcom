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
#include "../Engine/Yaml.h"
#include "BattlescapeGame.h"
#include "Position.h"
#include "Pathfinding.h"
#include "../Savegame/BattleUnit.h"
#include <vector>
#include <set>


namespace OpenXcom
{

class SavedBattleGame;
class BattleUnit;
struct BattleAction;
class BattlescapeState;
class Node;

enum AIMode { AI_PATROL, AI_AMBUSH, AI_COMBAT, AI_ESCAPE };
enum AIAttackWeight : int
{
	/// Base scale of attack weights
	AIW_SCALE = 100,
	AIW_IGNORED = 0,
};

/**
 * This class is used by the BattleUnit AI.
 */
class AIModule
{
private:
	SavedBattleGame *_save;
	BattleUnit *_unit;
	BattleUnit *_aggroTarget;
	int _knownEnemies, _visibleEnemies, _spottingEnemies;
	int _escapeTUs, _ambushTUs;
	int _walkAbortCounter;
	bool _weaponPickedUp;
	bool _wantToEndTurn;
	bool _rifle, _melee, _blaster, _grenade;
	bool _traceAI, _didPsi;
	bool _ranOutOfTUs;
	int _AIMode, _intelligence, _closestDist;
	Node *_fromNode, *_toNode;
	bool _foundBaseModuleToDestroy;
	std::vector<int> _reachable, _reachableWithAttack, _wasHitBy;
	std::vector<PathfindingNode*> _allPathFindingNodes;
	Position _positionAtStartOfTurn;
	int _tuCostToReachClosestPositionToBreakLos;
	int _energyCostToReachClosestPositionToBreakLos;
	int _tuWhenChecking;
	bool _allowedToCheckAttack = false;
	BattleActionType _reserve;
	UnitFaction _targetFaction;
	UnitFaction _myFaction;

	BattleAction _escapeAction, _ambushAction, _attackAction, _patrolAction, _psiAction;

	bool selectPointNearTargetLeeroy(BattleUnit *target, bool canRun);
	int selectNearestTargetLeeroy(bool canRun);
	void meleeActionLeeroy(bool canRun);
	void dont_think(BattleAction *action);
public:
	bool medikit_think(BattleMediKitType healOrStim);
public:
	/// Creates a new AIModule linked to the game and a certain unit.
	AIModule(SavedBattleGame *save, BattleUnit *unit, Node *node);
	/// Cleans up the AIModule.
	~AIModule();
	/// Sets the target faction.
	void setTargetFaction(UnitFaction f);
	/// Resets the unsaved AI state.
	void reset();
	/// Loads the AI Module from YAML.
	void load(const YAML::YamlNodeReader& reader);
	/// Saves the AI Module to YAML.
	void save(YAML::YamlNodeWriter writer) const;
	/// Runs Module functionality every AI cycle.
	void think(BattleAction *action);
	/// Sets the "unit was hit" flag true.
	void setWasHitBy(BattleUnit *attacker);
	/// Increases the walk abort counter.
	void increaseWalkAbortCounter() { _walkAbortCounter++; }
	/// Sets the "unit picked up a weapon" flag.
	void setWeaponPickedUp();
	/// Gets whether the unit was hit.
	bool getWasHitBy(int attacker) const;
	/// Set start node.
	void setStartNode(Node *node) { _fromNode = node; }
	/// setup a patrol objective.
	void setupPatrol();
	/// setup an ambush objective.
	void setupAmbush();
	/// setup a combat objective.
	void setupAttack();
	/// setup an escape objective.
	void setupEscape();
	/// count how many xcom/civilian units are known to this unit.
	int countKnownTargets() const;
	/// count how many known XCom units are able to see this unit.
	int getSpottingUnits(const Position& pos) const;
	/// Selects the nearest target we can see, and return the number of viable targets.
	int selectNearestTarget();
	/// Selects the closest known xcom unit for ambushing.
	bool selectClosestKnownEnemy();
	/// Selects a random known target.
	bool selectRandomTarget();
	/// Selects the nearest reachable point relative to a target.
	bool selectPointNearTarget(BattleUnit *target, int maxTUs);
	/// Selects a target from a list of units seen by spotter units for out-of-LOS actions
	bool selectSpottedUnitForSniper();
	/// Scores a firing mode action based on distance to target and accuracy.
	int scoreFiringMode(BattleAction *action, BattleUnit *target, bool checkLOF);
	/// re-evaluate our situation, and make a decision from our available options.
	void evaluateAIMode();
	/// Selects a suitable position from which to attack.
	bool findFirePoint();
	/// Decides if we should throw a grenade/launch a missile to this position.
	int explosiveEfficacy(Position targetPos, BattleUnit *attackingUnit, int radius, int diff, bool grenade = false) const;
	bool getNodeOfBestEfficacy(BattleAction *action, int radius);
	/// Attempts to take a melee attack/charge an enemy we can see.
	void meleeAction();
	/// Attempts to fire a waypoint projectile at an enemy we, or one of our teammates sees.
	void wayPointAction();
	/// Attempts to fire at an enemy spotted for us.
	bool sniperAction();
	/// Attempts to fire at an enemy we can see.
	void projectileAction();
	/// Chooses a firing mode for the AI based on expected number of hits per turn
	void extendedFireModeChoice(BattleActionCost& costAuto, BattleActionCost& costSnap, BattleActionCost& costAimed, BattleActionCost& costThrow, bool checkLOF = false);
	/// Attempts to throw a grenade at an enemy (or group of enemies) we can see.
	void grenadeAction();
	/// Performs a psionic attack.
	bool psiAction();
	/// Performs a melee attack action.
	void meleeAttack();

	/// How much given unit is worth as target of attack.
	AIAttackWeight getTargetAttackWeight(BattleUnit* target) const;
	/// Checks to make sure a target is valid, given the parameters
	bool validTarget(BattleUnit* target, bool assessDanger, bool includeCivs) const;

	/// Checks the alien's TU reservation setting.
	BattleActionType getReserveMode();
	/// Assuming we have both a ranged and a melee weapon, we have to select one.
	void selectMeleeOrRanged();
	/// Gets the current targetted unit.
	BattleUnit* getTarget();
	/// Frees up the destination node for another Unit to select
	void freePatrolTarget();

	/// Everything below belongs tu Brutal-AI
	/// Checks whether anyone on our team can see the target
	bool visibleToAnyFriend(BattleUnit *target) const;
	/// Handles behavior of brutalAI
	void brutalThink(BattleAction *action);
	/// Like selectSpottedUnitForSniper but works for everyone
	bool brutalSelectSpottedUnitForSniper();
	/// look up in _allPathFindingNodes how many time-units we need to get to a specific position
	int tuCostToReachPosition(Position pos, const std::vector<PathfindingNode *> nodeVector, BattleUnit* actor = NULL, bool forceExactPosition = false, bool energyInsteadOfTU = false);
	/// find the cloest Position to our target we can reach while reserving for a BattleAction
	Position furthestToGoTowards(Position target, BattleActionCost reserve, const std::vector<PathfindingNode *> nodeVector, bool encircleTileMode = false, Tile *encircleTile = NULL);
	/// find the closest Position that isn't our current position which is on the way to a target
	Position closestToGoTowards(Position target, const std::vector<PathfindingNode *> nodeVector, Position myPos, bool peakMode = false);
	/// checks if the path to a position is save
	bool isPathToPositionSave(Position target, bool &saveForProxies);
	/// Performs a psionic attack but allow multiple per turn and take success-chance into consideration
	bool brutalPsiAction();
	/// Chooses a firing mode for the AI based on expected damage dealt
	float brutalExtendedFireModeChoice(BattleActionCost &costAuto, BattleActionCost &costSnap, BattleActionCost &costAimed, BattleActionCost &costThrow, BattleActionCost &costHit, bool checkLOF = false, float previousHighScore = 0);
	/// Scores a firing mode action based on distance to target, accuracy and overall Damage dealt, also supports melee-hits
	float brutalScoreFiringMode(BattleAction *action, BattleUnit *target, bool checkLOF, bool reactionCheck = false);
	/// Used as multiplier for the throw-action in brutalScoreFiringMode
	float brutalExplosiveEfficacy(Position targetPos, BattleUnit *attackingUnit, int radius, bool grenade = false, bool validOnly = false) const;
	/// An inaccurate simplified check for line of fire from a specific position to a specific target
	bool quickLineOfFire(Position pos, BattleUnit *target, bool beOkayWithFriendOfTarget = false, bool lastLocationMode = false, bool fleeMode = false);
	/// checks whether there is clear sight between two tile-positions
	bool clearSight(Position pos, Position target);
	/// how many time-units would it take to turn to a specific target
	int getTurnCostTowards(Position target, Position from);
	/// overload without from
	int getTurnCostTowards(Position target);
	/// Using weapons like the blaster but actually hitting what we want while avoiding to mow down our allies
	void brutalBlaster();
	/// Attempts to throw a grenade at tiles near potential targets when target itself couldn't be hit
	void brutalGrenadeAction();
	/// Tells the AI of the unit whether it wants to end the turn or not
	void setWantToEndTurn(bool wantToEndTurn);
	/// Asks the unit's AI whether it wants to end the turn or not
	bool getWantToEndTurn();
	/// Attack tiles where units have been seen before but we are not sure where they are
	void blindFire();
	/// Validating the shot of an arcing weapon is way more compliacated than for a throw, that's why there's a separate method
	bool validateArcingShot(BattleAction *action, Tile* originTile = NULL);
	/// check if a unit is targetable according to aiTargetMode
	bool brutalValidTarget(BattleUnit *unit, bool moveMode = false, bool psiMode = false) const;
	/// check the path to an enemy and then subtracts their movement from the cost
	Position closestPositionEnemyCouldReach(BattleUnit *enemy);
	/// returns how far a unit can shoot while extender-accuracy is enabled with the given amount of time-units left 
	int maxExtenderRangeWith(BattleUnit *unit, int tus);
	/// Determines a new tile where to look for an enemy who's position is unknown
	int getNewTileIDToLookForEnemy(Position previousPosition, BattleUnit *unit);
	/// Calculates how much TU this unit can have at most considering it's carrying capacity and leg-damage
	int getMaxTU(BattleUnit *unit);
	/// Get the ID of the closest tile which is an entry-point for the player
	int getClosestSpawnTileId();
	/// Tells us whether a unit is an enemy
	bool isEnemy(BattleUnit* unit, bool ignoreSameOriginalFaction = false) const;
	/// Tells us whether a unit is an ally
	bool isAlly(BattleUnit *unit) const;
	/// Checks whether the trajectory of a projectile visits tiles occupied by our allies
	bool projectileMayHarmFriends(Position startPos, Position targetPos);
	/// Checks whether at least one of our allies is in range for a good attack
	bool inRangeOfAnyFriend(Position pos);
	/// Checks whether we should avoid melee-range against a specific enemy
	bool shouldAvoidMeleeRange(BattleUnit *enemy);
	/// Checks whether a unit has any means to fight
	bool isArmed(BattleUnit *unit) const;
	/// Checks whether there's a grenade on the ground and tries to pick it up
	void tryToPickUpGrenade(Tile* tile, BattleAction* action);
	/// returns a score for how much we like to pick up a specific kind of item
	float getItemPickUpScore(BattleItem *item);
	/// Non-cheating-AI needs to be able to determine whether the enemy is doing Triton-shenanigans, where we should prevent exposing ourselves or is exposed enough themselves for us to strike
	bool IsEnemyExposedEnough();
	/// Get the cover-value of a tile
	float getCoverValue(Tile *tile, BattleUnit *bu, int coverQuality = 1);
	/// checks whethere there's any cover in range
	float highestCoverInRange(const std::vector<PathfindingNode *> nodeVector);
	/// runs a very minimalist pathfinding just to see whether the unit could move
	bool isAnyMovementPossible();
	/// returns how much energy the unit can recover each turn
	int getEnergyRecovery(BattleUnit* unit);
	/// returns reachable tile-Ids by a particular unit
	std::map<Position, int, PositionComparator> getReachableBy(BattleUnit* unit, bool& ranOutOfTUs, bool forceRecalc = false, bool useMaxTUs = false, bool pruneAirTiles = false);
	/// checks whether it would be possible to see one tile from another
	bool hasTileSight(Position from, Position to);
	/// returns the amount of blaster-waypoints to reach a target-positon
	int requiredWayPointCount(Position to, const std::vector<PathfindingNode*> nodeVector);
	/// returns a vector of all positions we'd have to walk towards a specific location
	std::vector<Position> getPositionsOnPathTo(Position target, const std::vector<PathfindingNode*> nodeVector);
	/// returns fear of smoke
	std::map<Position, int, PositionComparator> getSmokeFearMap();
	/// returns how urgent it is to get rid of a grenade
	float grenadeRiddingUrgency();
	/// returns which side of the unit is facing the given position
	UnitSide getSideFacingToPosition(BattleUnit* unit, Position pos);
	/// returns whether the unit wants to run
	bool wantToRun();
	/// Pointer to save so that unit can access it
	SavedBattleGame* getSave() { return _save; };
	/// Determine a good position for indirect peeking
	Position getPeakPosition(bool oneStep = false);
	/// Gives an estimate of a unit's power-level
	float getUnitPower(BattleUnit* unit);
	/// returns a vector of Tiles next to doors
	std::vector<Tile*> getCorpseTiles(const std::vector<PathfindingNode*> nodeVector);
	/// tries to pick up weapon and ammo from current tile if it's an upgrade
	bool improveItemization(float currentItemScore, BattleAction* action);
	/// scores a set of tiles based on how long ago they were seen
	int scoreVisibleTiles(const std::set<Tile*>& tileSet);
	/// prepares a grenade-action to use with validateArcingShot
	BattleAction* grenadeThrowAction(Position pos);
	/// how much damage we can inflict to a given enemy
	float damagePotential(Position pos, BattleUnit* target, int tuTotal, int energyTotal);
	/// checks if a position is visible to the enemy
	bool isPositionVisibleToEnemy(Position pos, bool tileLOSMode = false);
};

struct MoveEvaluation
{
	int remainingTU;
	int remainingEnergy;
	int lastStepCost;
	float walkToDist;
	float attackPotential;
	bool IsDirectPeak;
	int visibleTiles;
	int bestDirection;
};

}
