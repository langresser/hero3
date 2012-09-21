#include "StdInc.h"
#include "CCampaignHandler.h"

#include "Filesystem/CResourceLoader.h"
#include "Filesystem/CCompressedStream.h"
#include "../lib/VCMI_Lib.h"
#include "../lib/vcmi_endian.h"
#include "CGeneralTextHandler.h"
#include "StartInfo.h"
#include "CArtHandler.h" //for hero crossover
#include "CObjectHandler.h" //for hero crossover
#include "CHeroHandler.h"

namespace fs = boost::filesystem;

/*
 * CCampaignHandler.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */


CCampaignHeader CCampaignHandler::getHeader( const std::string & name)
{
	std::vector<ui8> cmpgn = getFile(name, true)[0];

	int it = 0;//iterator for reading
	CCampaignHeader ret = readHeaderFromMemory(cmpgn.data(), it);
	ret.filename = name;

	return ret;
}

CCampaign * CCampaignHandler::getCampaign( const std::string & name)
{
	CCampaign * ret = new CCampaign();

	std::vector<std::vector<ui8>> file = getFile(name, false);

	int it = 0; //iterator for reading
	ret->header = readHeaderFromMemory(file[0].data(), it);
	ret->header.filename = name;

	int howManyScenarios = VLC->generaltexth->campaignRegionNames[ret->header.mapVersion].size();
	for(int g=0; g<howManyScenarios; ++g)
	{
		CCampaignScenario sc = readScenarioFromMemory(file[0].data(), it, ret->header.version, ret->header.mapVersion);
		ret->scenarios.push_back(sc);
	}


	int scenarioID = 0;

	//first entry is campaign header. start loop from 1
	for (int g=1; g<file.size() && g<howManyScenarios; ++g)
	{
		while(!ret->scenarios[scenarioID].isNotVoid()) //skip void scenarios
		{
			scenarioID++;
		}
		//set map piece appropriately
		ret->mapPieces[scenarioID++] = file[g];
	}

	return ret;
}

CCampaignHeader CCampaignHandler::readHeaderFromMemory( const ui8 *buffer, int & outIt )
{
	CCampaignHeader ret;
	ret.version = read_le_u32(buffer + outIt); outIt+=4;
	ret.mapVersion = buffer[outIt++]; //1 byte only
	ret.mapVersion -= 1; //change range of it from [1, 20] to [0, 19]
	ret.name = readString(buffer, outIt);
	ret.description = readString(buffer, outIt);
	if (ret.version > CampaignVersion::RoE)
		ret.difficultyChoosenByPlayer = readChar(buffer, outIt);
	else
		ret.difficultyChoosenByPlayer = 0;
	ret.music = readChar(buffer, outIt);
	return ret;
}

CCampaignScenario CCampaignHandler::readScenarioFromMemory( const ui8 *buffer, int & outIt, int version, int mapVersion )
{
	struct HLP
	{
		//reads prolog/epilog info from memory
		static CCampaignScenario::SScenarioPrologEpilog prologEpilogReader( const ui8 *buffer, int & outIt )
		{
			CCampaignScenario::SScenarioPrologEpilog ret;
			ret.hasPrologEpilog = buffer[outIt++];
			if(ret.hasPrologEpilog)
			{
				ret.prologVideo = buffer[outIt++];
				ret.prologMusic = buffer[outIt++];
				ret.prologText = readString(buffer, outIt);
			}
			return ret;
		}
	};
	CCampaignScenario ret;
	ret.conquered = false;
	ret.mapName = readString(buffer, outIt);
	ret.packedMapSize = read_le_u32(buffer + outIt); outIt += 4;
	if(mapVersion == 18)//unholy alliance
	{
		ret.loadPreconditionRegions(read_le_u16(buffer + outIt)); outIt += 2;
	}
	else
	{
		ret.loadPreconditionRegions(buffer[outIt++]);
	}
	ret.regionColor = buffer[outIt++];
	ret.difficulty = buffer[outIt++];
	ret.regionText = readString(buffer, outIt);
	ret.prolog = HLP::prologEpilogReader(buffer, outIt);
	ret.epilog = HLP::prologEpilogReader(buffer, outIt);

	ret.travelOptions = readScenarioTravelFromMemory(buffer, outIt, version);

	return ret;
}

void CCampaignScenario::loadPreconditionRegions(ui32 regions)
{
	for (int i=0; i<32; i++) //for each bit in region. h3c however can only hold up to 16
	{
		if ( (1 << i) & regions)
			preconditionRegions.insert(i);
	}
}

