//Support any GFX compatible fonts
//more fonts here https://github.com/igor-gia/Ukr_GFX_fonts
//or create own https://tchapi.github.io/Adafruit-GFX-Font-Customiser/

#include <Wire.h>
#include <Print.h>

//https://learn.adafruit.com/creating-custom-symbol-font-for-adafruit-gfx-library/understanding-the-font-specification
struct GFXglyph {
  uint16_t bitmapOffset;
  uint8_t columns;
  uint8_t rows;
  uint8_t xAdvance;
  uint8_t xOffset;
  int8_t yOffset;
};

struct GFXfont {
  uint8_t* bitmaps;
  GFXglyph* glyphs;
  uint8_t firstGlyph;
  uint8_t lastGlyph;
  uint8_t yAdvance;
};

#include "LynkFont.h"

const unsigned char lynk_load[] PROGMEM = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xfc, 0x00, 0x02, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x7f, 0x80, 0x03, 0x60, 0x00,
                                            0x00, 0x00, 0xe0, 0x1f, 0xf0, 0x03, 0x7c, 0x00,
                                            0x00, 0x00, 0xfc, 0x07, 0xfe, 0x83, 0x7f, 0x00,
                                            0x00, 0x00, 0xfe, 0x00, 0x7f, 0xc0, 0x1f, 0x00,
                                            0x00, 0xc0, 0x3f, 0xe0, 0x1f, 0xf8, 0x07, 0x00,
                                            0x00, 0xf0, 0x0f, 0xf8, 0x07, 0xfe, 0x01, 0x00,
                                            0x00, 0xfc, 0x03, 0xfe, 0x81, 0x7f, 0x00, 0x00,
                                            0x80, 0xff, 0xc0, 0x7f, 0xf0, 0x1f, 0x00, 0x00,
                                            0xe0, 0x3f, 0xf0, 0x1f, 0xfc, 0x07, 0x00, 0x00,
                                            0xf8, 0x0f, 0xfc, 0x07, 0xff, 0x01, 0x00, 0x00,
                                            0xfc, 0x07, 0xfe, 0x83, 0xff, 0xe0, 0x7f, 0x00,
                                            0xfc, 0x07, 0xfe, 0x83, 0xff, 0xe0, 0x7f, 0x00,
                                            0xf8, 0x0f, 0xfc, 0x07, 0xff, 0x01, 0x00, 0x00,
                                            0xe0, 0x3f, 0xf0, 0x1f, 0xfc, 0x07, 0x00, 0x00,
                                            0x80, 0xff, 0xc0, 0x7f, 0xf0, 0x1f, 0x00, 0x00,
                                            0x00, 0xfc, 0x03, 0xfe, 0x81, 0x7f, 0x00, 0x00,
                                            0x00, 0xf0, 0x0f, 0xf8, 0x07, 0xfe, 0x01, 0x00,
                                            0x00, 0xc0, 0x3f, 0xe0, 0x1f, 0xf8, 0x07, 0x00,
                                            0x00, 0x00, 0xfe, 0x00, 0x7f, 0xc0, 0x1f, 0x00,
                                            0x00, 0x00, 0xfc, 0x07, 0xfe, 0x83, 0x7f, 0x00,
                                            0x00, 0x00, 0xf0, 0x1f, 0xf8, 0x07, 0x7e, 0x00,
                                            0x00, 0x00, 0x00, 0x7f, 0x80, 0x07, 0x60, 0x00,
                                            0x00, 0x00, 0x00, 0xfc, 0x00, 0x06, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xfe, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xfe, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xfe, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0x00, 0x10, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0x07, 0x70, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0x3f, 0x70, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0xff, 0x79, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0xf8, 0x7f, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0xe0, 0x3f, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xc0, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x80, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xfe, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xfe, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xfe, 0xff, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xc0, 0x7f, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0xf9, 0x01, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0xe0, 0xe0, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x60, 0x80, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x20, 0x00, 0x03, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

