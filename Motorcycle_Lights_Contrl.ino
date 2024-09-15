/*ARDUNINO BOADRD APPLICATION
 * Upgrade a standard motorcycle flasher to support LED lights and hazard light functions. 
 * Add automatic control for daytime running lights and battery voltage monitoring.
 *
 *           
 *  Author Dafeng 2024
*/
#define RELAY_SW  20
#define RELAY_HAZARD 21
#define RELAY_LIGHT 22
const uint8_t kLedPin = 13;
const int left_pin = A0;
const int right_pin = A1;
const int light_intensity_pin = A2;
const int power12v_pin = A3;

static uint16_t ch0_val[4];
static uint16_t ch1_val[4];
static uint16_t ch2_val[4];
// static uint16_t ch3_val[4];
static bool channel_r = true;
static bool channel_l = false;
static bool channel_s = false;
static bool hazard_flag = false;

static bool llFlag = false;
static bool rlFlag = false;
static bool relayswt = false;
static bool autolight = false;
static bool read = false;
static bool input_power = false;

uint16_t raw_12V = 0;
unsigned long previousMillis = 0;
unsigned long previouscounter = 0;
unsigned long counter = 0;
uint16_t relay_counter = 0;

void setup() {
   pinMode(power12v_pin, INPUT);
   pinMode(kLedPin, OUTPUT);
   digitalWrite(kLedPin, LOW);
   pinMode(RELAY_HAZARD, OUTPUT);
   digitalWrite(RELAY_HAZARD, LOW);
   pinMode(RELAY_SW, OUTPUT);
   digitalWrite(RELAY_SW, LOW);
   pinMode(RELAY_LIGHT, OUTPUT);
   digitalWrite(RELAY_LIGHT, LOW);
   channel_r = true;
   channel_l = false;
   channel_s = false;
   hazard_flag = false;
   autolight = false;
   delay(100);
  //  Serial.begin(9600);
  //  analogReference(EXTERNAL);
}

void loop() {

   unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 15) {
        previousMillis = currentMillis;
        uint16_t tempCnt = 4;
        uint16_t tempAdc = 0;

        if(channel_r) {
          ch1_val[counter & 0x03] = analogRead(right_pin);
            while(tempCnt--)   
            {
              tempAdc += ch1_val[tempCnt];
            }
            tempAdc = tempAdc >> 2;
            
            if(tempAdc >= 450) {               // 3.3V*450/1023  = 1.45V,  12 * 2.2/(12+2.2) = 1.86V
              relayswt = true;
              rlFlag = true;
              read = true;
            }
            if(tempAdc <= 50 && input_power && rlFlag) {
              relayswt = false; 
              relay_counter = 0;
              digitalWrite(RELAY_SW, LOW);
              digitalWrite(RELAY_HAZARD, LOW);
              hazard_flag = false;
              rlFlag = false;
            }

            if((counter & 0x03) == 3) {
              channel_r = false;
              channel_l = false;
              channel_s = true;      
            }
        }
        
        else if(channel_s) {
            ch2_val[counter & 0x03] = analogRead(light_intensity_pin);
            while(tempCnt--)
            {
              tempAdc += ch2_val[tempCnt];
            }
            tempAdc = tempAdc >> 2;
            if (tempAdc > 384 && !autolight) {                
                digitalWrite(RELAY_LIGHT, HIGH);
                autolight = true;
            }
            else if(tempAdc < 96 && autolight) {
                digitalWrite(RELAY_LIGHT, LOW);
                autolight = false;
            }
            if((counter & 0x03) == 3) {
              channel_s = false;
              channel_r = false;
              channel_l = true;      
            }
        }

        else if(channel_l) {
                  ch0_val[counter & 0x03] = analogRead(left_pin);
                    while(tempCnt--)
                    {
                      tempAdc += ch0_val[tempCnt];
                    }
                    tempAdc = tempAdc >> 2;
                    if(tempAdc >= 450) {
                      relayswt = true;
                      llFlag = true;
                    }
                    if(tempAdc <= 50 && input_power && llFlag && relay_counter > 3) {
                      relayswt = false;
                      relay_counter = 0;
                      digitalWrite(RELAY_SW, LOW);
                      digitalWrite(RELAY_HAZARD, LOW);
                      hazard_flag = false;
                      llFlag = false;
                    }
                    if((counter & 0x03) == 3) {
                      channel_l = false;
                      channel_s = false;
                      channel_r = true;
                    }
                }

        counter++;      
  }

 if (counter - previouscounter >= 36 &&  relayswt) {       // 15ms*36 = 540ms
    previouscounter = counter;
    digitalWrite(RELAY_SW, !digitalRead(RELAY_SW));
    relay_counter++;
}

if (llFlag && rlFlag && !hazard_flag) {
       hazard_flag = true;
       digitalWrite(RELAY_HAZARD, HIGH);       
}

if(read && input_power && (counter%128 > 126)) {
            raw_12V = analogRead(right_pin);
            if((raw_12V > 50) && (raw_12V < 520))
                 digitalWrite(kLedPin, HIGH);
            else digitalWrite(kLedPin, LOW);
            //  Serial.print("RAW_12V_: ");
            //  Serial.println(raw_12V);
             read = false;         
}

if (digitalRead(power12v_pin)) input_power = true;
else input_power = false;

}

