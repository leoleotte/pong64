#include <assert.h>
#include <nusys.h>
#include "main.h"
#include "graphic.h"
#include "math.h"

static float padPos_x[MAX_PLAYERS];
static float padPos_y[MAX_PLAYERS];
static float padRotation[MAX_PLAYERS];

static float ballPos_x = 0.0;
static float ballPos_y = 0.0;
static float ballDirection_x = 0;
static float ballDirection_y = 0;
static float ballSpeed = 0.0;

static float debugValue = 0.0;

void resetRound();
void drawPad(Dynamic* dynamicPointer, int padIndex);
void drawBall(Dynamic* dynamicPointer);

void updateBall();
void updateAI(int padNumber);
void updateCollision();
void checkCollision(int padNumber);

/* The initialization of stage 0 */
void initStage00(void)
{
  resetRound();
}

void resetRound()
{
  ballPos_x = 0.0;
  ballPos_y = 0.0;
  ballSpeed = 0.5;

  if (RAND(11) >= 5)
    ballDirection_x = 1.0;
  else
    ballDirection_x = -1.0;

  if (RAND(11) >= 5)
    ballDirection_y = 1.0;
  else
    ballDirection_y = -1.0;


  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    double posX = 0.0;
    double posY = 0.0;
    double rotation = 0.0;

    if (i == LEFT_PAD_INDEX) {
      posX -= PAD_START_POS_OFFSET_X;
    }
    else if (i == RIGHT_PAD_INDEX) {
      posX = PAD_START_POS_OFFSET_X;
    }
    else if (i == TOP_PAD_INDEX) {
      posY = PAD_START_POS_OFFSET_Y;
      rotation = 90.0;
    } else if (i == BOTTOM_PAD_INDEX) {
      posY = -PAD_START_POS_OFFSET_Y;
      rotation = 90.0;
    }

    padPos_x[i] = posX;
    padPos_y[i] = posY;
    padRotation[i] = rotation;
  }
}

/* Make the display list and activate the task */
void makeDL00(void)
{
  char controllerBuffer[20];

  Dynamic* dynamicPointer;

  /* Specify the display list buffer */
  dynamicPointer = &gfx_dynamic[gfx_gtask_no];
  glistp = &gfx_glist[gfx_gtask_no][0];

  /* Initialize RCP */
  gfxRCPInit();

  /* Clear the frame and Z-buffer */
  gfxClearCfb();
  
  /* projection,modeling matrix set */
  guOrtho(&dynamicPointer->projection,
	  -(float)SCREEN_WD/2.0F, (float)SCREEN_WD/2.0F,
	  -(float)SCREEN_HT/2.0F, (float)SCREEN_HT/2.0F,
	  1.0F, 10.0F, 1.0F);
    
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicPointer->projection)),
		G_MTX_PROJECTION|G_MTX_LOAD|G_MTX_NOPUSH);

  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    drawPad(dynamicPointer, i);
  }

  drawBall(dynamicPointer);
  
  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);

  assert((glistp - gfx_glist[gfx_gtask_no]) < GFX_GLIST_LEN);

  /* Activate the task and 
     switch display buffers */
  nuGfxTaskStart(&gfx_glist[gfx_gtask_no][0],
		 (s32)(glistp - gfx_glist[gfx_gtask_no]) * sizeof (Gfx),
		 NU_GFX_UCODE_F3DEX , NU_SC_NOSWAPBUFFER);

  if(contPattern & 0x1)
    {
      nuDebConTextPos(0,9,20);
      nuDebConCPuts(0, "Controller1 connect");
    }
  else
    {
      nuDebConTextPos(0,9,20);
      nuDebConCPuts(0, "Controller1 not connect");
    }

    
      nuDebConTextPos(0,12,23);
      sprintf(controllerBuffer,"ballDir_x=%1.2f",ballDirection_x);
      nuDebConCPuts(0, controllerBuffer);

      nuDebConTextPos(0,12,24);
      sprintf(controllerBuffer,"ballDir_y=%1.2f",ballDirection_y);
      nuDebConCPuts(0, controllerBuffer);

    debugValue = ballDirection_x / 2.0 + abs(((padPos_y[LEFT_PAD_INDEX] - ballPos_y) / PAD_SIZE_Y * 2.0) / 1.0) * -1.0;
    nuDebConTextPos(0,12,25);
    sprintf(controllerBuffer,"debug=%1.2f",debugValue);
    nuDebConCPuts(0, controllerBuffer);
    
  /* Display characters on the frame buffer */
  nuDebConDisp(NU_SC_SWAPBUFFER);

  /* Switch display list buffers */
  gfx_gtask_no ^= 1;
}


