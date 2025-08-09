#ifndef TERMINALCOMMANDS_H
#define TERMINALCOMMANDS_H

#include <Arduino.h>
#include <M5Cardputer.h>
#include <FS.h>
#include <SD.h>
#include <WiFiClientSecure.h>
#include "duktape.h"
#include "keyboard.h"
#include "menu.h"
#include "bios.h"

// Forward declarations
void registerJSBindings(duk_context *ctx);

// ==== Terminal Command Processor ====
void processCommand(duk_context *ctx, String input) {
  input.trim();
  if (input.equalsIgnoreCase("help")) {
    shellPrint("Commands: help, clear, menu, about, bios, reboot, random, run <app>");
    shellPrint("JS: print(), load(), run(), require(), getKey(), etc.");
  }
  else if (input.equalsIgnoreCase("clear")) {
    M5Cardputer.Display.fillScreen(BLACK);
    outputLine = 0;
  }
  else if (input.equalsIgnoreCase("about")) {
   shellPrint("╔═══════════════════════════╗", CYAN);
   shellPrint("║  M5Cardputer JS Terminal  ║", CYAN);
   shellPrint("║  Created by [AMking123]   ║", CYAN);
   shellPrint("║ Version 7.2.0 new commands║", CYAN);
   shellPrint("╚═══════════════════════════╝", CYAN);
  }
  else if (input.equalsIgnoreCase("reboot")) {
    shellPrint("Rebooting...");
    delay(1000);
    ESP.restart();
  }
   else if (input == "bios") {
    openBIOS();
  }
  else if (input.equalsIgnoreCase("menu")) {
  openMenu(ctx);
  }
  else if (input.equalsIgnoreCase("random")) {
    shellPrint("Random: " + String(random(0, 1000)));
  }
  else if (input.startsWith("run ")) {
    String app = input.substring(4);
    app.trim();
    String path = "/apps/" + app + ".js";
    if (SD.exists(path)) {
      File f = SD.open(path);
      if (!f) {
        shellPrint("Failed to open app file: " + path, RED);
        return;
      }
      String script = f.readString();
      f.close();
      shellPrint("Running " + app);
      if (duk_peval_string(ctx, script.c_str()) != 0) {
        shellPrint("Error: " + String(duk_safe_to_string(ctx, -1)), RED);
      }
      duk_pop(ctx);
    } else {
      shellPrint("App not found: " + app, RED);
    }
  }
  else {
    if (duk_peval_string(ctx, input.c_str()) != 0) {
      shellPrint("Error: " + String(duk_safe_to_string(ctx, -1)), RED);
    } else if (!duk_is_undefined(ctx, -1)) {
      shellPrint(String(duk_safe_to_string(ctx, -1)), GREEN);
    }
    duk_pop(ctx);
  }
}

// ==== JS Binding Implementations ====

// print(...args)
static duk_ret_t js_print(duk_context *ctx) {
  duk_idx_t nargs = duk_get_top(ctx);
  String out = "";
  for (duk_idx_t i = 0; i < nargs; i++) {
    out += String(duk_safe_to_string(ctx, i));
    if (i < nargs - 1) out += " ";
  }
  shellPrint(out);
  return 0;
}

// delay(ms)
static duk_ret_t js_delay(duk_context *ctx) {
  int ms = duk_to_int(ctx, 0);
  delay(ms);
  return 0;
}

// now() -> milliseconds since boot
static duk_ret_t js_now(duk_context *ctx) {
  duk_push_number(ctx, millis());
  return 1;
}

// setTextColor(color)
static duk_ret_t js_setTextColor(duk_context *ctx) {
  uint16_t color = (uint16_t)duk_to_uint(ctx, 0);
  M5Cardputer.Display.setTextColor(color);
  return 0;
}

// setTextSize(size)
static duk_ret_t js_setTextSize(duk_context *ctx) {
  int size = duk_to_int(ctx, 0);
  M5Cardputer.Display.setTextSize(size);
  return 0;
}

// digitalWrite(pin, value)
static duk_ret_t js_digitalWrite(duk_context *ctx) {
  int pin = duk_to_int(ctx, 0);
  int val = duk_to_int(ctx, 1);
  digitalWrite(pin, val);
  return 0;
}

// pinMode(pin, mode)
static duk_ret_t js_pinMode(duk_context *ctx) {
  int pin = duk_to_int(ctx, 0);
  int mode = duk_to_int(ctx, 1);
  pinMode(pin, mode);
  return 0;
}

