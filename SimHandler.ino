#include <LiquidCrystal_I2C.h>
#include <timer-api.h>
#include <SoftwareSerial.h>

#define btn 4
#define BLT 3

SoftwareSerial SIM900(7, 8);
LiquidCrystal_I2C lcd(0x27, 16, 2);

volatile bool nextState = false;
byte state = 0;

void timer_handle_interrupts(int timer)
{
  static int lastBtnState = HIGH;
  static int currBtnState = HIGH;
  int btnState = digitalRead(btn);
  static int btnCount = 0;
  static int count = 1000;

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
              nextState = true;
            }
        }
    }

  lastBtnState = btnState;
}

void setup()
{
  timer_init_ISR_1KHz(TIMER_DEFAULT);
  lcd.init();
  delay(3000);
  Serial.begin(19200);
  SIM900.begin(19200);
  Serial.println("Started");
  SIM900.print("AT*PSSTK=\"SETUP MENU\",1,1\r");
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

  if (nextState)
    {
      state++;
      lcd.print(state);
      nextState = false;
    }
}

void handleSimMessage(String input)
{
  Serial.println(input);

  if (input.startsWith("*"))
    {
      if (input.indexOf("PLAY TONE") > 0)
        {
          lcd.clear();
          lcd.print("Play tone");
          lcd.backlight();
          Serial.println("play tone");
          SIM900.print("AT*PSSTK=\"PLAY TONE\",1,0\r");
        }
      else
        if (input.indexOf("DISPLAY TEXT") > 0)
          {
            lcd.clear();
            lcd.print("Play tone");
            lcd.backlight();
            Serial.println("display text");
            SIM900.print("AT*PSSTK=\"DISPLAY TEXT\",1,0\r");
          }
        else
          if (input.indexOf("SETUP MENU") > 0)
            {
              Serial.println("setup menu");
              SIM900.print("AT*PSSTK=\"SETUP MENU\",1,1\r");
            }
          else
            if (input.indexOf("NOTIFICATION") > 0)
              {
                lcd.clear();
                lcd.print("Notification");
                lcd.backlight();
                Serial.println("notification");
                SIM900.print("AT*PSSTK=\"NOTIFICATION\",1,0\r");
              }
            else
              if (input.indexOf("GET INPUT") > 0)
                {
                  lcd.clear();
                  lcd.print("Get input");
                  lcd.backlight();
                  Serial.println("get input");
                  SIM900.print("AT*PSSTK=\"GET INPUT\",1,4,\"111111\",0,0\r");
                }
              else
                if (input.indexOf("END SESSION") > 0)
                  {
                    lcd.clear();
                    lcd.print("End session");
                    lcd.noBacklight();
                    Serial.println("end");
                  }
    }
}