/* The game progressing process for stage 0 */
void updateGame00(void)
{
  /* Data reading of controller 1 */
  nuContDataGetAll(contdata);

  /* Change the display position by stick data */
  if (contdata[0].stick_y > 5 || contdata[0].stick_y < -5)
    padPos_y[0] += clamp(contdata[0].stick_y, -10, 10) * 0.05;
  if (contdata[1].stick_y > 5 || contdata[1].stick_y < -5)
    padPos_y[1] += clamp(contdata[1].stick_y, -10, 10) * 0.05;
    if (contdata[2].stick_x > 5 || contdata[2].stick_x < -5)
    padPos_x[2] += clamp(contdata[2].stick_x, -10, 10) * 0.05;
  if (contdata[3].stick_x > 5 || contdata[3].stick_x < -5)
    padPos_x[3] += clamp(contdata[3].stick_x, -10, 10) * 0.05;

  //updateAI(LEFT_PAD_INDEX);
  updateAI(RIGHT_PAD_INDEX);
  updateAI(TOP_PAD_INDEX);
  updateAI(BOTTOM_PAD_INDEX);
  updateBall();
  updateCollision();

  if (contdata[0].button & L_CBUTTONS){
    ballDirection_x -= 0.1;
  }
  if (contdata[0].button & R_CBUTTONS){
    ballDirection_x += 0.1;
  }
  if (contdata[0].button & U_CBUTTONS){
    ballDirection_y += 0.1;
  }
  if (contdata[0].button & D_CBUTTONS){
    ballDirection_y -= 0.1;
  }

  if (contdata[0].button & A_BUTTON){
    resetRound();
  }

  /* The reverse rotation by the A button 
  if(contdata[0].trigger & A_BUTTON)
    {
      vel = -vel;
      osSyncPrintf("A button Push\n");
    }
  */
  /* Rotate fast while pushing the B button */
  /*
  if(contdata[0].button & B_BUTTON)
    theta += vel * 3.0;
  else
    theta += vel;

  if(theta>360.0)
      theta-=360.0;
  else if (theta<0.0)
      theta+=360.0;
  */
}

void updateAI(int padNumber)
{
  if (padNumber == 0 || padNumber == 1){
    if (ballPos_y > padPos_y[padNumber] + PAD_SIZE_Y / 1.5)
      padPos_y[padNumber] += 0.55;
    else if (ballPos_y < padPos_y[padNumber] - PAD_SIZE_Y / 1.5)
      padPos_y[padNumber] -= 0.55;
  } else {
    if (ballPos_x > padPos_x[padNumber] + PAD_SIZE_X / 1.5)
      padPos_x[padNumber] += 0.55;
    else if (ballPos_x < padPos_x[padNumber] - PAD_SIZE_X / 1.5)
      padPos_x[padNumber] -= 0.55;
  }
}

void updateBall()
{
  if (ballPos_x > PAD_START_POS_OFFSET_X + PAD_SIZE_X || ballPos_x < -PAD_START_POS_OFFSET_X - PAD_SIZE_X)
    resetRound();

  if (ballPos_y > PAD_START_POS_OFFSET_Y + PAD_SIZE_Y || ballPos_y < -PAD_START_POS_OFFSET_Y - PAD_SIZE_Y)
    resetRound();

  if (ballDirection_x > 2.0)
    ballDirection_x = 2.0;
  if (ballDirection_x < -2.0)
    ballDirection_x = -2.0;
  if (ballDirection_y > 2.0)
    ballDirection_y = 2.0;
  if (ballDirection_y < -2.0)
    ballDirection_y = -2.0;

  ballPos_x += ballSpeed * ballDirection_x;
  ballPos_y += ballSpeed * ballDirection_y;
}

void updateCollision()
{
  if (ballDirection_y > 0){
    checkCollision(TOP_PAD_INDEX);

    if (ballPos_x < 0)
      checkCollision(LEFT_PAD_INDEX);
    else
      checkCollision(RIGHT_PAD_INDEX);
  } else {
    checkCollision(BOTTOM_PAD_INDEX);

    if (ballPos_x < 0)
      checkCollision(LEFT_PAD_INDEX);
    else
      checkCollision(RIGHT_PAD_INDEX);
  }
}

