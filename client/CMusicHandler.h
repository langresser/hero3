#pragma once

#include "CConfigHandler.h"
#include "CSoundBase.h"
#include "../lib/CCreatureHandler.h"

/*
 * CMusicHandler.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

class CSpell;
struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;
struct Mix_Chunk;


// Sound infos for creatures in combat
struct CreaturesBattleSounds {
	soundBase::soundID attack;
	soundBase::soundID defend;
	soundBase::soundID killed; // was killed or died
	soundBase::soundID move;
	soundBase::soundID shoot; // range attack
	soundBase::soundID wince; // attacked but did not die
	soundBase::soundID ext1;  // creature specific extension
	soundBase::soundID ext2;  // creature specific extension
	soundBase::soundID startMoving; // usually same as ext1
	soundBase::soundID endMoving;	// usually same as ext2

	CreaturesBattleSounds(): attack(soundBase::invalid),
							 defend(soundBase::invalid),
							 killed(soundBase::invalid),
							 move(soundBase::invalid),
							 shoot(soundBase::invalid),
							 wince(soundBase::invalid),
							 ext1(soundBase::invalid),
							 ext2(soundBase::invalid),
							 startMoving(soundBase::invalid),
							 endMoving(soundBase::invalid) {};
};

class CAudioBase {
protected:
	bool initialized;
	int volume;					// from 0 (mute) to 100

public:
	CAudioBase(): initialized(false), volume(0) {};
	virtual void init() = 0;
	virtual void release() = 0;

	virtual void setVolume(ui32 percent);
	ui32 getVolume() { return volume; };
};

class CSoundHandler: public CAudioBase
{
private:
	soundBase::soundID getSoundID(const std::string &fileName);
	//update volume on configuration change
	SettingsListener listener;
	void onVolumeChange(const JsonNode &volumeNode);

	std::map<soundBase::soundID, Mix_Chunk *> soundChunks;

	Mix_Chunk *GetSoundChunk(soundBase::soundID soundID);

	//have entry for every currently active channel
	//boost::function will be NULL if callback was not set
	std::map<int, boost::function<void()> > callbacks;

public:
	CSoundHandler();

	void init();
	void release();

	void initCreaturesSounds(const std::vector<ConstTransitivePtr<CCreature> > &creatures);
	void initSpellsSounds(const std::vector< ConstTransitivePtr<CSpell> > &spells);
	void setVolume(ui32 percent);

	// Sounds
	int playSound(soundBase::soundID soundID, int repeats=0);
	int playSoundFromSet(std::vector<soundBase::soundID> &sound_vec);
	void stopSound(int handler);

	void setCallback(int channel, boost::function<void()> function);
	void soundFinishedCallback(int channel);

	std::vector <struct CreaturesBattleSounds> CBattleSounds;
	std::map<const CSpell*, soundBase::soundID> spellSounds;

	// Sets
	std::vector<soundBase::soundID> pickupSounds;
	std::vector<soundBase::soundID> horseSounds;
	std::vector<soundBase::soundID> battleIntroSounds;
};

// Helper
#define battle_sound(creature,what_sound) CCS->soundh->CBattleSounds[(creature)->idNumber].what_sound

class CMusicHandler;

//Class for handling one music file
class MusicEntry
{
	CMusicHandler *owner;
	Mix_Music *music;
	int loop; // -1 = indefinite
	//if not null - set from which music will be randomly selected
	std::string setName;
	std::string currentName;


	void load(std::string musicURI);

public:
	bool isSet(std::string setName);
	bool isTrack(std::string trackName);

	MusicEntry(CMusicHandler *owner, std::string setName, std::string musicURI, bool looped);
	~MusicEntry();

	bool play();
	bool stop(int fade_ms=0);
};

class CMusicHandler: public CAudioBase
{
private:
	// Because we use the SDL music callback, our music variables must
	// be protected
	boost::mutex musicMutex;
	//update volume on configuration change
	SettingsListener listener;
	void onVolumeChange(const JsonNode &volumeNode);

	unique_ptr<MusicEntry> current;
	unique_ptr<MusicEntry> next;

	void queueNext(MusicEntry *queued);

	std::map<std::string, std::map<int, std::string> > musicsSet;
public:
	CMusicHandler();

	/// add entry with URI musicURI in set. Track will have ID musicID
	void addEntryToSet(std::string set, int musicID, std::string musicURI);

	void init();
	void release();
	void setVolume(ui32 percent);

	/// play track by URI, if loop = true music will be looped
	void playMusic(std::string musicURI, bool loop);
	/// play random track from this set
	void playMusicFromSet(std::string musicSet, bool loop);
	/// play specific track from set
	void playMusicFromSet(std::string musicSet, int entryID, bool loop);
	void stopMusic(int fade_ms=1000);
	void musicFinishedCallback(void);

	friend class MusicEntry;
};
