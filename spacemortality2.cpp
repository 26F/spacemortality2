
#include <Arduboy2.h>
#include <stdlib.h>


#define SCREENWIDTH          128
#define SCREENHEIGHT         64
#define NUMPOINTS            100
#define NUMWARPEFFECTFRAMES  240
#define MAXPLANETSIZE        10
#define PLANETSURFDETAIL     6
#define MOVESPEED            1
#define CURSX                SCREENWIDTH / 2
#define CURSY                SCREENHEIGHT / 2
#define AUTOPILOT            ((flags & (1 << 0)) > 0)
#define PLANET               ((flags & (1 << 1)) > 0)
#define HABITABLE            ((flags & (1 << 2)) > 0) 
#define PLANETALIGNED        ((flags & (1 << 3)) > 0)
#define ISINSPACE            ((flags & (1 << 4)) > 0)
#define AUTOPILOTBIT         0
#define PLANETBIT            1
#define HABITABLEBIT         2
#define PLANETALIGNBIT       3
#define ISINSPACEBIT         4
#define NUMWARPCRYSTALS      4
#define MAXWARPCRYSTALS      99
#define TARGETLOCK           254
#define NUMSTARTAMMO         1024

#define STARTINGSECTOR       32768


// arduboy instance
Arduboy2 arduboy;


bool restart = false;

static uint8_t planettextpts = 0;

static uint8_t planettexturex[MAXPLANETSIZE * MAXPLANETSIZE];
static uint8_t planettexturey[MAXPLANETSIZE * MAXPLANETSIZE];

static uint8_t targetlock = 0;

static int8_t arttile1x[7];
static int8_t arttile2x[7];
static int8_t arttile3x[7];

static int8_t arttile1y[7];
static int8_t arttile2y[7];
static int8_t arttile3y[7];

static int8_t arttile4x[36];
static int8_t arttile4y[36];

static uint8_t numused1 = 0;
static uint8_t numused2 = 0;
static uint8_t numused3 = 0;
static uint8_t numused4 = 0;

static int8_t tilecoords1x[6];
static int8_t tilecoords2x[6];
static int8_t tilecoords3x[6];
static int8_t tilecoords4x[2];

static int8_t tilecoords1y[6];
static int8_t tilecoords2y[6];
static int8_t tilecoords3y[6];
static int8_t tilecoords4y[2];



// both of these are set to zero if
// there is no planet
static uint16_t planetxcoord = 0;
static uint16_t planetycoord = 0;

static int16_t planetoffsetx = 0;
static int16_t planetoffsety = 0;

static int16_t playerposx = 0;
static int16_t playerposy = 0;

// and space enemies
static int16_t itemcoordsx = 0;
static int16_t itemcoordsy = 0;

// and space enemies
static int16_t itemxoffset = 0;
static int16_t itemyoffset = 0;

bool isitem = false;
bool itemwarp = false;
bool combatmode = false;
bool hostile = false;
bool indatabase = false;
bool battlewon = false;

static int16_t playerposxbackup = 0;
static int16_t playerposybackup = 0;

static int16_t planetoffsetxbackup = 0;
static int16_t planetoffsetybackup = 0;

// inspace tree
static uint8_t flags = 0x10;

static uint8_t planetradius = 0;

// mask for held down buttons
// reset where needed
static uint8_t held = 0;
static uint8_t heldprevious = 0;

// button indices used in setmovedeltas function
typedef enum BUTTON_INDX {BLEFT, BRIGHT, BUP, BDOWN, BB, BA};

// where we are in space (really seed for random number generator)
static uint16_t sector = STARTINGSECTOR;


// can add or subtract number of warp crystals to / from sector
static uint8_t warpcrystals = NUMWARPCRYSTALS;

static uint16_t ammo = NUMSTARTAMMO;

// stars 
// and procedural items
// differs when in space and on planet
static uint8_t pointsx[NUMPOINTS];
static uint8_t pointsy[NUMPOINTS];


static int8_t deltax = 0;
static int8_t deltay = 0;


static void newstars()
{

	
	for (uint8_t i = 0; i < NUMPOINTS; i++) {

		pointsx[i] = rand() % SCREENWIDTH;
		pointsy[i] = rand() % SCREENHEIGHT;

	}
	

}




void drawstars()
{

	for (uint8_t i = 0; i < NUMPOINTS; i++) {

		arduboy.drawPixel(pointsx[i], pointsy[i], WHITE);

	}
	
}



// set up arduboy
void setup()
{

	arduboy.begin();

	arduboy.setFrameRate(60);

	arduboy.clear();


	//

	arduboy.invert(false);

	sectorparameters();

}





void restartgamevisual()
{


	uint8_t frames = 100;

	for (uint8_t i = 0; i < frames; i++) {


		arduboy.clear();

		arduboy.print(i);

		arduboy.setCursor(0,8);

		arduboy.print("game over");

	
		arduboy.display();

		arduboy.idle();

		arduboy.delayShort(16);

	}

	arduboy.clear();


}




void restartgame()
{

	// display function
	restartgamevisual();

	planettextpts = 0;

	combatmode = false;

	hostile = false;

	ammo = NUMSTARTAMMO;

 	numused1 = 0;
 	numused2 = 0;
 	numused3 = 0;
 	numused4 = 0;
	
	planetxcoord = 0;
    planetycoord = 0;

    planetoffsetx = 0;
    planetoffsety = 0;

    playerposx = 0;
    playerposy = 0;

    itemcoordsx = 0;
    itemcoordsy = 0;

    itemxoffset = 0;
    itemyoffset = 0;

    isitem = false;
    itemwarp = false;

    playerposxbackup = 0;
    playerposybackup = 0;

    planetoffsetxbackup = 0;
    planetoffsetybackup = 0;

    flags = 0x10;

    planetradius = 0;
    held = 0;

    heldprevious = 0;

    targetlock = 0;

	sector = STARTINGSECTOR;


    // can add or subtract number of warp crystals to / from sector
    warpcrystals = NUMWARPCRYSTALS;

    deltax = 0;
    deltay = 0;

    arduboy.invert(false);

    sectorparameters();


}