// color(r, g, b) -> uint16_t 565 color
static duk_ret_t js_color(duk_context *ctx) {
  int r = duk_to_int(ctx, 0);
  int g = duk_to_int(ctx, 1);
  int b = duk_to_int(ctx, 2);
  uint16_t c = M5Cardputer.Display.color565(r, g, b);
  duk_push_uint(ctx, c);
  return 1;
}

// drawRect(x, y, w, h, color)
static duk_ret_t js_drawRect(duk_context *ctx) {
  int x = duk_to_int(ctx, 0);
  int y = duk_to_int(ctx, 1);
  int w = duk_to_int(ctx, 2);
  int h = duk_to_int(ctx, 3);
  uint16_t color = (uint16_t)duk_to_uint(ctx, 4);
  M5Cardputer.Display.drawRect(x, y, w, h, color);
  return 0;
}

// drawFillRect(x, y, w, h, color)
static duk_ret_t js_fillRect(duk_context *ctx) {
  int x = duk_to_int(ctx, 0);
  int y = duk_to_int(ctx, 1);
  int w = duk_to_int(ctx, 2);
  int h = duk_to_int(ctx, 3);
  uint16_t color = (uint16_t)duk_to_uint(ctx, 4);
  M5Cardputer.Display.fillRect(x, y, w, h, color);
  return 0;
}

// drawString(text, x, y, color)
static duk_ret_t js_drawString(duk_context *ctx) {
  const char *text = duk_to_string(ctx, 0);
  int x = duk_to_int(ctx, 1);
  int y = duk_to_int(ctx, 2);
  uint16_t color = (uint16_t)duk_to_uint(ctx, 3);
  M5Cardputer.Display.setCursor(x, y);
  M5Cardputer.Display.setTextColor(color);
  M5Cardputer.Display.print(text);
  return 0;
}

// width()
static duk_ret_t js_width(duk_context *ctx) {
  duk_push_int(ctx, M5Cardputer.Display.width());
  return 1;
}

// height()
static duk_ret_t js_height(duk_context *ctx) {
  duk_push_int(ctx, M5Cardputer.Display.height());
  return 1;
}

// getKeysPressed() -> array of chars pressed
static duk_ret_t js_getKeysPressed(duk_context *ctx) {
  M5Cardputer.update();
  auto ks = M5Cardputer.Keyboard.keysState();
  duk_idx_t arr = duk_push_array(ctx);
  int idx = 0;
  for (char c : ks.word) {
    duk_push_string(ctx, String(c).c_str());
    duk_put_prop_index(ctx, arr, idx++);
  }
  return 1;
}

// getKey() -> last key pressed as string or ""
static duk_ret_t js_getKey(duk_context *ctx) {
  M5Cardputer.update();
  auto ks = M5Cardputer.Keyboard.keysState();
  if (!ks.word.empty()) {
    char c = ks.word.back();
    String str = String(c);
    duk_push_string(ctx, str.c_str());
  } else {
    duk_push_string(ctx, "");
  }
  return 1;
}

// ==== Filesystem and Module Loading ====

// load(filename) -> returns file contents as string
static duk_ret_t js_load(duk_context *ctx) {
  const char *filename = duk_to_string(ctx, 0);
  String path = "/apps/" + String(filename);
  if (!SD.exists(path)) {
    duk_push_undefined(ctx);
    return 1;
  }
  File f = SD.open(path);
  if (!f) {
    duk_push_undefined(ctx);
    return 1;
  }
  String content = f.readString();
  f.close();
  duk_push_string(ctx, content.c_str());
  return 1;
}

// run(filename) -> loads and executes a JS file, returns result or undefined on failure
static duk_ret_t js_run(duk_context *ctx) {
  const char *filename = duk_to_string(ctx, 0);
  String path = "/apps/" + String(filename);
  if (!SD.exists(path)) {
    shellPrint("File not found: " + path, RED);
    duk_push_undefined(ctx);
    return 1;
  }
  File f = SD.open(path);
  if (!f) {
    shellPrint("Failed to open file: " + path, RED);
    duk_push_undefined(ctx);
    return 1;
  }
  String content = f.readString();
  f.close();
  if (duk_peval_string(ctx, content.c_str()) != 0) {
    shellPrint("Error running " + String(filename) + ": " + String(duk_safe_to_string(ctx, -1)), RED);
    duk_pop(ctx);
    duk_push_undefined(ctx);
    return 1;
  }
  // Return eval result
  return 1;
}

