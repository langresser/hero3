#pragma once


#include "BattleHex.h"
#include "HeroBonus.h"
#include "CCreatureSet.h"
#include "CObjectHandler.h"
#include "CCreatureHandler.h"
#include "CObstacleInstance.h"
#include "ConstTransitivePtr.h"
#include "GameConstants.h"
#include "CBattleCallback.h"

/*
 * BattleState.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

class CGHeroInstance;
class CStack;
class CArmedInstance;
class CGTownInstance;
class CStackInstance;
struct BattleStackAttacked;



//only for use in BattleInfo
struct DLL_LINKAGE SiegeInfo
{
	ui8 wallState[EWallParts::PARTS_COUNT]; 
	
	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & wallState;
	}
};



struct DLL_LINKAGE BattleInfo : public CBonusSystemNode, public CBattleInfoCallback
{
	ui8 sides[2]; //sides[0] - attacker, sides[1] - defender
	si32 round, activeStack, selectedStack;
	ui8 siege; //    = 0 ordinary battle    = 1 a siege with a Fort    = 2 a siege with a Citadel    = 3 a siege with a Castle
	const CGTownInstance * town; //used during town siege - id of attacked town; -1 if not town defence
	int3 tile; //for background and bonuses
	CGHeroInstance* heroes[2];
	CArmedInstance *belligerents[2]; //may be same as heroes
	std::vector<CStack*> stacks;
	std::vector<shared_ptr<CObstacleInstance> > obstacles;
	ui8 castSpells[2]; //how many spells each side has cast this turn [0] - attacker, [1] - defender
	std::vector<const CSpell *> usedSpellsHistory[2]; //each time hero casts spell, it's inserted here -> eagle eye skill
	si16 enchanterCounter[2]; //tends to pass through 0, so sign is needed
	SiegeInfo si;

	si32 battlefieldType; //like !!BA:B
	ui8 terrainType; //used for some stack nativity checks (not the bonus limiters though that have their own copy)

	ui8 tacticsSide; //which side is requested to play tactics phase
	ui8 tacticDistance; //how many hexes we can go forward (1 = only hexes adjacent to margin line)

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & sides & round & activeStack & selectedStack & siege & town & tile & stacks & belligerents & obstacles
			& castSpells & si & battlefieldType & terrainType;
		h & heroes;
		h & usedSpellsHistory & enchanterCounter;
		h & tacticsSide & tacticDistance;
		h & static_cast<CBonusSystemNode&>(*this);
	}

	//////////////////////////////////////////////////////////////////////////
	BattleInfo();
	~BattleInfo(){};

	//////////////////////////////////////////////////////////////////////////
	//void getBonuses(BonusList &out, const CSelector &selector, const CBonusSystemNode *root = NULL) const;
	//////////////////////////////////////////////////////////////////////////
	CStack * getStackT(BattleHex tileID, bool onlyAlive = true);
	CStack * getStack(int stackID, bool onlyAlive = true);

	const CStack * getNextStack() const; //which stack will have turn after current one
	//void getStackQueue(std::vector<const CStack *> &out, int howMany, int turn = 0, int lastMoved = -1) const; //returns stack in order of their movement action

	//void getAccessibilityMap(bool *accessibility, bool twoHex, bool attackerOwned, bool addOccupiable, std::set<BattleHex> & occupyable, bool flying, const CStack* stackToOmmit = NULL) const; //send pointer to at least 187 allocated bytes
	//static bool isAccessible(BattleHex hex, bool * accessibility, bool twoHex, bool attackerOwned, bool flying, bool lastPos); //helper for makeBFS
	int getAvaliableHex(TCreature creID, bool attackerOwned, int initialPos = -1) const; //find place for summon / clone effects
	//void makeBFS(BattleHex start, bool*accessibility, BattleHex *predecessor, int *dists, bool twoHex, bool attackerOwned, bool flying, bool fillPredecessors) const; //*accessibility must be prepared bool[187] array; last two pointers must point to the at least 187-elements int arrays - there is written result
	std::pair< std::vector<BattleHex>, int > getPath(BattleHex start, BattleHex dest, const CStack *stack); //returned value: pair<path, length>; length may be different than number of elements in path since flying vreatures jump between distant hexes
	//std::vector<BattleHex> getAccessibility(const CStack * stack, bool addOccupiable, std::vector<BattleHex> * attackable = NULL, bool forPassingBy = false) const; //returns vector of accessible tiles (taking into account the creature range)

	//bool isObstacleVisibleForSide(const CObstacleInstance &obstacle, ui8 side) const;
	shared_ptr<CObstacleInstance> getObstacleOnTile(BattleHex tile) const;
	std::set<BattleHex> getStoppers(bool whichSidePerspective) const;

	ui32 calculateDmg(const CStack* attacker, const CStack* defender, const CGHeroInstance * attackerHero, const CGHeroInstance * defendingHero, bool shooting, ui8 charge, bool lucky, bool deathBlow, bool ballistaDoubleDmg); //charge - number of hexes travelled before attack (for champion's jousting)
	void calculateCasualties(std::map<ui32,si32> *casualties) const; //casualties are array of maps size 2 (attacker, defeneder), maps are (crid => amount)
	
	//void getPotentiallyAttackableHexes(AttackableTiles &at, const CStack* attacker, BattleHex destinationTile, BattleHex attackerPos); //hexes around target that could be attacked in melee
	//std::set<CStack*> getAttackedCreatures(const CStack* attacker, BattleHex destinationTile, BattleHex attackerPos = BattleHex::INVALID); //calculates range of multi-hex attacks
	//std::set<BattleHex> getAttackedHexes(const CStack* attacker, BattleHex destinationTile, BattleHex attackerPos = BattleHex::INVALID); //calculates range of multi-hex attacks
	static int calculateSpellDuration(const CSpell * spell, const CGHeroInstance * caster, int usedSpellPower);
	CStack * generateNewStack(const CStackInstance &base, bool attackerOwned, int slot, BattleHex position) const; //helper for CGameHandler::setupBattle and spells addign new stacks to the battlefield
	CStack * generateNewStack(const CStackBasicDescriptor &base, bool attackerOwned, int slot, BattleHex position) const; //helper for CGameHandler::setupBattle and spells addign new stacks to the battlefield
	int getIdForNewStack() const; //suggest a currently unused ID that'd suitable for generating a new stack
	//std::pair<const CStack *, BattleHex> getNearestStack(const CStack * closest, boost::logic::tribool attackerOwned) const; //if attackerOwned is indetermnate, returened stack is of any owner; hex is the number of hex we should be looking from; returns (nerarest creature, predecessorHex)
	ui32 calculateHealedHP(const CGHeroInstance * caster, const CSpell * spell, const CStack * stack, const CStack * sacrificedStack = NULL) const;
	ui32 calculateHealedHP(int healedHealth, const CSpell * spell, const CStack * stack) const; //for Archangel
	ui32 calculateHealedHP(const CSpell * spell, int usedSpellPower, int spellSchoolLevel, const CStack * stack) const; //unused
	bool resurrects(TSpell spellid) const; //TODO: move it to spellHandler?
	
	const CGHeroInstance * getHero(int player) const; //returns fighting hero that belongs to given player


	std::vector<ui32> calculateResistedStacks(const CSpell * sp, const CGHeroInstance * caster, const CGHeroInstance * hero2, const std::set<const CStack*> affectedCreatures, int casterSideOwner, ECastingMode::ECastingMode mode, int usedSpellPower, int spellLevel) const;

	const CStack * battleGetStack(BattleHex pos, bool onlyAlive); //returns stack at given tile
	const CGHeroInstance * battleGetOwner(const CStack * stack) const; //returns hero that owns given stack; NULL if none
	void localInit();

	void localInitStack(CStack * s);
	static BattleInfo * setupBattle( int3 tile, int terrain, int battlefieldType, const CArmedInstance *armies[2], const CGHeroInstance * heroes[2], bool creatureBank, const CGTownInstance *town );
	//bool hasNativeStack(ui8 side) const;

	int theOtherPlayer(int player) const;
	ui8 whatSide(int player) const;

	static int battlefieldTypeToBI(int bfieldType); //converts above to ERM BI format
	static int battlefieldTypeToTerrain(int bfieldType); //converts above to ERM BI format
};

class DLL_LINKAGE CStack : public CBonusSystemNode, public CStackBasicDescriptor
{ 
public:
	const CStackInstance *base; //garrison slot from which stack originates (NULL for war machines, summoned cres, etc)

	ui32 ID; //unique ID of stack
	ui32 baseAmount;
	ui32 firstHPleft; //HP of first creature in stack
	ui8 owner, slot;  //owner - player colour (255 for neutrals), slot - position in garrison (may be 255 for neutrals/called creatures)
	ui8 attackerOwned; //if true, this stack is owned by attakcer (this one from left hand side of battle)
	BattleHex position; //position on battlefield; -2 - keep, -3 - lower tower, -4 - upper tower
	ui8 counterAttacks; //how many counter attacks can be performed more in this turn (by default set at the beginning of the round to 1)
	si16 shots; //how many shots left
	ui8 casts; //how many casts left

	std::set<EBattleStackState::EBattleStackState> state;
	//overrides
	const CCreature* getCreature() const {return type;}

	CStack(const CStackInstance *base, int O, int I, bool AO, int S); //c-tor
	CStack(const CStackBasicDescriptor *stack, int O, int I, bool AO, int S = 255); //c-tor
	CStack(); //c-tor
	~CStack();
	std::string nodeName() const OVERRIDE;

	void init(); //set initial (invalid) values
	void postInit(); //used to finish initialization when inheriting creature parameters is working
	std::string getName() const; //plural or singular
	bool willMove(int turn = 0) const; //if stack has remaining move this turn
	bool ableToRetaliate() const; //if stack can retaliate after attacked
	bool moved(int turn = 0) const; //if stack was already moved this turn
	bool waited(int turn = 0) const;
	bool canMove(int turn = 0) const; //if stack can move
	bool canBeHealed() const; //for first aid tent - only harmed stacks that are not war machines
	ui32 Speed(int turn = 0, bool useBind = false) const; //get speed of creature with all modificators
	si32 magicResistance() const; //include aura of resistance
	static void stackEffectToFeature(std::vector<Bonus> & sf, const Bonus & sse);
	std::vector<si32> activeSpells() const; //returns vector of active spell IDs sorted by time of cast
	const CGHeroInstance *getMyHero() const; //if stack belongs to hero (directly or was by him summoned) returns hero, NULL otherwise

	static inline Bonus featureGenerator(Bonus::BonusType type, si16 subtype, si32 value, ui16 turnsRemain, si32 additionalInfo = 0, si32 limit = Bonus::NO_LIMIT)
	{
		Bonus hb = makeFeatureVal(type, Bonus::N_TURNS, subtype, value, Bonus::SPELL_EFFECT, turnsRemain, additionalInfo);
		hb.effectRange = limit;
		hb.source = Bonus::SPELL_EFFECT;
		return hb;
	}

	static inline Bonus featureGeneratorVT(Bonus::BonusType type, si16 subtype, si32 value, ui16 turnsRemain, ui8 valType)
	{
		Bonus ret = makeFeatureVal(type, Bonus::N_TURNS, subtype, value, Bonus::SPELL_EFFECT, turnsRemain);
		ret.valType = valType;
		ret.source = Bonus::SPELL_EFFECT;
		return ret;
	}

	static bool isMeleeAttackPossible(const CStack * attacker, const CStack * defender, BattleHex attackerPos = BattleHex::INVALID, BattleHex defenderPos = BattleHex::INVALID);

	bool doubleWide() const;
	BattleHex occupiedHex() const; //returns number of occupied hex (not the position) if stack is double wide; otherwise -1
	BattleHex occupiedHex(BattleHex assumedPos) const; //returns number of occupied hex (not the position) if stack is double wide and would stand on assumedPos; otherwise -1
	std::vector<BattleHex> getHexes() const; //up to two occupied hexes, starting from front
	std::vector<BattleHex> getHexes(BattleHex assumedPos) const; //up to two occupied hexes, starting from front
	static std::vector<BattleHex> getHexes(BattleHex assumedPos, bool twoHex, bool AttackerOwned); //up to two occupied hexes, starting from front
	bool coversPos(BattleHex position) const; //checks also if unit is double-wide
	std::vector<BattleHex> getSurroundingHexes(BattleHex attackerPos = BattleHex::INVALID) const; // get six or 8 surrounding hexes depending on creature size

	void prepareAttacked(BattleStackAttacked &bsa) const; //requires bsa.damageAmout filled

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		assert(isIndependentNode());
		h & static_cast<CBonusSystemNode&>(*this);
		h & static_cast<CStackBasicDescriptor&>(*this);
		h & ID & baseAmount & firstHPleft & owner & slot & attackerOwned & position & state & counterAttacks
			& shots & casts & count;

		const CArmedInstance *army = (base ? base->armyObj : NULL);
		TSlot slot = (base ? base->armyObj->findStack(base) : -1);

		if(h.saving)
		{
			h & army & slot;
		}
		else
		{
			h & army & slot;
			if (slot == -2) //TODO
			{
				auto hero = dynamic_cast<const CGHeroInstance *>(army);
				assert (hero);
				base = hero->commander;
			}
			else if(!army || slot == -1 || !army->hasStackAtSlot(slot))
			{
				base = NULL;
				tlog3 << type->nameSing << " doesn't have a base stack!\n";
			}
			else
			{
				base = &army->getStack(slot);
			}
		}

	}
	bool alive() const //determines if stack is alive
	{
		return vstd::contains(state,EBattleStackState::ALIVE);
	}
	bool isValidTarget(bool allowDead = false) const; //alive non-turret stacks (can be attacked or be object of magic effect)
};

class DLL_LINKAGE CMP_stack
{
	int phase; //rules of which phase will be used
	int turn;
public:

	bool operator ()(const CStack* a, const CStack* b);
	CMP_stack(int Phase = 1, int Turn = 0);
};