//#define OLED_HEIGHT_32 0x02
#define OLED_HEIGHT_64 0x12
#define OLED_64 0x3F
//#define OLED_32 0x1F
#define OLED_SETCOMPINS 0xDA
#define OLED_SETMULTIPLEX 0xA8
#define OLED_COLUMNADDR 0x21
#define OLED_PAGEADDR 0x22

enum DisplayType {
  SSD1306_128x64
};

enum FillType {
  CLEAR,
  FILL,
  STROKE,
};

enum OVERWRITE_MODE {
  ADD,
  SUB,
  REPLACE,
};



static const uint8_t _oled_init[] PROGMEM = {
  0xAE,  //DISPLAY OFF
  0xD5,  //DISPLAY_CLOCK
  0x80,  // value
  0x8D,  //Charge Pump
  0x14,  // value
  0x20,  //Adressing mode
  0x01,  //Vertical
  0xA1,  //no flip horizontal
  0xC8,  //no flip vertical
  0x81,  //OLED_CONTRAST,
  0x7F,  // value
  0xDB,  //Set VCOMH Deselect Level
  0x40,  // value
  0xA6,  //not inverted
  0xAF   //OLED_DISPLAY_ON,
};

template<DisplayType _TYPE>
class LynkOLED : public Print {
public:
  LynkOLED(uint8_t address = 0x3C)
    : _address(address) {}

  void begin(int sda = 0, int scl = 0) {
    if (sda || scl)
      Wire.begin(sda, scl);
    else
      Wire.begin();
    beginCommand();
    for (uint8_t i = 0; i < 15; i++) sendByte(pgm_read_byte(&_oled_init[i]));
    endTransm();
    beginCommand();
    sendByte(OLED_SETCOMPINS);
    sendByte(OLED_HEIGHT_64);
    sendByte(OLED_SETMULTIPLEX);
    sendByte(OLED_64);
    endTransm();
    setCursorXY(0, 0);
    setFont(&TimesNewRomanHomenko8p);
    setWindow(0, 0, _maxX, _maxRow);
    memcpy_P(_oled_buffer, lynk_load, sizeof(_oled_buffer));
    update();
  }

  void setFont(const GFXfont* f) {
    gfxFont = (GFXfont*)f;
    _yAdvance = pgm_read_byte(&gfxFont->yAdvance);
    _firstGlyph = pgm_read_byte(&gfxFont->firstGlyph);
    _lastGlyph = pgm_read_byte(&gfxFont->lastGlyph);
  }

  void clear() {
    fill(0);
  }

  void clear(int x0, int y0, int x1, int y1) {
    rect(x0, y0, x1, y1, CLEAR);
  }

  void setContrast(uint8_t value) {
    sendCommand(0x81, value);
  }

  void setPower(bool mode) {
    sendCommand(mode ? 0xAF : 0xAE);
  }

  void flipH(bool mode) {
    sendCommand(mode ? 0xA0 : 0xA1);
  }

  void invertDisplay(bool mode) {
    sendCommand(mode ? 0xA7 : 0xA6);
  }

  void flipV(bool mode) {
    sendCommand(mode ? 0xC0 : 0xC8);
  }

