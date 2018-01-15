#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>

#define TFT_DC 9
#define TFT_CS 10

#define serverPin 13

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define JOY_VERT  A1 // should connect A1 to pin VRx
#define JOY_HORIZ A0 // should connect A0 to pin VRy
#define JOY_SEL   2

#define JOY_CENTER 512
#define JOY_SPEED 64

uint8_t selectedTank = 0;
uint8_t selectedStart = 0;
uint8_t selectedTank3 = 0;

typedef struct {
  int length;
  int height;
  int power_big;
  int power_small;
  int HP;
  uint16_t color;
} tank;

tank tanks[3];
tank Mytank;
tank othertank;

int fullHP;
int fullHP3;

int Wind = 0;
int HeightOfWall = 80;

void setup() {
  init();
  Serial.begin(9600);
  Serial3.begin(9600);
  tft.begin();

  tft.setRotation(3);

  tft.fillScreen(ILI9341_BLACK);

  pinMode(JOY_SEL, INPUT_PULLUP);
  pinMode(serverPin, INPUT);

  tanks[0] = {24, 10, 100, 400, 3000, ILI9341_RED};
  tanks[1] = {18, 8, 150, 450, 2000, ILI9341_YELLOW};
  tanks[2] = {12, 6, 250, 550, 1000, ILI9341_BLUE};
}





double degreeToRadians(int degree) {
  return degree*3.14159/180;
}





void selectStartMode() {
  String startMenu[3] = {"Server Start", "Client Start", "Random Start"};
  tft.setTextSize(2);
  tft.fillRect(0, 100, 320, 140,ILI9341_BLACK);

  do {
    int yVal = analogRead(JOY_VERT);
    int oldStart = selectedStart;
    if((yVal - JOY_CENTER) > JOY_SPEED) {
      selectedStart++;
    }
    else if((JOY_CENTER - yVal) > JOY_SPEED && selectedStart != 0) {
      selectedStart--;
    }

    selectedStart = constrain(selectedStart,0,2);

    for(int i = 0; i < 3; i++) {
      tft.setCursor(100,110+i*40);
      if(i != selectedStart) {
        tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      }
      else {
        tft.setTextColor(ILI9341_BLACK,ILI9341_WHITE);
      }
      tft.print(startMenu[i]);
      tft.print("\n");
    }
    tft.print("\n");

    if(!digitalRead(JOY_SEL)) {
      if(selectedStart == 2) {
        selectedStart = random(2);
      }
      Serial3.write(selectedStart);
      break;
    }

    delay(100);
  } while(true);
}

void startMenu(){
	tft.fillScreen(0);
  tft.setCursor(30, 50);
  tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
  tft.setTextSize(3);
  tft.println("ITALIAN CANNON");

  tft.setTextSize(3);
  tft.setCursor(120, 150);
  tft.println("Start");

  tft.fillTriangle(90, 174, 90, 150, 108, 162, ILI9341_WHITE);
  while(true) {
    if(!digitalRead(JOY_SEL)) {
      selectStartMode();
      break;
    }
  }
}








void drawTank(tank t, int x, int y, int LorR) {
  tft.fillRect(x-t.length/3, y-t.height/2, t.length*2/3, t.height/2, t.color);
  tft.fillRect(x-t.length/2, y, t.length, t.height/2, t.color);
  if(LorR == 1) {
    tft.fillRect(x,y-t.height/2+1, 16, 2, t.color);
  }
  else {
    tft.fillRect(x-16,y-t.height/2+1, 16, 2, t.color);
  }
  for(int i = -1; i < 2; i++) {
    tft.fillCircle(x + i*t.length/3, y+t.height/2+2, t.length/6, t.color);
  }
}

