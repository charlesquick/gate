#include <MKRGSM.h>

#define ON_TIME 30000
#define ADMIN "+447777777777" //set to admin's phone number
#define GATE_TRIG 5           //pin to trigger gate circuitry
#define STATLED 0             //status LED on PCB

/*
TODO
Voicemail handling/delete
more admin test functions - help (send command set), hold (x minutes), info (RSSI, voicemail/sms status) 
if SMS from unknown number: send contents and number to admin unless passphrase which opens gate.
*/



GSM gsmAccess;
GSM_SMS sms;
GSMVoiceCall vcs;

const char PINNUMBER[] = "";
char senderNumber[20];        //Array to hold the number an SMS is retreived from
unsigned long shutoff = 0;
unsigned long call_delay = 0;


void setup() {
  pinMode(STATLED, OUTPUT);
  pinMode(GATE_TRIG, OUTPUT);
  
  digitalWrite(STATLED, HIGH);
  digitalWrite(GATE_TRIG, LOW);

  bool connected = false;

  while (!connected) {
    if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
        connected = true;
      } else {
        digitalWrite(STATLED, HIGH);
        delay(1000);
        digitalWrite(STATLED, LOW);
        delay(1000);
      }
  }
  vcs.hangCall();
  digitalWrite(0, LOW);
      

}

void loop() {
  
  switch (vcs.getvoiceCallStatus()) {
    case IDLE_CALL: 
      break;

    case RECEIVINGCALL: 
      vcs.retrieveCallingNumber(senderNumber, 20);
      shutoff = millis() + ON_TIME;
      call_delay = millis() + 2000;       //set answering time
      vcs.answerCall();
      break;

    case TALKING:  
      if(call_delay < millis()){          //hang up after 2 seconds
        vcs.hangCall();
       }
      break;
  }
  
  if (sms.available()) {
    String msg;
    sms.remoteNumber(senderNumber, 20);     //get the number
    String strSenderNumber = senderNumber;
    
    if(strSenderNumber == "2732"){          //if from EE, send to admin
      int c;
      String msg;
      while ((c = sms.read()) != -1) {
        msg += (char)c;
      }
      sms.beginSMS(ADMIN);        
      sms.print(msg);
      sms.endSMS();
    }
    
    else if(strSenderNumber == ADMIN){    //if test from admin, reply
      int c;                                      
      String msg;
      while ((c = sms.read()) != -1) {              
        msg += (char)c;
      }
      if(msg == "test"){                          
        sms.beginSMS(ADMIN);
        sms.print(msg);
        sms.endSMS();
      }
      else{
        shutoff = millis() + ON_TIME;
      }
    }
    
    else {
      shutoff = millis() + ON_TIME;           //If from anyone else, open the gate
    }
    sms.flush();
  }

  if(shutoff > millis()){           //open solenoid
      digitalWrite(GATE_TRIG, HIGH);
      digitalWrite(STATLED, HIGH);
      }
    
  
  if(shutoff < millis()){           //release solenoid after specified time
      digitalWrite(GATE_TRIG, LOW);
      digitalWrite(STATLED, HIGH);        //toggle status LED
      delay(80);
      digitalWrite(STATLED, LOW);
      }

  if(millis() < 2000){            //catch a millis overflow
    shutoff == 0;
  }
      

}