  uint8_t _getNewGlyphCode(uint8_t data) {
    if (data == 0xF0) {  //4-byte UTF-8
      _charGroup = 0xF0;
      return 0;
    } else if (data == 0xE2) {  //3byte UTF-8
      _charGroup = 0xE2;
      return 0;
    } else if (_charGroup == 0xF0) {
      if (data == 0x9F) {
        _charGroup = 0x9F;
      } else return 0x7F;  //unknown 4byte
    } else if (_charGroup == 0x9F) {
      if (data == 0x94 || data == 0xAA) {
        _charGroup = data;
      } else return 0x7F;  //unknown 4byte
    } else if (_charGroup == 0xE2) {
      if (data == 0x82 || data == 0x84 || data == 0x86) {  //currency or lettel-like symbol or arrows
        _charGroup = data;
        return 0;
      } else {
        return 0x7F;  //unknown 3byte
      }
    }
    if (data >= 0xC0 && data <= 0xDF) {  //2byte
      _charGroup = data;
      return 0;
    }

    if (_charGroup == 0xD1 && data >= 0x80 && data <= 0x8F) return data;  //рстуфхцчшщъыьэюя
    if (_charGroup == 0xD0 && data >= 0x90 && data <= 0xBF) return data;  //АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмноп
    if (_charGroup == 0xD0 && data == 0x81) return 0xC0;                  //Ё
    if (_charGroup == 0xD1 && data == 0x91) return 0xC1;                  //ё
    if (_charGroup == 0xD0 && data == 0x84) return 0xC2;                  //Є
    if (_charGroup == 0xD1 && data == 0x94) return 0xC3;                  //є
    if (_charGroup == 0xD0 && data == 0x86) return 0xC4;                  //І
    if (_charGroup == 0xD1 && data == 0x96) return 0xC5;                  //і
    if (_charGroup == 0xD0 && data == 0x87) return 0xC6;                  //Ї
    if (_charGroup == 0xD1 && data == 0x97) return 0xC7;                  //ї
    if (_charGroup == 0xD2 && data == 0x90) return 0xC8;                  //Ґ
    if (_charGroup == 0xD2 && data == 0x91) return 0xC9;                  //ґ
    if (_charGroup == 0x82 && data == 0xB4) return 0xCA;                  //₴
    if (_charGroup == 0xCA && data == 0xBC) return 0xCB;                  //ʼ
    if (_charGroup == 0xC2 && data == 0xB0) return 0xCC;                  //°
    if (_charGroup == 0x84 && data == 0x83) return 0xCD;                  //℃
    if (_charGroup == 0x86 && data == 0x90) return 0xCE;                  //←
    if (_charGroup == 0x86 && data == 0x92) return 0xCF;                  //→
    if (_charGroup == 0x94 && data == 0x8B) return 0xCF;                  //🔋
    if (_charGroup == 0xAA && data == 0xAB) return 0xCF;                  //🪫
    Serial.print("WARNING: UNKNOWN CHAR ");
    Serial.print(_charGroup, HEX);
    Serial.print(" ");
    Serial.println(data, HEX);
    return 0x7F;  //?
  }

  virtual size_t write(uint8_t data) {
    //Serial.println(data, HEX);
    if (data > 0x7F) {
      data = _getNewGlyphCode(data);
      if (data != 0) _charGroup = 0;
      else return 1;
    } else if (data == '\r') {
      _x = 0;
      return 1;
    } else if (data == '\n') {
      setCursorXY(_x, _y + (_yAdvance * _scale));
      return 1;
    }

    data -= _firstGlyph;
    GFXglyph* glyph = gfxFont->glyphs + data;
    uint8_t _xAdvance = pgm_read_byte(&glyph->xAdvance);
    if (_x < 0) {
      return 1;
    } else {
      int xEnd = _x + (_scale * _xAdvance) - 1;
      if (xEnd > _maxX) {
        if (_println) {
          setCursorXY(0, _y + (_yAdvance * _scale));
        } else {
          return 1;
        }
      }
    }
    int yEnd = _y + (_yAdvance * _scale) - 1;
    if (yEnd > _maxY) return 1;

    //Serial.println(String("data: ") + data);
    //Serial.println(String("xAdvance: ") + _xAdvance);
    uint8_t* bitmap = gfxFont->bitmaps;
    uint16_t bitmapOffset = pgm_read_word(&glyph->bitmapOffset);
    uint8_t xOffset = pgm_read_byte(&glyph->xOffset);
    //Serial.println(String("xOffset: ") + xOffset);
    uint8_t width = pgm_read_byte(&glyph->columns);
    //Serial.println(String("width: ") + width);
    uint8_t height = pgm_read_byte(&glyph->rows);
    //Serial.println(String("height: ") + height);
    uint8_t bytesCount = (width * height + 7) / 8;
    uint8_t rowsCount = (height + 7) / 8;
    uint8_t result[rowsCount * width] = {};
    //transform GFX font bytes to column font bytes
    for (int i = 0; i < bytesCount; i++) {
      uint8_t bits = pgm_read_byte(&bitmap[bitmapOffset++]);
      for (int j = 7; j >= 0; j--) {  //
        int k = i * 8 + (7 - j);
        int l = k / 8 / width;
        bitWrite(result[l * width + k % width], (k / width) % 8, bitRead(bits, j));
      }
    }
    //Serial.println(String("======x: ") + _x + " y: " + _y + " data: " + data);
    for (uint8_t col = 0; col < _xAdvance; col++) {
      uint8_t bits = 0;
      if (col >= xOffset && col < (xOffset + width)) {
        bits = result[col - xOffset];
      }
      if (_invState) bits = ~bits;
      if (_scale == 1) {
        writeData2(bits, _yAdvance);
      } else {
      }
      _x += _scale;
    }
    if (_autoUpdateAfterEachLetter) {
      update();
      if (_delay) delay(_delay);
    }
    return 1;
  }

