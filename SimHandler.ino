#include <SoftwareSerial.h>

// Configure software serial port
SoftwareSerial SIM900(7, 8);

void setup()
{
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
}

void handleSimMessage(String input)
{
  Serial.println(input);

  if (input.startsWith("*"))
    {
      if (input.indexOf("PLAY TONE") > 0)
        {
          Serial.println("play tone");
          SIM900.print("AT*PSSTK=\"PLAY TONE\",1,0\r");
        }
      else
        if (input.indexOf("DISPLAY TEXT") > 0)
          {
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
                Serial.println("notification");
                SIM900.print("AT*PSSTK=\"NOTIFICATION\",1,0\r");
              }
            else
              if (input.indexOf("GET INPUT") > 0)
                {
                  Serial.println("get input");
                  SIM900.print("AT*PSSTK=\"GET INPUT\",1,4,\"111111\",0,0\r");
                }
    }
}