void checkCollision(int padIndex)
{
  float absBallDirection_x = abs(ballDirection_x);
  float absBallDirection_y = abs(ballDirection_y);

  if (padIndex == LEFT_PAD_INDEX){
    if (ballPos_x <= -PAD_START_POS_OFFSET_X + (PAD_SIZE_X) &&
        ballPos_y <= padPos_y[padIndex] + (PAD_SIZE_Y) && ballPos_y >= padPos_y[padIndex] - (PAD_SIZE_Y)){
      ballDirection_x = max(1, PAD_SIZE_Y / (abs(padPos_y[padIndex] - ballPos_y) + 1.0));
      ballDirection_y = ballDirection_y / 2.0 + abs(((padPos_y[padIndex] - ballPos_y) / PAD_SIZE_Y * 2.0) / 1) * -1.0;
    }
  } else if (padIndex == RIGHT_PAD_INDEX){
    if (ballPos_x >= PAD_START_POS_OFFSET_X - (PAD_SIZE_X) &&
        ballPos_y <= padPos_y[padIndex] + (PAD_SIZE_Y) && ballPos_y >= padPos_y[padIndex] - (PAD_SIZE_Y)){
      ballDirection_x = min(-1.0, (PAD_SIZE_Y / (abs(padPos_y[padIndex] - ballPos_y) + 1.0)) * -1.0);
      ballDirection_y = ballDirection_y / 2.0 + abs(((padPos_y[padIndex] - ballPos_y) / PAD_SIZE_Y * 2.0) / 1) * -1.0;
    }
  } else if (padIndex == BOTTOM_PAD_INDEX){
    if (ballPos_y <= -PAD_START_POS_OFFSET_Y + (PAD_SIZE_Y / 2.0) &&
        ballPos_x + (BALL_SIZE_X * 2.0) >= padPos_x[padIndex] - (PAD_SIZE_X * 2.0) && ballPos_x - (BALL_SIZE_X * 2.0) <= padPos_x[padIndex] + (PAD_SIZE_X * 2.0)){
      ballDirection_x = ballDirection_x / 2.0 + abs((((padPos_x[padIndex] - ballPos_x) / PAD_SIZE_X) / 1)) * -1.0;
      ballDirection_y = max(1, PAD_SIZE_X / (abs(padPos_x[padIndex] - ballPos_x) + 1.0));
    }
  } else if (padIndex == TOP_PAD_INDEX){
    if (ballPos_y >= PAD_START_POS_OFFSET_Y - (PAD_SIZE_Y / 2.0) &&
        ballPos_x + (BALL_SIZE_X * 2.0) >= padPos_x[padIndex] - (PAD_SIZE_X * 2.0) && ballPos_x - (BALL_SIZE_X * 2.0) <= padPos_x[padIndex] + (PAD_SIZE_X * 2.0)){
      ballDirection_x = ballDirection_x / 2.0 + abs((((padPos_x[padIndex] - ballPos_x) / PAD_SIZE_X) / 1)) * -1.0;
      ballDirection_y = min(-1.0, (PAD_SIZE_X / (abs(padPos_x[padIndex] - ballPos_x) + 1.0)) * -1.0);
    }
  }
}

/* The vertex coordinate */
static Vtx pad_vertex[] =  {
        {        -PAD_SIZE_X,  PAD_SIZE_Y, -5, 0, 0, 0, 0xff, 0xff, 0xff, 0xff	},
        {         PAD_SIZE_X,  PAD_SIZE_Y, -5, 0, 0, 0, 0xff, 0xff, 0xff, 0xff	},
        {         PAD_SIZE_X, -PAD_SIZE_Y, -5, 0, 0, 0, 0xff, 0xff, 0xff, 0xff	},
        {        -PAD_SIZE_X, -PAD_SIZE_Y, -5, 0, 0, 0, 0xff, 0xff, 0xff, 0xff	},
      };

static Vtx ball_vertex[] =  {
        {        -2,  2, -5, 0, 0, 0, 0xff, 0xff, 0xff, 0xff	},
        {         2,  2, -5, 0, 0, 0, 0xff, 0xff, 0xff, 0xff	},
        {         2, -2, -5, 0, 0, 0, 0xff, 0xff, 0xff, 0xff	},
        {        -2, -2, -5, 0, 0, 0, 0xff, 0xff, 0xff, 0xff	},
      };

void drawPad(Dynamic* dynamicPointer, int padIndex)
{
  guRotate(&dynamicPointer->player_rotation[padIndex], padRotation[padIndex], 0.0F, 0.0F, 1.0F);
  guTranslate(&dynamicPointer->player_translation[padIndex], padPos_x[padIndex], padPos_y[padIndex], 0.0F);
  
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicPointer->player_translation[padIndex])),
		G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicPointer->player_rotation[padIndex])),
		G_MTX_MODELVIEW|G_MTX_MUL|G_MTX_NOPUSH);

  gSPVertex(glistp++,&(pad_vertex[0]),4, 0);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++,G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++,G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
  gSPClearGeometryMode(glistp++,0xFFFFFFFF);
  gSPSetGeometryMode(glistp++,G_SHADE| G_SHADING_SMOOTH);

  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
}

void drawBall(Dynamic* dynamicPointer)
{
  guRotate(&dynamicPointer->ball_rotation, 0.0F, 0.0F, 0.0F, 1.0F);
  guTranslate(&dynamicPointer->ball_translation, ballPos_x, ballPos_y, 0.0F);  
  
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicPointer->ball_translation)),
		G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);
  gSPMatrix(glistp++,OS_K0_TO_PHYSICAL(&(dynamicPointer->ball_rotation)),
		G_MTX_MODELVIEW|G_MTX_MUL|G_MTX_NOPUSH);

  gSPVertex(glistp++,&(ball_vertex[0]),4, 0);

  gDPPipeSync(glistp++);
  gDPSetCycleType(glistp++,G_CYC_1CYCLE);
  gDPSetRenderMode(glistp++,G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);
  gSPClearGeometryMode(glistp++,0xFFFFFFFF);
  gSPSetGeometryMode(glistp++,G_SHADE| G_SHADING_SMOOTH);

  gSP2Triangles(glistp++,0,1,2,0,0,2,3,0);
}