  //n==0 print as is , n==2,fillChar='0'  1 -> 01, n==3,fillChar='_' 1 -> __1
  void printNumberFmt(uint8_t number, uint8_t n = 0, char fillChar = '0') {
    if(n > 3) n = 3;
    if (n != 0)
      for (uint8_t i = 1, pow = 10; i < n; i++, pow *= 10)
        if (number < pow)
          print(fillChar);

    print(number);
  }

  void printNumberFmt(uint16_t number, uint8_t n = 0, char fillChar = '0') {
    if(n > 5) n = 5;
    uint16_t pow = 10;
    if (n != 0)
      for (uint8_t i = 1; i < n; i++) {
        if (number < pow)
          print(fillChar);
        pow *= 10;
      }
    print(number);
  }

  void setAutoPrintln(bool enable) {
    _println = enable;
  }

  void setTypeEffect(bool enable, uint32_t delay = 30L) {
    _autoUpdateAfterEachLetter = enable;
    _delay = delay;
  }

  void setCursorXY(int x, int y) {
    _x = x;
    _y = y;
    _shift = y & 0b111;
    //setWindowShift(x, y, _maxX, _scale * _yAdvance);
  }

  void setScale(uint8_t scale) {
    scale = constrain(scale, 1, 4);
    _scale = scale;
    //setWindowShift(_x, _y, _maxX, _scale * _yAdvance);
  }

  void invertText(bool inv) {
    _invState = inv;
  }

  void overwriteMode(OVERWRITE_MODE mode) {
    _mode = mode;
  }

  bool isEnd() {
    return (_y > _maxRow);
  }

  void dot(int x, int y, byte fill = 1) {
    if (x < 0 || x > _maxX || y < 0 || y > _maxY) return;
    _x = 0;
    _y = 0;
    bitWrite(_oled_buffer[_bufIndex(x, y)], y & 0b111, fill);
  }

  void line(int x0, int y0, int x1, int y1, byte fill = 1) {
    _x = 0;
    _y = 0;
    if (x0 == x1) fastLineV(x0, y0, y1, fill);
    else if (y0 == y1) fastLineH(y0, x0, x1, fill);
    else {
      int sx, sy, e2, err;
      int dx = abs(x1 - x0);
      int dy = abs(y1 - y0);
      sx = (x0 < x1) ? 1 : -1;
      sy = (y0 < y1) ? 1 : -1;
      err = dx - dy;
      for (;;) {
        dot(x0, y0, fill);
        if (x0 == x1 && y0 == y1) return;
        e2 = err << 1;
        if (e2 > -dy) {
          err -= dy;
          x0 += sx;
        }
        if (e2 < dx) {
          err += dx;
          y0 += sy;
        }
      }
    }
  }

