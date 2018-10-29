#include "mod.h"
#include "drawstring.h"
#include "items.h"
#include "buttons.h"

#include <ttyd/fontmgr.h>
#include <ttyd/windowdrv.h>
#include <ttyd/seqdrv.h>
#include <ttyd/string.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mario_party.h>
#include <ttyd/__mem.h>
#include <ttyd/mariost.h>
#include <ttyd/mario.h>
#include <ttyd/dispdrv.h>

#include <cstdio>

extern char *NextMap;
extern bool TransparentTextFlag;
extern char *DisplayBuffer;
extern char *NextBero;
extern int32_t *AllFinalScoresArray;
extern uint32_t TotalFinalScoresCounter;
extern bool FinishedARun;
extern uint8_t FinishedRunCounter;
extern int32_t FinalScore;
extern bool NewFile;
extern bool GameOverChallengeMode;
extern uint32_t seqMainAddress;
extern uint32_t FinalScoresMenuCounter;
extern uint8_t JustDefeatedBoss;
extern bool BossDefeated[22];
extern uint32_t BossScore;
extern uint32_t GSWAddressesStart;
extern bool InGameOver;
extern uint16_t GameOverCount;
extern uint8_t TimesUpCounter;
extern bool ShowScoreSources;
extern uint32_t TimerCount;
extern bool TimerDisabled;
extern bool TimerActive;
extern bool DisablePlayerControl;
extern bool GameOverFlag;
extern uint32_t TitleWorkPointer2;
extern bool RandomizeCoins;
extern bool LZRando;
extern bool LZRandoChallenge;
extern uint8_t HelpMenuCounter;
extern uint8_t PreviousHelpMenuCounterValue;
extern const char *HelpMenuArray[];
extern uint8_t HelpMenuArraySize;