void transformspace()
{


	if (deltax == 0 && deltay == 0 && (!hostile)) {

		return;

	}

	if (combatmode) {

		deltax *= 2;
		deltay *= 2;	

	}


	// update stars
	for (uint8_t i = 0; i < NUMPOINTS; i++) {

		if (deltax != 0) {

			if (deltax < 0) {

				if (pointsx[i] + deltax >= 0) {

					pointsx[i] += deltax;

				} else {

					pointsx[i] = (SCREENWIDTH - 2);
					pointsy[i] = rand() % SCREENHEIGHT;

				}

			} else {

				if (pointsx[i] + deltay < SCREENWIDTH) {

					pointsx[i] += deltax;

				} else {

					pointsx[i] = 1;
					pointsy[i] = rand() % SCREENHEIGHT;

				}

			}

		}

		if (deltay != 0) {

			if (deltay < 0) {

				if (pointsy[i] + deltay >= 0) {

					pointsy[i] += deltay;

				} else {

					pointsy[i] = SCREENHEIGHT - 2;
					pointsx[i] = rand() % SCREENWIDTH;

				}

			} else {

				if (pointsy[i] + deltay < SCREENHEIGHT) {

					pointsy[i] += deltay;

				} else {

					pointsy[i] = 1;
					pointsx[i] = rand() % SCREENWIDTH;

				}

			}

		}

	}

	planetoffsetx += deltax;
	planetoffsety += deltay;

	if (hostile) {

		int16_t deltaxx = (((playerposx + SCREENWIDTH / 2) - (itemcoordsx)) < 0) ? -1 : 1;
		int16_t deltayy = (((playerposy + SCREENWIDTH / 2) - (itemcoordsy)) < 0) ? -1 : 1;

		itemcoordsx += deltaxx;
		itemcoordsy += deltayy;

	}

	playerposx += deltax;
	playerposy += deltay;

	deltay = 0;
	deltax = 0;

}




void transformonplanet() 
{


	if (deltax == 0 && deltay == 0) {

		return;

	}

	for (uint8_t i = 0; i < 6; i++) {

		if (tilecoords1x[i] + deltax >= SCREENWIDTH - 1) {

			tilecoords1x[i] = -2;
			tilecoords1y[i] = rand() % SCREENHEIGHT;

		} else if (tilecoords1x[i] + deltax <= -6) {

			tilecoords1x[i] = SCREENWIDTH - 2;
			tilecoords1y[i] = rand() % SCREENHEIGHT;

		} else {

			tilecoords1x[i] += deltax;

		}

		if (tilecoords1y[i] + deltay >= SCREENHEIGHT) {

			tilecoords1y[i] = -2;
			tilecoords1x[i] = rand() % (SCREENWIDTH - 1);

		} else if (tilecoords1y[i] + deltay <= -6) {

			tilecoords1y[i] = SCREENHEIGHT - 2;
			tilecoords1x[i] = rand() % (SCREENWIDTH - 1);

		} else {

			tilecoords1y[i] += deltay;

		}

	}


	for (uint8_t i = 0; i < 6; i++) {

		if (tilecoords2x[i] + deltax >= SCREENWIDTH - 1) {

			tilecoords2x[i] = -2;
			tilecoords2y[i] = rand() % SCREENHEIGHT;

		} else if (tilecoords2x[i] + deltax <= -6) {

			tilecoords2x[i] = SCREENWIDTH - 2;
			tilecoords2y[i] = rand() % SCREENHEIGHT;

		} else {

			tilecoords2x[i] += deltax;

		}

		if (tilecoords2y[i] + deltay >= SCREENHEIGHT) {

			tilecoords2y[i] = -2;
			tilecoords2x[i] = rand() % (SCREENWIDTH - 1);

		} else if (tilecoords2y[i] + deltay <= -6) {

			tilecoords2y[i] = SCREENHEIGHT - 2;
			tilecoords2x[i] = rand() % (SCREENWIDTH - 1);

		} else {

			tilecoords2y[i] += deltay;

		}

	}




	for (uint8_t i = 0; i < 6; i++) {

		if (tilecoords3x[i] + deltax >= SCREENWIDTH - 1) {

			tilecoords3x[i] = -2;
			tilecoords3y[i] = rand() % SCREENHEIGHT;

		} else if (tilecoords3x[i] + deltax <= -6) {

			tilecoords3x[i] = SCREENWIDTH - 2;
			tilecoords3y[i] = rand() % SCREENHEIGHT;

		} else {

			tilecoords3x[i] += deltax;

		}

		if (tilecoords3y[i] + deltay >= SCREENHEIGHT) {

			tilecoords3y[i] = -2;
			tilecoords3x[i] = rand() % (SCREENWIDTH - 1);

		} else if (tilecoords3y[i] + deltay <= -6) {

			tilecoords3y[i] = SCREENHEIGHT - 2;
			tilecoords3x[i] = rand() % (SCREENWIDTH - 1);

		} else {

			tilecoords3y[i] += deltay;

		}

	}



	for (uint8_t i = 0; i < 2; i++) {

		if (tilecoords4x[i] + deltax >= SCREENWIDTH - 1) {

			tilecoords4x[i] = -2;
			tilecoords4y[i] = rand() % SCREENHEIGHT;

		} else if (tilecoords4x[i] + deltax <= -6) {

			tilecoords4x[i] = SCREENWIDTH - 2;
			tilecoords4y[i] = rand() % SCREENHEIGHT;

		} else {

			tilecoords4x[i] += deltax;

		}

		if (tilecoords4y[i] + deltay >= SCREENHEIGHT) {

			tilecoords4y[i] = -2;
			tilecoords4x[i] = rand() % (SCREENWIDTH - 1);

		} else if (tilecoords4y[i] + deltay <= -6) {

			tilecoords4y[i] = SCREENHEIGHT - 2;
			tilecoords4x[i] = rand() % (SCREENWIDTH - 1);

		} else {

			tilecoords4y[i] += deltay;

		}

	}


	for (uint8_t i = 0; i < PLANETSURFDETAIL; i++) {

		if (planettexturex[i] + deltax >= SCREENWIDTH - 1) {

			planettexturex[i] = -2;
			planettexturey[i] = rand() % SCREENHEIGHT;

		} else if (planettexturex[i] + deltax <= -6) {

			planettexturex[i] = SCREENWIDTH - 2;
			planettexturey[i] = rand() % SCREENHEIGHT;

		} else {

			planettexturex[i] += deltax;

		}

		if (planettexturey[i] + deltay >= SCREENHEIGHT) {

			planettexturey[i] = -2;
			planettexturex[i] = rand() % (SCREENWIDTH - 1);

		} else if (planettexturey[i] + deltay <= -6) {

			planettexturey[i] = SCREENHEIGHT - 2;
			planettexturex[i] = rand() % (SCREENWIDTH - 1);

		} else {

			planettexturey[i] += deltay;

		}

	}




	playerposx += deltax;
	playerposy += deltay;

	if (isitem) {

		itemxoffset += deltax;
		itemyoffset += deltay;

	}

	deltax = 0;
	deltay = 0;

}