  void fastLineH(int y, int x0, int x1, byte fill = 1) {
    _x = 0;
    _y = 0;
    if (x0 > x1) _swap(x0, x1);
    if (y < 0 || y > _maxY) return;
    if (x0 == x1) {
      dot(x0, y, fill);
      return;
    }
    x1++;
    x0 = constrain(x0, 0, _maxX);
    x1 = constrain(x1, 0, _maxX);
    for (int x = x0; x < x1; x++) dot(x, y, fill);
  }


  void fastLineV(int x, int y0, int y1, byte fill = 1) {
    _x = 0;
    _y = 0;
    if (y0 > y1) _swap(y0, y1);
    if (x < 0 || x > _maxX) return;
    if (y0 == y1) {
      dot(x, y0, fill);
      return;
    }
    y1++;
    for (int y = y0; y < y1; y++) dot(x, y, fill);
  }

  void rect(int x0, int y0, int x1, int y1, FillType fillType = FILL) {
    _x = 0;
    _y = 0;
    if (x0 > x1) _swap(x0, x1);
    if (y0 > y1) _swap(y0, y1);
    if (fillType == STROKE) {
      fastLineH(y0, x0 + 1, x1 - 1);
      fastLineH(y1, x0 + 1, x1 - 1);
      fastLineV(x0, y0, y1);
      fastLineV(x1, y0, y1);
      return;
    }
    if (x0 == x1 && y0 == y1) {
      dot(x0, y0, (fillType == FILL));
      return;
    }
    if (x0 == x1) {
      fastLineV(x0, y0, y1, (fillType == FILL));
      return;
    }
    if (y0 == y1) {
      fastLineH(y0, x0, x1, (fillType == FILL));
      return;
    }

    _x = constrain(x0, 0, _maxX);
    x1 = constrain(x1, 0, _maxX);
    y0 = constrain(y0, 0, _maxY);
    y1 = constrain(y1, 0, _maxY);
    y1++;
    const uint8_t height = y1 - y0;
    for (; _x <= x1; _x++) {
      _y = y0;
      _shift = y0 & 0b111;
      //Serial.println(((height + _shift) >= 8) ? "header" : "height");
      if ((height + _shift) >= 8) {
        writeData2((fillType == FILL) ? 0xFF : 0, (8 - _shift));  //write 11111111 , will shifted inside write, or 00000000 if fillType!=FILL
        _y = y0 + (8 - _shift);
        _shift = 0;
        for (; _y < y1; _y += 8) {
          //Serial.println((y1 - _y >= 8) ? "full" : "footer");
          if (y1 - _y >= 8)
            writeData2((fillType == FILL) ? 0xFF : 0);  //write 11111111 or 00000000 if fillType!=FILL
          else {
            writeData2((fillType == FILL) ? ((1 << (y1 - _y)) - 1) : 0, y1 - _y);  // write e.g 00000111 if y1-_y==3 or 00000000 if fillType!=FILL
          }
        }
      } else writeData2((fillType == FILL) ? (((1 << height) - 1) << (8 - height)) : 0, height);  // write e.g 11111000 if height==5 or 00000000 if fillType!=FILL
    }
  }

  void roundRect(int x0, int y0, int x1, int y1, FillType fillType = FILL) {
    //TODO corner cases
    if (fillType == STROKE) {
      fastLineV(x0, y0 + 2, y1 - 2);
      fastLineV(x1, y0 + 2, y1 - 2);
      fastLineH(y0, x0 + 2, x1 - 2);
      fastLineH(y1, x0 + 2, x1 - 2);
      dot(x0 + 1, y0 + 1);
      dot(x1 - 1, y0 + 1);
      dot(x1 - 1, y1 - 1);
      dot(x0 + 1, y1 - 1);
    } else {
      fastLineV(x0, y0 + 2, y1 - 2, fillType == FILL);
      fastLineV(x0 + 1, y0 + 1, y1 - 1, fillType == FILL);
      fastLineV(x1 - 1, y0 + 1, y1 - 1, fillType == FILL);
      fastLineV(x1, y0 + 2, y1 - 2, fillType == FILL);
      rect(x0 + 2, y0, x1 - 2, y1, fillType);
    }
  }

