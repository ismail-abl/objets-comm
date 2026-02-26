#include <PortentaUWBShield.h>

/**
 * this demo shows how to setup the Arduino Stella tag as a multicast
 * UWB Ranging Controller (one-to-many)
 * It expects multiple counterparts setup as Responders/Controlees
 * This example demonstrates multicast ranging with up to 8 controlees
 */

// Moving average configuration
#define SAMPLES 10                // Number of samples for moving average
long distances[SAMPLES] = {0};    // Circular buffer for distance measurements
int sample_index = 0;              // Current position in circular buffer

// LED and status configuration
#define NEARBY_THRESHOLD 300       // Distance threshold for green LED (cm)
#define CONNECTION_TIMEOUT 2000    // Time before considering tag lost (ms)
#define LED_BLINK_INTERVAL 500     // Red LED blink interval (ms)

// System state variables
unsigned long lastMeasurement = 0;
unsigned long lastLedBlink = 0;
bool ledState = false;

// Direction variables
const int SAMPLES_BUFFER = 10;            // Size of your circular buffer
long d1_buffer[SAMPLES_BUFFER] = {0};    // Distances to Stella A
long d2_buffer[SAMPLES_BUFFER] = {0};    // Distances to Stella B
int idx_buffer = 0;


// --------------------------------------------
// Insert new measurement in circular buffer
// --------------------------------------------
void addSample(long d1, long d2) {
    d1_buffer[idx_buffer] = d1;
    d2_buffer[idx_buffer] = d2;
    idx_buffer = (idx_buffer + 1) % SAMPLES_BUFFER;
}

// --------------------------------------------
// Compute moving average
// --------------------------------------------
long movingAverage(long *buf) {
    long sum = 0;
    for (int i = 0; i < SAMPLES_BUFFER; i++) sum += buf[i];
    return sum / SAMPLES_BUFFER;
}

// -------------------------------------------------------
// Classify movement direction AND FILTER OUT no-movement
// Returns "" when no gesture (so nothing is printed)
// -------------------------------------------------------
String classifyMovement(long d1_prev, long d1_now,
                                long d2_prev, long d2_now)
{
    long speed1 = d1_now - d1_prev;   // <0 => toward Stella A
    long speed2 = d2_now - d2_prev;   // <0 => toward Stella B

    const float TH = 0.50f;  // 4 cm noise threshold

    bool towardA = speed1 < -TH;
    bool towardB = speed2 < -TH;
    bool awayA   = speed1 >  TH;
    bool awayB   = speed2 >  TH;

    // ---- Only return text when a REAL gesture happens ----
    if (towardA && awayB)
        return "Moving toward Stella A";

    if (towardB && awayA)
        return "Moving toward Stella B";

    if (towardA && towardB)
        return "Moving between anchors (toward center)";

    if (awayA && awayB)
        return "Moving away from both anchors";

    // ---- No significant gesture → return NOTHING ----
    return "";
}

// handler for ranging notifications
void rangingHandler(UWBRangingData &rangingData) {
  if(rangingData.measureType()==(uint8_t)uwb::MeasurementType::TWO_WAY)
  {

    RangingMeasures twr=rangingData.twoWayRangingMeasure();

    // Loop through all available measurements
    for (int j = 0; j < rangingData.available(); j++) {
      // Only process valid measurements
      if (twr[j].status == 0 && twr[j].distance != 0xFFFF) {
        // Update connection tracking
        lastMeasurement = millis();

        // Gets current distance
        float d1;
        float d2;
        
        // Identification de la distances des peers
        if (j == 0) {
          d1 = twr[j].distance;
          d2 = twr[j+1].distance;
        } else if (j == 1) {
          d1 = twr[j-1].distance;
          d2 = twr[j].distance;
        } else {
          Serial.print("System error: Couldn't get peer distance");
        }
        
        Serial.print(" REAL D1: ");
        Serial.print(d1);
        Serial.print(" REAL D2: ");
        Serial.print(d2);

        // Ajout des samples dans un buffer 
        addSample(d1, d2);

        // Calcul de la moyenne 
        long d1_avg_now = movingAverage(d1_buffer);
        long d2_avg_now = movingAverage(d2_buffer);

        static long d1_avg_prev = d1_avg_now;
        static long d2_avg_prev = d2_avg_now;

        delay(50);  // À ajuster
        
        // Classification du mouvement 
        String direction = classifyMovement(d1_avg_prev, d1_avg_now,
                                            d2_avg_prev, d2_avg_now);

        Serial.print("d1_avg: "); Serial.print(d1_avg_now);
        Serial.print("   d2_avg: "); Serial.print(d2_avg_now);
        Serial.print("   → Direction: ");
        Serial.println(direction);

        // Store new distance measurement in circular buffer
        distances[sample_index] = twr[j].distance;

        // Calculate moving average
        long avg = 0;
        for (int i = 0; i < SAMPLES; i++) {
          avg += distances[i];
        }
        avg = avg / SAMPLES;

        // Update distance indicator LED (Green LED)
        // LED ON when tag is nearby, OFF when far away
        digitalWrite(LEDG, (twr[j].distance <= NEARBY_THRESHOLD) ? LOW : HIGH);

        // Output formatted data for Serial Plotter
        Serial.print("Peer : ");
        Serial.print(j);
        //Serial.print("Distance(cm):");
        //Serial.print(twr[j].distance);
        //Serial.print(",");
        //Serial.print("Average (cm):");
        //Serial.println(avg);

        // Update circular buffer index
        sample_index = (sample_index + 1) % SAMPLES;

        // Update previous
        d1_avg_prev = d1_avg_now;
        d2_avg_prev = d2_avg_now;
      }
    }

  }

}

void setup() {

  Serial.begin(115200);

#if defined(ARDUINO_PORTENTA_C33)
  /* Only the Portenta C33 has an RGB LED. */
  pinMode(LEDR, OUTPUT);
  digitalWrite(LEDR, LOW);
#endif

  // Define the source (this device) MAC address using 2-bytes MAC
  uint8_t devAddr[]={0x07,0x07};
  UWBMacAddress srcAddr(UWBMacAddress::Size::SHORT,devAddr);

  // Define multiple destination MAC addresses (controlees)
  uint8_t destination1[]={0x11,0x11};
  uint8_t destination2[]={0x12,0x12};

  UWBMacAddress dstAddr1(UWBMacAddress::Size::SHORT,destination1);
  UWBMacAddress dstAddr2(UWBMacAddress::Size::SHORT,destination2);

  // Create a list of destination addresses
  UWBMacAddressList dest(UWBMacAddress::Size::SHORT);
  dest.add(dstAddr1);
  dest.add(dstAddr2);

  // register the ranging notification handler before starting
  UWB.registerRangingCallback(rangingHandler);

  UWB.begin(); //start the UWB stack, use Serial for the log output
  Serial.println("Starting UWB ...");

  //wait until the stack is initialised
  while(UWB.state()!=0)
    delay(10);

  Serial.println("Starting multicast session ...");
  //setup a multicast session with ID 0x11223344
  UWBRangingOneToMany myController(0x11223344, srcAddr, dest);

  //add the session to the session manager, in case you want to manage multiple connections
  UWBSessionManager.addSession(myController);

  //prepare the session applying the default parameters
  myController.init();

  //start the session
  myController.start();

}

void loop() {
#if defined(ARDUINO_PORTENTA_C33)
  /* Only the Portenta C33 has an RGB LED. */
  digitalWrite(LEDR, !digitalRead(LEDR));
#endif

  delay(100);
}