void warpeffect()
{

	arduboy.setRGBled(10, 10, 10);

	for (int i = 0; i < NUMWARPEFFECTFRAMES; i++) {

        arduboy.clear();

        for (int c = 0; c < 10; c++) {

            uint8_t xrandom = rand() % SCREENWIDTH;
            uint8_t yrandom = rand() % SCREENHEIGHT;


     
            arduboy.drawLine(xrandom, yrandom, SCREENWIDTH / 2, SCREENHEIGHT / 2, WHITE);
            
            xrandom = rand() % SCREENWIDTH;
            yrandom = rand() % SCREENHEIGHT;

            arduboy.drawLine(xrandom, yrandom, SCREENWIDTH / 2, SCREENHEIGHT / 2, BLACK);

            
        }

        arduboy.display();

        arduboy.idle();

        arduboy.delayShort(16);

    }

    arduboy.setRGBled(0, 0, 0);

}

static uint8_t helddown()
{

	static uint8_t modulu = 0;

	heldprevious = held;

	// target lock occurs when player does not move around enough.

	if (arduboy.justPressed(RIGHT_BUTTON) || ((held & (1 << BRIGHT)) > 0)) {

		held |= (1 << BRIGHT);

	} 

	if (arduboy.justPressed(LEFT_BUTTON) || ((held & (1 << BLEFT)) > 0)) {

		held |= (1 << BLEFT);

	} 


	if (arduboy.justPressed(UP_BUTTON) || ((held & (1 << BUP)) > 0)) {

		held |= (1 << BUP);


	} 


	if (arduboy.justPressed(DOWN_BUTTON) || ((held & (1 << BDOWN)) > 0)) {

		held |= (1 << BDOWN);

	}

	if (arduboy.justPressed(A_BUTTON) || ((held & (1 << BA)) > 0)) {

		 if ((combatmode && (modulu % 4 == 0)) && (ammo > 0) && (!indatabase)) {

			arduboy.drawLine(0, 64, SCREENWIDTH / 2, SCREENHEIGHT / 2, WHITE);
			arduboy.drawLine(128, 64, SCREENWIDTH / 2, SCREENHEIGHT / 2, WHITE);

			if (abs((SCREENWIDTH / 2) - itemcoordsx) < 3 && abs((SCREENHEIGHT / 2) - itemcoordsy) < 3) {

				if (hostile) {

					hostiles();

				}

			}

			ammo--;

		}
		

		held |= (1 << BA);

		modulu++;

	}

	if (arduboy.justPressed(B_BUTTON) || ((held & (1 << BB)) > 0)) {

		held |= (1 << BB);

	}


	if (arduboy.justReleased(RIGHT_BUTTON)) {

		held &= ~(1 << BRIGHT);

	}

	if (arduboy.justReleased(LEFT_BUTTON)) {

		held &= ~(1 << BLEFT);

	}

	if (arduboy.justReleased(UP_BUTTON)) {

		held &= ~(1 << BUP);

	}

	if (arduboy.justReleased(DOWN_BUTTON)) {

		held &= ~(1 << BDOWN);

	}

	if (arduboy.justReleased(A_BUTTON)) {

		held &= ~(1 << BA);

	}

	if (arduboy.justReleased(B_BUTTON)) {

		held &= ~(1 << BB);

	}

   


  return held;

}



void planetdesignation(uint16_t n)
{


	uint16_t base = 25;
	uint16_t exp  = 0;

	char str[5] = {0};

	uint8_t idx = 0;

	while (n >= 25) {

		exp = 0;

		while ((uint16_t)pow(base, exp) < n && exp < 4) {

			exp++;

		}

		str[idx++] = (char)((n / ((uint16_t)pow(base, exp - 1))) + 65);

		n = n % ((uint16_t)pow(base, exp - 1)); 

	}

	str[idx++] = (char)(n + 65);

	arduboy.setCursor(0, 16);
	arduboy.print(str);

}




bool radiusdist(uint8_t origx, uint8_t origy, uint8_t x, uint8_t y, uint8_t radius)
{


	uint8_t dist = (uint8_t)sqrt(((x - origx) * (x - origx)) + ((y - origy) * (y - origy)));

	return dist < radius;

}



