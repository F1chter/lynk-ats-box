/* =====================================================================*/
/* =========== ENUMS & CONSTANTS ===========*/
/* =====================================================================*/

#define ENCODER_S1 33   //D33
#define ENCODER_S2 32   //D32
#define ENCODER_KEY 35  //D35
#define ENC_BUTTON_INV 1
#define ENCODER_TWOSTEP_TYPE true

#define ENC_DEBOUNCE_TURN 0
#define ENC_DEBOUNCE_BUTTON 80
#define ENC_HOLD_TIMEOUT 700
#define ENC_DOUBLE_TIMEOUT 300
#define ENC_FAST_TIMEOUT 200


/* =====================================================================*/
/* =========== VARIABLES ===========*/
/* =====================================================================*/
struct EncoderFlagsStruct {
  bool hold_flag : 1;
  bool butt_flag : 1;
  bool turn_flag : 1;
  bool isTurn_f : 1;
  bool isPress_f : 1;
  bool isRelease_f : 1;
  bool isReleaseHold_f : 1;
  bool isHolded_f : 1;
  bool isFastR_f : 1;
  bool isFastL_f : 1;
  bool extCLK : 1;
  bool extDT : 1;
  bool extSW : 1;
  bool isSingle_f : 1;
  bool isDouble_f : 1;
  bool countFlag : 1;
  bool doubleFlag : 1;
  bool doubleAllow : 1;
  bool rst_flag : 1;
  bool dir_flag : 1;
} encFlags;


uint8_t prevState = 0;
uint8_t encState = 0;  // 0 no rotate, 1 left, 2 right, 3 holdleft, 4 holdright
uint32_t debounce_timer = 0, fast_timer = 0;
//uint8_t _CLK = 0, _DT = 0, _SW = 0;
bool turnFlag = false, extTick = false, SW_state = false;


//int8_t counter = 0;  //CW/CCW tracking
uint8_t s1LastState = 0;
uint8_t s2LastState = 0;
uint8_t keyLastState = 0;

/* =====================================================================*/
/* =========== BEGIN ===========*/
/* =====================================================================*/

void encBegin() {
  pinMode(ENCODER_S1, INPUT);
  pinMode(ENCODER_S2, INPUT);
  pinMode(ENCODER_KEY, INPUT);

  prevState = digitalRead(ENCODER_S1) | (digitalRead(ENCODER_S2) << 1);
}

/* =====================================================================*/
/* =========== TICK ===========*/
/* =====================================================================*/

void tickEnc() {
  uint32_t thisMls = millis();
  uint32_t debounceDelta = thisMls - debounce_timer;


  if (!extTick) SW_state = digitalRead(ENCODER_KEY) ^ ENC_BUTTON_INV;  // читаем состояние кнопки SW
  else SW_state = encFlags.extSW;

  if (SW_state && !encFlags.butt_flag && (debounceDelta > ENC_DEBOUNCE_BUTTON)) {
    encFlags.butt_flag = true;
    encFlags.turn_flag = false;
    debounce_timer = thisMls;
    debounceDelta = 0;
    encFlags.isPress_f = true;
    encFlags.isHolded_f = true;
    encFlags.doubleAllow = true;
  }
  if (!SW_state && encFlags.butt_flag && (debounceDelta > ENC_DEBOUNCE_BUTTON)) {
    if (!encFlags.turn_flag && !encFlags.hold_flag) {  // если кнопка отпущена и ручка не поворачивалась
      encFlags.turn_flag = false;
      encFlags.isRelease_f = true;
    }
    if (debounceDelta > ENC_HOLD_TIMEOUT) encFlags.isReleaseHold_f = true;
    encFlags.butt_flag = false;
    debounce_timer = thisMls;
    debounceDelta = 0;
    encFlags.hold_flag = false;

    if (encFlags.doubleAllow && !encFlags.doubleFlag) {
      encFlags.doubleFlag = true;
      encFlags.countFlag = false;
    } else {
      encFlags.countFlag = true;
    }
  }
  if (encFlags.doubleFlag && debounceDelta > ENC_DOUBLE_TIMEOUT) {
    if (!encFlags.turn_flag) {
      if (!encFlags.countFlag) encFlags.isSingle_f = true;
      else encFlags.isDouble_f = true;
    }
    encFlags.doubleFlag = false;
  }
  if (encFlags.butt_flag && debounceDelta > ENC_HOLD_TIMEOUT && !encFlags.turn_flag) {
    if (SW_state) {
      encFlags.hold_flag = true;
      encFlags.doubleAllow = false;
    } else {
      encFlags.butt_flag = false;
      encFlags.hold_flag = false;
      debounce_timer = thisMls;
      debounceDelta = 0;
    }
  }




  uint8_t curState = (extTick) ? (encFlags.extCLK | (encFlags.extDT << 1)) : (digitalRead(ENCODER_S1) | (digitalRead(ENCODER_S2) << 1));

  if (curState != prevState
#if (ENC_DEBOUNCE_TURN > 0)
      && (debounceDelta > ENC_DEBOUNCE_TURN)
#endif
  ) {
    encState = 0;
    if (encFlags.rst_flag) {
      if (curState == 0b11) {
        encFlags.rst_flag = 0;
        switch (prevState) {
          case 0b10: encState = 1; break;  // 2 - 1
          case 0b01: encState = 2; break;  // 1 - 2
        }
      }
#if (ENCODER_TWOSTEP_TYPE == 0)
      else if (curState == 0b00) {
        encFlags.rst_flag = 0;
        //encState = prevState;
        switch (prevState) {
          case 0b01: encState = 1; break;
          case 0b10: encState = 2; break;
        }
      }
#endif
    }
    if (curState == 0b00
#if (ENCODER_TWOSTEP_TYPE == 0)
        || curState == 0b11
#endif
    )
      encFlags.rst_flag = 1;


    if (encState != 0) {
      encFlags.isTurn_f = true;
      if (encState <= 2 && encFlags.dir_flag) encState = 3 - encState;
      if (!SW_state && thisMls - fast_timer < ENC_FAST_TIMEOUT) {
        if (encState == 1) encFlags.isFastL_f = true;
        else if (encState == 2) encFlags.isFastR_f = true;
        fast_timer = thisMls;
      } else fast_timer = thisMls;

      if (SW_state) encState += 2;
    }
    prevState = curState;
    encFlags.turn_flag = true;
    debounce_timer = thisMls;
    debounceDelta = 0;
  }
}


