/*
   Simple data logger.
*/
#include <SPI.h>
#include "SdFat.h"

// SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
const uint8_t chipSelect = 10;

// Interval between data records in milliseconds.
// The interval must be greater than the maximum SD write latency plus the
// time to acquire and write data to the SD to avoid overrun errors.
// Run the bench example to check the quality of your SD card.
const uint32_t SAMPLE_INTERVAL_MS = 1000;

// Log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "Data"
//------------------------------------------------------------------------------
// File system object.
SdFat sd;

// Log file.
SdFile file;
int threshold = 35;
int slowDown = 0;
int slowCount = 0;
float slowMult = 1;
float timeSince = 0;
// Time in micros for next data record.
uint32_t logTime;

//==============================================================================
// User functions.  Edit writeHeader() and logData() for your requirements.

//------------------------------------------------------------------------------
// Write data header.
void writeHeader() {
  file.print(F("sec"));
  file.print(F(", distance"));
  file.print(A4, DEC);
  file.println();
}
//------------------------------------------------------------------------------
// Log a data record.
void logData() {
  uint16_t data;

  // Read all channels to avoid SD write latency between readings.
  data = 26.4 * pow(analogRead(A4) * 0.0048828125, -1);
  // Write data to file.  Start with log time in micros.
  if (data <= threshold) {
    slowDown = 0;
    slowMult = 1;
    slowCount = 0;
    file.print(logTime / pow(10, 6));

    // Write ADC data to CSV record.
    file.write(',');
    file.print(data);
    file.println();
  }
  else {
    if (slowCount == 0) {
      timeSince = micros();
    }
    slowCount += 1;
    slowMult += slowCount / 6;
    slowDown = 1;
    if (slowMult >= 7) {
      slowMult = 1;
    }
    if (micros() - timeSince > 3.6*pow(10, 9)){
      delay(5.04*pow(10,7));
    }
  }

}
//==============================================================================
// Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))
//------------------------------------------------------------------------------
void setup() {
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";

  Serial.begin(9600);

  // Wait for USB Serial
  while (!Serial) {
    SysCall::yield();
  }
  delay(1000);

  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }

  // Find an unused file name.
  if (BASE_NAME_SIZE > 6) {
    error("FILE_BASE_NAME too long");
  }
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      error("Can't create file name");
    }
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    error("file.open");
  }
  // Read any Serial data.
  do {
    delay(10);
  } while (Serial.available() && Serial.read() >= 0);

  Serial.print(F("Logging to: "));
  Serial.println(fileName);

  // Write data header.
  writeHeader();

  // Start on a multiple of the sample interval.
  logTime = micros() / (1000UL * SAMPLE_INTERVAL_MS) + 1;
  logTime *= 1000UL * SAMPLE_INTERVAL_MS;
}
//------------------------------------------------------------------------------
void loop() {
  // Time for next record.
  logTime += 1000UL * SAMPLE_INTERVAL_MS;
  if (slowDown == 1) {
    delay(2000 * slowMult);
  }
  // Wait for log time.
  int32_t diff;
  do {
    diff = micros() - logTime;
  } while (diff < 0);

  logData();

  // Force data to SD and update the directory entry to avoid data loss.
  if (!file.sync() || file.getWriteError()) {
    error("write error");
  }

  if (Serial.available()) {
    // Close file and stop.
    file.close();
    SysCall::halt();
  }
}