namespace mod {

void startDrawString(uint8_t alpha, uint32_t color, float Scale)
{
  ttyd::fontmgr::FontDrawStart_alpha(alpha);
  ttyd::fontmgr::FontDrawColor(reinterpret_cast<uint8_t *>(&color));
  ttyd::fontmgr::FontDrawEdge();
  ttyd::fontmgr::FontDrawScale(Scale);
}

void drawStringSingleLine(int32_t PosX, int32_t PosY, uint8_t alpha, uint32_t color, const char *String, float Scale)
{
  startDrawString(alpha, color, Scale);
  ttyd::fontmgr::FontDrawString(PosX, PosY, String);
}

void drawStringMultipleLines(int32_t PosX, int32_t PosY, uint8_t alpha, uint32_t color, const char *String, float Scale)
{
  startDrawString(alpha, color, Scale);
  drawstring::drawStringMultiline(PosX, PosY, String);
}

void displayWindow(uint32_t color, int32_t x, int32_t y, int32_t width, int32_t height, int32_t curve)
{
  ttyd::windowdrv::windowDispGX_Waku_col(0, reinterpret_cast<uint8_t *>(&color), x, y, width, height, curve);
}

void Mod::LZRandoDisplayStuff()
{
  ttyd::seqdrv::SeqIndex NextSeq = ttyd::seqdrv::seqGetNextSeq();
  ttyd::seqdrv::SeqIndex Game = ttyd::seqdrv::SeqIndex::kGame;
  ttyd::seqdrv::SeqIndex GameOver = ttyd::seqdrv::SeqIndex::kGameOver;
  
  char *tempNextMap = NextMap; // Prevent NextMap from being loaded in multiple times
  bool dmo_comparison = ttyd::string::strcmp(tempNextMap, "dmo_00") == 0;
  bool title_comparison = ttyd::string::strcmp(tempNextMap, "title") == 0;
  
  // Only display while a file is loaded, and while not in battles
  // Don't display while in the intro or on the title screen
  if ((NextSeq < Game) || (NextSeq > GameOver) || dmo_comparison || title_comparison)
  {
    return;
  }
  
  // Check for user input
  uint32_t ButtonInput = ttyd::system::keyGetButton(0);
  uint16_t TransparentTextCombo = PAD_Z | PAD_DPAD_UP;
  if ((ButtonInput & TransparentTextCombo) == TransparentTextCombo)
  {
    uint32_t ButtonInputTrg = ttyd::system::keyGetButtonTrg(0);
    if (ButtonInputTrg & TransparentTextCombo)
    {
      TransparentTextFlag = !TransparentTextFlag;
    }
  }
  
  uint32_t color = 0xFFFFFFFF;
  int32_t PosX = -232;
  int32_t PosY = -120;
  float Scale = 0.75;
  char *tempDisplayBuffer = DisplayBuffer; // Prevent DisplayBuffer from being loaded in multiple times
  
  sprintf(tempDisplayBuffer,
    "Seq: %lu\nLZ: %s\nMap: %s",
    ttyd::swdrv::swByteGet(0),
    NextBero,
    tempNextMap);
  
  uint8_t alpha;
  if (!TransparentTextFlag)
  {
    alpha = 0xFF;
  }
  else
  {
    alpha = 0x50;
  }
  
  drawStringMultipleLines(PosX, PosY, alpha, color, tempDisplayBuffer, Scale);
}

void adjustAllFinalScoresArray()
{
  // Initialize array if not already initialized
  if (AllFinalScoresArray == nullptr)
  {
    AllFinalScoresArray = new int32_t[1];
  }
  else
  {
    // Create new array with a new spot for the newewst Final Score
    uint32_t Size = TotalFinalScoresCounter;
    int32_t *tempFinalScoresArray = new int32_t[Size + 1];
    
    // Copy the contents of the old array to the new array
    ttyd::system::memcpy_as4(tempFinalScoresArray, AllFinalScoresArray, (Size * sizeof(int32_t)));
    
    // Delete the old array
    delete[] (AllFinalScoresArray);
    
    // Set the new array as the current array
    AllFinalScoresArray = tempFinalScoresArray;
  }
  
  TotalFinalScoresCounter++;
}

void displayAllFinalScores()
{
  if (FinishedARun)
  {
    // Add the newest Final Score to the array
    FinishedARun = false;
    
    if (FinishedRunCounter != 2)
    {
      adjustAllFinalScoresArray();
      AllFinalScoresArray[TotalFinalScoresCounter - 1] = FinalScore;
    }
    
    // Prevent new scores from being added if the player chose to continue playing instead of warping back to the title screen
    if (FinishedRunCounter == 1)
    {
      FinishedRunCounter = 2;
    }
  }
  else if (NewFile && GameOverChallengeMode)
  {
    // The current run was reset, so write 0x80000000
    adjustAllFinalScoresArray();
    AllFinalScoresArray[TotalFinalScoresCounter - 1] = 0x80000000;
  }
  
  ttyd::seqdrv::SeqIndex NextSeq = ttyd::seqdrv::seqGetNextSeq();
  ttyd::seqdrv::SeqIndex Load = ttyd::seqdrv::SeqIndex::kLoad;
  uint32_t seqMainCheck = *reinterpret_cast<uint32_t *>(seqMainAddress + 0x4);
  
  bool Comparisons = (NextSeq == Load) && (seqMainCheck == 4);
  
  // Only run on the file select screen
  if (!Comparisons)
  {
    // Turn the menu off it's still on
    FinalScoresMenuCounter = 0;
    
    return;
  }
  
  // Get input from the player
  uint32_t ButtonInput = ttyd::system::keyGetButton(0);
  uint32_t ButtonInputTrg = ttyd::system::keyGetButtonTrg(0);
  uint16_t OpenFinalScoreListCombo = PAD_L | PAD_Y;
  
  // Display/Remove the menu
  if ((ButtonInput & OpenFinalScoreListCombo) == OpenFinalScoreListCombo)
  {
    if (ButtonInputTrg & OpenFinalScoreListCombo)
    {
      if (FinalScoresMenuCounter == 0)
      {
        // Turn the menu on and set it to the first page
        FinalScoresMenuCounter = 1;
      }
      else
      {
        // Turn the menu off
        FinalScoresMenuCounter = 0;
      }
    }
  }
  
  // Only run if the menu should be displayed
  if (FinalScoresMenuCounter == 0)
  {
    // Menu is not displayed, so exit
    return;
  }
  
  // Set total amount of scores to display in one column
  uint32_t TotalInColumn = 16;
  
  // Set the total amount of columns on one page
  uint32_t TotalColumns = 4;
  
  // Set total amount of scores to display on one page
  uint32_t TotalOnPage = TotalColumns * TotalInColumn;
  
  // Prevent TotalFinalScoresCounter from being loaded multiple times
  uint32_t FinalScoresCounter = TotalFinalScoresCounter;
  
  // Get the total amount of pages
  uint32_t TotalPages;
  if (FinalScoresCounter > 0)
  {
    TotalPages = 1 + ((FinalScoresCounter - 1) / TotalOnPage); // Round up
  }
  else
  {
    TotalPages = 1;
  }
  
  // Check for D-Pad inputs
  if ((ButtonInputTrg & PAD_DPAD_LEFT) == PAD_DPAD_LEFT)
  {
    // Go to the previous page
    FinalScoresMenuCounter--;
    
    if (FinalScoresMenuCounter < 1)
    {
      // Loop around to the last page
      FinalScoresMenuCounter = TotalPages;
    }
  }
  else if ((ButtonInputTrg & PAD_DPAD_RIGHT) == PAD_DPAD_RIGHT)
  {
    // Go to the next page
    FinalScoresMenuCounter++;
    
    if (FinalScoresMenuCounter > TotalPages)
    {
      // Loop around to the first page
      FinalScoresMenuCounter = 1;
    }
  }
  
  // Display the window for the text to go in
  uint32_t MenuColor = 0x000000F4;
  int32_t CoordX = -245;
  int32_t CoordY = 190;
  int32_t Width = 490;
  int32_t Height = 375;
  int32_t Curve = 0;
  
  displayWindow(MenuColor, CoordX, CoordY, Width, Height, Curve);
  
  // Display the current page
  uint8_t alpha = 0xFF;
  uint32_t TextColor = 0xFFFFFFFF;
  int32_t PosX = 185;
  int32_t PosY = 180;
  float Scale = 0.6;
  char *tempDisplayBuffer = DisplayBuffer; // Prevent DisplayBuffer from being loaded in multiple times

  sprintf(tempDisplayBuffer,
    "%ld/%ld",
    FinalScoresMenuCounter,
    TotalPages);
    
  drawStringSingleLine(PosX, PosY, alpha, TextColor, tempDisplayBuffer, Scale);
  
  // Draw the page title
  PosX = -80;
  PosY = 180;
  // Scale = 0.6;
  
  const char *TitleText = "Final Scores List";
  drawStringSingleLine(PosX, PosY, alpha, TextColor, TitleText, Scale);
  
  // Don't draw anything else if there are no scores in the array
  if (AllFinalScoresArray == nullptr)
  {
    // Array is empty, so exit
    return;
  }
  
  // Draw the main page text
  PosX = -232;
  PosY = 150;
  // Scale = 0.6;
  
  // Get the starting index
  uint32_t StartingIndex = (FinalScoresMenuCounter - 1) * TotalOnPage;
  
  // Draw the current page
  for (uint32_t j = 0; j < TotalColumns; j++)
  {
    // Draw the current column
    uint32_t CurrentStartingIndex = StartingIndex + (j * TotalInColumn);
    for (uint32_t i = CurrentStartingIndex; i < (CurrentStartingIndex + TotalInColumn); i++)
    {
      // Stop drawing if at the end of the array
      if (i >= FinalScoresCounter)
      {
        // No other code has to run, so exit
        return;
      }
      
      // Get the current line to draw
      if (AllFinalScoresArray[i] != static_cast<int32_t>(0x80000000))
      {
        // The player did not reset this run
        sprintf(tempDisplayBuffer,
          "%ld. %ld",
          i + 1,
          AllFinalScoresArray[i]);
      }
      else
      {
        // The player reset this run
        sprintf(tempDisplayBuffer,
          "%ld. Reset",
          i + 1);
      }
      
      // Draw the current line
      drawStringSingleLine(PosX, PosY, alpha, TextColor, tempDisplayBuffer, Scale);
      
      // Implement a new line
      PosY -= 20;
    }
    
    // Implement the next column
    PosX += 115;
    PosY = 150;
  }
}

void Mod::LZRandoChallengeStuff()
{
  ttyd::seqdrv::SeqIndex NextSeq = ttyd::seqdrv::seqGetNextSeq();
  ttyd::seqdrv::SeqIndex Title = ttyd::seqdrv::SeqIndex::kTitle;
  ttyd::seqdrv::SeqIndex Game = ttyd::seqdrv::SeqIndex::kGame;
  ttyd::seqdrv::SeqIndex Battle = ttyd::seqdrv::SeqIndex::kBattle;
  ttyd::seqdrv::SeqIndex GameOver = ttyd::seqdrv::SeqIndex::kGameOver;
  ttyd::seqdrv::SeqIndex Load = ttyd::seqdrv::SeqIndex::kLoad;
  
  uint32_t color = 0xFFFFFFFF;
  
  uint8_t alpha;
  if (!TransparentTextFlag)
  {
    alpha = 0xFF;
  }
  else
  {
    alpha = 0x50;
  }
  
  char *tempNextMap = NextMap; // Prevent NextMap from being loaded in multiple times
  bool dmo_comparison = ttyd::string::strcmp(tempNextMap, "dmo_00") == 0;
  bool title_comparison = ttyd::string::strcmp(tempNextMap, "title") == 0;
  
  uint32_t ButtonInputTrg = ttyd::system::keyGetButtonTrg(0);
  char *tempDisplayBuffer = DisplayBuffer; // Prevent DisplayBuffer from being loaded in multiple times
  
  // Get and display Score
  // Don't display while in the intro or on the title screen
  if ((NextSeq >= Game) && (NextSeq <= GameOver) && !dmo_comparison && !title_comparison)
  {
    int32_t Score = 0;
    int32_t MainScores[11] = { 0 };
    uint32_t PouchPointer = reinterpret_cast<uint32_t>(ttyd::mario_pouch::pouchGetPtr());
    
    // Check Important Items
    uint32_t ImportantItemsAddressesStart = PouchPointer + 0xA0;
    for (int i = 0; i < 121; i++)
    {
      uint16_t ImportantItem = *reinterpret_cast<uint16_t *>(ImportantItemsAddressesStart + (i * 0x2));
      if ((ImportantItem != Hammer) && (ImportantItem != MagicalMap) && (ImportantItem != Boots) && (ImportantItem != MailboxSP) && (ImportantItem != StrangeSack) && (ImportantItem != BattleTrunks))
      {
        if ((ImportantItem == MagicalMapBigger) || ((ImportantItem >= DiamondStar) && (ImportantItem <= CrystalStar)))
        {
          // Add 2 points for the magical map or a crystal star
          MainScores[0] += 2;
        }
        else if ((ImportantItem >= PaperModeCurse) && (ImportantItem <= BoatModeCurse))
        {
          // Add 6 points for curses
          MainScores[1] += 6;
        }
        else if (ImportantItem == 0)
        {
          // No more important items, so exit loop
          break;
        }
        else
        {
          // Generic Important Item, so add 1 point
          MainScores[2]++;
        }
      }
    }
    
    // Add 1 point per 10 Battle Trunks in the player's inventory
    MainScores[2] += ttyd::mario_pouch::pouchCheckItem(BattleTrunks) / 10;
    
    // Check for a follower
    uint32_t FollowerPointer = reinterpret_cast<uint32_t>(ttyd::party::partyGetPtr(ttyd::mario_party::marioGetExtraPartyId()));
    if (FollowerPointer)
    {
      // Add 2 points if the player has a follower
      MainScores[3] = 2;
    }
    
    // Check for level ups
    int16_t MarioLevel = *reinterpret_cast<int16_t *>(PouchPointer + 0x8A);
    if (MarioLevel > 1)
    {
      // Add 2 points for each level up
      MainScores[4] = (MarioLevel - 1) * 2;
    }
    
    // Check for bosses
    uint32_t TotalBosses = sizeof(BossDefeated) / sizeof(BossDefeated[0]);
    uint32_t TotalBossesTextbox10Points = 10;
    uint32_t StartSequenceChecks = TotalBossesTextbox10Points + 3; // 2 used for Grodus and Bowser & Kammy
    uint32_t EndSequenceChecks = StartSequenceChecks + 6; // 7 Total
    uint32_t ShadowQueenNumCheck = EndSequenceChecks + 1;
    uint32_t AtomicBooNumCheck = EndSequenceChecks + 2;
    uint32_t BonetailNumCheck = EndSequenceChecks + 3;

    uint32_t SequencePosition = ttyd::swdrv::swByteGet(0);
    const uint16_t SequenceChecks[] = { 39, 85, 200, 211, 260, 383, 388 };

    for (uint32_t i = 1; i <= TotalBosses; i++)
    {
      // Check for bosses based on current textbox
      if (i <= TotalBossesTextbox10Points)
      {
        if (i == JustDefeatedBoss)
        {
          if (!BossDefeated[i - 1])
          {
            BossDefeated[i - 1] = true;
            BossScore += 10;
          }
        }
        else
        {
          BossDefeated[i - 1] = false;
        }
      }
      else if ((i == (TotalBossesTextbox10Points + 1)) || 
        (i == (TotalBossesTextbox10Points + 2)))
      {
        // Check for Grodus and Bowser & Kammy
        if (i == JustDefeatedBoss)
        {
          if (!BossDefeated[i - 1])
          {
            BossDefeated[i - 1] = true;
            BossScore += 8;
          }
        }
        else
        {
          BossDefeated[i - 1] = false;
        }
      }
      else if ((i >= StartSequenceChecks) && (i <= EndSequenceChecks))
      {
        // Check for bosses based on Sequence
        if (SequencePosition == SequenceChecks[i - StartSequenceChecks])
        {
          if (!BossDefeated[i - 1])
          {
            BossDefeated[i - 1] = true;
            BossScore += 10;
          }
        }
        else
        {
          BossDefeated[i - 1] = false;
        }
      }
      else if (i == ShadowQueenNumCheck)
      {
        // Check for the Shadow Queen
        if (SequencePosition == 401)
        {
          if (!BossDefeated[i - 1])
          {
            BossDefeated[i - 1] = true;
            BossScore += 25;
          }
        }
        else
        {
          BossDefeated[i - 1] = false;
        }
      }
      else if (i == AtomicBooNumCheck)
      {
        // Check for the Atomic Boo
        if (ttyd::string::strcmp(tempNextMap, "jin_00") == 0)
        {
          // Check GSWF(2226)
          bool AtomicBooCheck = ttyd::swdrv::swGet(2226);
          
          if (AtomicBooCheck && !BossDefeated[i - 1])
          {
            BossDefeated[i - 1] = true;
            BossScore += 10;
          }
        }
        else
        {
          BossDefeated[i - 1] = false;
          
          // Turn off GSWF(2226) so that the player can refight the Atomic Boo
          ttyd::swdrv::swClear(2226);
        }
      }
      else if (i == BonetailNumCheck)
      {
        // Check for Bonetail
        if (ttyd::string::strcmp(tempNextMap, "jon_06") == 0)
        {
          // Check GSWF(5085)
          bool BonetailCheck = ttyd::swdrv::swGet(5085);
          
          if (BonetailCheck && !BossDefeated[i - 1])
          {
            BossDefeated[i - 1] = true;
            BossScore += 30;
          }
        }
        else
        {
          BossDefeated[i - 1] = false;
          
          // Turn off GSWF(5084) and GSWF(5085) so that the player can refight Bonetail
          ttyd::swdrv::swClear(5084);
          ttyd::swdrv::swClear(5085);
        }
      }
    }

    // Add 10 points for each boss defeated; 25 points for the Shadow Queens and 30 points for Bonetail
    MainScores[5] = BossScore;
    
    // Add 1 point for every 100 coins that the player has
    int16_t CoinCount = ttyd::mario_pouch::pouchGetCoin();
    MainScores[6] = CoinCount / 100;
    
    // Add additional point for 999 coins
    if (CoinCount >= 999)
    {
      MainScores[6]++;
    }
    
    // Check badge log
    uint32_t GSWAddresses = *reinterpret_cast<uint32_t *>(GSWAddressesStart);
    uint32_t BadgeLogAddressesStart = GSWAddresses + 0x188;
    uint32_t BadgeLogCount = 0;
    
    // Check each bit in the bitfield
    for (int i = 0; i < 3; i++)
    {
      uint32_t CurrentAddress = *reinterpret_cast<uint32_t *>(BadgeLogAddressesStart + (i * 0x4));
      for (int x = 0; x < 32; x++)
      {
        if (CurrentAddress & (1 << x))
        {
          // Add 1 to the count if the bit is on
          BadgeLogCount++;
        }
      }
    }
    
    // Check for the 3 badges that were not checked in the previous code
    uint32_t CurrentAddress = *reinterpret_cast<uint32_t *>(GSWAddresses + 0x194);
    for (int i = 0; i < 3; i++)
    {
      if (CurrentAddress & (1 << i)) // Check the 0, 1, and 2 bits
      {
        // Add 1 to the count if the bit is on
        BadgeLogCount++;
      }
    }
    
    // Add 1 point for the badge log count divided by 10
    MainScores[7] = BadgeLogCount / 10;
    
    // Add 5 points for each partner upgrade
    for (int i = 1; i <= 7; i++)
    {
      MainScores[8] += ttyd::mario_pouch::pouchGetPartyAttackLv(i) * 5;
    }
    
    // Check for Game Over
    if (!InGameOver && (NextSeq == GameOver))
    {
      InGameOver = true;
      GameOverCount++;
    }
    else if (NextSeq != GameOver)
    {
      InGameOver = false;
    }
    
    // Add 1 point for every Star Piece the player has
    MainScores[9] = ttyd::mario_pouch::pouchGetStarPiece();
    
    // Subtract 5 points for each Game Over
    MainScores[10] -= GameOverCount * 5;
    
    // Get total score
    int32_t MainScoresArraySize = sizeof(MainScores) / sizeof(MainScores[0]);
    for (int i = 0; i < MainScoresArraySize; i++)
    {
      Score += MainScores[i];
    }
    
    if (TimesUpCounter == 1)
    {
      // Update final score
      TimesUpCounter = 2;
      FinalScore = Score;
    }
    
    // Display Score
    int32_t PosX = -232;
    int32_t PosY = -100;
    float Scale = 0.75;
    
    sprintf(tempDisplayBuffer,
      "Score: %ld",
      Score);
      
    drawStringSingleLine(PosX, PosY, alpha, color, tempDisplayBuffer, Scale);
    
    // Display the Final Score if time is up
    if (TimesUpCounter > 0)
    {
      // PosX = -232;
      PosY = -80;
      // Scale = 0.75;
      
      sprintf(tempDisplayBuffer,
        "Final Score: %ld",
        FinalScore);
      
      drawStringSingleLine(PosX, PosY, alpha, color, tempDisplayBuffer, Scale);
    }
    
    // Display where the points came from
    if (ShowScoreSources)
    {
      // PosX = -232;
      PosY = -80;
      // Scale = 0.75;
      
      // Move the display up if the Final Score is being displayed
      if (TimesUpCounter > 0)
      {
        PosY += 20;
      }
      
      sprintf(tempDisplayBuffer,
        "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
        MainScores[0], // Crystal Stars/Magical Map
        MainScores[1], // Curses
        MainScores[2], // Important Items
        MainScores[3], // Follower
        MainScores[4], // Level Ups
        MainScores[5], // Bosses
        MainScores[6], // Coins
        MainScores[7], // Badge Log
        MainScores[8], // Partner Upgrades
        MainScores[9], // Star Pieces
        MainScores[10]); // Game Overs
      
      drawStringSingleLine(PosX, PosY, alpha, color, tempDisplayBuffer, Scale);
    }
    
    // Get input for whether to display the score sources or not
    uint32_t ButtonInput = ttyd::system::keyGetButton(0);
    uint16_t ShowScoreSourceCombo = PAD_L | PAD_R;
    
    if ((ButtonInput & ShowScoreSourceCombo) == ShowScoreSourceCombo)
    {
      if (ButtonInputTrg & ShowScoreSourceCombo)
      {
        ShowScoreSources = !ShowScoreSources;
      }
    }
  }
  
  // Reset values if starting a new file
  if (NewFile)
  {
    TimerCount = ttyd::system::sysMsec2Frame(3600000); // 1 hour
    ShowScoreSources = false;
    TimerDisabled = false;
    TimerActive = false;
    DisablePlayerControl = false;
    TimesUpCounter = 0;
    BossScore = 0;
    GameOverCount = 0;
    FinishedRunCounter = 0;
    ttyd::__mem::memset(BossDefeated, false, sizeof(BossDefeated));
  }
  
  // Don't display timer if disabled
  if (!TimerDisabled)
  {
    // Activate timer
    if (!TimerActive)
    {
      // Don't activate timer if in the intro or on the title screen
      if (!dmo_comparison && !title_comparison)
      {
        if (NextSeq == Game)
        {
          // Activate timer
          TimerActive = true;
        }
      }
    }
    
    // Display timer
    ttyd::seqdrv::SeqIndex Seq = ttyd::seqdrv::seqGetSeq();
    bool Comparisons = ((Seq == Title) || (Seq == Load) || dmo_comparison || title_comparison) && (TimerCount == 0);
    
    // Don't show the timer on the title screen or file select screen while it is at 0
    // Don't show the timer during the intro
    if (!Comparisons)
    {
      // Display timer
      int32_t FontDrawX = 105;
      int32_t FontDrawY = -160;
      float Scale = 0.8;
      
      #ifdef TTYD_JP
        FontDrawX += 5;
        FontDrawY += 2;
        Scale += 0.05;
      #endif
      
      // Get the proper FPS for the timer
      uint32_t FPS = *reinterpret_cast<uint32_t *>(GSWAddressesStart);
      FPS = *reinterpret_cast<uint32_t *>(FPS + 0x4);
      
      uint32_t hour = TimerCount / 3600 / FPS;
      uint32_t minute = (TimerCount / 60 / FPS) % 60;
      uint32_t second = (TimerCount / FPS) % 60;
      uint32_t frame = TimerCount % FPS;
      
      sprintf(tempDisplayBuffer,
        "%.2ld:%.2ld:%.2ld.%.2ld",
        hour,
        minute,
        second,
        frame);
      
      drawStringSingleLine(FontDrawX, FontDrawY, alpha, color, tempDisplayBuffer, Scale);
    }
    
    if (TimerActive)
    {
      if (TimerCount > 0)
      {
        // Run timer
        TimerCount--;
      }
      else if ((Seq >= Game) && Seq <= GameOver)
      {
        // Update Final Score
        if (TimesUpCounter == 0)
        {
          TimesUpCounter = 1;
          ShowScoreSources = true;
          TransparentTextFlag = false;
        }
        
        uint32_t SystemLevel = ttyd::mariost::marioStGetSystemLevel();
        
        // Pause everything and disable player control
        if (!DisablePlayerControl && (NextSeq == Game) && (SystemLevel != 15))
        {
          DisablePlayerControl = true;
          ttyd::mariost::marioStSystemLevel(1);
          
          // Only turn the key off if it's not already off
          if (ttyd::mario::marioKeyOffChk() == 0)
          {
            ttyd::mario::marioKeyOff();
          }
        }
        
        // Don't display while on the title screen or file select screen
        // Display text for time being up
        int32_t PosX = -90;
        int32_t PosY = 80;
        float Scale = 1.5;
        
        const char *TimesUpText = "Time's Up!";
        drawStringSingleLine(PosX, PosY, alpha, color, TimesUpText, Scale);
        
        // // Don't display or allow modifying of the options in battles, as trying to warp as the curtain is coming up will crash the game
        if (NextSeq != Battle)
        {
          // Display text for how the player wants to continue
          PosX = -225;
          PosY = 25;
          Scale = 0.9;
          
          sprintf(tempDisplayBuffer,
            "%s\n%s",
            "Press Y to continue playing",
            "Press X to return to the title screen");
          
          drawStringMultipleLines(PosX, PosY, alpha, color, tempDisplayBuffer, Scale);
          
          // Get input for what to do next
          if ((ButtonInputTrg & PAD_Y) == PAD_Y)
          {
            // Reset the timer to continue playing
            TimerCount = ttyd::system::sysMsec2Frame(3600000); // 1 hour
            ShowScoreSources = false;
            DisablePlayerControl = false;
            ttyd::mariost::marioStSystemLevel(0);
            FinishedARun = true;
            
            if (FinishedRunCounter == 0)
            {
              FinishedRunCounter = 1;
            }
            
            // Only turn the key on if it's not already on
            if (ttyd::mario::marioKeyOffChk() != 0)
            {
              ttyd::mario::marioKeyOn();
            }
          }
          else if ((ButtonInputTrg & PAD_X) == PAD_X)
          {
            // Return to the title screen
            FinishedARun = true;
            TimerDisabled = true;
            GameOverFlag = false;
            GameOverChallengeMode = false;
            ttyd::seqdrv::seqSetSeq(ttyd::seqdrv::SeqIndex::kMapChange, "title", nullptr);
            ttyd::mariost::marioStSystemLevel(0);
          }
        }
      }
    }
  }
  
  // Handle the menu for all of the final scores
  displayAllFinalScores();
}

void Mod::titleScreenStuff()
{
  ttyd::seqdrv::SeqIndex NextSeq = ttyd::seqdrv::seqGetNextSeq();
  ttyd::seqdrv::SeqIndex Title = ttyd::seqdrv::SeqIndex::kTitle;
  
  // Only run on the title screen
  if (NextSeq != Title)
  {
    return;
  }
  
  // Display Item Randomizer versions and Loading Zone Randomizer version
  // Draw window for the text to go in
  uint32_t WindowColor = 0x000000CC;
  int32_t CoordX = -219;
  int32_t CoordY = -25;
  int32_t Width = 453;
  int32_t Height = 65;
  int32_t Curve = 10;
  
  #ifdef TTYD_JP
    CoordX += 7;
    CoordY += 30;
    Width -= 11;
  #endif
  
  displayWindow(WindowColor, CoordX, CoordY, Width, Height, Curve);
  
  // Display the text
  uint8_t alpha = 0xFF;
  uint32_t TextColor = 0xFFFFFFFF;
  int32_t PosX = -203;
  int32_t PosY = -35;
  float Scale = 0.9;
  char *tempDisplayBuffer = DisplayBuffer; // Prevent DisplayBuffer from being loaded in multiple times
  
  #ifdef TTYD_JP
    PosX += 5;
    PosY += 30;
  #endif
  
  sprintf(tempDisplayBuffer,
    "%s\n%s",
    "Item Randomizer - v1.3.2",
    "Loading Zone Randomizer - v1.0.16");
  
  drawStringMultipleLines(PosX, PosY, alpha, TextColor, tempDisplayBuffer, Scale);
  
  // Display overall version
  PosX = -230;
  PosY = -160;
  // Scale = 0.9;
  
  #ifdef TTYD_JP
    PosX += 15;
    PosY += 15;
  #endif
  
  const char *VersionNumber = "v2.0.18";
  drawStringSingleLine(PosX, PosY, alpha, TextColor, VersionNumber, Scale);
  
  // Draw the remaining time for when gameplay will resume
  if (GameOverFlag && LZRando)
  {
    // The text should only be displayed if the player gets a Game Over and is using the Loading Zone randomizer
    // Draw window for the text to go in
    WindowColor = 0x000000F4;
    CoordX = -199;
    CoordY = 65;
    Width = 405;
    Height = 45;
    // Curve = 10;
    
    #ifdef TTYD_JP
      CoordX += 7;
      CoordY += 30;
      Width -= 13;
    #endif
    
    displayWindow(WindowColor, CoordX, CoordY, Width, Height, Curve);
    
    // Display the text
    // alpha = 0xFF;
    // TextColor = 0xFFFFFFFF;
    PosX = -183;
    PosY = 55;
    // Scale = 0.9;
    
    #ifdef TTYD_JP
      PosX += 5;
      PosY += 30;
    #endif
    
    uint32_t TotalWaitTime = ttyd::system::sysMsec2Frame(19000); // 19 seconds
    uint32_t tempTitleWorkPointer2 = *reinterpret_cast<uint32_t *>(TitleWorkPointer2);
    uint32_t CurrentWaitTime = *reinterpret_cast<uint32_t *>(tempTitleWorkPointer2 + 0x28);
    int32_t RemainingTime = TotalWaitTime - CurrentWaitTime;
    
    // CurrentWaitTime will always end up being 1 frame over TotalWaitTime, so prevent that from displaying
    if (RemainingTime < 0)
    {
      RemainingTime = 0;
    }
    
    // Get the proper FPS for the timer
    uint32_t FPS = *reinterpret_cast<uint32_t *>(GSWAddressesStart);
    FPS = *reinterpret_cast<uint32_t *>(FPS + 0x4);
    
    uint32_t second = (RemainingTime / FPS) % 60;
    uint32_t frame = RemainingTime % FPS;
    
    sprintf(tempDisplayBuffer,
      "Gameplay will resume in: %.2ld.%.2ld",
      second,
      frame);
    
    drawStringSingleLine(PosX, PosY, alpha, TextColor, tempDisplayBuffer, Scale);
  }
}

void Mod::gameModes()
{
  ttyd::seqdrv::SeqIndex NextSeq = ttyd::seqdrv::seqGetNextSeq();
  ttyd::seqdrv::SeqIndex Load = ttyd::seqdrv::SeqIndex::kLoad;
  uint32_t seqMainCheck = *reinterpret_cast<uint32_t *>(seqMainAddress + 0x4);
  
  bool Comparisons = (NextSeq == Load) && (seqMainCheck == 2);
  
  // Only run on the file select screen while the curtain is up
  if (!Comparisons)
  {
    return;
  }
  
  // Display text for opening the help menu
  uint8_t alpha = 0xFF;
  uint32_t color = 0xFFFFFFFF;
  int32_t PosX = -255;
  int32_t PosY = 40;
  float Scale = 0.75;
  
  const char *HelpMenuText = "Press L + Start anytime to open the Help menu";
  drawStringSingleLine(PosX, PosY, alpha, color, HelpMenuText, Scale);
  
  // Display game modes
  // PosX = -255;
  PosY = -10;
  // Scale = 0.75;
  char *tempDisplayBuffer = DisplayBuffer; // Prevent DisplayBuffer from being loaded in multiple times
  
  sprintf(tempDisplayBuffer,
    "%s\n%s\n%s\n%s",
    "Hold L and press the following to toggle modes:",
    "Y = Randomize Coins",
    "X = Loading Zone Randomizer",
    "Z = Loading Zone Randomizer - 1 Hour Challenge");
  
  drawStringMultipleLines(PosX, PosY, alpha, color, tempDisplayBuffer, Scale);
  
  // Display the Item Randomizer setting as On or Off
  char ItemRandoText[4]; // 3 bytes for ItemRandoText, 1 byte for NULL
  if (RandomizeCoins)
  {
    ttyd::string::strcpy(ItemRandoText, "On");
  }
  else
  {
    ttyd::string::strcpy(ItemRandoText, "Off");
  }
  
  // Display the Loading Zone Randomizer setting as On or Off
  char LZRandoText[4]; // 3 bytes for LZRandoText, 1 byte for NULL
  if (LZRando)
  {
    ttyd::string::strcpy(LZRandoText, "On");
  }
  else
  {
    ttyd::string::strcpy(LZRandoText, "Off");
  }
  
  // Display the Challenge Mode setting as On or Off
  char LZRandoChallengeText[4]; // 3 bytes for LZRandoChallengeText, 1 byte for NULL
  if (LZRandoChallenge)
  {
    ttyd::string::strcpy(LZRandoChallengeText, "On");
    
    // Don't randomize coins when using the challenge mode
    RandomizeCoins = false;
  }
  else
  {
    ttyd::string::strcpy(LZRandoChallengeText, "Off");
  }
  
  // Move the text down
  // PosX = -255;
  PosY = -120;
  // Scale = 0.75;
  
  // Display current modes
  sprintf(tempDisplayBuffer,
    "%s%s\n%s%s\n%s%s",
    "Randomize Coins: ",
    ItemRandoText,
    "Loading Zone Randomizer: ",
    LZRandoText,
    "Loading Zone Randomizer - 1 Hour Challenge: ",
    LZRandoChallengeText);
    
  drawStringMultipleLines(PosX, PosY, alpha, color, tempDisplayBuffer, Scale);
  
  // Get input for game modes
  uint32_t ButtonInput = ttyd::system::keyGetButton(0);
  uint32_t ButtonInputTrg = ttyd::system::keyGetButtonTrg(0);
  uint16_t ItemRandoButtonCombo = PAD_L | PAD_Y;
  uint16_t LZRandoButtonCombo = PAD_L | PAD_X;
  uint16_t LZRandoChallengeButtonCombo = PAD_L | PAD_Z;
  
  if ((ButtonInput & ItemRandoButtonCombo) == ItemRandoButtonCombo)
  {
    if (ButtonInputTrg & ItemRandoButtonCombo)
    {
      RandomizeCoins = !RandomizeCoins;
    }
  }
  else if ((ButtonInput & LZRandoButtonCombo) == LZRandoButtonCombo)
  {
    if (ButtonInputTrg & LZRandoButtonCombo)
    {
      LZRando = !LZRando;
      
      if (!LZRando)
      {
        // Turn the challenge mode off if it's currently on
        LZRandoChallenge = false;
      }
    }
  }
  else if ((ButtonInput & LZRandoChallengeButtonCombo) == LZRandoChallengeButtonCombo)
  {
    if (ButtonInputTrg & LZRandoChallengeButtonCombo)
    {
      LZRandoChallenge = !LZRandoChallenge;
      
      if (LZRandoChallenge)
      {
        // Turn the Loading Zone randomizer on if it's currently off
        LZRando = true;
      }
    }
  }
}

void Mod::helpMenu()
{
  ttyd::seqdrv::SeqIndex Seq = ttyd::seqdrv::seqGetSeq();
  ttyd::seqdrv::SeqIndex Title = ttyd::seqdrv::SeqIndex::kTitle;
  
  // Don't run during the bootup process nor during the intro
  if (Seq < Title)
  {
    return;
  }
  
  // Get input from the player
  uint32_t ButtonInput = ttyd::system::keyGetButton(0);
  uint32_t ButtonInputTrg = ttyd::system::keyGetButtonTrg(0);
  uint16_t OpenMenuCombo = PAD_L | PAD_START;
  
  // Check if the menu should be displayed or not
  if ((ButtonInput & OpenMenuCombo) == OpenMenuCombo)
  {
    if (ButtonInputTrg & OpenMenuCombo)
    {
      if (HelpMenuCounter == 0) // Display the menu
      {
        if (PreviousHelpMenuCounterValue > 0)
        {
          // Set the page to the previous page
          HelpMenuCounter = PreviousHelpMenuCounterValue;
        }
        else
        {
          // Set the page to the first page
          HelpMenuCounter = 1;
        }
      }
      else // Hide the menu
      {
        // Store the current page as the previous page
        PreviousHelpMenuCounterValue = HelpMenuCounter;
        
        // Hide the menu
        HelpMenuCounter = 0;
      }
    }
  }
  
  // Only run if the menu should be displayed
  if (HelpMenuCounter == 0)
  {
    // Menu is not displayed, so exit
    return;
  }
  
  // Check for D-Pad inputs
  if ((ButtonInputTrg & PAD_DPAD_LEFT) == PAD_DPAD_LEFT)
  {
    // Go to the previous page
    HelpMenuCounter--;
    
    if (HelpMenuCounter < 1)
    {
      // Loop around to the last page
      HelpMenuCounter = HelpMenuArraySize;
    }
  }
  else if ((ButtonInputTrg & PAD_DPAD_RIGHT) == PAD_DPAD_RIGHT)
  {
    // Go to the next page
    HelpMenuCounter++;
    
    if (HelpMenuCounter > HelpMenuArraySize)
    {
      // Loop around to the first page
      HelpMenuCounter = 1;
    }
  }
  
  // Display the window for the text to go in
  uint32_t MenuColor = 0x000000F4;
  int32_t CoordX = -245;
  int32_t CoordY = 190;
  int32_t Width = 490;
  int32_t Height = 375;
  int32_t Curve = 0;
  
  displayWindow(MenuColor, CoordX, CoordY, Width, Height, Curve);
  
  // Display the current page
  uint8_t alpha = 0xFF;
  uint32_t TextColor = 0xFFFFFFFF;
  int32_t PosX = 185;
  int32_t PosY = 180;
  float Scale = 0.6;
  char *tempDisplayBuffer = DisplayBuffer; // Prevent DisplayBuffer from being loaded in multiple times
  
  sprintf(tempDisplayBuffer,
    "%d/%d",
    HelpMenuCounter,
    HelpMenuArraySize);
    
  drawStringSingleLine(PosX, PosY, alpha, TextColor, tempDisplayBuffer, Scale);
  
  // Draw the main text
  PosX = -232;
  PosY = 150;
  // Scale = 0.6;
  
  // Get and draw the text for the current page
  const char *PageText = HelpMenuArray[HelpMenuCounter - 1];
  drawStringMultipleLines(PosX, PosY, alpha, TextColor, PageText, Scale);
  
  // Display page titles
  uint32_t ItemRandoStartingPage = 2;
  uint32_t ItemRandoTotalPages = 8;
  uint32_t LZRandoStartingPage = ItemRandoStartingPage + ItemRandoTotalPages;
  uint32_t LZRandoTotalPages = 11;
  uint32_t ChallengeModeStartingPage = LZRandoStartingPage + LZRandoTotalPages;
  uint32_t ChallengeModeTotalPages = 9;
  
  if ((HelpMenuCounter >= ItemRandoStartingPage) && (HelpMenuCounter <= (ItemRandoStartingPage + ItemRandoTotalPages - 1)))
  {
    // Draw the Item Randomizer page title
    PosX = -80;
    PosY = 180;
    // Scale = 0.6;
    
    sprintf(tempDisplayBuffer,
      "%s (%ld)",
      "Item Randomizer",
      HelpMenuCounter - ItemRandoStartingPage + 1);
  }
  else if ((HelpMenuCounter >= LZRandoStartingPage) && (HelpMenuCounter <= (LZRandoStartingPage + LZRandoTotalPages - 1)))
  {
    if (HelpMenuCounter == (LZRandoStartingPage + 4))
    {
      // Check if currently on page 5 of the Loading Zone Randomizer section
      PosX = 0;
      PosY = 110;
      // Scale = 0.6;
      
      // Draw second column of Loading Zone Randomizer page 5
      const char *BossesText = "15. Magnus 2\n16. Dark Bones (las_05)\n17. Gloomtail\n18. Shadow Sirens (Ch8)\n19. Grodus\n20. Bowser & Kammy\n21. Shadow Queen\n22. Bonetail";
      drawStringMultipleLines(PosX, PosY, alpha, TextColor, BossesText, Scale);
    }
    
    // Draw the Loading Zone Randomizer page title
    PosX = -130;
    PosY = 180;
    // Scale = 0.6;
    
    sprintf(tempDisplayBuffer,
      "%s (%ld)",
      "Loading Zone Randomizer",
      HelpMenuCounter - LZRandoStartingPage + 1);
  }
  else if ((HelpMenuCounter >= ChallengeModeStartingPage) && (HelpMenuCounter <= (ChallengeModeStartingPage + ChallengeModeTotalPages - 1)))
  {
    // Draw the 1 Hour Challenge page title
    PosX = -80;
    PosY = 180;
    // Scale = 0.6;
    
    sprintf(tempDisplayBuffer,
      "%s (%ld)",
      "1 Hour Challenge",
      HelpMenuCounter - ChallengeModeStartingPage + 1);
  }
  else
  {
    // Exit the function if there is no more text to draw
    return;
  }
  
  drawStringSingleLine(PosX, PosY, alpha, TextColor, tempDisplayBuffer, Scale);
}

void Mod::displayStuff()
{
  ttyd::dispdrv::dispEntry(ttyd::dispdrv::DisplayLayer::kDebug3d, 0, [](ttyd::dispdrv::DisplayLayer layerId, void *user)
  {
    if (LZRando)
    {
      reinterpret_cast<Mod *>(user)->LZRandoDisplayStuff();
    
      if (LZRandoChallenge)
      {
        reinterpret_cast<Mod *>(user)->LZRandoChallengeStuff();
      }
    }
    
    reinterpret_cast<Mod *>(user)->helpMenu();
  }, this);
  
  ttyd::dispdrv::dispEntry(ttyd::dispdrv::DisplayLayer::k2d, 0, [](ttyd::dispdrv::DisplayLayer layerId, void *user)
  {
    reinterpret_cast<Mod *>(user)->titleScreenStuff();
    reinterpret_cast<Mod *>(user)->gameModes();
  }, this);
}

}