CScenarioTravel CCampaignHandler::readScenarioTravelFromMemory( const ui8 * buffer, int & outIt , int version )
{
	CScenarioTravel ret;

	ret.whatHeroKeeps = buffer[outIt++];
	memcpy(ret.monstersKeptByHero, buffer+outIt, ARRAY_COUNT(ret.monstersKeptByHero));
	outIt += ARRAY_COUNT(ret.monstersKeptByHero);
	int artifBytes;
	if (version < CampaignVersion::SoD)
	{
		artifBytes = 17;
		ret.artifsKeptByHero[17] = 0;
	} 
	else
	{
		artifBytes = 18;
	}
	memcpy(ret.artifsKeptByHero, buffer+outIt, artifBytes);
	outIt += artifBytes;

	ret.startOptions = buffer[outIt++];
	
	switch(ret.startOptions)
	{
	case 0:
		//no bonuses. Seems to be OK
		break;
	case 1: //reading of bonuses player can choose
		{
			ret.playerColor = buffer[outIt++];
			ui8 numOfBonuses = buffer[outIt++];
			for (int g=0; g<numOfBonuses; ++g)
			{
				CScenarioTravel::STravelBonus bonus;
				bonus.type = buffer[outIt++];
				//hero: FFFD means 'most powerful' and FFFE means 'generated'
				switch(bonus.type)
				{
				case 0: //spell
					{
						bonus.info1 = read_le_u16(buffer + outIt); outIt += 2; //hero
						bonus.info2 = buffer[outIt++]; //spell ID
						break;
					}
				case 1: //monster
					{
						bonus.info1 = read_le_u16(buffer + outIt); outIt += 2; //hero
						bonus.info2 = read_le_u16(buffer + outIt); outIt += 2; //monster type
						bonus.info3 = read_le_u16(buffer + outIt); outIt += 2; //monster count
						break;
					}
				case 2: //building
					{
						bonus.info1 = buffer[outIt++]; //building ID (0 - town hall, 1 - city hall, 2 - capitol, etc)
						break;
					}
				case 3: //artifact
					{
						bonus.info1 = read_le_u16(buffer + outIt); outIt += 2; //hero
						bonus.info2 = read_le_u16(buffer + outIt); outIt += 2; //artifact ID
						break;
					}
				case 4: //spell scroll
					{
						bonus.info1 = read_le_u16(buffer + outIt); outIt += 2; //hero
						bonus.info2 = buffer[outIt++]; //spell ID
						break;
					}
				case 5: //prim skill
					{
						bonus.info1 = read_le_u16(buffer + outIt); outIt += 2; //hero
						bonus.info2 = read_le_u32(buffer + outIt); outIt += 4; //bonuses (4 bytes for 4 skills)
						break;
					}
				case 6: //sec skills
					{
						bonus.info1 = read_le_u16(buffer + outIt); outIt += 2; //hero
						bonus.info2 = buffer[outIt++]; //skill ID
						bonus.info3 = buffer[outIt++]; //skill level
						break;
					}
				case 7: //resources
					{
						bonus.info1 = buffer[outIt++]; //type
						//FD - wood+ore
						//FE - mercury+sulfur+crystal+gem
						bonus.info2 = read_le_u32(buffer + outIt); outIt += 4; //count
						break;
					}
				}
				ret.bonusesToChoose.push_back(bonus);
			}
			break;
		}
	case 2: //reading of players (colors / scenarios ?) player can choose
		{
			ui8 numOfBonuses = buffer[outIt++];
			for (int g=0; g<numOfBonuses; ++g)
			{
				CScenarioTravel::STravelBonus bonus;
				bonus.type = 8;
				bonus.info1 = buffer[outIt++]; //player color
				bonus.info2 = buffer[outIt++]; //from what scenario

				ret.bonusesToChoose.push_back(bonus);
			}
			break;
		}
	case 3: //heroes player can choose between
		{
			ui8 numOfBonuses = buffer[outIt++];
			for (int g=0; g<numOfBonuses; ++g)
			{
				CScenarioTravel::STravelBonus bonus;
				bonus.type = 9;
				bonus.info1 = buffer[outIt++]; //player color
				bonus.info2 = read_le_u16(buffer + outIt); outIt += 2; //hero, FF FF - random

				ret.bonusesToChoose.push_back(bonus);
			}
			break;
		}
	default:
		{
			tlog1<<"Corrupted h3c file"<<std::endl;
			break;
		}
	}

	return ret;
}

std::vector< std::vector<ui8> > CCampaignHandler::getFile(const std::string & name, bool headerOnly)
{
	CCompressedStream stream(std::move(CResourceHandler::get()->load(ResourceID(name, EResType::CAMPAIGN))), true);

	std::vector< std::vector<ui8> > ret;
	do
	{
		std::vector<ui8> block(stream.getSize());
		stream.read(block.data(), block.size());
		ret.push_back(block);
	}
	while (!headerOnly && stream.getNextBlock());

	return ret;
}