// require(filename) with CommonJS-style module system
static duk_ret_t js_require(duk_context *ctx) {
  const char *filename = duk_to_string(ctx, 0);
  String path = "/modules/" + String(filename);
  if (!SD.exists(path)) {
    duk_push_error_object(ctx, DUK_ERR_ERROR, "Module not found: %s", path.c_str());
    return duk_throw(ctx);
  }

  // Module cache object (global.__modcache)
  duk_push_global_object(ctx);
  duk_get_prop_string(ctx, -1, "__modcache");
  if (!duk_is_object(ctx, -1)) {
    duk_pop(ctx);  // pop undefined
    duk_push_object(ctx);
    duk_dup(ctx, -1); // duplicate
    duk_put_prop_string(ctx, -3, "__modcache"); // global.__modcache = {}
  }

  // Check if module is already cached
  duk_get_prop_string(ctx, -1, path.c_str());
  if (duk_is_object(ctx, -1)) {
    return 1; // Return cached exports
  }
  duk_pop(ctx); // remove undefined

  // Read module file
  File f = SD.open(path);
  if (!f) {
    duk_push_error_object(ctx, DUK_ERR_ERROR, "Failed to open: %s", path.c_str());
    return duk_throw(ctx);
  }
  String code = f.readString();
  f.close();

  // Wrap module code in function: (function(exports, module, require){ ... })
  String wrapped = "(function(exports, module, require){\n" + code + "\n})";
  if (duk_peval_string(ctx, wrapped.c_str()) != 0) {
    shellPrint("Module eval error: " + String(duk_safe_to_string(ctx, -1)), RED);
    return duk_throw(ctx);
  }

  // Set up exports/module
  duk_push_object(ctx); // exports
  duk_push_object(ctx); // module
  duk_dup(ctx, -2);      // copy exports
  duk_put_prop_string(ctx, -2, "exports"); // module.exports = exports

  duk_dup(ctx, -3); // the require() function
  duk_dup(ctx, -4); // the function from eval

  // Call function with (exports, module, require)
  duk_dup(ctx, -4); // exports
  duk_dup(ctx, -4); // module
  duk_dup(ctx, -4); // require
  if (duk_pcall(ctx, 3) != 0) {
    shellPrint("Module run error: " + String(duk_safe_to_string(ctx, -1)), RED);
    return duk_throw(ctx);
  }

  // Get module.exports
  duk_get_prop_string(ctx, -2, "exports");

  // Cache the result
  duk_dup(ctx, -1);               // duplicate exports
  duk_put_prop_string(ctx, -6, path.c_str()); // __modcache[path] = exports

  return 1;
}

// ==== NEW: Advanced Display & Graphics Commands ====

// clear() and fillScreen()
static duk_ret_t js_fillScreen(duk_context *ctx) {
  uint16_t color = (uint16_t)duk_to_uint(ctx, 0);
  M5Cardputer.Display.fillScreen(color);
  return 0;
}
static duk_ret_t js_clear(duk_context *ctx) {
  // Alias for fillScreen
  return js_fillScreen(ctx);
}

// drawCircle(x, y, r, color)
static duk_ret_t js_drawCircle(duk_context *ctx) {
  int x = duk_to_int(ctx, 0);
  int y = duk_to_int(ctx, 1);
  int r = duk_to_int(ctx, 2);
  uint16_t color = (uint16_t)duk_to_uint(ctx, 3);
  M5Cardputer.Display.drawCircle(x, y, r, color);
  return 0;
}

// fillCircle(x, y, r, color)
static duk_ret_t js_fillCircle(duk_context *ctx) {
  int x = duk_to_int(ctx, 0);
  int y = duk_to_int(ctx, 1);
  int r = duk_to_int(ctx, 2);
  uint16_t color = (uint16_t)duk_to_uint(ctx, 3);
  M5Cardputer.Display.fillCircle(x, y, r, color);
  return 0;
}

// drawLine(x1, y1, x2, y2, color)
static duk_ret_t js_drawLine(duk_context *ctx) {
  int x1 = duk_to_int(ctx, 0);
  int y1 = duk_to_int(ctx, 1);
  int x2 = duk_to_int(ctx, 2);
  int y2 = duk_to_int(ctx, 3);
  uint16_t color = (uint16_t)duk_to_uint(ctx, 4);
  M5Cardputer.Display.drawLine(x1, y1, x2, y2, color);
  return 0;
}

