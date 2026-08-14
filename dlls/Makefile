#
# Half-Life Full SDK 2.3 hl_i386.so Makefile for x86 Linux
#
# October 2002 by Leon Hartwig (hartwig@valvesoftware.com)
#

DLLNAME=hl

ARCH=i386

#make sure this is the correct compiler for your system
CC=gcc

DLL_SRCDIR=.
CORE_SRCDIR=./core
ENGINE_SRCDIR=../engine
COMMON_SRCDIR=../common
WPN_SHARED_SRCDIR=./wpn_shared
PM_SHARED_SRCDIR=../pm_shared
GAME_SHARED_SRCDIR=../game_shared

DLL_OBJDIR=$(DLL_SRCDIR)/obj
SYSTEMS_SRCDIR=./systems
SYSTEMS_OBJDIR=$(SYSTEMS_SRCDIR)/obj
CORE_OBJDIR=$(CORE_SRCDIR)/obj
WPN_SHARED_OBJDIR=$(WPN_SHARED_SRCDIR)/obj
PM_SHARED_OBJDIR=$(PM_SHARED_SRCDIR)/obj
GAME_SHARED_OBJDIR=$(GAME_SHARED_SRCDIR)/obj

BASE_CFLAGS=	-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp \
		-DCLIENT_WEAPONS

#safe optimization
CFLAGS=$(BASE_CFLAGS) -w -O1

#full optimization
#CFLAGS=$(BASE_CFLAGS) -w -O1 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations \
	-malign-loops=2 -malign-jumps=2 -malign-functions=2

#use these when debugging
#CFLAGS=$(BASE_CFLAGS) -g

INCLUDEDIRS=-I. -I$(CORE_SRCDIR) -I$(ENGINE_SRCDIR) -I../public -I$(COMMON_SRCDIR) -I$(PM_SHARED_SRCDIR) -I$(GAME_SHARED_SRCDIR)

LDFLAGS=

SHLIBEXT=so
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) $(INCLUDEDIRS) -o $@ -c $<

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

$(DLL_OBJDIR)/%.o: $(DLL_SRCDIR)/%.cpp
	$(DO_CC)

$(SYSTEMS_OBJDIR)/%.o: $(SYSTEMS_SRCDIR)/%.cpp
	$(DO_CC)

$(CORE_OBJDIR)/%.o: $(CORE_SRCDIR)/%.cpp
	$(DO_CC)

$(WPN_SHARED_OBJDIR)/%.o: $(WPN_SHARED_SRCDIR)/%.cpp
	$(DO_CC)

$(GAME_SHARED_OBJDIR)/%.o: $(GAME_SHARED_SRCDIR)/%.cpp
	$(DO_CC)

$(PM_SHARED_OBJDIR)/%.o: $(PM_SHARED_SRCDIR)/%.c
	$(DO_CC)