void selectTank() {
  for(int i = 0; i < 3; i++) {
    int Vcentre = 50+i*100+(i+1)*5;
    if(i == selectedTank) {
      tft.fillRect(Vcentre-50, 10, 100, 206,ILI9341_WHITE);
      tft.fillRect(Vcentre-30, 30, 60, 60, ILI9341_BLACK);
      tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
      tft.fillCircle(Vcentre-15, 110, 2, ILI9341_BLACK);
      tft.fillCircle(Vcentre+15, 110, 4, ILI9341_BLACK);
    }
    else {
      tft.drawRect(Vcentre-50, 10, 100, 206,ILI9341_WHITE);
      tft.drawRect(Vcentre-30, 30, 60, 60, ILI9341_WHITE);
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      tft.fillCircle(Vcentre-15, 110, 2, ILI9341_WHITE);
      tft.fillCircle(Vcentre+15, 110, 4, ILI9341_WHITE);
    }
    tft.setCursor(Vcentre-40, 130);
    tft.setTextSize(2);
    tft.print(tanks[i].power_small);
    tft.setCursor(Vcentre+5, 130);
    tft.print(tanks[i].power_big);
    tft.setCursor(Vcentre-30, 160);
    tft.setTextSize(2);
    tft.print("HP:");
    tft.setCursor(Vcentre-30, 180);
    tft.print(tanks[i].HP);

    drawTank(tanks[i], Vcentre, 60, 1);
  }
}

void drawpanel() {
  while(Serial3.available() > 0) {
    Serial3.read();
  }
  tft.fillScreen(0);
  selectTank();
  while(true) {
    int xVal = analogRead(JOY_HORIZ);
    int oldtank = selectedTank;
    if((xVal - JOY_CENTER) > JOY_SPEED && selectedTank > 0) {
      selectedTank--;
    }
    else if((JOY_CENTER - xVal) > JOY_SPEED) {
      selectedTank++;
    }

    selectedTank = constrain(selectedTank, 0, 2);

    if(oldtank != selectedTank) {
      tft.fillRect(oldtank*100+5*(oldtank+1), 10, 100, 206, ILI9341_BLACK);
      selectTank();
    }


    if(!digitalRead(JOY_SEL)) {
      Serial3.write(selectedTank);
      while(Serial3.available() == 0) { delay(10); }
      selectedTank3 = Serial3.read();
      break;
    }
    delay(100);
  }
}







void redrawbullet(int curX, int curY, int nextX, int nextY, int rad) {
  tft.fillCircle(curX, curY, rad, ILI9341_BLACK);
  tft.fillCircle(nextX, nextY, rad, ILI9341_WHITE);
}

void drawMap(){
	tft.fillScreen(0);
	tft.fillRect(10, 15, 120, 10, ILI9341_RED); // HP left
	tft.fillRect(190, 15, 120, 10, ILI9341_RED); // HP right
	tft.fillRect(145, 210-HeightOfWall, 30, HeightOfWall, ILI9341_YELLOW); // Wall
  tft.fillRect(0, 210, 320, 30, 0x826735); // ground
	tft.setCursor(138, 15);
	tft.setTextColor(ILI9341_WHITE);
	tft.print("WIND");
  tft.fillTriangle(172, 46, 172, 30, 182, 38, ILI9341_WHITE);
  tft.fillRect(138, 30, 36, 16, ILI9341_BLACK);
  tft.setCursor(138, 30);
  tft.setTextSize(2);
  tft.print(Wind);
  if(digitalRead(serverPin)) {
    tft.setCursor(10, 30);
    tft.print(Mytank.HP);
    tft.setCursor(262, 30);
    tft.print(othertank.HP);
  }
  else {
    tft.setCursor(10, 30);
    tft.print(othertank.HP);
    tft.setCursor(262, 30);
    tft.print(Mytank.HP);
  }

}

void drawMapTank(boolean server) {
  if(server) {
    drawTank(tanks[selectedTank], 20, 210 - tanks[selectedTank].height/2, 1);
    drawTank(tanks[selectedTank3], 300, 210 - tanks[selectedTank3].height/2, 2);
  }
  else {
    drawTank(tanks[selectedTank3], 20, 210 - tanks[selectedTank3].height/2, 1);
    drawTank(tanks[selectedTank], 300, 210 - tanks[selectedTank].height/2, 22222);
  }
}

