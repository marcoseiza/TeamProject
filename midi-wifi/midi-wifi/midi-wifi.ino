#include <WiFi.h>
#include <SPIFFS.h>

// Library Dependencies:
// - https://github.com/me-no-dev/ESPAsyncWebServer
// - https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h>

// Library Depndencies:
// - https://github.com/pschatzmann/arduino-midi-fileparser
#include <MidiFileParser.h>

// const char *ssid = "RedRover";

const char *ssid = "MarcosEizayaga";
// const char *password = "A52AB1F101582F29EC52CF273FC1CE5459FFD9369B256DB8FCE54FB8B0AB7663";
const char *password = "001037060";

// const char *ssid = "Collegetown_Resident";
// const char *password = "cttapts4";

AsyncWebServer server(80);
midi::MidiFileParser parser;

bool done_upload = false;
int uploaded_midi_file_length = 0;
String uploaded_midi_filename;
const int kWriteSize = 256;
int parser_upload_pos = 0;

const int kBassDrumKey = 36;
const int kSnareDrumKey = 38;
const int kHiHatPedalKey = 44;
const int kHiHatClosedKey = 42;
const int kHiHatOpenKey = 70;
const int kCowbellKey = 56;

uint64_t timeout = 0l;
midi::midi_parser_state cur_state;

// Sate Machine:

const int kVerticalPlayingStick1Pin = 27;    // 0 = not playing, 1 = playing (only one tick)
const int kVerticalDirectionStick1Pin = 33;  // // 0 = snare drum, 1 = hi-hat

const int kVerticalPlayingStick2Pin = 32;    // 0 = not playing, 1 = playing (only one tick)
const int kVerticalDirectionStick2Pin = 14;  // // 0 = snare drum, 1 = hi-hat

const int kHorizontalPositionPin = 15;  // // 0 = snare drum and hi-hat, 1 = cowbell

const int kPlayBassPedalPin = 12;   // 0 = pedal up, 1 = pedal down
const int kPlayHiHatPedalPin = 13;  // 0 = pedal up, 1 = pedal down

void onUploadMidi(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/" + filename, "w");
    Serial.println(logmessage);
    done_upload = false;
    parser_upload_pos = 0;
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    uploaded_midi_file_length = index + len;
    uploaded_midi_filename = filename;
    done_upload = true;
  }
}

void setup() {
  pinMode(kVerticalPlayingStick1Pin, OUTPUT);
  pinMode(kVerticalDirectionStick1Pin, OUTPUT);
  pinMode(kVerticalPlayingStick2Pin, OUTPUT);
  pinMode(kVerticalDirectionStick2Pin, OUTPUT);
  pinMode(kHorizontalPositionPin, OUTPUT);
  pinMode(kPlayBassPedalPin, OUTPUT);
  pinMode(kPlayHiHatPedalPin, OUTPUT);

  Serial.begin(9600);

  Serial.println("\n\nMounting SPIFFS ...");
  if (!SPIFFS.begin(true)) {
    // if you have not used SPIFFS before on a ESP32, it will show this error.
    // after a reboot SPIFFS will be configured and will happily work.
    Serial.println("ERROR: Cannot mount SPIFFS, Rebooting");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // WiFi.begin(ssid);

  Serial.print("\nYour MAC Adress: ");
  Serial.println(WiFi.macAddress());

  Serial.println("\nConnecting");

  wl_status_t status = wl_status_t::WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.status();
    if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
      Serial.print("\nERROR:");
      Serial.println(status);
      break;
    }
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());


  server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("JSON-GET");
    request->send(200, "application/json", R"({ "hello": "world })");
  });

  server.on(
    "/midi-upload", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200);
    },
    onUploadMidi);

  server.begin();
  parser.begin();
}

const char *getReadableParamName(const int param) {
  switch (param) {
    case kSnareDrumKey:
      return "Snare Drum";
    case kBassDrumKey:
      return "Bass Drum";
    case kHiHatPedalKey:
      return "Hi-Hat Pedal";
    case kHiHatClosedKey:
      return "Hi-Hat Closed";
    case kHiHatOpenKey:
      return "Hi-Hat Open";
    case kCowbellKey:
      return "Cowbell";
  }
  return "";
}

