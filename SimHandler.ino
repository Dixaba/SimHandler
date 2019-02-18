#include <LiquidCrystal_I2C.h>
#include <timer-api.h>
#include <SoftwareSerial.h>

#define btn 4
#define BLT 3
#define LT 3

SoftwareSerial SIM900(7, 8);
LiquidCrystal_I2C lcd(0x27, 16, 2);

volatile bool btnPressed = false;
volatile bool off = true;
volatile bool waitLight = false;
volatile bool needUpdate = false;
bool showStat = true;
bool emph = true;
byte state = 0;
byte lightTime = 0;

enum OPERATION
{
  DEMOVHOD, PODPIS, TESTPODPIS, TEST, CHISTKA, SUCCESS, UNKNOWN
} operation;

void timer_handle_interrupts(int timer)
{
  static int lastBtnState = HIGH;
  static int currBtnState = HIGH;
  int btnState = digitalRead(btn);
  static int btnCount = 0;
  static int count = 3000;

  if (count == 0)
    {
      needUpdate = true;
      count = 3000;
    }
  else
    {
      count--;
    }

  if (btnState != lastBtnState)
    {
      btnCount = 50;
    }

  if (btnCount > 0)
    {
      btnCount--;
    }
  else
    {
      if (btnState != currBtnState)
        {
          currBtnState = btnState;

          if (currBtnState == LOW)
            {
              btnPressed = true;
            }
        }
    }

  lastBtnState = btnState;
}

void setup()
{
  timer_init_ISR_1KHz(TIMER_DEFAULT);
  lcd.init();
  Serial.begin(9600);
  SIM900.begin(9600);
  Serial.println("Started");
}

String input = "";
void loop()
{
  while (Serial.available() > 0)
    {
      SIM900.write(Serial.read());
    }

  while (SIM900.available() > 0)
    {
      char in = SIM900.read();

      if (in == '\n')
        {
          handleSimMessage(input);
          input = "";
          continue;
        }

      input += in;
    }

  if (btnPressed)
    {
      showStat = false;
      lcd.clear();
      lcd.backlight();
      operation = CHISTKA;
      state = 1;
      btnPressed = false;
    }

  if (needUpdate)
    {
      needUpdate = false;

      if (state > 0)
        { state++; }

      if (waitLight)
        {
          waitLight = false;
          off = true;
          lightTime = LT;
        }

      if (lightTime > 0)
        {
          lightTime--;
        }

      if (showStat)
        {
          emph = !emph;
          lcd.clear();
          lcd.print(F("Rabotaem"));

          if (emph)
            { lcd.print(F("...")); }

          SIM900.print(F("AT+CSQ\r"));
        }
    }

  if (off && lightTime == 0)
    {
      off = false;
      lcd.noBacklight();
      showStat = true;
    }

  switch (state)
    {
      case 1:
      {
        SIM900.print(F("AT*PSSTK=\"MENU SELECTION\",128\r"));
        lcd.clear();
        lcd.print(F("Select 1C-SIM"));
        Serial.println(F("select 1C-SIM"));
        printOperation();
        state++;
        break;
      }

      case 3:
      {
        SIM900.print(F("AT*PSSTK=\"GET ITEM LIST\",10\r"));
        lcd.clear();
        lcd.print(F("Main menu"));
        Serial.println(F("Main menu"));
        printOperation();
        state++;
        break;
      }

      case 5:
      {
        SIM900.print(F("AT*PSSTK=\"SELECT ITEM\",128,1,0,0\r"));
        lcd.clear();
        lcd.print(F("Select kluchi"));
        Serial.println(F("select kluchi"));
        printOperation();
        state++;
        break;
      }

      case 7:
      {
        SIM900.print(F("AT*PSSTK=\"GET ITEM LIST\",10\r"));
        lcd.clear();
        lcd.print(F("Kluchi"));
        Serial.println(F("Kluchi"));
        printOperation();
        state++;
        break;
      }

      case 9:
      {
        SIM900.print(F("AT*PSSTK=\"SELECT ITEM\",1,1,0,0\r"));
        lcd.clear();
        lcd.print(F("1-y kluch"));
        Serial.println(F("1-y kluch"));
        printOperation();
        state++;
        break;
      }

      case 11:
      {
        SIM900.print(F("AT*PSSTK=\"GET ITEM LIST\",10\r"));
        lcd.clear();
        lcd.print(F("Deystviya"));
        Serial.println(F("Deystviya"));
        printOperation();
        state++;
        break;
      }

      case 13:
      {
        SIM900.print(F("AT*PSSTK=\"SELECT ITEM\",4,6,0,0\r"));
        lcd.clear();
        lcd.print(F("Udalit'"));
        Serial.println(F("Udalit'"));
        printOperation();
        state++;
        break;
      }

      case 50:
      case 51:
      {
        SIM900.print(F("AT*PSSTK=\"COMMAND REJECTED\",1,16\r"));
        state = 0;
        break;
      }
    }
}