// drawTriangle(x1, y1, x2, y2, x3, y3, color)
static duk_ret_t js_drawTriangle(duk_context *ctx) {
  int x1 = duk_to_int(ctx, 0);
  int y1 = duk_to_int(ctx, 1);
  int x2 = duk_to_int(ctx, 2);
  int y2 = duk_to_int(ctx, 3);
  int x3 = duk_to_int(ctx, 4);
  int y3 = duk_to_int(ctx, 5);
  uint16_t color = (uint16_t)duk_to_uint(ctx, 6);
  M5Cardputer.Display.drawTriangle(x1, y1, x2, y2, x3, y3, color);
  return 0;
}

// fillTriangle(x1, y1, x2, y2, x3, y3, color)
static duk_ret_t js_fillTriangle(duk_context *ctx) {
  int x1 = duk_to_int(ctx, 0);
  int y1 = duk_to_int(ctx, 1);
  int x2 = duk_to_int(ctx, 2);
  int y2 = duk_to_int(ctx, 3);
  int x3 = duk_to_int(ctx, 4);
  int y3 = duk_to_int(ctx, 5);
  uint16_t color = (uint16_t)duk_to_uint(ctx, 6);
  M5Cardputer.Display.fillTriangle(x1, y1, x2, y2, x3, y3, color);
  return 0;
}

// setCursor(x, y)
static duk_ret_t js_setCursor(duk_context *ctx) {
  int x = duk_to_int(ctx, 0);
  int y = duk_to_int(ctx, 1);
  M5Cardputer.Display.setCursor(x, y);
  return 0;
}

// hexToColor(hexString) -> uint16_t color
static duk_ret_t js_hexToColor(duk_context *ctx) {
  const char *hex_str = duk_to_string(ctx, 0);
  if (!hex_str) {
    duk_push_uint(ctx, 0); // Return black on invalid input
    return 1;
  }
  // Remove '#' if present
  if (hex_str[0] == '#') {
    hex_str++;
  }
  long hex_val = strtol(hex_str, NULL, 16);
  int r = (hex_val >> 16) & 0xFF;
  int g = (hex_val >> 8) & 0xFF;
  int b = hex_val & 0xFF;
  uint16_t c = M5Cardputer.Display.color565(r, g, b);
  duk_push_uint(ctx, c);
  return 1;
}

// ==== NEW: Advanced Filesystem Commands ====

// listDir(path) -> array of filenames
static duk_ret_t js_listDir(duk_context *ctx) {
  const char *path = duk_to_string(ctx, 0);
  File root = SD.open(path);
  if (!root || !root.isDirectory()) {
    duk_push_array(ctx); // Return empty array on failure
    return 1;
  }
  duk_idx_t arr = duk_push_array(ctx);
  int idx = 0;
  File file = root.openNextFile();
  while (file) {
    duk_push_string(ctx, file.name());
    duk_put_prop_index(ctx, arr, idx++);
    file.close();
    file = root.openNextFile();
  }
  root.close();
  return 1;
}

// deleteFile(path) -> boolean success
static duk_ret_t js_deleteFile(duk_context *ctx) {
  const char *path = duk_to_string(ctx, 0);
  duk_push_boolean(ctx, SD.remove(path));
  return 1;
}

// renameFile(oldPath, newPath) -> boolean success
static duk_ret_t js_renameFile(duk_context *ctx) {
  const char *oldPath = duk_to_string(ctx, 0);
  const char *newPath = duk_to_string(ctx, 1);
  duk_push_boolean(ctx, SD.rename(oldPath, newPath));
  return 1;
}

// makeDir(path) -> boolean success
static duk_ret_t js_makeDir(duk_context *ctx) {
  const char *path = duk_to_string(ctx, 0);
  duk_push_boolean(ctx, SD.mkdir(path));
  return 1;
}

// ==== NEW: Event-Based Input (REQUIRES MAIN LOOP MODIFICATION) ====
static duk_idx_t onKeyPress_cb_idx = DUK_INVALID_INDEX;

// onKeyPress(callback) -> registers a JS function to be called on key press
static duk_ret_t js_onKeyPress(duk_context *ctx) {
  // Check if a callback is provided
  if (duk_is_function(ctx, 0)) {
    // If a previous callback exists, remove its reference
    if (onKeyPress_cb_idx != DUK_INVALID_INDEX) {
      duk_remove(ctx, onKeyPress_cb_idx);
    }
    
    // Duplicate the function from index 0 and push it to the top of the stack.
    duk_dup(ctx, 0);
    
    // Store the index of the new item we just pushed.
    onKeyPress_cb_idx = duk_get_top(ctx) - 1;
  } else {
    // Clear the callback if no function is passed
    onKeyPress_cb_idx = DUK_INVALID_INDEX;
  }
  return 0;
}

