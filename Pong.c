#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Uefi/UefiSpec.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include "Numbers.h"
#define CELLSIZE 10
#define BALLSIZE CELLSIZE
#define BATCELLS 10
#define BATSIZE  (CELLSIZE * BATCELLS)

static EFI_GRAPHICS_OUTPUT_PROTOCOL* Gop = NULL;
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL White = {0xFF, 0xFF, 0xFF, 0};
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black = {0, 0, 0, 0};

VOID
DrawCell(UINT32 X, UINT32 Y, BOOLEAN Reset)
{
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color = (Reset)?Black:White;
  Gop->Blt(Gop, &Color, EfiBltVideoFill,
           0, 0,
           X*CELLSIZE, Y*CELLSIZE,
           CELLSIZE, CELLSIZE,
           0);
}

VOID
DrawBall(UINT32 X, UINT32 Y)
{
  DrawCell(X, Y, FALSE);
}

VOID
EraseBall(UINT32 X, UINT32 Y)
{
  DrawCell(X, Y, TRUE);
}

VOID
DrawBat(UINT32 X, UINT32 Y)
{
  UINT32 i;
  for(i = 0; i < BATCELLS; i++){
    DrawCell(X, Y+i, 0);
  }
}

VOID
EraseBat(UINT32 X, UINT32 Y)
{
  UINT32 i;
  for(i = 0; i < BATCELLS; i++){
    DrawCell(X, Y+i, 1);
  }
}

BOOLEAN
BatBlockBall(UINT32 BatX, UINT32 BatY, UINT32 BallX, UINT32 BallY)
{
  INT32 SBatX  = BatX;
  INT32 SBallX = BallX;
  if(ABS(SBatX-SBallX) != 1){
    return FALSE;
  }

  return (BatY <= BallY) && ((BatY + BATCELLS) >= BallY);
}

VOID
DrawOneNumber(UINT8 Score, UINT32 X)
{
  UINT32 i, j;
  for(i = 0; i < NUMBERHEIGHT; i++){
    for(j = 0; j < NUMBERWIDTH; j++){
      if(NUMBERS[Score][i][j]){
	DrawCell(X+j, i, 0);
      }
    }
  }
}

VOID
DrawScore(UINT8 RightScore, UINT8 LeftScore, CONST UINT32 GameWidth)
{
  UINT32 X = (GameWidth / 2);
  DrawOneNumber(RightScore, X-1-NUMBERWIDTH);
  DrawOneNumber(LeftScore, X+1);
}

VOID
EraseScore(CONST UINT32 GameWidth)
{
  UINT32 i, j;
  UINT32 start = (GameWidth / 2) - 1 - NUMBERWIDTH;
  for(i = 0 ; i < NUMBERHEIGHT; i++){
    for(j = start; j < start + 2 + 2*NUMBERWIDTH; j++){
      DrawCell(j, i, 1);
    }
  }
}

EFI_STATUS EFIAPI
UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  // Initialize the screen
  EFI_STATUS St = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,
                                      NULL,
                                      (VOID**) &Gop);
  if (EFI_ERROR(St)) {
    Print(L"Unable to locate Graphics Output Protocol\n");
    Print(L"Press any key to exit...\n");
    UINTN EventIndex;
    gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &EventIndex);
    Exit(1);
  }
  CONST UINT32 ScreenWidth  = Gop->Mode->Info->HorizontalResolution;
  CONST UINT32 ScreenHeight = Gop->Mode->Info->VerticalResolution;

  CONST UINT32 GameWidth  = ScreenWidth / CELLSIZE;
  CONST UINT32 GameHeight = ScreenHeight / CELLSIZE;

  //Ball at the center of the game, bats on the sides
  UINT32 BallX = GameWidth / 2;
  UINT32 BallY = GameHeight / 2;
  UINT32 LeftBatPos = (GameHeight / 2) - (BATCELLS / 2);
  UINT32 RightBatPos = (GameHeight / 2) - (BATCELLS / 2);
  CONST UINT32 LOWESTPOS = GameHeight - BATCELLS;
  CONST UINT32 XRIGHTBAT = GameWidth-1;
  
  DrawBat(0, LeftBatPos);
  DrawBat(XRIGHTBAT, RightBatPos);
  DrawBall(BallX, BallY);

  //Setup speeds and score
  INT32 SpeedX = -1;
  INT32 SpeedY = 1;
  CONST INT32 BATSPEED = 4;
  UINT8 LeftScore  = 0;
  UINT8 RightScore = 0;

  DrawScore(LeftScore, RightScore, GameWidth);

  //Main loop
  BOOLEAN Stop = FALSE;
  while(!Stop){
    //Check for keys event
    EFI_STATUS Poll;
    do{
      //Check the keys and move the bats if needed
      EFI_INPUT_KEY Key;
      Poll = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
      if(Poll == EFI_SUCCESS){
        if(Key.ScanCode == 0 && Key.UnicodeChar == 's'){
          EraseBat(0, LeftBatPos);
          if(LeftBatPos < BATSPEED){
            LeftBatPos = 0;
          } else {
            LeftBatPos -= BATSPEED;
          }
          DrawBat(0, LeftBatPos);
        } else if(Key.ScanCode == 0 && Key.UnicodeChar == 'x'){
          EraseBat(0, LeftBatPos);        
          LeftBatPos += BATSPEED;
          if(LeftBatPos > LOWESTPOS)
            LeftBatPos = LOWESTPOS;
          DrawBat(0, LeftBatPos);
        } else if(Key.ScanCode == 1){
          EraseBat(XRIGHTBAT, RightBatPos);
          if(RightBatPos < BATSPEED){
            RightBatPos = 0;
          } else {
            RightBatPos -= BATSPEED;
          }
          DrawBat(XRIGHTBAT, RightBatPos);
        }  else if(Key.ScanCode == 2){
          EraseBat(XRIGHTBAT, RightBatPos);       
          RightBatPos += BATSPEED;
          if(RightBatPos > LOWESTPOS)
            RightBatPos = LOWESTPOS;
          DrawBat(XRIGHTBAT, RightBatPos);
        }
      }
    }while(Poll == EFI_SUCCESS);
    //Check and change ball position
    EraseBall(BallX, BallY);

    if(BallX == 0){
      ++RightScore;
      if(RightScore >= 9){
        Stop = TRUE;
      }
      BallX = GameWidth / 2;
      BallY = GameHeight / 2;
      SpeedX *= -1;
      EraseScore(GameWidth);
      DrawScore(LeftScore, RightScore, GameWidth);
    } else if(BallX == GameWidth){
      ++LeftScore;
      if(LeftScore >= 9){
        Stop = TRUE;
      }
      BallX = GameWidth / 2;
      BallY = GameHeight / 2;
      SpeedX *= -1;
      EraseScore(GameWidth);
      DrawScore(LeftScore, RightScore, GameWidth);
    } else if(BatBlockBall(0, LeftBatPos, BallX, BallY) ||
              BatBlockBall(XRIGHTBAT, RightBatPos, BallX, BallY)){
      SpeedX *= -1;
    }

    if(BallY <= 0 || BallY >= GameHeight) {
      SpeedY *= -1;
    }

    BallX += SpeedX;
    BallY += SpeedY;
    DrawBall(BallX, BallY);
    
    gBS->Stall(50000);
  }

  Print(L"Game over !\nPress any key to exit...\n");
  EFI_INPUT_KEY Key;
  UINTN EventIndex;
  gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &EventIndex);
  gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  return EFI_SUCCESS;
}