// for simplicit, there can only be one planet in a 
// given solar system
void generateplanet()
{


	if (rand() % 2 == 1) {


		// sometimes planets spawn off screen 

		planetoffsetx = (rand() % 500) * (-1 + (2 * (rand() % 2)));
		planetoffsety = (rand() % 500) * (-1 + (2 * (rand() % 2)));

		planettextpts = 0;

		
		flags |= (1 << PLANETBIT);

		//habitable = (rand() % 2 == 0) ? true : false;
		flags = (rand() % 2 == 0) ? (flags | (1 << HABITABLEBIT)) : (flags & ~(1 << HABITABLEBIT));

		planetradius = (rand() % 5) + 5;

		planetxcoord = (rand() % (SCREENWIDTH - planetradius))  + planetradius;
		planetycoord = (rand() % (SCREENHEIGHT - planetradius)) + planetradius;

		// generate texture

		for (uint8_t i = 0; i < MAXPLANETSIZE * MAXPLANETSIZE; i++) {

			planettexturex[i] = 0;
			planettexturey[i] = 0;

		}

		uint8_t displacementx = MAXPLANETSIZE  * MAXPLANETSIZE;
		uint8_t displacementy = MAXPLANETSIZE  * MAXPLANETSIZE;

		uint8_t sqr = planetradius * planetradius;

		for (uint8_t i = 0; i < sqr; i++) {

			uint8_t xp = displacementx + (rand() % planetradius) * (-1 + ( 2 * (rand() % 2)));

			uint8_t yp = displacementy + (rand() % planetradius) * (-1 + ( 2 * (rand() % 2)));

			if (radiusdist(displacementx, displacementy, xp, yp, planetradius)) {

				planettexturex[planettextpts] = xp;
				planettexturey[planettextpts] = yp;

				planettextpts++;

			}

		}
			
		return;

	}

	uint8_t i;

	for (i = 0; i < MAXPLANETSIZE * MAXPLANETSIZE; i++) {

		planettexturex[i] = 0;
		planettexturey[i] = 0;

	}

	planettextpts = 0;

	
	planetxcoord = 0;
	planetycoord = 0;

	planetradius = 0;

	flags &= ~(1 << HABITABLEBIT);

	flags &= ~(1 << PLANETBIT);


}







void hostiles()
{

	if (battlewon && (!indatabase)) {

		return ;

	}

	if (rand() % 2 == 0) {

		hostile = true;

	} else {

		battlewon = true;

		hostile = false;

	}


	if (hostile) {

		itemcoordsx = rand() % SCREENWIDTH;

		itemcoordsy = rand() % SCREENHEIGHT;


	}

}





void drawhostile()
{


	uint8_t size = 5;

	uint8_t abitmore = 3;

	arduboy.drawCircle(itemcoordsx, itemcoordsy, size, WHITE);

	size += abitmore;
	
	arduboy.drawLine(itemcoordsx, itemcoordsy, itemcoordsx + size, itemcoordsy, WHITE);

	arduboy.drawLine(itemcoordsx, itemcoordsy, itemcoordsx - size, itemcoordsy, WHITE);

	arduboy.drawLine(itemcoordsx, itemcoordsy, itemcoordsx, itemcoordsy + size, WHITE);

	arduboy.drawLine(itemcoordsx, itemcoordsy, itemcoordsx, itemcoordsy - size, WHITE);


}



void drawplanet()
{

	if (!(PLANET)) {

		return;

	}


	arduboy.drawCircle(planetxcoord + planetoffsetx, planetycoord + planetoffsety, planetradius, WHITE);

	uint8_t displacementx = MAXPLANETSIZE * MAXPLANETSIZE;
	uint8_t displacementy = MAXPLANETSIZE * MAXPLANETSIZE;

	for (uint8_t i = 0; i < planettextpts; i++) {

		arduboy.drawPixel(planettexturex[i] - displacementx + planetxcoord + planetoffsetx, 
			              planettexturey[i] - displacementy + planetycoord + planetoffsety, WHITE);

	}

}




static void drawcursor()
{


	// horizontal
	arduboy.drawLine(CURSX - 4, CURSY, CURSX + 4, CURSY, WHITE);

	// vertical
	arduboy.drawLine(CURSX, CURSY - 4, CURSX, CURSY + 4, WHITE);


}



void sectorfromseed(uint16_t seed)
{


	srand(seed);

	newstars();

	generateplanet();

	hostiles();


}



void sectorparameters()
{


	srand(sector);

	newstars();

	generateplanet();

	hostiles();


}



void gotowarp()
{

	// note here is why dynamic seeding occurrs
	warpeffect();

	hostile = false;

	battlewon = false;

	playerposx = 0;
	playerposy = 0;

	sectorparameters();

}




bool autopilot()
{

	if (!(PLANET)) {

		arduboy.clear();

		arduboy.drawRect(0, 0, SCREENWIDTH, SCREENHEIGHT);

		arduboy.setCursor(34, 0);

		arduboy.print("autopilot");

		arduboy.setCursor(8, 16);
		arduboy.print("error");

		arduboy.setCursor(8, 24);
		arduboy.print("navigation failed");

		arduboy.setCursor(8, 32);
		arduboy.print("terminated");

		arduboy.display();

		arduboy.delayShort(3000);

		return true;

	}

	arduboy.setRGBled(0, 0, 0);

	int16_t distx = (SCREENWIDTH / 2) - (planetxcoord + planetoffsetx);
	int16_t disty = (SCREENHEIGHT / 2) - (planetycoord + planetoffsety);

	if (distx == 0 && disty == 0) {

		deltax = 0;
		deltay = 0;

		return true;

	}

	if (distx < 0) {

		deltax =-1;

	} else if (distx > 0) {

		deltax = 1;

	}


	if (disty < 0) {

		deltay = -1;

	} else if (disty > 0) {

		deltay = 1;

	}

	return false;

}