OBJ = \
	$(DLL_OBJDIR)/monsters/aflock.o \
	$(DLL_OBJDIR)/monsters/agrunt.o \
	$(DLL_OBJDIR)/airtank.o \
	$(DLL_OBJDIR)/animating.o \
	$(DLL_OBJDIR)/animation.o \
	$(DLL_OBJDIR)/monsters/apache.o \
	$(DLL_OBJDIR)/monsters/barnacle.o \
	$(DLL_OBJDIR)/monsters/barney.o \
	$(DLL_OBJDIR)/monsters/bigmomma.o \
	$(DLL_OBJDIR)/monsters/bloater.o \
	$(DLL_OBJDIR)/bmodels.o \
	$(DLL_OBJDIR)/monsters/bullsquid.o \
	$(DLL_OBJDIR)/buttons.o \
	$(CORE_OBJDIR)/cbase.o \
	$(DLL_OBJDIR)/client.o \
	$(DLL_OBJDIR)/combat.o \
	$(DLL_OBJDIR)/monsters/controller.o \
	$(DLL_OBJDIR)/crossbow.o \
	$(DLL_OBJDIR)/weapons/weapon_crowbar.o \
	$(DLL_OBJDIR)/ai/defaultai.o \
	$(DLL_OBJDIR)/doors.o \
	$(DLL_OBJDIR)/effects.o \
	$(DLL_OBJDIR)/weapons/weapon_egon.o \
	$(DLL_OBJDIR)/explode.o \
	$(DLL_OBJDIR)/flyingmonster.o \
	$(DLL_OBJDIR)/func_break.o \
	$(DLL_OBJDIR)/systems/func_tank.o \
	$(DLL_OBJDIR)/systems/func_tankgun.o \
	$(DLL_OBJDIR)/systems/func_tanklaser.o \
	$(DLL_OBJDIR)/systems/func_tankrocket.o \
	$(DLL_OBJDIR)/systems/func_tankmortar.o \
	$(DLL_OBJDIR)/game.o \
	$(DLL_OBJDIR)/gameplay/gamerules.o \
	$(DLL_OBJDIR)/monsters/gargantua.o \
	$(DLL_OBJDIR)/weapons/weapon_gauss.o \
	$(DLL_OBJDIR)/monsters/genericmonster.o \
	$(DLL_OBJDIR)/weapons/projectile_grenade.o \
	$(CORE_OBJDIR)/globals.o \
	$(DLL_OBJDIR)/monsters/gman.o \
	$(DLL_OBJDIR)/h_ai.o \
	$(DLL_OBJDIR)/h_battery.o \
	$(DLL_OBJDIR)/h_cine.o \
	$(DLL_OBJDIR)/h_cycler.o \
	$(DLL_OBJDIR)/h_export.o \
	$(DLL_OBJDIR)/weapons/weapon_handgrenade.o \
	$(DLL_OBJDIR)/monsters/hassassin.o \
	$(DLL_OBJDIR)/monsters/headcrab.o \
	$(DLL_OBJDIR)/healthkit.o \
	$(DLL_OBJDIR)/monsters/hgrunt.o \
	$(DLL_OBJDIR)/hornet.o \
	$(DLL_OBJDIR)/hornetgun.o \
	$(DLL_OBJDIR)/monsters/houndeye.o \
	$(DLL_OBJDIR)/monsters/ichthyosaur.o \
	$(DLL_OBJDIR)/monsters/islave.o \
	$(DLL_OBJDIR)/items/item_base.o \
	$(DLL_OBJDIR)/items/item_battery.o \
	$(DLL_OBJDIR)/items/item_suit.o \
	$(DLL_OBJDIR)/items/item_antidote.o \
	$(DLL_OBJDIR)/items/item_security.o \
	$(DLL_OBJDIR)/items/item_longjump.o \
	$(DLL_OBJDIR)/monsters/leech.o \
	$(DLL_OBJDIR)/lights.o \
	$(DLL_OBJDIR)/gameplay/maprules.o \
	$(DLL_OBJDIR)/gameplay/game_score.o \
	$(DLL_OBJDIR)/gameplay/game_end.o \
	$(DLL_OBJDIR)/gameplay/game_text.o \
	$(DLL_OBJDIR)/gameplay/game_team_master.o \
	$(DLL_OBJDIR)/gameplay/game_team_set.o \
	$(DLL_OBJDIR)/gameplay/game_player_zone.o \
	$(DLL_OBJDIR)/gameplay/game_player_hurt.o \
	$(DLL_OBJDIR)/gameplay/game_counter.o \
	$(DLL_OBJDIR)/gameplay/game_counter_set.o \
	$(DLL_OBJDIR)/gameplay/game_player_equip.o \
	$(DLL_OBJDIR)/gameplay/game_player_team.o \
	$(DLL_OBJDIR)/monstermaker.o \
	$(DLL_OBJDIR)/ai/monsters.o \
	$(DLL_OBJDIR)/monsterstate.o \
	$(DLL_OBJDIR)/mortar.o \
	$(DLL_OBJDIR)/weapons/weapon_mp5.o \
	$(DLL_OBJDIR)/gameplay/multiplay_gamerules.o \
	$(DLL_OBJDIR)/monsters/nihilanth.o \
	$(DLL_OBJDIR)/monsters/osprey.o \
	$(DLL_OBJDIR)/ai/nodes.o \
	$(DLL_OBJDIR)/pathcorner.o \
	$(DLL_OBJDIR)/plane.o \
	$(DLL_OBJDIR)/plats.o \
	$(CORE_OBJDIR)/player.o \
	$(DLL_OBJDIR)/weapons/weapon_spraycan.o \
	$(DLL_OBJDIR)/monsters/monster_deadhev.o \
	$(DLL_OBJDIR)/gameplay/stripweapons.o \
	$(DLL_OBJDIR)/systems/revertsaved.o \
	$(DLL_OBJDIR)/systems/info_intermission.o \
	$(DLL_OBJDIR)/weapons/weapon_python.o \
	$(DLL_OBJDIR)/monsters/rat.o \
	$(DLL_OBJDIR)/monsters/roach.o \
	$(DLL_OBJDIR)/weapons/weapon_rpg.o \
	$(DLL_OBJDIR)/weapons/weapon_satchel.o \
	$(DLL_OBJDIR)/gameplay/scripted.o \
	$(DLL_OBJDIR)/monsters/scientist.o \
	$(DLL_OBJDIR)/ai/schedule.o \
	$(DLL_OBJDIR)/weapons/weapon_shotgun.o \
	$(DLL_OBJDIR)/gameplay/singleplay_gamerules.o \
	$(DLL_OBJDIR)/skill.o \
	$(DLL_OBJDIR)/sound.o \
	$(DLL_OBJDIR)/soundent.o \
	$(DLL_OBJDIR)/spectator.o \
	$(DLL_OBJDIR)/ai/squadmonster.o \
	$(DLL_OBJDIR)/weapons/weapon_snark.o \
	$(DLL_OBJDIR)/subs.o \
	$(DLL_OBJDIR)/talkmonster.o \
	$(DLL_OBJDIR)/gameplay/teamplay_gamerules.o \
	$(DLL_OBJDIR)/tempmonster.o \
	$(DLL_OBJDIR)/systems/trigger_relay.o \
	$(DLL_OBJDIR)/systems/trigger_camera.o \
	$(DLL_OBJDIR)/systems/trigger_base.o \
	$(DLL_OBJDIR)/systems/trigger_cdaudio.o \
	$(DLL_OBJDIR)/systems/trigger_save.o \
	$(DLL_OBJDIR)/systems/trigger_teleport.o \
	$(DLL_OBJDIR)/systems/trigger_endsection.o \
	$(DLL_OBJDIR)/systems/trigger_counter.o \
	$(DLL_OBJDIR)/systems/trigger_friction.o \
	$(DLL_OBJDIR)/systems/trigger_ladder.o \
	$(DLL_OBJDIR)/systems/trigger_gravity.o \
	$(DLL_OBJDIR)/systems/trigger_render_fx.o \
	$(DLL_OBJDIR)/systems/trigger_multi_manager.o \
	$(DLL_OBJDIR)/systems/trigger_monsterjump.o \
	$(DLL_OBJDIR)/systems/trigger_multiple.o \
	$(DLL_OBJDIR)/systems/trigger_changetarget.o \
	$(DLL_OBJDIR)/systems/trigger_push.o \
	$(DLL_OBJDIR)/systems/trigger_once.o \
	$(DLL_OBJDIR)/systems/trigger_target_cdaudio.o \
	$(DLL_OBJDIR)/systems/trigger_changelevel.o \
	$(DLL_OBJDIR)/systems/trigger_hurt.o \
	$(DLL_OBJDIR)/systems/trigger_auto.o \
	$(DLL_OBJDIR)/systems/trigger_fireanddie.o \
	$(DLL_OBJDIR)/systems/trigger_volume.o \
	$(DLL_OBJDIR)/monsters/tentacle.o \
	$(DLL_OBJDIR)/tripmine.o \
	$(SYSTEMS_OBJDIR)/turret.o \
	$(SYSTEMS_OBJDIR)/turret_base.o \
	$(SYSTEMS_OBJDIR)/miniturret.o \
	$(SYSTEMS_OBJDIR)/sentry.o \
	$(CORE_OBJDIR)/util.o \
	$(DLL_OBJDIR)/weapons.o \
	$(DLL_OBJDIR)/weapons/weapon_base.o \
	$(DLL_OBJDIR)/world.o \
	$(DLL_OBJDIR)/xen.o \
	$(DLL_OBJDIR)/monsters/zombie.o \
	$(DLL_OBJDIR)/weapons/weapon_glock.o \
	$(GAME_SHARED_OBJDIR)/voice_gamemgr.o \
	$(PM_SHARED_OBJDIR)/pm_debug.o \
	$(PM_SHARED_OBJDIR)/pm_math.o \
	$(PM_SHARED_OBJDIR)/pm_shared.o