  void circle(int x, int y, int radius, FillType fillType = FILL) {
    int f = 1 - radius;
    int ddF_x = 1;
    int ddF_y = -2 * radius;
    int x1 = 0;
    int y1 = radius;

    byte fillLine = (fillType == CLEAR) ? 0 : 1;
    dot(x, y + radius, fillLine);
    dot(x, y - radius, fillLine);
    dot(x + radius, y, fillLine);
    dot(x - radius, y, fillLine);
    if (fillType != STROKE) fastLineV(x, y - radius, y + radius - 1, fillLine);
    while (x1 < y1) {
      if (f >= 0) {
        y1--;
        ddF_y += 2;
        f += ddF_y;
      }
      x1++;
      ddF_x += 2;
      f += ddF_x;
      if (fillType == STROKE) {
        dot(x + x1, y + y1);
        dot(x - x1, y + y1);
        dot(x + x1, y - y1);
        dot(x - x1, y - y1);
        dot(x + y1, y + x1);
        dot(x - y1, y + x1);
        dot(x + y1, y - x1);
        dot(x - y1, y - x1);
      } else {  // FILL / CLEAR
        fastLineV(x + x1, y - y1, y + y1, fillLine);
        fastLineV(x - x1, y - y1, y + y1, fillLine);
        fastLineV(x + y1, y - x1, y + x1, fillLine);
        fastLineV(x - y1, y - x1, y + x1, fillLine);
      }
    }
  }

  void bezier(int* arr, uint8_t size, uint8_t dense, uint8_t fill = 1) {
    int a[size * 2];
    for (int i = 0; i < (1 << dense); i++) {
      for (int j = 0; j < size * 2; j++) a[j] = arr[j];
      for (int j = (size - 1) * 2 - 1; j > 0; j -= 2)
        for (int k = 0; k <= j; k++)
          a[k] = a[k] + (((a[k + 2] - a[k]) * i) >> dense);
      dot(a[0], a[1], fill);
    }
  }

  void drawArrowHead(uint8_t x, uint8_t y, uint8_t dir = 0, uint8_t size = 3) {  //clocklike direction:0-up, 1-right, 2-down 3-left
    for (int i = 0, p = 1; i < size; i++, p += 2) {
      for (int j = 0; j < p; j++) {
        if (dir == 0 || dir == 2) dot(x - i + j, (dir == 0) ? y + i : y - i);
        else dot((dir == 3) ? x + i : x - i, y - i + j);
      }
    }
  }

  void drawBitmap2(int x, int y, const uint8_t* frame, int width, int height, bool invert = false) {
    setCursorXY(x, y);
    uint8_t div8 = height % 8;
    uint8_t rows = height / 8;
    if (div8) rows += 1;
    for (int i = 0; i < rows; i++, _y += 8) {
      if (!_inRange(y + (i * 8), 0, _maxY)) continue;
      for (int j = 0; j < width; j++) {
        if (!_inRange(x + j, 0, _maxX)) continue;
        _x = x + j;
        uint8_t data = pgm_read_byte(&(frame[i * width + j]));
        //Serial.println(String("x") + _x + " y" + _y);
        //Serial.println(data, HEX);
        if (invert) data ^= 0xFF;
        if (div8 && i == (rows - 1)) {  //last rows with not full byte
          //TODO constrain (y+div8, 0, maxY)
          writeData2(data, div8);
        } else {
          writeData2(data);
        }
      }
    }
  }

  void fill(uint8_t data) {
    memset(_oled_buffer, data, _bufSize);
  }