void positionwarp()
{

	combatmode = false;
	held = 0;

	arduboy.setRGBled(0, 0, 0);

	for (;;) {

		arduboy.clear();

		if (playerposx < 0) {

			deltax = 1;

		} else if (playerposx > 0) {

			deltax = -1;

		}

		if (playerposy < 0) {

			deltay = 1;

		} else if (playerposy > 0) {

			deltay = -1;

		}

		transformspace();
		
		drawstars();

		drawplanet();

		arduboy.display();

		arduboy.idle();

		arduboy.delayShort(16);

		if (playerposx == 0 && playerposy == 0) {

			deltay = 0;
			deltax = 0;

			return;

		}

	}


}




bool alignedwithplanet()
{


	int16_t distx = (SCREENWIDTH / 2) - (planetxcoord + planetoffsetx);
	int16_t disty = (SCREENHEIGHT / 2) - (planetycoord + planetoffsety);

	return (distx == 0 && disty == 0);


}





void printplanetcoords()
{


	int16_t x = planetxcoord + planetoffsetx;
	int16_t y = planetycoord + planetoffsety;

	arduboy.setCursor(0, 8);
	arduboy.print("x");
	arduboy.setCursor(16, 8);
	arduboy.print(x);

	arduboy.setCursor(0, 16);
	arduboy.print("y");
	arduboy.setCursor(16, 16);
	arduboy.print(y);

}




void setmovedeltas()
{


	uint8_t buttonsheld = helddown();

	bool left = (buttonsheld & (1 << BLEFT)) > 0;
	bool right = (buttonsheld & (1 << BRIGHT)) > 0;
	bool up = (buttonsheld & (1 << BUP)) > 0;
	bool down = (buttonsheld & (1 << BDOWN)) > 0;

	if (left) {

		deltax = MOVESPEED;

	}

	if (right) {

		deltax = -MOVESPEED;

	}

	if (up) {

		deltay = MOVESPEED;

	}

	if (down) {

		deltay = -MOVESPEED;

	}


}




void printsector()
{


	arduboy.setCursor(0, 56);

	arduboy.print(sector);


}



void printammo()
{


	arduboy.setCursor(0, 56);

	arduboy.print(ammo);	

}




void printwarpcrystals()
{


	arduboy.setCursor(0, 0);
	arduboy.print("warp");

	arduboy.setCursor(30, 0);
	arduboy.print(warpcrystals);

}




bool setwarpcoords()
{

	if (warpcrystals <= 0) {

		return false;

	}

	bool watinginput = true;

	int8_t coursechange = 0;

	held = 0; // clear held down buttons

	while (watinginput) {

		arduboy.clear();
	
		drawstars();

		drawcursor();

		arduboy.pollButtons();

		uint8_t buttonsheld = helddown();

		bool left = (buttonsheld & (1 << BLEFT)) > 0;
		bool right = (buttonsheld & (1 << BRIGHT)) > 0;
		bool up = (buttonsheld & (1 << BUP)) > 0;
		bool down = (buttonsheld & (1 << BDOWN)) > 0;
		bool b  = (buttonsheld & (1 << BB)) > 0;
		bool a  = (buttonsheld & (1 << BA)) > 0;
		
		if (left) {

			if ((uint8_t)abs(coursechange - 1) <= warpcrystals) {

				coursechange--;

			}

		} else if (right) {

			if ((uint8_t)abs(coursechange + 1) <= warpcrystals) {

				coursechange++;

			}			

		} 

		if (b) {

			return false;

		} else if (a && (coursechange != 0)) {

			warpcrystals -= (uint8_t)abs(coursechange);

			sector += coursechange;

			return true;

		} else {

			held &= ~(1 << BA); // set a false for next iteration to prevent glitch

		}

		drawplanet();


		arduboy.setCursor(0, 48);
		arduboy.print("set coordinates:");

		arduboy.setCursor(0, 56);
		arduboy.print(sector + coursechange);

		arduboy.setCursor(0, 0);
		arduboy.print("warp");

		arduboy.setCursor(30, 0);
		arduboy.print(warpcrystals - abs(coursechange));

		arduboy.display();

		arduboy.idle();

		arduboy.delayShort(64);

	}

}




void planetlocatedframe()
{

	uint8_t planetcenterx = SCREENWIDTH / 2;
	uint8_t planetcentery = SCREENHEIGHT / 2;

	uint8_t spacing = 2;

	uint8_t left = planetcenterx - planetradius - spacing;
	uint8_t right = planetcenterx + planetradius + spacing;
	uint8_t top = planetcentery - planetradius - spacing;
	uint8_t bottom = planetcentery + planetradius + spacing;

	uint8_t linesize = 2;

	arduboy.drawLine(left, top, left + linesize, top, WHITE);
	arduboy.drawLine(left, top, left, top + linesize, WHITE);

	arduboy.drawLine(left, bottom, left + linesize, bottom, WHITE);
	arduboy.drawLine(left, bottom, left, bottom - linesize, WHITE);



	arduboy.drawLine(right, top, right - linesize, top, WHITE);
	arduboy.drawLine(right, top, right, top + linesize, WHITE);

	arduboy.drawLine(right, bottom, right - linesize, bottom, WHITE);
	arduboy.drawLine(right, bottom, right, bottom - linesize, WHITE);

}