void redrawWall() {
  tft.fillRect(135, 60, 50, 150, ILI9341_BLACK);
  tft.fillRect(145, 210-HeightOfWall, 30, HeightOfWall, ILI9341_YELLOW);
}

void redrawWind() {
  tft.fillTriangle(172, 46, 172, 30, 182, 38, ILI9341_WHITE);
  tft.fillRect(138, 30, 36, 16, ILI9341_BLACK);
  tft.setCursor(138, 15);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("WIND");
  tft.setCursor(138, 30);
  tft.print(Wind);
}

void redrawHP(tank t, int full_HP, int position) {
  double pHP = t.HP;
  double newfullHP = full_HP;
  double percentHP = pHP/newfullHP;
  int HP = percentHP * 120;
  if(position == 0) {
    tft.fillRect(10, 15, 120, 10, ILI9341_BLACK);
    tft.fillRect(10, 15, HP, 10, ILI9341_RED);
    tft.fillRect(10, 30, 50, 20, ILI9341_BLACK);
    tft.setCursor(10, 30);
    tft.print(t.HP);
  }
  else {
    tft.fillRect(190, 15, 120, 10, ILI9341_BLACK);
    tft.fillRect(310-HP, 15, HP, 10, ILI9341_RED);
    tft.fillRect(310-50, 30, 50, 20, ILI9341_BLACK);
    if(t.HP >= 1000) {
      tft.setCursor(310-4*12, 30);
    }
    else if(t.HP >= 100) {
      tft.setCursor(310-3*12, 30);
    }
    else if(t.HP >= 10) {
      tft.setCursor(310-2*12, 30);
    }
    else {
      tft.setCursor(310-12, 30);
    }
    tft.print(t.HP);
  }
}

boolean hitTank(tank t, int target, int x, int y, int bullet) {
  boolean hit = false;
  if(target == 0) {
    // upper rectangle
    if(abs(x-20) < t.length/3+bullet && y > 210-t.height) {
      hit = true;
    }
    // lower rectangle
    else if(abs(x-20) < t.length/2+bullet && y > 210-t.height/2) {
      hit = true;
    }
  }
  else if(target == 1) {
    if(abs(x-300) < t.length/3+bullet && y > 210-t.height) {
      hit = true;
    }
    else if(abs(x-300) < t.length/2+bullet && y > 210-t.length/2) {
      hit = true;
    }
  }
  return hit;
}

boolean hitEdgeandGround(int x, int y, int bullet) {
  if(x >= 320 || x <= 0 || y >= 210-bullet) {
    return true;
  }
  else {
    return false;
  }
}

boolean hitWall(int x, int y, int bullet) {
  if((abs(x-160) <= bullet+15) && abs(210-y) < bullet+HeightOfWall) {
    return true;
  }
  else {
    return false;
  }
}