void parseMessage(String &input)
{
  if (input.indexOf(F("0422043504410442043E")) > 0)
    {
      // testovaya podpis'
      operation = TESTPODPIS;
      return;
    }

  if (input.indexOf(F("04140435043C043E0020043F")) > 0)
    {
      // demo portal
      operation = DEMOVHOD;
      return;
    }

  if (input.indexOf(F("041F043E0434043F04380441044C")) > 0)
    {
      // podpis'
      operation = PODPIS;
      return;
    }

  if (input.indexOf(F("800B041A043B044E04470020044304340430043B0435043D")) > 0)
    {
      //udalily kluch
      operation = SUCCESS;
      return;
    }

  if (input.indexOf(F("043504390020043D04350442")) > 0)
    {
      // net kluchey
      operation = UNKNOWN;
      state = 48;
      return;
    }
}

void printOperation()
{
  lcd.setCursor(0, 1);

  switch (operation)
    {
      case DEMOVHOD:
      {
        lcd.print(F("DEMO vhod"));
        break;
      }

      case TESTPODPIS:
      {
        lcd.print(F("Test podpis'"));
        break;
      }

      case PODPIS:
      {
        lcd.print(F("Podpis' shtuki"));
        break;
      }

      case TEST:
      {
        lcd.print(F("Timer test OK"));
        break;
      }

      case CHISTKA:
      {
        lcd.print(F("Udaleniye klucha"));
        break;
      }

      case SUCCESS:
      {
        lcd.print(F("Udaleno norm"));
        break;
      }

      default:
      {
        lcd.print(F("Ne znayu, otstan"));
        break;
      }
    }
}

void handleSimMessage(String &input)
{
  Serial.println(input);

  if (input.startsWith("*"))
    {
      showStat = false;
      lcd.backlight();

      if (input.indexOf(F("PLAY TONE")) > 0)
        {
          lcd.clear();
          lcd.print(F("Play tone"));
          Serial.println(F("play tone"));
          SIM900.print(F("AT*PSSTK=\"PLAY TONE\",1,0\r"));
          return;
        }

      if (input.indexOf(F("DISPLAY TEXT")) > 0)
        {
          lcd.clear();
          lcd.print(F("Display text"));
          parseMessage(input);
          Serial.println(F("display text"));
          SIM900.print(F("AT*PSSTK=\"DISPLAY TEXT\",1,0\r"));
          printOperation();
          return;
        }

      if (input.indexOf(F("SETUP MENU")) > 0)
        {
          Serial.println(F("setup menu"));
          SIM900.print(F("AT*PSSTK=\"SETUP MENU\",1,1\r"));
          return;
        }

      if (input.indexOf(F("NOTIFICATION")) > 0)
        {
          lcd.clear();
          lcd.print(F("Notification"));
          Serial.println(F("notification"));
          SIM900.print(F("AT*PSSTK=\"NOTIFICATION\",1,0\r"));
          printOperation();
          return;
        }

      if (input.indexOf(F("GET INPUT")) > 0)
        {
          lcd.clear();
          lcd.print(F("Get input"));
          Serial.println(F("get input"));
          SIM900.print(F("AT*PSSTK=\"GET INPUT\",1,4,\"111111\",0,0\r"));
          printOperation();
          return;
        }

      if (input.indexOf(F("END SESSION")) > 0)
        {
          lcd.clear();
          lcd.print(F("End session"));
          Serial.println(F("end"));
          lcd.setCursor(0, 1);
          state = 50;
          waitLight = true;
          return;
        }
        
      if (input.indexOf(F("SELECT ITEM")) > 0 && state > 15)
        {
          lcd.clear();
          lcd.print(F("End session"));
          Serial.println(F("end"));
          lcd.setCursor(0, 1);
          state = 50;
          waitLight = true;
          return;
        }
    }

  if (input.indexOf(F("CSQ: ")) == 1)
    {
      lcd.setCursor(0, 1);
      lcd.print(F("Signal: "));
      input.replace("\r", "");
      lcd.print(input.substring(6));
      showStat = true;
    }
}