uint8_t mapmenu()
{

	arduboy.clear();

	// auto pilot false
	flags &= ~(1 << AUTOPILOTBIT);


	uint8_t padding = 4;

	bool canautopilot = true;

	if (PLANET) {

		arduboy.setCursor(padding, padding);

		arduboy.print("planet detected");

		if (HABITABLE && (PLANETALIGNED) && !(hostile)) {

			arduboy.setCursor(1 + padding, 8 + padding);

			arduboy.print("habitable");

			arduboy.setCursor(1 + padding, 16 + padding);

			arduboy.print("Down board planet");

		} else if (HABITABLE && (!hostile)) {

			arduboy.setCursor(1 + padding, 8 + padding);

			arduboy.print("habitable");

			arduboy.setCursor(1 + padding, 16 + padding);

			arduboy.print("A autopilot");

		} else if (!(HABITABLE) && (!hostile)) {

			arduboy.setCursor(1 + padding, 8 + padding);

			arduboy.print("uninhabitable");

			arduboy.setCursor(1 + padding, 16 + padding);

			arduboy.print("A autopilot");

		} else {

			canautopilot = false;

		}


	} else {

		arduboy.setCursor(padding, padding);

		arduboy.print("no planets detected");

	}

	arduboy.drawRect(0, 0, 128, 64, WHITE);

	bool yesrestart = false;

	if (warpcrystals == 0 && ((!(HABITABLE)) || (!(PLANET)))) {

		yesrestart = true;

		arduboy.setCursor(padding, 24 + padding);
		arduboy.print("right restart");	

	}

	arduboy.setCursor(padding, 32 + padding);

	arduboy.print("left combat mode");


	if (!(hostile) && (!combatmode)) {

		arduboy.setCursor(padding, 40 + padding);
		arduboy.print("up database");

	}


	for (;;) {

		arduboy.pollButtons();


		if (arduboy.justPressed(RIGHT_BUTTON) && yesrestart) {

			restart = true;

			return;

		}


		if (arduboy.justPressed(B_BUTTON)) {

			held = 0;

			return 0;

		} else if (arduboy.justPressed(A_BUTTON) && (!PLANETALIGNED) && (canautopilot)) {

			held = 0;

			return (PLANET) ? 1 : 0;

		} else if (PLANETALIGNED && ((!hostile)) && (arduboy.justPressed(DOWN_BUTTON)) && HABITABLE) {

			held = 0;

			return 2;

		}

		if (arduboy.justPressed(LEFT_BUTTON) && (!(AUTOPILOT))) {

			combatmode = !(combatmode);

			held = 0;

			return 0;

		}

		if (arduboy.justPressed(UP_BUTTON) && (!hostile) && (!(combatmode))) {

			indatabase = true;

			held = 0;

			return 0;

		}

		arduboy.display();
		arduboy.idle();
		arduboy.delayShort(10);

	}


	return 0;

}



void printlocation()
{

	arduboy.setCursor(80, 56);
	arduboy.print("x");

	arduboy.setCursor(90, 56);
	arduboy.print(playerposx);

	arduboy.setCursor(80, 48);
	arduboy.print("y");

	arduboy.setCursor(90, 48);
	arduboy.print(playerposy);

}



void drawplanetsurface()
{

	for (uint8_t i = 0; i < planettextpts; i++) {

		// colors inverted
		arduboy.drawPixel(planettexturex[i], planettexturey[i], WHITE);

	}



	for (uint8_t i = 0; i < 6; i++) {

		for (uint8_t j = 0; j < numused1; j++) {

			arduboy.drawPixel(arttile1x[j] + tilecoords1x[i], arttile1y[j] + tilecoords1y[i], WHITE);

		}

	}

	for (uint8_t i = 0; i < 6; i++) {

		for (uint8_t j = 0; j < numused2; j++) {

			arduboy.drawPixel(arttile2x[j] + tilecoords2x[i], arttile2y[j] + tilecoords2y[i], WHITE);

		}

	}

	for (uint8_t i = 0; i < 6; i++) {

		for (uint8_t j = 0; j < numused3; j++) {

			arduboy.drawPixel(arttile3x[j] + tilecoords3x[i], arttile3y[j] + tilecoords3y[i], WHITE);

		}

	}

	for (uint8_t i = 0; i < 2; i++) {

		for (uint8_t j = 0; j < numused4; j++) {

			arduboy.drawPixel(arttile4x[j] + tilecoords4x[i], arttile4y[j] + tilecoords4y[i], WHITE);

		}

	}


	// draw item
	if (isitem) {

		if (itemwarp) {

			arduboy.drawCircle(itemcoordsx + itemxoffset + (SCREENWIDTH / 2), itemcoordsy + itemyoffset + (SCREENHEIGHT / 2), 2, WHITE);
			arduboy.fillCircle(itemcoordsx + itemxoffset + (SCREENWIDTH / 2), itemcoordsy + itemyoffset + (SCREENHEIGHT / 2), 1, BLACK);

		} else {

			arduboy.drawCircle(itemcoordsx + itemxoffset + (SCREENWIDTH / 2), itemcoordsy + itemyoffset + (SCREENHEIGHT / 2), 2, WHITE);
			arduboy.fillCircle(itemcoordsx + itemxoffset + (SCREENWIDTH / 2), itemcoordsy + itemyoffset + (SCREENHEIGHT / 2), 1, WHITE);

		}

		

	}




}




bool itemclose()
{


	int16_t xdist = abs(0 - (itemcoordsx + itemxoffset));
	int16_t ydist = abs(0 - (itemcoordsy + itemyoffset));

	if (xdist < 40 && ydist < 40) {

		return true;

	} 



	return false;  

}




void generateitem()
{

	if (rand() % 2 == 0) {

		isitem = true;

		itemwarp = (rand() % 2 == 0) ? true : false;

		itemcoordsx = (rand() % 100) * (-1 + 2 * (rand() % 2));
		itemcoordsy = (rand() % 100) * (-1 + 2 * (rand() % 2));

	} else {

		isitem = false;

	}


}



bool isresourcerich()
{


	if (rand() % 2 == 0) {

		return true;

	} else {

		return false;

	}

}




