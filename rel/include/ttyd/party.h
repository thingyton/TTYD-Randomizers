#pragma once

#include <cstdint>

namespace ttyd::party {

enum class Party : uint8_t
{
  Goombella = 1,
  Koops,
  Bobbery,
  Yoshi,
  Flurrie,
  Vivian,
  MsMowz,
  Egg,
  Flavio,
  Punio,
  Frankly,
  Gus,
  GoombellaFollower,
  KoopsFollower,
  BobberyFollower,
  YoshiFollower,
  FlurrieFollower,
  VivianFollower,
  MsMowzFollower,
};

extern "C" {

// partySetFamicomMode
// partyShadowOff
// partyUpdateKeyData
// partyClearCont
// partyKeyOff
// partyKeyOn
// partyDisp
// partyPreDisp
// partyMakeDispDir
// partySetCamId
// partyInitCamId
// party_damage_return
// partyRideYoshiMain
// party_ride_yoshi_force_move
// party_force_ride_yoshi
// partyRideMain
// partyRideChk
// allPartyRideOff2
// allPartyRideOff
// unk_JP_US_PAL_029
// allPartyForceRideOn
// getRidePos
// allPartyRideShip
// allPartyRidePlane
// partyDokanEnd
// partyDokanMain
// partyDokanInit
// partyGoodbyeMain
// partyGoodbyeInit
// partyHelloMain
// partyHello
// partyDoWork
// driveParty
// partyMain
// unk_JP_US_PAL_030
// unk_JP_US_PAL_031
// unk_JP_US_PAL_032
// partyChgRunMode
// partyUsePost
// partyReInit
// partyInit
int32_t partyEntry2Pos(Party id, float x, float y, float z);
// partyEntry2Hello
// partyEntry2
// partyEntryMain
// partyGetHeight
uint32_t yoshiSetColor();
// partyKill2
// partyKill
// partyGoodbye
// partyEntryPos
// partyEntry
// partyEntryHello
// partySetForceMove
// partyStop
// partyRun
// partyCtrlOn
// partyCtrlOff
// partyChgPaper
// partyChgPose
// partyChgPoseId
// partyPaperLightOff
// partyPaperOff
// partyPaperOn
// anotherPartyGetPtr
void *partyGetPtr(uint32_t partyId);

}

}