void fire(int select, int initial, int angle, int selectBullet, int server) {
  double newAngle = degreeToRadians(angle);
  double newInitial = initial;
  if(select == 0) {
    int pX = 40;
    int pY = 205;
    int nX = pX;
    int nY = pY;
    while(true) {
      redrawbullet(pX, pY, nX, nY, selectBullet);
      pX = nX;
      pY = nY;
      nX += initial*cos(newAngle)+Wind;
      nY -= newInitial*sin(newAngle);
      newInitial -= 1;
      redrawWind();
      if(server == 0) {
        redrawHP(othertank, tanks[selectedTank3].HP, 1);
      }
      else {
        redrawHP(Mytank, tanks[selectedTank].HP, 1);
      }
      if(server == 0) {
        if(hitTank(othertank, 1, pX, pY, selectBullet)) {
          if(selectBullet == 2) {
            othertank.HP -= Mytank.power_small;
          }
          else {
            othertank.HP -= Mytank.power_big;
          }
          redrawHP(othertank, tanks[selectedTank3].HP, 1);
          tft.fillRect(0, 0, 10, 240, ILI9341_BLACK);
          tft.fillRect(310, 0, 10, 240, ILI9341_BLACK);
          tft.fillRect(0, 180, 320, 60, ILI9341_BLACK);
          tft.fillRect(0, 210, 320, 30, 0x826735);
          redrawWall();
          drawMapTank(true);
          break;
        }
      }
      else {
        if(hitTank(Mytank, 1, pX, pY, selectBullet)) {
          if(selectBullet == 2) {
            Mytank.HP -= othertank.power_small;
          }
          else {
            Mytank.HP -= othertank.power_big;
          }
          redrawHP(Mytank, tanks[selectedTank].HP, 1);
          tft.fillRect(0, 0, 10, 240, ILI9341_BLACK);
          tft.fillRect(310, 0, 10, 240, ILI9341_BLACK);
          tft.fillRect(0, 180, 320, 60, ILI9341_BLACK);
          tft.fillRect(0, 210, 320, 30, 0x826735);
          redrawWall();
          drawMapTank(false);
          break;
        }
      }

      if(hitWall(pX, pY, selectBullet)) {
        redrawWall();
        break;
      }
      else if(hitEdgeandGround(pX, pY, selectBullet)) {
        tft.fillRect(0, 0, 10, 240, ILI9341_BLACK);
        tft.fillRect(310, 0, 10, 240, ILI9341_BLACK);
        tft.fillRect(0, 180, 320, 60, ILI9341_BLACK);
        tft.fillRect(0, 210, 320, 30, 0x826735);
        redrawWall();
        if(server == 0) {
          drawMapTank(true);
        }
        else {
          drawMapTank(false);
        }
        break;
      }
      delay(50);
    }
  }
  else if(select == 1) {
    int pX = 280;
    int pY = 205;
    int nX = pX;
    int nY = pY;
    while(true) {
      redrawbullet(pX, pY, nX, nY, selectBullet);
      pX = nX;
      pY = nY;
      nX -= initial*cos(newAngle)-Wind;
      nY -= newInitial*sin(newAngle);
      newInitial -= 1;
      redrawWind();
      if(server == 0) {
        redrawHP(Mytank, tanks[selectedTank].HP, 0);
      }
      else {
        redrawHP(othertank, tanks[selectedTank3].HP, 0);
      }
      if(server == 0) {
        if(hitTank(Mytank, 0, pX, pY, selectBullet)) {
          if(selectBullet == 2) {
            Mytank.HP -= othertank.power_small;
          }
          else {
            Mytank.HP -= othertank.power_big;
          }
          redrawHP(Mytank, tanks[selectedTank].HP, 0);
          tft.fillRect(0, 0, 10, 240, ILI9341_BLACK);
          tft.fillRect(310, 0, 10, 240, ILI9341_BLACK);
          tft.fillRect(0, 180, 320, 60, ILI9341_BLACK);
          tft.fillRect(0, 210, 320, 30, 0x826735);
          redrawWall();
          drawMapTank(true);
          break;
        }
      }
      else {
        if(hitTank(othertank, 0, pX, pY, selectBullet)) {
          if(selectBullet == 2) {
            othertank.HP -= Mytank.power_small;
          }
          else {
            othertank.HP -= Mytank.power_big;
          }
          redrawHP(othertank, tanks[selectedTank3].HP, 0);
          tft.fillRect(0, 0, 10, 240, ILI9341_BLACK);
          tft.fillRect(310, 0, 10, 240, ILI9341_BLACK);
          tft.fillRect(0, 180, 320, 60, ILI9341_BLACK);
          tft.fillRect(0, 210, 320, 30, 0x826735);
          redrawWall();
          drawMapTank(false);
          break;
        }
      }

      if(hitWall(pX, pY, selectBullet)) {
        redrawWall();
        break;
      }
      else if(hitEdgeandGround(pX, pY, selectBullet)) {
        tft.fillRect(0, 0, 10, 240, ILI9341_BLACK);
        tft.fillRect(310, 0, 10, 240, ILI9341_BLACK);
        tft.fillRect(0, 180, 320, 60, ILI9341_BLACK);
        tft.fillRect(0, 210, 320, 30, 0x826735);
        redrawWall();
        if(server == 0) {
          drawMapTank(true);
        }
        else {
          drawMapTank(false);
        }
        break;
      }
      delay(50);
    }
  }
}


