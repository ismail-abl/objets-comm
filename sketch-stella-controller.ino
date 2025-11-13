/**
  UWB Tag (Controller/Initiator) with Dynamic Proximity Detection
*/

// Include required UWB library for Arduino Stella
#include <StellaUWB.h>

// Define proximity thresholds (centimeters)
#define WORKSPACE_ACCESS_THRESHOLD 100        // 1 meter for door unlock
#define EQUIPMENT_OPERATION_THRESHOLD 30      // 30 cm for machine enable

// Variables for LED feedback
int blinkInterval = 0;
bool ledState = false;
bool lastLedPhysicalState = true;
unsigned long lastStateChange = 0;
unsigned long lastBlinkTime = 0;
#define DEBOUNCE_TIME 200

// Stella's built-in LED
#define LED_PIN p37

/**
  Processes ranging data and provides visual feedback
  @param rangingData Reference to UWB ranging data object
*/
void rangingHandler(UWBRangingData &rangingData) {
  Serial.print("GOT RANGING DATA - Type: ");
  Serial.println(rangingData.measureType());
  
  if(rangingData.measureType() == (uint8_t)uwb::MeasurementType::TWO_WAY) {
    RangingMeasures twr = rangingData.twoWayRangingMeasure();
    
    for(int j = 0; j < rangingData.available(); j++) {
      if(twr[j].status == 0 && twr[j].distance != 0xFFFF) {
        Serial.print("Distance to base: ");
        Serial.print(twr[j].distance);
        Serial.println(" cm");
        
        // Determine LED behavior based on proximity zones
        int newBlinkInterval;
        if(twr[j].distance > WORKSPACE_ACCESS_THRESHOLD) {
          newBlinkInterval = 0;                                 // Outside workspace - LED off
          Serial.println("Status: Outside workspace");
        }
        else if(twr[j].distance <= EQUIPMENT_OPERATION_THRESHOLD) {
          newBlinkInterval = 100;                               // Equipment zone - very fast blink
          Serial.println("Status: Equipment operation zone");
        }
        else if(twr[j].distance <= WORKSPACE_ACCESS_THRESHOLD) {
          newBlinkInterval = 500;                               // Workspace zone - slow blink
          Serial.println("Status: Workspace access zone");
        }
        
        // Apply debouncing
        if(newBlinkInterval != blinkInterval && (millis() - lastStateChange > DEBOUNCE_TIME)) {
          blinkInterval = newBlinkInterval;
          lastStateChange = millis();
          lastBlinkTime = millis();
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Start with LED off
  lastLedPhysicalState = true;
  
  Serial.println("UWB Personal Access Tag - Stella");

  // Define the source (this device) and destination MAC addresses, using 2-bytes MACs
  // This device is the controller/initiator, so it uses 0x22,0x22
  // and targets the responder/controlee at 0x11,0x11
  uint8_t devAddr[] = {0x41, 0x42};
  uint8_t destination[] = {0x11, 0x11};
  UWBMacAddress srcAddr(UWBMacAddress::Size::SHORT, devAddr);
  UWBMacAddress dstAddr(UWBMacAddress::Size::SHORT, destination);
  
  // Register the ranging notification handler before starting
  UWB.registerRangingCallback(rangingHandler);
  
  UWB.begin(); // Start the UWB stack, use Serial for the log output
  Serial.println("Starting UWB ...");
  
  // Wait until the stack is initialized
  while(UWB.state() != 0)
    delay(10);
  
  Serial.println("Starting session ...");
  
  // Setup a session with ID 0x11223344 as Controller/Initiator (default role)
  UWBTracker myTracker(0x11223344, srcAddr, dstAddr);
  
  // Add the session to the session manager
  UWBSessionManager.addSession(myTracker);
  
  // Prepare the session applying the default parameters
  myTracker.init();
  
  // Start the session
  myTracker.start();
  
  Serial.println("Personal tag ready!");
  Serial.println("Proximity zones:");
  Serial.println("- Outside workspace: LED off");
  Serial.println("- Workspace zone (≤" + String(WORKSPACE_ACCESS_THRESHOLD) + "cm): Slow blink");
  Serial.println("- Equipment zone (≤" + String(EQUIPMENT_OPERATION_THRESHOLD) + "cm): Fast blink");
  
  // Initialization complete signal
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  // Handle LED control based on proximity zones
  if(blinkInterval == 0) {
    // Outside workspace - LED off
    if(lastLedPhysicalState != true) {
      digitalWrite(LED_PIN, HIGH);
      lastLedPhysicalState = true;
      ledState = false;
    }
  } else {
    // Inside workspace - blink at appropriate interval
    if(currentTime - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = currentTime;
      ledState = !ledState;
      
      bool newPhysicalState = !ledState;
      if(newPhysicalState != lastLedPhysicalState) {
        digitalWrite(LED_PIN, newPhysicalState ? HIGH : LOW);
        lastLedPhysicalState = newPhysicalState;
      }
    }
  }
  
  delay(10);
}