  //TODO
  void drawByte(uint8_t data) {
    if (++_x > _maxX) return;
    if (_shift == 0) {                             // если вывод без сдвига на строку
      writeData(data);                             // выводим
    } else {                                       // со сдвигом
      writeData(data << _shift, 0, 0, ADD);        // верхняя часть
      writeData(data >> (8 - _shift), 1, 0, ADD);  // нижняя часть со сдвигом на 1
    }
  }

  //TODO
  void drawBytes(uint8_t* data, byte size) {
    for (byte i = 0; i < size; i++) {
      if (++_x > _maxX) return;
      if (_shift == 0) {                                // если вывод без сдвига на строку
        writeData(data[i]);                             // выводим
      } else {                                          // со сдвигом
        writeData(data[i] << _shift, 0, 0, ADD);        // верхняя часть
        writeData(data[i] >> (8 - _shift), 1, 0, ADD);  // нижняя часть со сдвигом на 1
      }
    }
  }

  void update() {
    setWindow(0, 0, _maxX, _maxRow);
    beginData();
    for (int i = 0; i < 1024; i++) sendByte(_oled_buffer[i]);  //1024 - 128x64 , 512 - 64x32
    endTransm();
  }

  void update(int x0, int y0, int x1, int y1) {
    y0 >>= 3;
    y1 >>= 3;
    setWindow(x0, y0, x1, y1);
    beginData();
    for (int x = x0; x <= x1; x++)
      for (int y = y0; y <= y1; y++)
        if (x >= 0 && x <= _maxX && y >= 0 && y <= _maxRow)
          sendByte(_oled_buffer[y + x * 8]);  //8 - 128x64 , 4 - 64x32
    endTransm();
  }

  // отправить байт по i2с или в буфер
  void writeData(byte data, byte offsetY = 0, byte offsetX = 0, OVERWRITE_MODE mode = REPLACE) {
    switch (mode) {
      case ADD:
        _oled_buffer[_bufIndex(_x + offsetX, _y + offsetY)] |= data;  // добавить
        break;
      case SUB:
        _oled_buffer[_bufIndex(_x + offsetX, _y + offsetY)] &= ~data;  // вычесть
        break;
      case REPLACE:
        _oled_buffer[_bufIndex(_x + offsetX, _y + offsetY)] = data;  // заменить
        break;
    }
  }

  void writeData2(byte data, uint8_t nBits = 8) {
    _oled_buffer[_bufIndex(_x, _y)] = _firstByte(_oled_buffer[_bufIndex(_x, _y)], nBits, data);
    if (_shift + nBits > 8) _oled_buffer[_bufIndex(_x, _y + 8)] = _secondByte(_oled_buffer[_bufIndex(_x, _y + 8)], nBits, data);
  }

  uint8_t _firstByte(uint8_t firstByte, uint8_t nBits, uint8_t data) {
    if (_shift == 0 && nBits == 8) return data;
    if (_shift != 0) data <<= _shift;
    //Apply mask to firstByte (((1 << x) - 1) << (8 - x)) | ((1 << y) - 1); first x, last y
    if (_shift == 0 && nBits < 8) firstByte &= ((((1 << (8 - nBits))) - 1) << nBits);                                                            //leave first (8-nBits) bits
    else if (_shift != 0 && nBits + _shift >= 8) firstByte &= ((1 << _shift) - 1);                                                               //leave last _shift bits
    else if (_shift != 0 && nBits + _shift < 8) firstByte &= (((((1 << (8 - nBits - _shift))) - 1) << (nBits + _shift)) | ((1 << _shift) - 1));  //leave first (8-nBits-_shift) bits and last _shift bits
    return data | firstByte;
  }

  uint8_t _secondByte(uint8_t secondByte, uint8_t nBits, uint8_t data) {
    if (_shift + nBits <= 8) return secondByte;
    data >>= (byte)(8 - _shift);
    secondByte &= (byte)(((1 << (16 - _shift - nBits)) - 1) << (_shift + nBits - 8));  //leave first (16 - _shift - nBits)
    return data | secondByte;
  }