void attack(int select, int server) {
  uint8_t initial = 0;
  uint8_t angle = 15;
  uint8_t selectBullet = 2;

  if(select == 0) {
    tft.fillCircle(40, 205, 4, ILI9341_BLACK);
    tft.fillCircle(40, 205, selectBullet, ILI9341_WHITE);
  }
  else {
    tft.fillCircle(280, 205, 4, ILI9341_BLACK);
    tft.fillCircle(280, 205, selectBullet, ILI9341_WHITE);
  }


  while(true) {
    int yval = analogRead(JOY_VERT);
    int oldBullet = selectBullet;


    if((yval - JOY_CENTER) > JOY_SPEED && selectBullet > 2) {
      selectBullet -= 2;
    }
    else if((JOY_CENTER - yval) > JOY_SPEED && selectBullet < 4) {
      selectBullet += 2;
    }

    if(oldBullet != selectBullet) {
      if(select == 0) {
        tft.fillCircle(40, 205, 4, ILI9341_BLACK);
        tft.fillCircle(40, 205, selectBullet, ILI9341_WHITE);
      }
      else {
        tft.fillCircle(280, 205, 4, ILI9341_BLACK);
        tft.fillCircle(280, 205, selectBullet, ILI9341_WHITE);
      }
    }
    if(!digitalRead(JOY_SEL)) {
      while(!digitalRead(JOY_SEL)) { delay(10); }
      break;
    }
    delay(100);
  }

  if(select == 0) {
    tft.fillRect(5, 160, 70, 20, ILI9341_BLACK);
    tft.setCursor(5, 160);
    tft.setTextSize(1);
    tft.print("speed: ");
    tft.print(initial);
    tft.print("00");
    tft.setCursor(5, 170);
    tft.print("angle: ");
    tft.print(angle);
  }
  else {
    tft.fillRect(245, 160, 70, 20, ILI9341_BLACK);
    tft.setCursor(245, 160);
    tft.setTextSize(1);
    tft.print("speed: ");
    tft.print(initial);
    tft.print("00");
    tft.setCursor(245, 170);
    tft.print("angle: ");
    tft.print(angle);
  }

  while(true) {
    int yVal = analogRead(JOY_VERT);
    int xVal = analogRead(JOY_HORIZ);

    uint8_t oldi = initial;
    uint8_t olda = angle;
    if((yVal - JOY_CENTER) > JOY_SPEED && angle > 0) {
      angle -= 1;
    }
    else if((JOY_CENTER - yVal) > JOY_SPEED) {
      angle += 1;
    }

    if(select == 0) {
      if((xVal - JOY_CENTER) > JOY_SPEED && initial > 0) {
        initial -= 1;
      }
      else if((JOY_CENTER - xVal) > JOY_SPEED) {
        initial += 1;
      }
    }
    else if(select == 1) {
      if((xVal - JOY_CENTER) > JOY_SPEED) {
        initial += 1;
      }
      else if((JOY_CENTER - xVal) > JOY_SPEED && initial > 0) {
        initial -= 1;
      }
    }


    initial = constrain(initial, 0, 20);
    angle = constrain(angle, 15, 60);

    if(oldi != initial || olda != angle) {
      if(select == 0) {
        tft.fillRect(5, 160, 70, 20, ILI9341_BLACK);
        tft.setCursor(5, 160);
        tft.setTextSize(1);
        tft.print("speed: ");
        tft.print(initial);
        tft.print("00");
        tft.setCursor(5, 170);
        tft.print("angle: ");
        tft.print(angle);
      }
      else {
        tft.fillRect(245, 160, 70, 20, ILI9341_BLACK);
        tft.setCursor(245, 160);
        tft.setTextSize(1);
        tft.print("speed: ");
        tft.print(initial);
        tft.print("00");
        tft.setCursor(245, 170);
        tft.print("angle: ");
        tft.print(angle);
      }
    }

    if(!digitalRead(JOY_SEL)) {
      tft.fillRect(5, 160, 70, 20, ILI9341_BLACK);
      tft.fillRect(245, 160, 70, 20, ILI9341_BLACK);
      break;
    }

    delay(100);
  }

  Serial3.write(selectBullet);
  Serial3.write(initial);
  Serial3.write(angle);
  fire(select, initial, angle, selectBullet, server);
}