// generate procedural planet surface 
void generateplanetsurf()
{

	arduboy.setRGBled(0, 0, 0);

	planettextpts = 0;

	uint8_t i = 0;

	generateitem();

	for (i = 0; i < PLANETSURFDETAIL; i++) {

		planettexturex[i] = rand() % SCREENWIDTH;
		planettexturey[i] = rand() % SCREENHEIGHT;

	}

	planettextpts = PLANETSURFDETAIL;


	for (uint8_t i = 0; i < 6; i++) {

		tilecoords1x[i] = rand() % SCREENWIDTH;
		tilecoords2x[1] = rand() % SCREENWIDTH;
		tilecoords3x[i] = rand() % SCREENWIDTH;
		tilecoords4x[i] = rand() % SCREENWIDTH;

		tilecoords1y[i] = rand() % SCREENHEIGHT;
		tilecoords2y[i] = rand() % SCREENHEIGHT;
		tilecoords3y[i] = rand() % SCREENHEIGHT;
		tilecoords4y[i] = rand() % SCREENHEIGHT;

	}


	numused1 = (rand() % 6) + 1;

	int8_t pt[2] = {0, 0};

	for (uint8_t i = 0; i < numused1; i++) {

		int8_t add[2] = {-1 + (rand() % 3), -1 + (rand() % 3)};
		int8_t np[2] = {pt[0] + add[0], pt[1] + add[1]};

		arttile1x[i] = np[0];
		arttile1y[i] = np[1];

		pt[0] = np[0];
		pt[1] = np[1];

	}

	numused2 = (rand() % 6) + 1; 

	//pt[2] = {SCREENWIDTH / 2, SCREENHEIGHT / 2};
	pt[0] = 0;
	pt[1] = 0;

	for (uint8_t i = 0; i < numused2; i++) {

		int8_t add[2] = {-1 + (rand() % 3), -1 + (rand() % 3)};
		int8_t np[2] = {pt[0] + add[0], pt[1] + add[1]};

		arttile2x[i] = np[0];
		arttile2y[i] = np[1];

		pt[0] = np[0];
		pt[1] = np[1];

	} 

	numused3 = (rand() % 6) + 1;

	pt[0] = 0;
	pt[1] = 0;

	for (uint8_t i = 0; i < numused3; i++) {

		int8_t add[2] = {-1 + (rand() % 3), -1 + (rand() % 3)};
		int8_t np[2] = {pt[0] + add[0], pt[1] + add[1]};

		arttile3x[i] = np[0];
		arttile3y[i] = np[1];

		pt[0] = np[0];
		pt[1] = np[1];

	}



	numused4 = (rand() % 35) + 1;

	pt[0] = 0;
	pt[1] = 0;

	for (uint8_t i = 0; i < numused4; i++) {

		int8_t add[2] = {-1 + (rand() % 3), -1 + (rand() % 3)};
		int8_t np[2] = {pt[0] + add[0], pt[1] + add[1]};

		arttile4x[i] = np[0];
		arttile4y[i] = np[1];

		pt[0] = np[0];
		pt[1] = np[1];

	} 	


	arduboy.invert(true);

	arduboy.idle();

	arduboy.delayShort(2);


}




void onplanetmenu(bool close)
{


	uint8_t padding = 6;

	arduboy.invert(false);

	held = 0;

	for (;;) {

		arduboy.pollButtons();

		arduboy.clear();

		arduboy.drawRect(2, 2, 124, 60, WHITE);

		arduboy.setCursor(padding, padding);

		arduboy.print("A back to space");

		if (warpcrystals == 0 && (!isitem)) {

			arduboy.setCursor(padding, padding + 8);
			arduboy.print("right restart");

		}

		if (itemclose() && isitem) {

			arduboy.setCursor(padding, 8 + padding);
			arduboy.print("up collect item");

			arduboy.setCursor(padding, 16 + padding);

			if (itemwarp) {

				arduboy.print("+9 warp crystals");

			} else {

				arduboy.print("+512 ammo");

			}

			

			if (arduboy.justPressed(UP_BUTTON)) {

				if (itemwarp) {

					if (warpcrystals + 9 <= MAXWARPCRYSTALS) {

						warpcrystals += 9;

					}	

				} else {

					if (ammo + 512 < 65535) {

						ammo += 512;

					}

				}

				generateitem();

				arduboy.invert(true);

				return;


			}

		}

		// returning to space from planet
		if (arduboy.justPressed(A_BUTTON)) {

			playerposx = playerposxbackup;
			playerposy = playerposybackup;

			sectorparameters();

			planetoffsetx = planetoffsetxbackup;
			planetoffsety = planetoffsetybackup;

			arduboy.invert(false);

			flags |= (1 << ISINSPACEBIT);

			return;

		}

	
		if (arduboy.justPressed(B_BUTTON)) {

			arduboy.invert(true);

			return;

		}

		if (arduboy.justPressed(RIGHT_BUTTON) && (warpcrystals == 0) && (!(isitem))) {

			restart = true;

			arduboy.invert(false);

			return;

		}

		arduboy.display();

		arduboy.idle();

		arduboy.delayShort(16);

	}

}


void onplanet()
{

	setmovedeltas();

	transformonplanet();

	drawplanetsurface();

	printlocation();

	if (isitem) {

		arduboy.setCursor(0, 0);
		arduboy.print("item");

		arduboy.setCursor(0, 8);
		arduboy.print("y");
		arduboy.setCursor(8, 8);
		arduboy.print(-itemcoordsy);

		arduboy.setCursor(0, 16);
		arduboy.print("x");
		arduboy.setCursor(8, 16);
		arduboy.print(-itemcoordsx);

		if (itemclose()) {

			arduboy.setRGBled(0, 25, 0);

		} else {

			arduboy.setRGBled(0, 0, 0);

		}

	} else {

		arduboy.setRGBled(0, 0, 0);

	}

	if (arduboy.justPressed(B_BUTTON)) {


		onplanetmenu(true);
	}


}




