#include <Arduino.h>
#include <SHA256.h>

// -------------------------------------------------------------------------------------------
// -- LCD 
#include <UC1701.h>

static UC1701 lcd;
const int PIN_LCD_BACKLIGHT = 26; 


uint8_t sym_hash_key[32];

char key_phrase[] = "This is a key phrase.";
int NUM_HASHING_ROUNDS = 10;

char data[] = "This is something to hash.";


void derive_key_from_key_phrase(const char *keyphrase, uint8_t *p_keybuf, const int key_size = 32) {
  memset(p_keybuf, 0, key_size);
  memcpy(p_keybuf, key_phrase, strlen(key_phrase));
  for ( int i = 0; i < NUM_HASHING_ROUNDS; i++) {
    SHA256 keyhash;
    keyhash.reset();
    keyhash.update(p_keybuf, key_size);

    char new_round[key_size];    
    keyhash.finalize(new_round, key_size);

    memcpy(p_keybuf, new_round, key_size);
  }

}

void setup() {
  SerialUSB.begin(115200);
  while(!SerialUSB);

  lcd.begin();
  analogWrite(PIN_LCD_BACKLIGHT, 255);
  lcd.clear();

  // turn key phrase into key by hashing it.
  derive_key_from_key_phrase(key_phrase, sym_hash_key);

  SerialUSB.println("---");

}

void loop() {
  
  uint8_t value[32];

  SHA256  hash;
  hash.reset();
  hash.resetHMAC(sym_hash_key, sizeof(sym_hash_key));
  hash.update(data, strlen(data));
  hash.finalizeHMAC(sym_hash_key, sizeof(sym_hash_key), value, sizeof(value));

  for ( int i = 0; i < sizeof(value); i++) {
    char buf[] = "\0\0\0\0";
    sprintf(buf,"%0.2x", value[i]);
    SerialUSB.print(buf);
  }
  SerialUSB.println();

  delay(1000000);
  
}