  //fix for known issue in Wire.h
  void sendByte(uint8_t data) {
    sendByteRaw(data);
    _writes++;
    if (_writes >= 16) {
      endTransm();
      beginData();
    }
  }

  void sendByteRaw(uint8_t data) {
    Wire.write(data);
  }

  void sendCommand(uint8_t cmd1) {
    beginOneCommand();
    sendByteRaw(cmd1);
    endTransm();
  }

  void sendCommand(uint8_t cmd1, uint8_t cmd2) {
    beginCommand();
    sendByteRaw(cmd1);
    sendByteRaw(cmd2);
    endTransm();
  }

  void setWindow(int x0, int y0, int x1, int y1) {
    beginCommand();
    sendByteRaw(OLED_COLUMNADDR);
    sendByteRaw(constrain(x0, 0, _maxX));
    sendByteRaw(constrain(x1, 0, _maxX));
    sendByteRaw(OLED_PAGEADDR);
    sendByteRaw(constrain(y0, 0, _maxRow));
    sendByteRaw(constrain(y1, 0, _maxRow));
    endTransm();
  }

  void beginData() {
    startTransm();
    sendByteRaw(0x40);
  }

  void beginCommand() {
    startTransm();
    sendByteRaw(0x00);
  }

  void beginOneCommand() {
    startTransm();
    sendByteRaw(0x80);
  }

  void endTransm() {
    Wire.endTransmission();
    _writes = 0;
    delayMicroseconds(2);  // https://github.com/GyverLibs/GyverOLED/issues/45
  }

  void startTransm() {
    Wire.beginTransmission(_address);
  }




  const uint8_t _address = 0x3C;
  const uint8_t _maxRow = 64 / 8 - 1;
  const uint8_t _maxY = 64 - 1;
  const uint8_t _maxX = 128 - 1;

  /*
      //For conversion from vertical bitmap to buffer use:
      for(int i = 0 ; i < 128; i++) {
        for(int j = 0 ; j < 8; j++) {
            std::cout<<"0x";
            if(epd_bitmap_pixil_frame_0[j*128+i] < 16) std::cout<<"0";
            std::cout<< std::hex << epd_bitmap_pixil_frame_0[j*128+i];
            std::cout<<", ";
        }
        std::cout<<std::endl;
    }
  */

  const int _bufSize = (128 * 64 / 8);
  uint8_t _oled_buffer[(128 * 64 / 8)];


private:

  int _bufIndex(int x, int y) {
    return (y >> 3) + (x << 3);  // x*64/8 - 128x64, x*32/8 - 128x32
  }

  void _swap(int& x, int& y) {
    int z = x;
    x = y;
    y = z;
  }
  bool _inRange(int x, int mi, int ma) {
    return x >= mi && x <= ma;
  }


  __attribute__((always_inline)) inline static GFXglyph* pgm_read_glyph_ptr(const GFXfont* gfxFont, uint8_t c) {
    return gfxFont->glyphs + c;
  }

  __attribute__((always_inline)) inline static uint8_t* pgm_read_bitmap_ptr(const GFXfont* gfxFont) {
    return gfxFont->bitmaps;
  }

  uint8_t _yAdvance = 0;
  uint8_t _firstGlyph = 0;
  uint8_t _lastGlyph = 0;
  uint8_t _charGroup = 0;
  bool _invState = 0;
  bool _println = false;
  bool _autoUpdateAfterEachLetter = false;
  uint32_t _delay = 0;
  bool _startFromNewLine = false;
  uint8_t _scale = 1;
  int _x = 0, _y = 0;
  uint8_t _shift = 0;
  uint8_t _lastChar;
  uint8_t _writes = 0;
  OVERWRITE_MODE _mode = REPLACE;

  // дин. буфер
  //int _bufsizeX, _bufsizeY;
  //int _bufX, _bufY;
  //uint8_t* _buf_ptr;
  //bool _buf_flag = false;

  GFXfont* gfxFont;
};