void querydatabase(uint16_t seed)
{


	sectorfromseed(seed);

	srand(seed);

	bool isrich = isresourcerich();

	planetxcoord = (SCREENWIDTH / 2);
	planetycoord = (SCREENHEIGHT / 2);

	planetoffsetx = 0;
	planetoffsety = 0;

	for (;;) {

		arduboy.pollButtons();

		if (arduboy.justPressed(B_BUTTON)) {

			return ;

		}

		arduboy.clear();

		if (PLANET) {

			drawplanet();

			if (HABITABLE) {

				arduboy.print("planet is habitable");

				if (isrich) {

					arduboy.setCursor(0, 8);

					arduboy.print("resource rich");

				}



			} else {

				arduboy.print("planet not habitable");

			}

			planetdesignation(seed);

		} else {

			arduboy.print("no planet");

		}

		if (hostile) {

			arduboy.setCursor(0, 40);

			arduboy.print("Hostiles");

		}
		

		arduboy.display();

		arduboy.idle();

		arduboy.delayShort(16);

	}

}	




void enterdatabase()
{

	static uint16_t sect = sector;

	planetoffsetxbackup = planetoffsetx;
	planetoffsetybackup = planetoffsety;

	for (;;) {

		arduboy.clear();

		arduboy.pollButtons();

		uint8_t buttonsheld = helddown();

		bool left = (buttonsheld & (1 << BLEFT)) > 0;
		bool right = (buttonsheld & (1 << BRIGHT)) > 0;
		bool up = (buttonsheld & (1 << BUP)) > 0;
		bool down = (buttonsheld & (1 << BDOWN)) > 0;

		if (left) {

			sect--;

		} else if (right) {

			sect++;

		} else if (up) {

			sect *= 2;

		} else if (down) {

			sect /= 2;

		} else if (arduboy.justPressed(B_BUTTON)) {

			sectorparameters();

			if (hostile && battlewon) {

				hostile = false;

			}

			//playerposx = 0;
			//playerposy = 0;

			indatabase = false;

			held = 0;

			planetoffsetx = planetoffsetxbackup;
			planetoffsety = planetoffsetybackup;

			return;

		} else if (arduboy.justPressed(A_BUTTON)) {

			querydatabase(sect);

			continue;

		}

		arduboy.print("sector: ");

		arduboy.setCursor(50,0);

		arduboy.print(sect);

		arduboy.setCursor(0, 8);

		arduboy.print("view sector: A");

		arduboy.display();


		arduboy.idle();

		arduboy.delayShort(64);

	}


}





void inspace()
{

	if (!(indatabase)) {

		if (arduboy.justPressed(A_BUTTON)) {


		if ((!combatmode) && !(hostile)) {

			// user can cancel by press B
			bool warp = setwarpcoords();

			if (warp) {

				//planetoffsetx = 0;
				//planetoffsety = 0;

				positionwarp();

				gotowarp();

				held = 0;

			}

		} 

	} else {

		if (arduboy.justPressed(B_BUTTON)) {

			uint8_t flag = mapmenu();

			if (indatabase) {

				return ;

			} else if (flag != 2) {

				flags = (flag == 1) ? (flags | (1 << AUTOPILOTBIT)) : (flags & ~(1 << AUTOPILOTBIT));

				if (AUTOPILOT) {

					combatmode = false;

				}

			} else {


				// boarding planet
				srand(sector);

				combatmode = false;

				deltay = 0;
				deltax = 0;

				held = 0;

				generateplanetsurf();

				playerposxbackup = playerposx;
				playerposybackup = playerposy;

				planetoffsetxbackup = planetoffsetx;
				planetoffsetybackup = planetoffsety;

				playerposx = 0;
				playerposy = 0;

				itemxoffset = 0;
				itemyoffset = 0;

				//isinspace = false;
				flags &= ~(1 << ISINSPACEBIT);
				flags &= ~(1 << AUTOPILOTBIT);

				return;


			}

		}

	}

	if (!(AUTOPILOT) && (!(indatabase))) {

		setmovedeltas();

	} else if (!(indatabase)) {

		bool terminated = autopilot();

		if (terminated) {

			flags &= ~(1 << AUTOPILOTBIT);

		}

	}

	bool isaligned = alignedwithplanet();
	flags = (isaligned) ? (flags | (1 << (PLANETALIGNBIT))) : (flags & ~(1 << (PLANETALIGNBIT)));


	if (PLANETALIGNED) {

		planetlocatedframe();

		planetdesignation(sector);


		if (HABITABLE) {

			arduboy.setRGBled(0, 50, 0);

		} else {

			arduboy.setRGBled(50, 0, 0);

		}

	} else {

		arduboy.setRGBled(0, 0, 0);

	}

	
	// move everything in space
	transformspace();

	drawcursor();

	drawstars();

	if (PLANET) {

		drawplanet();

	}

	printwarpcrystals();

	if (!(combatmode)) {

		printsector();

	} else {

		arduboy.setCursor(0, 48);

		arduboy.print("combat mode");

		printammo();

	}

	

	printlocation();


	if (hostile) {

		arduboy.setCursor(80, 0);
		
		arduboy.print("hostiles");

		if (targetlock >= 50) {

			arduboy.setCursor(0, 8);
			arduboy.print("target lock");

			if (!(combatmode)) {

				arduboy.setCursor(0, 16);
				arduboy.print("set combat mode");

			}

		}

		if ((held == heldprevious) || (itemcoordsx < 0 || itemcoordsx > SCREENWIDTH || itemcoordsy < 0 || itemcoordsy > SCREENHEIGHT)) {

			targetlock += 2;

		} else {

			if (combatmode) {

				targetlock = 0;

			}

		}

		if (targetlock == TARGETLOCK) {



			restart = true;

		}

		if (targetlock > 0) {

			arduboy.setRGBled(targetlock, 0, 0);

		}

		drawhostile();

	}


	} else {

		enterdatabase();

	}

}




// main game loop
void loop()
{


	if (!arduboy.nextFrame()) {

		return;

	}


	arduboy.clear();

	// all functions which use this come after here
	// to avoid accidentally calling it twice.
	arduboy.pollButtons();

	if (ISINSPACE) {

		inspace();

	} else {

		onplanet();

	}

	
	

	if (!(restart)) {

		arduboy.display();

	}

	

	if (restart) {

		restartgame();

		restart = false;

	}

	arduboy.idle();

}