$(DLLNAME)_$(ARCH).$(SHLIBEXT) : neat $(OBJ)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) $(LDFLAGS) -o $@ $(OBJ)

neat:
	-mkdir $(SYSTEMS_OBJDIR)
	-mkdir $(DLL_OBJDIR)
	-mkdir $(DLL_OBJDIR)/gameplay
	-mkdir $(DLL_OBJDIR)/weapons
	-mkdir $(DLL_OBJDIR)/items
	-mkdir $(DLL_OBJDIR)/ai
	-mkdir $(CORE_OBJDIR)
	-mkdir $(GAME_SHARED_OBJDIR)
	-mkdir $(PM_SHARED_OBJDIR)
	-mkdir $(DLL_OBJDIR)/weapons
	-mkdir $(DLL_OBJDIR)/monsters
	-mkdir $(DLL_OBJDIR)/gameplay
	-mkdir $(DLL_OBJDIR)/systems
clean:
	-rm -f $(OBJ)
	-rm -f $(DLLNAME)_$(ARCH).$(SHLIBEXT)
spotless: clean
	-rm -r $(DLL_OBJDIR)
	-rm -r $(SYSTEMS_OBJDIR)
	-rm -r $(CORE_OBJDIR)
	-rm -r $(GAME_SHARED_OBJDIR)
	-rm -r $(PM_SHARED_OBJDIR)