void response(int select, int server) {
  while(Serial3.available() > 0) {
    Serial3.read();
  }
  while(Serial3.available() == 0) { delay(10); }
  uint8_t selectBullet = Serial3.read();
  uint8_t initial = Serial3.read();
  uint8_t angle = Serial3.read();
  fire(select, initial, angle, selectBullet, server);
}






void newWind() {
  Wind = -((Wind * 29) % 10 - 7);
  Wind = constrain(Wind, -3, 3);
}

void newWall() {
  HeightOfWall = HeightOfWall * 13 % 120 - HeightOfWall/5;
}





void serverMode2(int att) {
  if(att == 0) {
    attack(0,0);
  }
  else if(att == 1) {
    response(1,0);
  }
}

void clientMode2(int att) {
  if(att == 0) {
    response(0,1);
  }
  else if(att == 1) {
    attack(1,1);
  }
}

void server() {
  startMenu();
  drawpanel();
  Mytank = tanks[selectedTank];
  othertank = tanks[selectedTank3];
  drawMap();
  drawMapTank(true);
}

void client() {
  tft.fillScreen(0);
  tft.setCursor(55,113);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
  tft.print("WAIT FOR BEGINNING");
  while(Serial3.available() == 0) { delay(10); }
  selectedStart = Serial3.read();
  drawpanel();
  Mytank = tanks[selectedTank];
  othertank = tanks[selectedTank3];
  drawMap();
  drawMapTank(false);
}






int main() {
  setup();

  boolean SOrC = digitalRead(serverPin);
  if(SOrC) {
    server();
  }
  else {
    client();
  }


  if(selectedStart == 0) {
    int select = 0;

    while(true) {
      if(SOrC) {
        serverMode2(select);
      }
      else {
        clientMode2(select);
      }
      if(select == 0) {
        select = 1;
      }
      else{
        select = 0;
      }
      newWind();
      newWall();
      redrawWind();
      redrawWall();
      if(Mytank.HP <= 0) {
        tft.fillScreen(0);
        tft.setCursor(55, 100);
        tft.setTextSize(5);
        tft.print("YOU WIN");
        break;
      }
      else if(othertank.HP <= 0) {
        tft.fillScreen(0);
        tft.setCursor(40, 100);
        tft.setTextSize(5);
        tft.print("YOU LOSE");
        break;
      }
    }
  }
  else {
    int select = 1;

    while(true) {
      if(SOrC) {
        serverMode2(select);
      }
      else {
        clientMode2(select);
      }
      if(select == 0) {
        select = 1;
      }
      else{
        select = 0;
      }
      newWind();
      newWall();
      redrawWind();
      redrawWall();
      if(Mytank.HP <= 0) {
        tft.fillScreen(0);
        tft.setCursor(40, 100);
        tft.setTextSize(5);
        tft.print("YOU LOSE");
        break;
      }
      else if(othertank.HP <= 0) {
        tft.fillScreen(0);
        tft.setCursor(55, 100);
        tft.setTextSize(5);
        tft.print("YOU WIN");
        break;
      }
    }
  }
  Serial.flush();
  Serial3.flush();

  return 0;
}