bool CCampaign::conquerable( int whichScenario ) const
{
	//check for void scenraio
	if (!scenarios[whichScenario].isNotVoid())
	{
		return false;
	}

	if (scenarios[whichScenario].conquered)
	{
		return false;
	}
	//check preconditioned regions
	for (int g=0; g<scenarios.size(); ++g)
	{
		if( vstd::contains(scenarios[whichScenario].preconditionRegions, g) && !scenarios[g].conquered)
			return false; //prerequisite does not met
			
	}
	return true;
}

CCampaign::CCampaign()
{

}

bool CCampaignScenario::isNotVoid() const
{
	return mapName.size() > 0;
}

void CCampaignScenario::prepareCrossoverHeroes( std::vector<CGHeroInstance *> heroes )
{
	crossoverHeroes = heroes;

	
	if (!(travelOptions.whatHeroKeeps & 1))
	{
		//trimming experience
		BOOST_FOREACH(CGHeroInstance * cgh, crossoverHeroes)
		{
			cgh->initExp();
		}
	}
	if (!(travelOptions.whatHeroKeeps & 2))
	{
		//trimming prim skills
		BOOST_FOREACH(CGHeroInstance * cgh, crossoverHeroes)
		{
#define RESET_PRIM_SKILL(NAME, VALNAME) \
			cgh->getBonusLocalFirst(Selector::type(Bonus::PRIMARY_SKILL) && \
				Selector::subtype(PrimarySkill::NAME) && \
				Selector::sourceType(Bonus::HERO_BASE_SKILL) )->val = cgh->type->heroClass->VALNAME;

			RESET_PRIM_SKILL(ATTACK, initialAttack);
			RESET_PRIM_SKILL(DEFENSE, initialDefence);
			RESET_PRIM_SKILL(SPELL_POWER, initialPower);
			RESET_PRIM_SKILL(KNOWLEDGE, initialKnowledge);
#undef RESET_PRIM_SKILL
		}
	}
	if (!(travelOptions.whatHeroKeeps & 4))
	{
		//trimming sec skills
		BOOST_FOREACH(CGHeroInstance * cgh, crossoverHeroes)
		{
			cgh->secSkills = cgh->type->secSkillsInit;
		}
	}
	if (!(travelOptions.whatHeroKeeps & 8))
	{
		//trimming spells
		BOOST_FOREACH(CGHeroInstance * cgh, crossoverHeroes)
		{
			cgh->spells.clear();
		}
	}
	if (!(travelOptions.whatHeroKeeps & 16))
	{
		//trimming artifacts
		BOOST_FOREACH(CGHeroInstance * hero, crossoverHeroes)
		{
			size_t totalArts = GameConstants::BACKPACK_START + hero->artifactsInBackpack.size();
			for (size_t i=0; i<totalArts; i++ )
			{
				const ArtSlotInfo *info = hero->getSlot(i);
				if (!info)
					continue;
				
				const CArtifactInstance *art = info->artifact;
				if (!art)//FIXME: check spellbook and catapult behaviour
					continue;

				int id  = art->artType->id;
				assert( 8*18 > id );//number of arts that fits into h3m format
				bool takeable = travelOptions.artifsKeptByHero[id / 8] & ( 1 << (id%8) );

				if (takeable)
					hero->eraseArtSlot(i);
			}
		}
	}

	//trimming creatures
	BOOST_FOREACH(CGHeroInstance * cgh, crossoverHeroes)
	{
		for (TSlots::const_iterator j = cgh->Slots().begin(); j != cgh->Slots().end(); j++)
		{
			if (!(travelOptions.monstersKeptByHero[j->first / 8] & (1 << (j->first%8)) ))
			{
				cgh->eraseStack(j->first);
				j = cgh->Slots().begin();
			}
		}
	}
}

bool CScenarioTravel::STravelBonus::isBonusForHero() const
{
	return type == 0 || type == 1 || type == 3 || type == 4 || type == 5 || type == 6;
}

void CCampaignState::initNewCampaign( const StartInfo &si )
{
	assert(si.mode == StartInfo::CAMPAIGN);
	campaignName = si.mapname;
	currentMap = si.campSt->currentMap;

	camp = CCampaignHandler::getCampaign(campaignName);
	for (ui8 i = 0; i < camp->mapPieces.size(); i++)
		mapsRemaining.push_back(i);
}

void CCampaignState::mapConquered( const std::vector<CGHeroInstance*> & heroes )
{
	camp->scenarios[currentMap].prepareCrossoverHeroes(heroes);
	mapsConquered.push_back(currentMap);
	mapsRemaining -= currentMap;
	camp->scenarios[currentMap].conquered = true;
}