$(DLL_OBJDIR)/weapons/weapon_glock.o: weapons/weapon_glock.cpp
	$(DO_CC)

$(DLL_OBJDIR)/weapons/weapon_egon.o: weapons/weapon_egon.cpp
	$(DO_CC)

$(DLL_OBJDIR)/weapons/weapon_handgrenade.o: weapons/weapon_handgrenade.cpp
	$(DO_CC)

$(DLL_OBJDIR)/weapons/weapon_gauss.o: weapons/weapon_gauss.cpp
	$(DO_CC)

$(DLL_OBJDIR)/weapons/weapon_rpg.o: weapons/weapon_rpg.cpp
	$(DO_CC)

$(DLL_OBJDIR)/weapons/weapon_satchel.o: weapons/weapon_satchel.cpp
	$(DO_CC)

$(DLL_OBJDIR)/weapons/weapon_shotgun.o: weapons/weapon_shotgun.cpp
	$(DO_CC)

$(DLL_OBJDIR)/weapons/weapon_base.o: weapons/weapon_base.cpp
	$(DO_CC)

$(DLL_OBJDIR)/weapons/weapon_spraycan.o: weapons/weapon_spraycan.cpp
	$(DO_CC)

$(DLL_OBJDIR)/weapons/weapon_python.o: $(DLL_SRCDIR)/weapons/weapon_python.cpp
	$(DO_CC)

$(DLL_OBJDIR)/monsters/%.o: $(DLL_SRCDIR)/monsters/%.cpp
	$(DO_CC)

$(DLL_OBJDIR)/monsters/monster_deadhev.o: monsters/monster_deadhev.cpp
	$(DO_CC)

$(DLL_OBJDIR)/gameplay/stripweapons.o: gameplay/stripweapons.cpp
	$(DO_CC)

$(DLL_OBJDIR)/systems/revertsaved.o: systems/revertsaved.cpp
	$(DO_CC)

$(DLL_OBJDIR)/systems/info_intermission.o: systems/info_intermission.cpp
	$(DO_CC)

$(DLL_OBJDIR)/weapons/weapon_crowbar.o: weapons/weapon_crowbar.cpp
	$(DO_CC)
