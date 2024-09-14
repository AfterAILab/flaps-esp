#include "env.h"
#include "morseCode.h"

String getMorseCode(char c) {
  switch (c) {
    case '1': return ".----";
    case '2': return "..---";
    case '3': return "...--";
    case '4': return "....-";
    case '5': return ".....";
    case '6': return "-....";
    case '7': return "--...";
    case '8': return "---..";
    case '9': return "----.";
    case '0': return "-----";
    default: return "";
  }
}

void flashMorseCodeOfChar(char c) {
    String morseCode = getMorseCode(c);
    for (int i = 0; i < morseCode.length(); i++) {
        char c = morseCode[i];
        if (c == '.') {
            // Flash a dot
            digitalWrite(LED_PIN, HIGH);
            delay(MORSE_CODE_UNIT_DURATION);
            digitalWrite(LED_PIN, LOW);
        } else {
            // Flash a dash
            digitalWrite(LED_PIN, HIGH);
            delay(MORSE_CODE_UNIT_DURATION * 3);
            digitalWrite(LED_PIN, LOW);
        }
        delay(MORSE_CODE_UNIT_DURATION);
    }
}

void flashMorseCode(String str) {
    for (int i = 0; i < str.length(); i++) {
        char c = str[i];
        if (c == '.') {
            delay(MORSE_CODE_UNIT_DURATION * MORSE_CODE_WORD_SEPARATION_DURATION_FACTOR);
        } else {
            // Flash a dash
            flashMorseCodeOfChar(c);
            delay(MORSE_CODE_UNIT_DURATION * MORSE_CODE_LETTER_SEPARATION_DURATION_FACTOR);
        }
    }
}