// ==== Register All Bindings to Duktape Context ====

void registerJSBindings(duk_context *ctx) {
  duk_push_global_object(ctx);

  duk_push_c_function(ctx, js_print, DUK_VARARGS);
  duk_put_prop_string(ctx, -2, "print");

  duk_push_c_function(ctx, js_delay, 1);
  duk_put_prop_string(ctx, -2, "delay");

  duk_push_c_function(ctx, js_now, 0);
  duk_put_prop_string(ctx, -2, "now");

  duk_push_c_function(ctx, js_setTextColor, 1);
  duk_put_prop_string(ctx, -2, "setTextColor");

  duk_push_c_function(ctx, js_setTextSize, 1);
  duk_put_prop_string(ctx, -2, "setTextSize");

  duk_push_c_function(ctx, js_digitalWrite, 2);
  duk_put_prop_string(ctx, -2, "digitalWrite");

  duk_push_c_function(ctx, js_pinMode, 2);
  duk_put_prop_string(ctx, -2, "pinMode");

  duk_push_c_function(ctx, js_color, 3);
  duk_put_prop_string(ctx, -2, "color");
  
  // New Hex to color
  duk_push_c_function(ctx, js_hexToColor, 1);
  duk_put_prop_string(ctx, -2, "hexToColor");

  duk_push_c_function(ctx, js_drawRect, 5);
  duk_put_prop_string(ctx, -2, "drawRect");

  // Corrected name for fillRect, and an alias for backwards compatibility
  duk_push_c_function(ctx, js_fillRect, 5);
  duk_put_prop_string(ctx, -2, "fillRect");
  duk_push_c_function(ctx, js_fillRect, 5);
  duk_put_prop_string(ctx, -2, "drawFillRect");

  duk_push_c_function(ctx, js_drawString, 4);
  duk_put_prop_string(ctx, -2, "drawString");
  
  // New Graphics Primitives
  duk_push_c_function(ctx, js_fillScreen, 1);
  duk_put_prop_string(ctx, -2, "fillScreen");
  duk_push_c_function(ctx, js_clear, 1);
  duk_put_prop_string(ctx, -2, "clear");

  duk_push_c_function(ctx, js_drawCircle, 4);
  duk_put_prop_string(ctx, -2, "drawCircle");
  duk_push_c_function(ctx, js_fillCircle, 4);
  duk_put_prop_string(ctx, -2, "fillCircle");

  duk_push_c_function(ctx, js_drawLine, 5);
  duk_put_prop_string(ctx, -2, "drawLine");

  duk_push_c_function(ctx, js_drawTriangle, 7);
  duk_put_prop_string(ctx, -2, "drawTriangle");
  duk_push_c_function(ctx, js_fillTriangle, 7);
  duk_put_prop_string(ctx, -2, "fillTriangle");

  duk_push_c_function(ctx, js_setCursor, 2);
  duk_put_prop_string(ctx, -2, "setCursor");

  duk_push_c_function(ctx, js_width, 0);
  duk_put_prop_string(ctx, -2, "width");

  duk_push_c_function(ctx, js_height, 0);
  duk_put_prop_string(ctx, -2, "height");

  duk_push_c_function(ctx, js_getKeysPressed, 0);
  duk_put_prop_string(ctx, -2, "getKeysPressed");

  duk_push_c_function(ctx, js_getKey, 0);
  duk_put_prop_string(ctx, -2, "getKey");
  
  // NEW: Event-based input
  duk_push_c_function(ctx, js_onKeyPress, 1);
  duk_put_prop_string(ctx, -2, "onKeyPress");

  duk_push_c_function(ctx, js_load, 1);
  duk_put_prop_string(ctx, -2, "load");

  duk_push_c_function(ctx, js_run, 1);
  duk_put_prop_string(ctx, -2, "run");

  duk_push_c_function(ctx, js_require, 1);
  duk_put_prop_string(ctx, -2, "require");

  // New Filesystem commands
  duk_push_c_function(ctx, js_listDir, 1);
  duk_put_prop_string(ctx, -2, "listDir");

  duk_push_c_function(ctx, js_deleteFile, 1);
  duk_put_prop_string(ctx, -2, "deleteFile");

  duk_push_c_function(ctx, js_renameFile, 2);
  duk_put_prop_string(ctx, -2, "renameFile");

  duk_push_c_function(ctx, js_makeDir, 1);
  duk_put_prop_string(ctx, -2, "makeDir");


  duk_pop(ctx); // pop global object
}

#endif