/* =====================================================================*/
/* =========== IS_SOMETHING_HAPPENED ===========*/
/* =====================================================================*/

bool encIsTurn() {
  if (encFlags.isTurn_f) {
    encFlags.isTurn_f = false;
    return true;
  } else return false;
}

bool encIsRight() {
  if (encState == 2) {
    encState = 0;
    return true;
  } else return false;
}

bool encIsLeft() {
  if (encState == 1) {
    encState = 0;
    return true;
  } else return false;
}

bool encIsRightH() {
  if (encState == 4) {
    encState = 0;
    return true;
  } else return false;
}

bool encIsLeftH() {
  if (encState == 3) {
    encState = 0;
    return true;
  } else return false;
}
bool encIsFastR() {
  if (encFlags.isFastR_f) {
    encFlags.isFastR_f = false;
    return true;
  } else return false;
}
bool encIsFastL() {
  if (encFlags.isFastL_f) {
    encFlags.isFastL_f = false;
    return true;
  } else return false;
}

// кнопка
bool encIsPress() {
  if (encFlags.isPress_f) {
    encFlags.isPress_f = false;
    return true;
  } else return false;
}
bool encIsRelease() {
  if (encFlags.isRelease_f) {
    encFlags.isRelease_f = false;
    return true;
  } else return false;
}
bool encIsReleaseHold() {
  if (encFlags.isReleaseHold_f) {
    encFlags.isReleaseHold_f = false;
    return true;
  } else return false;
}
bool encIsClick() {
  return encIsRelease();
}
bool encIsHolded() {
  if (encFlags.hold_flag && encFlags.isHolded_f) {
    encFlags.isHolded_f = false;
    return true;
  } else return false;
}
bool encIsSingle() {
  if (encFlags.isSingle_f) {
    encFlags.isSingle_f = false;
    encFlags.isDouble_f = false;
    return true;
  } else return false;
}
bool encIsDouble() {
  if (encFlags.isDouble_f) {
    encFlags.isDouble_f = false;
    encFlags.isSingle_f = false;
    return true;
  } else return false;
}
bool encIsHold() {
  return (SW_state);
}

void encResetStates() {
  encState = 0;
  encFlags.isTurn_f = false;
  encFlags.isFastR_f = false;
  encFlags.isFastL_f = false;
  encFlags.isPress_f = false;
  encFlags.isRelease_f = false;
  encFlags.isReleaseHold_f = false;
  encFlags.isHolded_f = false;
  encFlags.isSingle_f = false;
  encFlags.isDouble_f = false;
}