bool note_played = false;

int kVerticalDirectionStick1Pin_value = 0;
int kVerticalPlayingStick1Pin_value = 0;
int kPlayBassPedalPin_value = 0;

void handleNote(const midi::midi_midi_event &midi) {
  switch (midi.param1) {
    case kSnareDrumKey:
      digitalWrite(kVerticalDirectionStick1Pin, LOW);  // Stick move down.
      kVerticalDirectionStick1Pin_value = 0;
      digitalWrite(kVerticalPlayingStick1Pin, HIGH);  // Play On.
      kVerticalPlayingStick1Pin_value = 1;
      note_played = true;
      break;
    case kBassDrumKey:
      digitalWrite(kPlayBassPedalPin, HIGH);  // Play On.
      kPlayBassPedalPin_value = 1;
      note_played = true;
      break;
    case kHiHatPedalKey:
      // Unimplemented
      break;
    case kHiHatClosedKey:
      // Unimplemented
      break;
    case kHiHatOpenKey:
      // Unimplemented
      break;
    case kCowbellKey:
      // Unimplemented
      break;
  }
}

void updateInstruments() {
  if (kVerticalDirectionStick1Pin_value != 0 || kVerticalPlayingStick1Pin_value != 0 || kPlayBassPedalPin_value != 0) {
    Serial.printf("%d%d%d\n", kVerticalDirectionStick1Pin_value, kVerticalPlayingStick1Pin_value, kPlayBassPedalPin_value);
  }

  if (note_played) {
    // Reset state.
    switch (cur_state.midi.param1) {
      case kSnareDrumKey:
        digitalWrite(kVerticalDirectionStick1Pin, LOW);  // Stick move down.
        digitalWrite(kVerticalPlayingStick1Pin, LOW);    // Play On.
        kVerticalDirectionStick1Pin_value = 0;
        kVerticalPlayingStick1Pin_value = 0;
        break;
      case kBassDrumKey:
        digitalWrite(kPlayBassPedalPin, LOW);  // Play On.
        kPlayBassPedalPin_value = 0;
        break;
      case kHiHatPedalKey:
        // Unimplemented
        break;
      case kHiHatClosedKey:
        // Unimplemented
        break;
      case kHiHatOpenKey:
        // Unimplemented
        break;
      case kCowbellKey:
        // Unimplemented
        break;
    }
    note_played = false;
  }
}

void loop() {
  if (!done_upload) { return; }

  fs::File file = SPIFFS.open("/" + uploaded_midi_filename, "r");

  if (file.available() && parser.availableForWrite() > kWriteSize) {
    int written = 0;
    int len = std::min(kWriteSize, uploaded_midi_file_length - parser_upload_pos);
    if (parser_upload_pos < uploaded_midi_file_length) {
      byte buf[len];
      size_t size = file.readBytes((char *)buf, len);
      written = parser.write((uint8_t *)buf, len);
    } else {
      parser.end();
    }
    parser_upload_pos += written;
  }

  if (timeout != 0l) {
    if (millis() >= timeout) timeout = 0;
  } else {
    cur_state = parser.parse();

    if (cur_state.status == midi::MIDI_PARSER_TRACK_MIDI) {
      int64_t time_in_ms = cur_state.timeInMs();
      Serial.printf("time: %ld - %ld", (long)cur_state.timeInTicks(), (long)time_in_ms);
      Serial.printf("\tnote: %s [%s]\n", getReadableParamName(cur_state.midi.param1),
                    parser.midi_status_name(cur_state.midi.status));

      bool status_on = cur_state.midi.status == midi::MIDI_STATUS_NOTE_ON;
      bool status_off = cur_state.midi.status == midi::MIDI_STATUS_NOTE_OFF;

      if ((status_on || status_off) && time_in_ms > 0)
        timeout = millis() + time_in_ms;

      if (status_on)
        handleNote(cur_state.midi);
    }
  }

  updateInstruments();
}
