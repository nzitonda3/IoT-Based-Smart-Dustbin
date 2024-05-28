#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2 line display

const int trigPin1 = 5;
const int echoPin1 = 6;
const int trigPin2 = 9;
const int echoPin2 = 8;
const int maxDistance = 18;  // Maximum distance in cm
const int lcdRefreshInterval = 500; // Refresh interval for LCD in milliseconds
const int smsThresholdPercentage = 85; // Percentage threshold for sending SMS
const String number = "+250788869331"; // Phone number to send SMS

SoftwareSerial sim(2, 3); // Initialize a SoftwareSerial object with RX on pin 2 and TX on pin 3

void setup() {
  Serial.begin(9600);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  servo.attach(7);
  lcd.init();                      // Initialize the lcd 
  lcd.backlight();                 // Turn on backlight
  sim.begin(9600); // Initialize SoftwareSerial communication with SIM module at 9600 baud
  delay(1000); // Delay for 1 second
}

void loop() {
  long duration1, distance1, duration2, distance2;
  
  // Clear the LCD
  lcd.clear();
  lcd.setCursor(0,0);
  
  // Trigger the first ultrasonic sensor
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  
  // Read the echo pulse from the first ultrasonic sensor
  duration1 = pulseIn(echoPin1, HIGH);
  
  // Convert the duration to distance in cm for the first ultrasonic sensor
  distance1 = duration1 * 0.034 / 2;
  
  // Check if distance from first sensor is within threshold for turning servo
  if (distance1 < 30) { // Adjust this threshold as needed
    servo.write(250); // Turn servo to 180 degrees
    delay(1000); // Wait for servo to reach position
  } else {
    servo.write(0); // Turn servo to 0 degrees
    delay(1000); // Wait for servo to reach position
  }
  
  // Trigger the second ultrasonic sensor
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  
  // Read the echo pulse from the second ultrasonic sensor
  duration2 = pulseIn(echoPin2, HIGH);
  
  // Convert the duration to distance in cm for the second ultrasonic sensor
  distance2 = duration2 * 0.034 / 2;
  
  // Map the distance to a percentage
  int percentage = map(distance2, 0, maxDistance, 100, 0);
  
  // Display the distance percentage on the LCD
  lcd.print("Occupancy: ");
  lcd.print(percentage);
  lcd.print("%");
  
  delay(lcdRefreshInterval);

  // Check if distance exceeds threshold for sending SMS
  if (percentage > smsThresholdPercentage) {
    lcd.setCursor(0,1);
    lcd.print("ALLMOST FULL");
    SendMessage(); // Call SendMessage function to send SMS
    delay(5000); // Wait for 5 seconds to avoid sending multiple SMS rapidly
  }
}

void SendMessage() {
  Serial.println("Sending Message"); // Print message to serial monitor
  sim.println("AT+CMGF=1"); // Set GSM module in text mode
  delay(500); // Delay for 500 milliseconds
  if (!waitForResponse()) return; // Wait for response from SIM module

  sim.println("AT+CMGS=\"" + number + "\"\r"); // Set SMS recipient number
  delay(500); // Delay for 500 milliseconds
  if (!waitForResponse()) return; // Wait for response from SIM module

  String SMS = "Trash bin in House 1 is almost full"; // Message content
  sim.println(SMS); // Send SMS content
  delay(500); // Delay for 500 milliseconds
  sim.println((char)26); // Send CTRL+Z to indicate end of message
  if (!waitForResponse()) return; // Wait for response from SIM module

  Serial.println("Message sent successfully."); // Print message to serial monitor
}

bool waitForResponse() {
  int timeout = 0; // Initialize timeout counter
  while (!sim.available() && timeout < 1000) { // Loop until data available or timeout reached
    delay(10); // Delay for 10 milliseconds
    timeout += 10; // Increment timeout counter
  }

  if (sim.available()) { // Check if data is available
    String buffer = sim.readStringUntil('\n'); // Read data from SIM module until newline character
    Serial.println(buffer); // Print received data to serial monitor
    if (buffer.indexOf("ERROR") != -1) { // Check if received data contains "ERROR"
      Serial.println("Error response received."); // Print error message to serial monitor
      return false; // Return false to indicate error
    }
    return true; // Return true to indicate success
  } else {
    Serial.println("No response received."); // Print message to serial monitor
    return false; // Return false to indicate no response
  }
}
