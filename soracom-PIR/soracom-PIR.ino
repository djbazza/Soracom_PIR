#include <WioLTEforArduino.h>
#include <stdio.h>

#define INTERVAL        (30000)
#define RECEIVE_TIMEOUT (10000)

// uncomment following line to use Temperature & Humidity sensor
#define PIR_PIN    (WIOLTE_D39)
#define BAT_PIN    (WIOLTE_A4)
#define LED_VALUE (10)

//bool SMSSent;
int motionCount = 1;

WioLTE Wio;

void setup() {
  delay(200);

//  SMSSent = false;
  
  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  pinMode(PIR_PIN, INPUT);
  pinMode(BAT_PIN, INPUT);

  SerialUSB.println("### Setup completed.");
}

void loop() {
  Wio.LedSetRGB(0, 50, 0); //R,G,B
  Wio.PowerSupplyGrove(true);
  char data[1024];

#ifdef PIR_PIN
//  INT PIR_Status = ;
  while(!digitalRead(PIR_PIN))
    delay(1000);

  float v = analogRead(BAT_PIN);
  float batVolts = v/1023.0*3.3*2.0;
  
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  delay(500);

  SerialUSB.println("### Turn on or reset.");
  if (!SerialUSB.println(Wio.TurnOnOrReset())) {
    SerialUSB.println("### ERROR! ###");
    Wio.LedSetRGB(50, 0, 0); //R,G,B
    return;
  }

  SerialUSB.println("### Connecting to \"soracom.io\".");
  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("### ERROR! ###");
    Wio.LedSetRGB(50, 0, 0); //R,G,B
    return;
  }
  
  sprintf(data,"{\"motion\":%i,\"battery\":%f}",motionCount++,batVolts);
//#else
//  sprintf(data, "{\"uptime\":%lu}", millis() / 1000);
#endif // PIR_PIN

  SerialUSB.println("### Open.");
  int connectId;
  connectId = Wio.SocketOpen("uni.soracom.io", 23080, WIOLTE_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR! ###");
    Wio.LedSetRGB(50, 0, 0); //R,G,B
    goto err;
  }

  SerialUSB.println("### Send.");
  SerialUSB.print("Send:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if (!Wio.SocketSend(connectId, data)) {
    SerialUSB.println("### ERROR! ###");
    Wio.LedSetRGB(50, 0, 0); //R,G,B
    goto err_close;
  }

  SerialUSB.println("### Receive.");
  int length;
  length = Wio.SocketReceive(connectId, data, sizeof (data), RECEIVE_TIMEOUT);
  if (length < 0) {
    SerialUSB.println("### ERROR! ###");
    Wio.LedSetRGB(50, 0, 0); //R,G,B
    goto err_close;
  }
  if (length == 0) {
    SerialUSB.println("### RECEIVE TIMEOUT! ###");
    goto err_close;
  }
  SerialUSB.print("Receive:");
  SerialUSB.print(data);
  SerialUSB.println("");

/* if(!SMSSent)
{
  Wio.SendSMS("0437199907", "Found Motion");
  SerialUSB.println("### SMS SENT ###");
  SMSSent = true;
}*/
  
err_close:
  SerialUSB.println("### Close.");
  if (!Wio.SocketClose(connectId)) {
    SerialUSB.println("### ERROR! ###");
    Wio.LedSetRGB(50, 0, 0); //R,G,B
    goto err;
  }

err:
  Wio.PowerSupplyGrove(false);
  Wio.PowerSupplyLTE(false);
  Wio.LedSetRGB(0, 0, 0); //R,G,B
  delay(INTERVAL);
}
