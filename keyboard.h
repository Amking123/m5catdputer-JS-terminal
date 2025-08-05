#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <Arduino.h>
#include "M5Cardputer.h"

String shellInput = "";
bool shellUpdate = true;
int outputLine = 0;  // Track output line number for scrolling

// Clear and draw the prompt at the bottom
void shellDrawPrompt() {
  int y = M5Cardputer.Display.height() - 16;
  M5Cardputer.Display.fillRect(0, y, M5Cardputer.Display.width(), 16, BLACK);
  M5Cardputer.Display.setCursor(0, y);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.print("> ");
  M5Cardputer.Display.print(shellInput);
}

// Print output text above the prompt area and scroll if needed
void shellPrint(String text, uint16_t color = WHITE) {
  int lineHeight = 16;
  int maxLines = (M5Cardputer.Display.height() / lineHeight) - 1;  // leave 1 line for prompt

  // Scroll output if exceed maxLines
  if (outputLine >= maxLines) {
    // Scroll screen up by one line
    M5Cardputer.Display.writeFillRect(0, 0, M5Cardputer.Display.width(), M5Cardputer.Display.height() - lineHeight, BLACK);
    M5Cardputer.Display.drawBitmap(0, -lineHeight, M5Cardputer.Display.width(), M5Cardputer.Display.height(), nullptr);
    outputLine = maxLines - 1;
  }

  M5Cardputer.Display.setCursor(0, outputLine * lineHeight);
  M5Cardputer.Display.setTextColor(color);
  M5Cardputer.Display.println(text);
  M5Cardputer.Display.setTextColor(WHITE);
  outputLine++;
}


// Process keyboard input and update input string, return true if Enter pressed
bool shellHandleKeyboard() {
  bool entered = false;
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isChange()) {
    auto keys = M5Cardputer.Keyboard.keysState();
    for (auto c : keys.word) {
      shellInput += c;
      shellUpdate = true;
    }
    if (keys.del && shellInput.length() > 0) {
      shellInput.remove(shellInput.length() - 1);
      shellUpdate = true;
    }
    if (keys.enter) {
      // Print the command entered on the output area, not on prompt line
      shellPrint("> " + shellInput, WHITE);
      entered = true;
    }
  }
  return entered;
}

#endif