/*
 * Copyright 2023 Kevin Gee <info@controller.works>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
#include "analog.h"
#include "pointing_device.h"
#include "pmw3610.h"
//#include "./drivers/sensors/adns5050.h"
#include "./drivers/sensors/analog_joystick.h"
#include "transactions.h"
#include "print.h"

enum layers {
    _BASE = 0,
    _LOWER,
    _RAISE,
    _ADJUST,
    _MEDIA,
    _GAMINGL
};

enum custom_keycodes {
#ifdef VIA_ENABLE
  //QWERTY = USER00,
  BASE = QK_KB_0,
  //QWERTY = CS00, // qmk v1.1.1
#else
  QWERTTY = SAFE_RANGE,
  BASE = QK_KB_0,
#endif
  LOWER,
  RAISE,
  ADJUST,
  JOYMODE1,
  JOYMODE2,
  JOYMODE3,
  JOYMODE4,
  MCPI,
  SCPI,
  GAMING
};

enum jmodes {
    _UWASD = 0,
    _WASD,
    _MOUSE,
    _SCROLL,
    _JOYSL,
    _JOYSR,
    _JOYSZ,
    _3DROT,
    _3DPAN,
    _GAMING
};

enum cpis {
    _GEAR1 = 0,
    _GEAR2,
    _GEAR3,
    _GEAR4
};

const uint16_t CPIn[]={ PMW361_CPI_400, PMW361_CPI_800, PMW361_CPI_1600, PMW361_CPI_3200};  //pmw3610
//const uint16_t CPIn[]={ 1, 2, 3, 4};  //pmw3610
//const uint16_t CPIn[]={ 125, 500, 1000, 1375}; //adns5050

typedef union {
  uint32_t raw;
  struct {
    uint8_t jMode1;
    uint8_t jMode2;
    uint8_t jMode3;
    uint8_t jMode4;
    uint8_t nMCPI;
    uint8_t nSCPI;
  };
} user_config_t;

user_config_t user_config;


typedef struct _slave_to_master_t {
    int jx1;
    int jy1;
    int jx2;
    int jy2;
} slave_to_master_t;

slave_to_master_t s2m;

uint8_t jMode1=0;
uint8_t jMode2=0;
uint8_t jMode3=0;
uint8_t jMode4=0;
uint8_t nMCPI;
uint8_t nSCPI;

bool f_oledchange = false;

bool f_JMOUS = false;
bool f_JOYSL = false;
bool f_JOYSR = false;
bool f_JOYSZ = false;
bool f_3DROT = false;
bool f_3DPAN = false;
bool f_GAMING = false;



void joystick_mode_fckeck(uint8_t jmode){
  if(jmode == _JOYSL){
    f_JOYSL = false;
  }else if(jmode == _JOYSR){
    f_JOYSR = false;
  }else if(jmode == _JOYSZ){
    f_JOYSZ = false;
  }else if(jmode == _3DROT){
    f_3DROT = false;
  }else if(jmode == _3DPAN){
    f_3DPAN = false;
  }else if(jmode == _MOUSE){
    f_JMOUS = false;
  }
}

uint8_t joystick_mode_INC(uint8_t jmode){
  uint8_t newjmode;
  newjmode = jmode;

  joystick_mode_fckeck(jmode); // clear flag
  newjmode++;
  if(newjmode == _MOUSE){if(f_JMOUS){newjmode++;}else{f_JMOUS = true;}}
  if(newjmode == _JOYSL){if(f_JOYSL){newjmode++;}else{f_JOYSL = true;}}
  if(newjmode == _JOYSR){if(f_JOYSR){newjmode++;}else{f_JOYSR = true;}}
  if(newjmode == _JOYSZ){if(f_JOYSZ){newjmode++;}else{f_JOYSZ = true;}}
  if(newjmode == _3DROT){if(f_3DROT){newjmode++;}else{f_3DROT = true;}}
  if(newjmode == _3DPAN){if(f_3DPAN){newjmode++;}else{f_3DPAN = true;}}
  if(newjmode > _3DPAN){newjmode = _UWASD;}
  return newjmode;
}

#if 1
void joysitck_mode1_INC(void) {

  if(jMode1 != _GAMING){
    jMode1 = joystick_mode_INC(jMode1);
  }

}
void joysitck_mode2_INC(void) {
  if(jMode2 != _GAMING){
    jMode2 = joystick_mode_INC(jMode2);
  }
  
}
void joysitck_mode3_INC(void) {
  if(jMode3 != _GAMING){
    jMode3 = joystick_mode_INC(jMode3);
  }
}
void joysitck_mode4_INC(void) {
  if(jMode4 != _GAMING){
    jMode4 = joystick_mode_INC(jMode4);
  }
}

#else
void joysitck_mode1_INC(void) {

  if(jMode1 != _GAMING){

    if(jMode1 == _3DPAN) {
      jMode1 = _UWASD;
    }else{
      jMode1++;
    }

    if(jMode1 == jMode2 && jMode1 >= _WASD){

      if(jMode1 ==_3DPAN){
        jMode1 = _UWASD;
      }else{
        jMode1++;
      }
    }
  
  }
}

void joysitck_mode2_INC(void) {

  if(jMode1 != _GAMING){

    if(jMode2 == _3DPAN) {
      jMode2 = _UWASD;
    }else{
      jMode2++;
    }

    if(jMode2 == jMode1 && jMode2 >= _WASD){
      if(jMode2 ==_3DPAN){
        jMode2 = _UWASD;
      }else{
        jMode2++;
      }    
    }

  }
}

void joysitck_mode3_INC(void) {

  if(jMode1 != _GAMING){

    if(jMode3 == _3DPAN) {
      jMode3 = _UWASD;
    }else{
      jMode3++;
    }

    if(jMode3 == jMode1 && jMode3 >= _WASD){
      if(jMode3 ==_3DPAN){
        jMode3 = _UWASD;
      }else{
        jMode2++;
      }    
    }

  }
}

void joysitck_mode4_INC(void) {

  if(jMode1 != _GAMING){

    if(jMode4 == _3DPAN) {
      jMode4 = _UWASD;
    }else{
      jMode4++;
    }

    if(jMode4 == jMode1 && jMode4>= _WASD){
      if(jMode4 ==_3DPAN){
        jMode4 = _UWASD;
      }else{
        jMode4++;
      }    
    }

  }
}
#endif

//void ADNS5050_CPI_INC(void){
void PMW3610_MCPI_INC(void){
  //uint16_t cpi_status;

  if(nMCPI == 3) {
    nMCPI = 0;
  }else{
    nMCPI++;
  }
}

void PMW3610_SCPI_INC(void){
  //uint16_t cpi_status;

  if(nSCPI == 3) {
    nSCPI = 0;
  }else{
    nSCPI++;
  }
}


void showreceiver(void){

    uprintf("receiver jx1 = %d\n",s2m.jx1);
    uprintf("receiver jy1 = %d\n",s2m.jy1);
    uprintf("receiver jx2 = %d\n",s2m.jx2);
    uprintf("receiver jy2 = %d\n",s2m.jy2);

}


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_BASE] = LAYOUT_vertical_5x6(
  //,-----------------------------------------------------.
        C(KC_Y),C(KC_Z),GAMING, MCPI,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,MS_BTN1,KC_ENT, KC_ESC, KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        MS_BTN3,MS_BTN2,KC_BSPC,C(KC_S),KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_C),C(KC_V),C(KC_X),KC_L,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,KC_TRNS,KC_BSPC,KC_TAB, MS_WHLU,MS_WHLD,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_Y),C(KC_Z),GAMING, SCPI,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,MS_BTN1,KC_ENT, KC_ESC, KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        MS_BTN3,MS_BTN2,KC_BSPC,C(KC_S),KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_C),C(KC_V),C(KC_X),KC_L,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,KC_TRNS,KC_BSPC,KC_TAB, MS_WHLU,MS_WHLD
  //|--------+--------+--------+--------+--------+--------|
  ),

  [_LOWER] = LAYOUT_vertical_5x6(
  //,-----------------------------------------------------.
        C(KC_Y),C(KC_Z),GAMING, KC_3,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,MS_BTN1,KC_ENT, KC_ESC, KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        MS_BTN3,MS_BTN2,KC_BSPC,C(KC_S),KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_C),C(KC_V),C(KC_X),KC_L,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,KC_TRNS,KC_BSPC,KC_TAB, MS_WHLU,MS_WHLD,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_Y),C(KC_Z),GAMING, KC_3,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,MS_BTN1,KC_ENT, KC_ESC, KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        MS_BTN3,MS_BTN2,KC_BSPC,C(KC_S),KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_C),C(KC_V),C(KC_X),KC_L,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,KC_TRNS,KC_BSPC,KC_TAB, MS_WHLU,MS_WHLD
  //|--------+--------+--------+--------+--------+--------|
  ),

  [_RAISE] = LAYOUT_vertical_5x6(
  //,-----------------------------------------------------.
        C(KC_Y),C(KC_Z),GAMING, KC_3,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,MS_BTN1,KC_ENT, KC_ESC, KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        MS_BTN3,MS_BTN2,KC_BSPC,C(KC_S),KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_C),C(KC_V),C(KC_X),KC_L,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,KC_TRNS,KC_BSPC,KC_TAB, MS_WHLU,MS_WHLD,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_Y),C(KC_Z),GAMING, KC_3,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,MS_BTN1,KC_ENT, KC_ESC, KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        MS_BTN3,MS_BTN2,KC_BSPC,C(KC_S),KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_C),C(KC_V),C(KC_X),KC_L,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,KC_TRNS,KC_BSPC,KC_TAB, MS_WHLU,MS_WHLD
  //|--------+--------+--------+--------+--------+--------|
  ),

  [_ADJUST] = LAYOUT_vertical_5x6(
  //,-----------------------------------------------------.
        C(KC_Y),C(KC_Z),GAMING, KC_3,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,MS_BTN1,KC_ENT, KC_ESC, KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        MS_BTN3,MS_BTN2,KC_BSPC,C(KC_S),KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_C),C(KC_V),C(KC_X),KC_L,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,KC_TRNS,KC_BSPC,KC_TAB, MS_WHLU,MS_WHLD,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_Y),C(KC_Z),GAMING, KC_3,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,MS_BTN1,KC_ENT, KC_ESC, KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        MS_BTN3,MS_BTN2,KC_BSPC,C(KC_S),KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_C),C(KC_V),C(KC_X),KC_L,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,KC_TRNS,KC_BSPC,KC_TAB, MS_WHLU,MS_WHLD
  //|--------+--------+--------+--------+--------+--------|
  ),

  [_MEDIA] = LAYOUT_vertical_5x6(
  //,-----------------------------------------------------.
        C(KC_Y),C(KC_Z),GAMING, KC_3,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,MS_BTN1,KC_ENT, KC_ESC, KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        MS_BTN3,MS_BTN2,KC_BSPC,C(KC_S),KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_C),C(KC_V),C(KC_X),KC_L,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,KC_TRNS,KC_BSPC,KC_TAB, MS_WHLU,MS_WHLD,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_Y),C(KC_Z),GAMING, KC_3,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,MS_BTN1,KC_ENT, KC_ESC, KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        MS_BTN3,MS_BTN2,KC_BSPC,C(KC_S),KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        C(KC_C),C(KC_V),C(KC_X),KC_L,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,KC_TRNS,KC_BSPC,KC_TAB, MS_WHLU,MS_WHLD
  //|--------+--------+--------+--------+--------+--------|
  ),

    [_GAMINGL] = LAYOUT_vertical_5x6(
  //,-----------------------------------------------------.
        JS_0,   JS_1,    GAMING, JS_0,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
        KC_TRNS,JS_3,    JS_4,   JS_5,   KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        JS_6,   JS_7,    JS_8,   JS_9,   KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        JS_10,  JS_11,   JS_12,  JS_13,  KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
      KC_TRNS,KC_TRNS,   JS_14,  JS_15,  MS_WHLU,MS_WHLD,
  //|--------+--------+--------+--------+--------+--------|
        JS_16,  JS_17,  GAMING,   JS_16,   KC_P8,   KC_UP,
  //|--------+--------+--------+--------+--------+--------|
      KC_TRNS,  JS_19,   JS_20,   JS_21,   KC_P4,   KC_LEFT,
  //|--------+--------+--------+--------+--------+--------|
        JS_22,  JS_23,   JS_24,   JS_25,   KC_P6,   KC_RIGHT,
  //|--------+--------+--------+--------+--------+--------|
        JS_26,  JS_27,   JS_28,   JS_29,   KC_P2,   KC_DOWN,
  //|--------+--------+--------+--------+--------+--------|
      KC_TRNS,KC_TRNS,   JS_30,   JS_31,MS_WHLU,MS_WHLD
  //|--------+--------+--------+--------+--------+--------|
  ),
};

#if 1
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case BASE:
      if (record->event.pressed) {
         print("mode just switched to qwerty and this is a huge string\n");
        set_single_persistent_default_layer(_BASE);
      }
      return false;
      //break;
    case LOWER:
      if (record->event.pressed) {
        layer_on(_LOWER);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      } else {
        layer_off(_LOWER);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      }
      f_oledchange = true;
      return false;
      //break;
    case RAISE:
      if (record->event.pressed) {
        layer_on(_RAISE);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      } else {
        layer_off(_RAISE);
        update_tri_layer(_LOWER, _RAISE, _ADJUST);
      }
      f_oledchange = true;
      return false;
      //break;
    case ADJUST:
      if (record->event.pressed) {
        layer_on(_ADJUST);
      } else {
        layer_off(_ADJUST);
      }
      f_oledchange = true;
      return false;
      //break;

    case JOYMODE1:
      if (record->event.pressed) {
        joysitck_mode1_INC();
        user_config.jMode1 = jMode1;
        eeconfig_update_user(user_config.raw); 
      }
      f_oledchange = true;
      clear_keyboard();
      return false;

    case JOYMODE2:
      if (record->event.pressed) {
        joysitck_mode2_INC();
        user_config.jMode2 = jMode2;
        eeconfig_update_user(user_config.raw); 
      }
      f_oledchange = true;
      clear_keyboard();
      return false;
    
    case JOYMODE3:
      if (record->event.pressed) {
        joysitck_mode3_INC();
        user_config.jMode3 = jMode3;
        eeconfig_update_user(user_config.raw); 
      }
      f_oledchange = true;
      clear_keyboard();
      return false;

    case JOYMODE4:
      if (record->event.pressed) {
        joysitck_mode4_INC();
        user_config.jMode4 = jMode4;
        eeconfig_update_user(user_config.raw); 
      }
      f_oledchange = true;
      clear_keyboard();
      return false;

    case MCPI:
      if (record->event.pressed) {
        PMW3610_MCPI_INC();
        //ADNS5050_CPI_INC();
        user_config.nMCPI = nMCPI;
        eeconfig_update_user(user_config.raw);
        pointing_device_driver_set_cpi(CPIn[nMCPI]);
        //pointing_device_driver_get_cpi();
        showreceiver();
      }
      f_oledchange = true;
      return false;

    case SCPI:
      if (record->event.pressed) {
        PMW3610_SCPI_INC();
        //ADNS5050_CPI_INC();
        user_config.nSCPI = nSCPI;
        eeconfig_update_user(user_config.raw);
        //pointing_device_driver_set_cpi(CPIn[nSCPI]);
        pointing_device_set_cpi_on_side(false, CPIn[nSCPI]); //right side cpi
        //pointing_device_driver_get_cpi();
      }
      f_oledchange = true;
      return false;

    case GAMING:
      if (record->event.pressed) {

        if(f_GAMING){
          f_GAMING = false;
          layer_off(_GAMINGL);
          jMode1 = user_config.jMode1;
          jMode2 = user_config.jMode2;
          jMode3 = user_config.jMode3;
          jMode4 = user_config.jMode4;
        }else{
          f_GAMING = true;
          layer_on(_GAMINGL);
          jMode1 = _GAMING;
          jMode2 = _UWASD;
          jMode3 = _GAMING;
          jMode4 = _UWASD;
        }

      }
      f_oledchange = true;
      clear_keyboard();
      return false;
  }
  return true;
}
#endif

#ifdef JOYSTICK_ENABLE

int16_t xPos = 0;
int16_t yPos = 0;
int16_t xPos2 = 0;
int16_t yPos2 = 0;
int16_t xPos3 = 0;
int16_t yPos3 = 0;
int16_t xPos4 = 0;
int16_t yPos4 = 0;

bool wasdShiftMode = false;
//bool autorun = false;

bool yDownHeld = false;
bool yUpHeld = false;
bool xLeftHeld = false;
bool xRightHeld = false;
//bool shiftHeld = false;

bool yDownHeld2 = false;
bool yUpHeld2 = false;
bool xLeftHeld2 = false;
bool xRightHeld2 = false;
//bool shiftHeld = false;

bool yDownHeld3 = false;
bool yUpHeld3 = false;
bool xLeftHeld3 = false;
bool xRightHeld3 = false;


bool yDownHeld4 = false;
bool yUpHeld4 = false;
bool xLeftHeld4 = false;
bool xRightHeld4 = false;


#if 1
uint8_t maxCursorSpeed2 = ANALOG_JOYSTICK_SPEED_MAX;
uint8_t speedRegulator2 = ANALOG_JOYSTICK_SPEED_REGULATOR; // Lower Values Create Faster Movement

int16_t xOrigin, yOrigin;
int16_t xOrigin2, yOrigin2;
int16_t xOrigin3, yOrigin3;
int16_t xOrigin4, yOrigin4;

int16_t maxisCoordinate(pin_t pin, uint16_t origin) {

    int8_t  direction;
    int16_t distanceFromOrigin;
    int16_t range;

    int16_t position = analogReadPin(pin);

    if (origin == position) {
        return 0;
    } else if (origin > position) {
        distanceFromOrigin = origin - position;
        range              = origin - _MIN;
        direction          = -1;
    } else {
        distanceFromOrigin = position - origin;
        range              = _MAX - origin;
        direction          = 1;
    }

    //return distanceFromOrigin * direction;

    //_J_REP_MAX

    float   percent    = (float)distanceFromOrigin / range;
    if(percent >1)percent =1;

    int16_t coordinate = (int16_t)(percent * _J_REP_MAX);

    return coordinate * direction;
#if 0
    int16_t coordinate = (int16_t)(percent * _CENTER);
    
    if (coordinate < 0) {
        return 0;
    } else if (coordinate > _CENTER) {
        return _CENTER * direction;
    } else {
        return coordinate * direction;
    }
#endif
}


int16_t saxisCoordinate(int16_t position, uint16_t origin) {

    int8_t  direction;
    int16_t distanceFromOrigin;
    int16_t range;

    //int16_t position = analogReadPin(pin);

    if (origin == position) {
        return 0;
    } else if (origin > position) {
        distanceFromOrigin = origin - position;
        range              = origin - _MIN;
        direction          = -1;
    } else {
        distanceFromOrigin = position - origin;
        range              = _MAX - origin;
        direction          = 1;
    }


    float   percent    = (float)distanceFromOrigin / range;
    if(percent >1)percent =1;

    int16_t coordinate = (int16_t)(percent * _J_REP_MAX);

    return coordinate * direction;

}



int16_t axisCoordinate2(pin_t pin, uint16_t origin) {
    int8_t  direction;
    int16_t distanceFromOrigin;
    int16_t range;

    int16_t position = analogReadPin(pin);

    if (origin == position) {
        return 0;
    } else if (origin > position) {
        distanceFromOrigin = origin - position;
        range = origin - ANALOG_JOYSTICK_AXIS_MIN;
        direction = -1;
    } else {
        distanceFromOrigin = position - origin;
        range = ANALOG_JOYSTICK_AXIS_MAX - origin;
        direction = 1;
    }

    float   percent    = (float)distanceFromOrigin / range;
    int16_t coordinate = (int16_t)(percent * 100);
    if (coordinate < 0) {
        return 0;
    } else if (coordinate > 100) {
        return 100 * direction;
    } else {
        return coordinate * direction;
    }
}

int8_t axisToMouseComponent2(pin_t pin, int16_t origin, uint8_t maxSpeed) {
    int16_t coordinate = axisCoordinate2(pin, origin);
    int8_t  result;
    if (coordinate != 0) {
        float percent = (float)coordinate / 100;
        result        = percent * maxCursorSpeed2 * (abs(coordinate) / speedRegulator2);
    } else {
        return 0;
    }
    return result;
}

int16_t axisCoordinate3(int16_t position, uint16_t origin) {
    int8_t  direction;
    int16_t distanceFromOrigin;
    int16_t range;

    //int16_t position = analogReadPin(pin);

    if (origin == position) {
        return 0;
    } else if (origin > position) {
        distanceFromOrigin = origin - position;
        range = origin - ANALOG_JOYSTICK_AXIS_MIN;
        direction = -1;
    } else {
        distanceFromOrigin = position - origin;
        range = ANALOG_JOYSTICK_AXIS_MAX - origin;
        direction = 1;
    }

    float   percent    = (float)distanceFromOrigin / range;
    int16_t coordinate = (int16_t)(percent * 100);
    if (coordinate < 0) {
        return 0;
    } else if (coordinate > 100) {
        return 100 * direction;
    } else {
        return coordinate * direction;
    }
}

int8_t axisToMouseComponent3(int16_t position, int16_t origin, uint8_t maxSpeed) {
    int16_t coordinate = axisCoordinate3(position, origin);
    int8_t  result;
    if (coordinate != 0) {
        float percent = (float)coordinate / 100;
        result        = percent * maxCursorSpeed2 * (abs(coordinate) / speedRegulator2);
    } else {
        return 0;
    }
    return result;
}


uint16_t lastCursor2 = 0;
uint16_t lastCursor3 = 0;
uint16_t lastCursor4 = 0;
report_analog_joystick_t analog_joystick2_read(void) {
    report_analog_joystick_t report = {0};

    if (timer_elapsed(lastCursor2) > ANALOG_JOYSTICK_READ_INTERVAL) {
        lastCursor2 = timer_read();
        report.x   = axisToMouseComponent2(JSH2, xOrigin2, maxCursorSpeed2);
        report.y   = axisToMouseComponent2(JSV2, yOrigin2, maxCursorSpeed2);
    }
    return report;
}
report_analog_joystick_t analog_joystick3_read(void) {
    report_analog_joystick_t report = {0};

    if (timer_elapsed(lastCursor3) > ANALOG_JOYSTICK_READ_INTERVAL) {
        lastCursor3 = timer_read();
        report.x   = axisToMouseComponent3(s2m.jy1, xOrigin3, maxCursorSpeed2);
        report.y   = axisToMouseComponent3(s2m.jx1, yOrigin3, maxCursorSpeed2);
    }
    return report;
}
report_analog_joystick_t analog_joystick4_read(void) {
    report_analog_joystick_t report = {0};

    if (timer_elapsed(lastCursor4) > ANALOG_JOYSTICK_READ_INTERVAL) {
        lastCursor4 = timer_read();
        report.x   = axisToMouseComponent3(s2m.jy2, xOrigin4, maxCursorSpeed2);
        report.y   = axisToMouseComponent3(s2m.jx2, yOrigin4, maxCursorSpeed2);
    }
    return report;
}

#endif

void matrix_scan_user(void) {
    
    int16_t jy;
    int16_t jx;
    int16_t jy2;
    int16_t jx2;

        //-----------------------WASD mode---------------------------
    if (jMode1 == _UWASD) {
      // W & S
      //if (!autorun) {
      yPos = analogReadPin(JSV);      
      //yPos = maxisCoordinate(JSV, yOrigin);
      if (!yDownHeld && yPos >= _DOWN_TRESHOLD) {
        //register_code(KC_H);
        register_code(dynamic_keymap_get_keycode(biton32(layer_state),1,5));
        yDownHeld = true;
      } else if (yDownHeld && yPos < _DOWN_TRESHOLD) {
        yDownHeld = false;
        //unregister_code(KC_H);
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),1,5));
      } else if (!yUpHeld && yPos <= _UP_TRESHOLD) {
        yUpHeld = true;
        //register_code(KC_F);
        register_code(dynamic_keymap_get_keycode(biton32(layer_state),2,5));
      } else if (yUpHeld && yPos > _UP_TRESHOLD) {
        yUpHeld = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),2,5));
        //unregister_code(KC_F);
      }
      //}
      
      xPos = analogReadPin(JSH);
      //xPos = maxisCoordinate(JSH, xOrigin);
      if (!xLeftHeld && xPos >= _DOWN_TRESHOLD) {
        //register_code(KC_T);
        register_code(dynamic_keymap_get_keycode(biton32(layer_state),3,5));
        xLeftHeld = true;
      } else if (xLeftHeld && xPos < _DOWN_TRESHOLD) {
        xLeftHeld = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),3,5));
        //unregister_code(KC_T);
      } else if (!xRightHeld && xPos <= _UP_TRESHOLD) {
        xRightHeld = true;
        register_code(dynamic_keymap_get_keycode(biton32(layer_state),0,5));
        //register_code(KC_G);
      } else if (xRightHeld && xPos > _UP_TRESHOLD) {
        xRightHeld = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),0,5));
        //unregister_code(KC_G);
      }

    //-----------------------WASD mode---------------------------
    }else if(jMode1 == _WASD) {
      // W & S
      //if (!autorun) {
      yPos = analogReadPin(JSV);      
      //yPos = maxisCoordinate(JSV, yOrigin);
      if (!yDownHeld && yPos >= _DOWN_TRESHOLD) {
        register_code(KC_A);
        yDownHeld = true;
      } else if (yDownHeld && yPos < _DOWN_TRESHOLD) {
        yDownHeld = false;
        unregister_code(KC_A);
      } else if (!yUpHeld && yPos <= _UP_TRESHOLD) {
        yUpHeld = true;
        register_code(KC_D);
      } else if (yUpHeld && yPos > _UP_TRESHOLD) {
        yUpHeld = false;
        unregister_code(KC_D);
      }
      //}
      
      xPos = analogReadPin(JSH);
      //xPos = maxisCoordinate(JSH, xOrigin);
      if (!xLeftHeld && xPos >= _DOWN_TRESHOLD) {
        register_code(KC_S);
        xLeftHeld = true;
      } else if (xLeftHeld && xPos < _DOWN_TRESHOLD) {
        xLeftHeld = false;
        unregister_code(KC_S);
      } else if (!xRightHeld && xPos <= _UP_TRESHOLD) {
        xRightHeld = true;
        register_code(KC_W);
      } else if (xRightHeld && xPos > _UP_TRESHOLD) {
        xRightHeld = false;
        unregister_code(KC_W);
      }
#if 0
      if (wasdShiftMode) {
        bool yShifted = yPos < _SHIFT;
        if (!shiftHeld && yShifted) {
          register_code(KC_LSFT);
          shiftHeld = true;
        } else if (shiftHeld && !yShifted) {
          unregister_code(KC_LSFT);
          shiftHeld = false;
        }
      }
#endif
    //---------------------joystick mode----------------------------
    //}else if(jMode == _JOYS){
    }else if(jMode1 == _JOYSL){

      jy = maxisCoordinate(JSH, xOrigin);
      jx = maxisCoordinate(JSV, yOrigin);
      
      joystick_set_axis( 0, -jx);
      joystick_set_axis( 1, jy);

#if 0

      jx2 = saxisCoordinate(s2m.jx1, xOrigin3);
      jy2 = saxisCoordinate(s2m.jy1, yOrigin3);
      joystick_set_axis( 2, -jx2);
      joystick_set_axis( 3, jy2);
      uprintf("receiver jx3 joy3x = %d\n",jx2);
      uprintf("receiver jy3 joy3y = %d\n",jy2);

#endif

    }else if(jMode1 == _JOYSR){
      jy = maxisCoordinate(JSH, xOrigin);
      jx = maxisCoordinate(JSV, yOrigin);
      joystick_set_axis( 2, -jx);
      joystick_set_axis( 3, jy);
    }else if(jMode1 == _JOYSZ){
      jy = maxisCoordinate(JSH, xOrigin);
      jx = maxisCoordinate(JSV, yOrigin);
      joystick_set_axis( 4, -jx);
      joystick_set_axis( 5, jy); 

    //----------------------mouse mode------------------------------
    }else if(jMode1 == _MOUSE){


    //----------------------scroll mode-----------------------------
    }else if(jMode1 == _SCROLL){
      yPos = analogReadPin(JSV);
      if (!yDownHeld && yPos >= _DOWN_TRESHOLD) {
        register_code(KC_WH_L);
        yDownHeld = true;
      } else if (yDownHeld && yPos < _DOWN_TRESHOLD) {
        yDownHeld = false;
        unregister_code(KC_WH_L);
      } else if (!yUpHeld && yPos <= _UP_TRESHOLD) {
        yUpHeld = true;
        register_code(KC_WH_R);
      } else if (yUpHeld && yPos > _UP_TRESHOLD) {
        yUpHeld = false;
        unregister_code(KC_WH_R);
      }
      //}
      
      xPos = analogReadPin(JSH);
      if (!xLeftHeld && xPos >= _DOWN_TRESHOLD) {
        register_code(KC_WH_D);
        xLeftHeld = true;
      } else if (xLeftHeld && xPos < _DOWN_TRESHOLD) {
        xLeftHeld = false;
        unregister_code(KC_WH_D);
      } else if (!xRightHeld && xPos <= _UP_TRESHOLD) {
        xRightHeld = true;
        register_code(KC_WH_U);
      } else if (xRightHeld && xPos > _UP_TRESHOLD) {
        xRightHeld = false;
        unregister_code(KC_WH_U);
      }
    //-----------------------gaming mode---------------------------
    }else if(jMode1 == _GAMING){
#if 1
      jy = maxisCoordinate(JSH, xOrigin);
      jx = maxisCoordinate(JSV, yOrigin);
      
      joystick_set_axis( 0, -jx);
      joystick_set_axis( 1, jy);

      jx2 = saxisCoordinate(s2m.jx1, xOrigin3);
      jy2 = saxisCoordinate(s2m.jy1, yOrigin3);
      joystick_set_axis( 2, -jx2);
      joystick_set_axis( 3, jy2);
      uprintf("receiver jx3 joy3x = %d\n",jx2);
      uprintf("receiver jy3 joy3y = %d\n",jy2);
    }
#endif
/*-------------------------jmod2--------------------------------*/
#if 1
        //-----------------------WASD mode---------------------------

      if (jMode2 == _UWASD) {
      // W & S
      //if (!autorun) {
      yPos2 = analogReadPin(JSV2);      
      //yPos = maxisCoordinate(JSV, yOrigin);
      if (!yDownHeld2 && yPos2 >= _DOWN_TRESHOLD) {
        //register_code(KC_H);
        register_code(dynamic_keymap_get_keycode(biton32(layer_state),2,4));
        yDownHeld2 = true;
      } else if (yDownHeld2 && yPos2 < _DOWN_TRESHOLD) {
        yDownHeld2 = false;
        //unregister_code(KC_H);
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),2,4));
      } else if (!yUpHeld2 && yPos2 <= _UP_TRESHOLD) {
        yUpHeld2 = true;
        //register_code(KC_F);
        register_code(dynamic_keymap_get_keycode(biton32(layer_state),1,4));
      } else if (yUpHeld2 && yPos2 > _UP_TRESHOLD) {
        yUpHeld2 = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),1,4));
        //unregister_code(KC_F);
      }
      //}
      
      xPos2 = analogReadPin(JSH2);
      //xPos = maxisCoordinate(JSH, xOrigin);
      if (!xLeftHeld2 && xPos2 >= _DOWN_TRESHOLD) {
        //register_code(KC_T);
        register_code(dynamic_keymap_get_keycode(biton32(layer_state),0,4));
        xLeftHeld2 = true;
      } else if (xLeftHeld2 && xPos2 < _DOWN_TRESHOLD) {
        xLeftHeld2 = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),0,4));
        //unregister_code(KC_T);
      } else if (!xRightHeld2 && xPos2 <= _UP_TRESHOLD) {
        xRightHeld2 = true;
        register_code(dynamic_keymap_get_keycode(biton32(layer_state),3,4));
        //register_code(KC_G);
      } else if (xRightHeld2 && xPos2 > _UP_TRESHOLD) {
        xRightHeld2 = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),3,4));
        //unregister_code(KC_G);
      }
    //-----------------------WASD mode---------------------------
    }else if(jMode2 == _WASD) {
      // W & S
      //if (!autorun) {
      yPos2 = analogReadPin(JSV2);      
      //yPos = maxisCoordinate(JSV, yOrigin);
      if (!yDownHeld2 && yPos2 >= _DOWN_TRESHOLD) {
        register_code(KC_D);
        yDownHeld2 = true;
      } else if (yDownHeld2 && yPos2 < _DOWN_TRESHOLD) {
        yDownHeld2 = false;
        unregister_code(KC_D);
      } else if (!yUpHeld2 && yPos2 <= _UP_TRESHOLD) {
        yUpHeld2 = true;
        register_code(KC_A);
      } else if (yUpHeld2 && yPos2 > _UP_TRESHOLD) {
        yUpHeld2 = false;
        unregister_code(KC_A);
      }
      //}
      
      xPos2 = analogReadPin(JSH2);
      //xPos = maxisCoordinate(JSH, xOrigin);
      if (!xLeftHeld2 && xPos2 >= _DOWN_TRESHOLD) {
        register_code(KC_W);
        xLeftHeld2 = true;
      } else if (xLeftHeld2 && xPos2 < _DOWN_TRESHOLD) {
        xLeftHeld2 = false;
        unregister_code(KC_W);
      } else if (!xRightHeld2 && xPos2 <= _UP_TRESHOLD) {
        xRightHeld2 = true;
        register_code(KC_S);
      } else if (xRightHeld2 && xPos2 > _UP_TRESHOLD) {
        xRightHeld2 = false;
        unregister_code(KC_S);
      }
#if 0
      if (wasdShiftMode) {
        bool yShifted = yPos < _SHIFT;
        if (!shiftHeld && yShifted) {
          register_code(KC_LSFT);
          shiftHeld = true;
        } else if (shiftHeld && !yShifted) {
          unregister_code(KC_LSFT);
          shiftHeld = false;
        }
      }
#endif
    //---------------------joystick mode----------------------------
    }else if(jMode2 == _JOYSL){
      jy = maxisCoordinate(JSH2, xOrigin2);
      jx = maxisCoordinate(JSV2, yOrigin2);
      joystick_set_axis( 0, jx);
      joystick_set_axis( 1, -jy);
    }else if(jMode2 == _JOYSR){
      jy = maxisCoordinate(JSH2, xOrigin2);
      jx = maxisCoordinate(JSV2, yOrigin2);
      joystick_set_axis( 2, jx);
      joystick_set_axis( 3, -jy);
    }else if(jMode2 == _JOYSZ){
      jy = maxisCoordinate(JSH2, xOrigin2);
      jx = maxisCoordinate(JSV2, yOrigin2);
      joystick_set_axis( 4, jx);
      joystick_set_axis( 5, -jy); 

    //----------------------mouse mode------------------------------
    }else if(jMode2 == _MOUSE){


    //----------------------scroll mode-----------------------------
    }else if(jMode2 == _SCROLL){
      yPos2 = analogReadPin(JSV2);
      if (!yDownHeld2 && yPos2 >= _DOWN_TRESHOLD) {
        register_code(KC_WH_R);
        yDownHeld2 = true;
      } else if (yDownHeld2 && yPos2 < _DOWN_TRESHOLD) {
        yDownHeld2 = false;
        unregister_code(KC_WH_R);
      } else if (!yUpHeld2 && yPos2 <= _UP_TRESHOLD) {
        yUpHeld2 = true;
        register_code(KC_WH_L);
      } else if (yUpHeld2 && yPos2 > _UP_TRESHOLD) {
        yUpHeld2 = false;
        unregister_code(KC_WH_L);
      }
      //}
      
      xPos2 = analogReadPin(JSH2);
      if (!xLeftHeld2 && xPos2 >= _DOWN_TRESHOLD) {
        register_code(KC_WH_U);
        xLeftHeld2 = true;
      } else if (xLeftHeld2 && xPos2 < _DOWN_TRESHOLD) {
        xLeftHeld2 = false;
        unregister_code(KC_WH_U);
      } else if (!xRightHeld2 && xPos2 <= _UP_TRESHOLD) {
        xRightHeld2 = true;
        register_code(KC_WH_D);
      } else if (xRightHeld2 && xPos2 > _UP_TRESHOLD) {
        xRightHeld2 = false;
        unregister_code(KC_WH_D);
      }
    }
#endif     /*------jmod2 end---------*/

/*-------------------------jmod3--------------------------------*/
#if 1

        //-----------------------WASD mode---------------------------
    if (jMode3 == _UWASD) {

      //yPos = analogReadPin(JSV);
      yPos3 = s2m.jx1;

      if (!yDownHeld3 && yPos3 >= _DOWN_TRESHOLD) {

        register_code(dynamic_keymap_get_keycode(biton32(layer_state),6,5));
        yDownHeld3 = true;
      } else if (yDownHeld3 && yPos3 < _DOWN_TRESHOLD) {
        yDownHeld3 = false;

        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),6,5));
      } else if (!yUpHeld3 && yPos3 <= _UP_TRESHOLD) {
        yUpHeld3 = true;

        register_code(dynamic_keymap_get_keycode(biton32(layer_state),7,5));
      } else if (yUpHeld3 && yPos3 > _UP_TRESHOLD) {
        yUpHeld3 = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),7,5));
      }

      
      //xPos = analogReadPin(JSH);
      xPos3 = s2m.jy1;

      if (!xLeftHeld3 && xPos3 >= _DOWN_TRESHOLD) {

        register_code(dynamic_keymap_get_keycode(biton32(layer_state),8,5));
        xLeftHeld3 = true;
      } else if (xLeftHeld3 && xPos3 < _DOWN_TRESHOLD) {
        xLeftHeld3 = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),8,5));

      } else if (!xRightHeld3 && xPos3 <= _UP_TRESHOLD) {
        xRightHeld3 = true;
        register_code(dynamic_keymap_get_keycode(biton32(layer_state),5,5));

      } else if (xRightHeld3 && xPos3 > _UP_TRESHOLD) {
        xRightHeld3 = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),5,5));
      }

    //-----------------------WASD mode---------------------------
    }else if(jMode3 == _WASD) {

      //yPos = analogReadPin(JSV);
      yPos3 = s2m.jx1;

      if (!yDownHeld3 && yPos3 >= _DOWN_TRESHOLD) {
        register_code(KC_A);
        yDownHeld3 = true;
      } else if (yDownHeld3 && yPos3 < _DOWN_TRESHOLD) {
        yDownHeld3 = false;
        unregister_code(KC_A);
      } else if (!yUpHeld3 && yPos3 <= _UP_TRESHOLD) {
        yUpHeld3 = true;
        register_code(KC_D);
      } else if (yUpHeld3 && yPos3 > _UP_TRESHOLD) {
        yUpHeld3 = false;
        unregister_code(KC_D);
      }

      
      //xPos = analogReadPin(JSH);
      xPos3 = s2m.jy1;

      if (!xLeftHeld3 && xPos3 >= _DOWN_TRESHOLD) {
        register_code(KC_S);
        xLeftHeld3 = true;
      } else if (xLeftHeld3 && xPos3 < _DOWN_TRESHOLD) {
        xLeftHeld3 = false;
        unregister_code(KC_S);
      } else if (!xRightHeld3 && xPos3 <= _UP_TRESHOLD) {
        xRightHeld3 = true;
        register_code(KC_W);
      } else if (xRightHeld3 && xPos3 > _UP_TRESHOLD) {
        xRightHeld3 = false;
        unregister_code(KC_W);
      }

    //---------------------joystick mode----------------------------
      //jx2 = saxisCoordinate(s2m.jx1, xOrigin3);
      //jy2 = saxisCoordinate(s2m.jy1, yOrigin3);


    }else if(jMode3 == _JOYSL){
      //jy = maxisCoordinate(JSH, xOrigin);
      //jx = maxisCoordinate(JSV, yOrigin);
      jy = saxisCoordinate(s2m.jy1, yOrigin3);
      jx = saxisCoordinate(s2m.jx1, xOrigin3);
      joystick_set_axis( 0, -jx);
      joystick_set_axis( 1, jy);
    }else if(jMode3 == _JOYSR){
      //jy = maxisCoordinate(JSH, xOrigin);
      //jx = maxisCoordinate(JSV, yOrigin);
      jy = saxisCoordinate(s2m.jy1, yOrigin3);
      jx = saxisCoordinate(s2m.jx1, xOrigin3);
      joystick_set_axis( 2, -jx);
      joystick_set_axis( 3, jy);
    }else if(jMode3 == _JOYSZ){
      //jy = maxisCoordinate(JSH, xOrigin);
      //jx = maxisCoordinate(JSV, yOrigin);
      jy = saxisCoordinate(s2m.jy1, yOrigin3);
      jx = saxisCoordinate(s2m.jx1, xOrigin3);
      joystick_set_axis( 4, -jx);
      joystick_set_axis( 5, jy); 

    //----------------------mouse mode------------------------------
    }else if(jMode3 == _MOUSE){


    //----------------------scroll mode-----------------------------
    }else if(jMode3 == _SCROLL){
      //yPos = analogReadPin(JSV);
      yPos3 = s2m.jx1;
      if (!yDownHeld3 && yPos3 >= _DOWN_TRESHOLD) {
        register_code(KC_WH_L);
        yDownHeld3 = true;
      } else if (yDownHeld3 && yPos3 < _DOWN_TRESHOLD) {
        yDownHeld3 = false;
        unregister_code(KC_WH_L);
      } else if (!yUpHeld3 && yPos3 <= _UP_TRESHOLD) {
        yUpHeld3 = true;
        register_code(KC_WH_R);
      } else if (yUpHeld3 && yPos3 > _UP_TRESHOLD) {
        yUpHeld3 = false;
        unregister_code(KC_WH_R);
      }
      
      //xPos = analogReadPin(JSH);
      xPos3 = s2m.jy1;
      if (!xLeftHeld3 && xPos3 >= _DOWN_TRESHOLD) {
        register_code(KC_WH_D);
        xLeftHeld3 = true;
      } else if (xLeftHeld3 && xPos3 < _DOWN_TRESHOLD) {
        xLeftHeld3 = false;
        unregister_code(KC_WH_D);
      } else if (!xRightHeld3 && xPos3 <= _UP_TRESHOLD) {
        xRightHeld3 = true;
        register_code(KC_WH_U);
      } else if (xRightHeld3 && xPos3 > _UP_TRESHOLD) {
        xRightHeld3 = false;
        unregister_code(KC_WH_U);
      }
    }

#endif
/*-------------------------jmod4--------------------------------*/
#if 1
    if (jMode4 == _UWASD) {

      //yPos = analogReadPin(JSV);
      yPos4 = s2m.jx2;

      if (!yDownHeld4 && yPos4 >= _DOWN_TRESHOLD) {

        register_code(dynamic_keymap_get_keycode(biton32(layer_state),7,4));
        yDownHeld4 = true;
      } else if (yDownHeld4 && yPos4 < _DOWN_TRESHOLD) {
        yDownHeld4 = false;

        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),7,4));
      } else if (!yUpHeld4 && yPos4 <= _UP_TRESHOLD) {
        yUpHeld4 = true;

        register_code(dynamic_keymap_get_keycode(biton32(layer_state),6,4));
      } else if (yUpHeld4 && yPos4 > _UP_TRESHOLD) {
        yUpHeld4 = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),6,4));
      }

      
      //xPos = analogReadPin(JSH);
      xPos4 = s2m.jy2;

      if (!xLeftHeld4 && xPos4 >= _DOWN_TRESHOLD) {

        register_code(dynamic_keymap_get_keycode(biton32(layer_state),5,4));
        xLeftHeld4 = true;
      } else if (xLeftHeld4 && xPos4 < _DOWN_TRESHOLD) {
        xLeftHeld4 = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),5,4));

      } else if (!xRightHeld4 && xPos4 <= _UP_TRESHOLD) {
        xRightHeld4 = true;
        register_code(dynamic_keymap_get_keycode(biton32(layer_state),8,4));

      } else if (xRightHeld4 && xPos4 > _UP_TRESHOLD) {
        xRightHeld4 = false;
        unregister_code(dynamic_keymap_get_keycode(biton32(layer_state),8,4));
      }

    //-----------------------WASD mode---------------------------
    }else if(jMode4 == _WASD) {

      //yPos = analogReadPin(JSV);
      yPos4 = s2m.jx2;

      if (!yDownHeld4 && yPos4 >= _DOWN_TRESHOLD) {
        register_code(KC_D);
        yDownHeld4 = true;
      } else if (yDownHeld4 && yPos4 < _DOWN_TRESHOLD) {
        yDownHeld4 = false;
        unregister_code(KC_D);
      } else if (!yUpHeld4 && yPos4 <= _UP_TRESHOLD) {
        yUpHeld4 = true;
        register_code(KC_A);
      } else if (yUpHeld4 && yPos4 > _UP_TRESHOLD) {
        yUpHeld4 = false;
        unregister_code(KC_A);
      }

      
      //xPos = analogReadPin(JSH);
      xPos4 = s2m.jy2;

      if (!xLeftHeld4 && xPos4 >= _DOWN_TRESHOLD) {
        register_code(KC_W);
        xLeftHeld4 = true;
      } else if (xLeftHeld4 && xPos4 < _DOWN_TRESHOLD) {
        xLeftHeld4 = false;
        unregister_code(KC_W);
      } else if (!xRightHeld4 && xPos4 <= _UP_TRESHOLD) {
        xRightHeld4 = true;
        register_code(KC_S);
      } else if (xRightHeld4 && xPos4 > _UP_TRESHOLD) {
        xRightHeld4 = false;
        unregister_code(KC_S);
      }

    //---------------------joystick mode----------------------------
      //jx2 = saxisCoordinate(s2m.jx1, xOrigin3);
      //jy2 = saxisCoordinate(s2m.jy1, yOrigin3);


    }else if(jMode4 == _JOYSL){
      //jy = maxisCoordinate(JSH, xOrigin);
      //jx = maxisCoordinate(JSV, yOrigin);
      jy = saxisCoordinate(s2m.jy2, yOrigin4);
      jx = saxisCoordinate(s2m.jx2, xOrigin4);
      joystick_set_axis( 0, jx);
      joystick_set_axis( 1, -jy);
    }else if(jMode4 == _JOYSR){
      //jy = maxisCoordinate(JSH, xOrigin);
      //jx = maxisCoordinate(JSV, yOrigin);
      jy = saxisCoordinate(s2m.jy2, yOrigin4);
      jx = saxisCoordinate(s2m.jx2, xOrigin4);
      joystick_set_axis( 2, jx);
      joystick_set_axis( 3, -jy);
    }else if(jMode4 == _JOYSZ){
      //jy = maxisCoordinate(JSH, xOrigin);
      //jx = maxisCoordinate(JSV, yOrigin);
      jy = saxisCoordinate(s2m.jy2, yOrigin4);
      jx = saxisCoordinate(s2m.jx2, xOrigin4);
      joystick_set_axis( 4, jx);
      joystick_set_axis( 5, -jy); 

    //----------------------mouse mode------------------------------
    }else if(jMode4 == _MOUSE){


    //----------------------scroll mode-----------------------------
    }else if(jMode4 == _SCROLL){
      //yPos = analogReadPin(JSV);
      yPos4 = s2m.jx2;
      if (!yDownHeld4 && yPos4 >= _DOWN_TRESHOLD) {
        register_code(KC_WH_R);
        yDownHeld4 = true;
      } else if (yDownHeld4 && yPos4 < _DOWN_TRESHOLD) {
        yDownHeld4 = false;
        unregister_code(KC_WH_R);
      } else if (!yUpHeld4 && yPos4 <= _UP_TRESHOLD) {
        yUpHeld4 = true;
        register_code(KC_WH_L);
      } else if (yUpHeld4 && yPos4 > _UP_TRESHOLD) {
        yUpHeld4 = false;
        unregister_code(KC_WH_L);
      }
      
      //xPos = analogReadPin(JSH);
      xPos4 = s2m.jy2;
      if (!xLeftHeld4 && xPos4 >= _DOWN_TRESHOLD) {
        register_code(KC_WH_U);
        xLeftHeld4 = true;
      } else if (xLeftHeld4 && xPos4 < _DOWN_TRESHOLD) {
        xLeftHeld4 = false;
        unregister_code(KC_WH_U);
      } else if (!xRightHeld4 && xPos4 <= _UP_TRESHOLD) {
        xRightHeld4 = true;
        register_code(KC_WH_D);
      } else if (xRightHeld4 && xPos4 > _UP_TRESHOLD) {
        xRightHeld4 = false;
        unregister_code(KC_WH_D);
      }
    }
#endif
}

  //joystick config
  joystick_config_t joystick_axes[JOYSTICK_AXIS_COUNT] = {
    JOYSTICK_AXIS_VIRTUAL,
    JOYSTICK_AXIS_VIRTUAL,
    JOYSTICK_AXIS_VIRTUAL,
    JOYSTICK_AXIS_VIRTUAL,
    JOYSTICK_AXIS_VIRTUAL,
    JOYSTICK_AXIS_VIRTUAL
    //JOYSTICK_AXIS_IN(JSH, _MAX, _CENTER, _MIN),
    //JOYSTICK_AXIS_IN(JSV, _MIN, _CENTER, _MAX)  
  };
#endif // joystick

void pointing_device_driver_init(void) {

  analog_joystick_init();
  xOrigin = analogReadPin(JSH);
  yOrigin = analogReadPin(JSV);

  gpio_set_pin_input_high(JSH2);
  gpio_set_pin_input_high(JSV2);
  xOrigin2 =analogReadPin(JSH2); 
  yOrigin2 =analogReadPin(JSV2);

  pmw3610_sync();
  pmw3610_init();
  //adns5050_sync();
  //adns5050_init();


}

report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report){

#if 1
      report_pmw3610_t data = pmw3610_read_burst();

      mouse_report.x =  data.dy;
      mouse_report.y = -data.dx;
#endif  

#if 0  
     //report_pmw3610_t data = pmw3610_read_burst();
     report_adns5050_t data = adns5050_read_burst();
     mouse_report.x = -data.dx;
     mouse_report.y = -data.dy;
     //mouse_report.x = data.dx / nCPI; // fake cpi
     //mouse_report.y = data.dy / nCPI; // fake cpi
#endif

#if 1     
     if(jMode1 == _MOUSE){
      report_analog_joystick_t jdata = analog_joystick_read();
      //mouse_report.h = 0;
      //mouse_report.v = 0;
      mouse_report.x += -jdata.y;
      mouse_report.y += jdata.x;
     }else if(jMode2 == _MOUSE){
      report_analog_joystick_t jdata = analog_joystick2_read();
      //mouse_report.h = 0;
      //mouse_report.v = 0;
      mouse_report.x += jdata.y;
      mouse_report.y += -jdata.x;
     }else if(jMode3 == _MOUSE){
      report_analog_joystick_t jdata = analog_joystick3_read();
      mouse_report.x += -jdata.y;
      mouse_report.y += jdata.x;
     }else if(jMode4 == _MOUSE){
      report_analog_joystick_t jdata = analog_joystick4_read();
      mouse_report.x += jdata.y;
      mouse_report.y += -jdata.x;
     }

#endif
     return mouse_report; 
}


uint16_t pointing_device_driver_get_cpi(void){
     uint16_t cpi = pmw3610_get_cpi();
     //uint16_t cpi = adns5050_get_cpi();
     uprintf("Get CPI = %d\n",cpi);
     return cpi;
}


void pointing_device_driver_set_cpi(uint16_t cpi){
    pmw3610_set_cpi(cpi);
    //adns5050_set_cpi(cpi);
}

#if 0
//#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise)
{
    if(clockwise){
      tap_code(KC_AUDIO_VOL_UP);
    }else{
      tap_code(KC_AUDIO_VOL_DOWN);
    }
    return true;
}
#else
bool encoder_update_user(uint8_t index, bool clockwise) {
	if(index == 0){
		if(clockwise){
			tap_code(dynamic_keymap_get_keycode(biton32(layer_state),4,4));
		}else{
			tap_code(dynamic_keymap_get_keycode(biton32(layer_state),4,5));
		}
  } else if (index == 1) {
    if (clockwise) {
      tap_code(dynamic_keymap_get_keycode(biton32(layer_state),4,5));
		}else{
		  tap_code(dynamic_keymap_get_keycode(biton32(layer_state),4,4));
    }
  }
  return true;
}
#endif

#if 0
//#ifdef OLED_ENABLE
bool oled_task_user(void) {
    // Host Keyboard Layer Status
    oled_write_P(PSTR("Layer: "), false);

    switch (get_highest_layer(layer_state)) {
        case _BASE:
            oled_write_P(PSTR("BASE\n"), false);
            break;
        case _LOWER:
            oled_write_P(PSTR("LOWER\n"), false);
            break;
        case _RAISE:
            oled_write_P(PSTR("RAISE\n"), false);
            break;
        case _ADJUST:
            oled_write_P(PSTR("ADJUS\n"), false);
            break;
        default:
            // Or use the write_ln shortcut over adding '\n' to the end of your string
            oled_write_ln_P(PSTR("Undefined"), false);
    }

    // Host Keyboard LED Status
    led_t led_state = host_keyboard_led_state();
    oled_write_P(led_state.num_lock ? PSTR("NUM ") : PSTR("    "), false);
    oled_write_P(led_state.caps_lock ? PSTR("CAP ") : PSTR("    "), false);
    oled_write_P(led_state.scroll_lock ? PSTR("SCR ") : PSTR("    "), false);
    
    return false;
}
#endif

#ifdef OLED_ENABLE
#if 1
static void render_status(void) {
    // Host Keyboard Layer Status
    
    oled_write_P(PSTR("-----"), false);
    //oled_write_P(PSTR("-----LAYER"), false);
    switch (get_highest_layer(layer_state)) {
        case _BASE:
            oled_write_P(PSTR(" BAS "), false);
            break;
        case _LOWER:
            oled_write_P(PSTR(" LWR "), false);
            break;
        case _RAISE:
            oled_write_P(PSTR(" RIS "), false);
            break;
        case _ADJUST:
            oled_write_P(PSTR(" ADJ "), false);
            break;
        case _MEDIA:
            oled_write_P(PSTR(" MDI "), false);
            break;
        case _GAMINGL:
            oled_write_P(PSTR(" GAM "), false);
            break;
        default:
            oled_write_P(PSTR(" UDF "), false);
    }
    oled_write_P(PSTR("-----"), false);
    //oled_write_P(PSTR("-----STATS-----"), false);
    //oled_write_P(PSTR("STATS"), false);
    //oled_write_P(PSTR("\n\n\n"), false);
    // Host Keyboard LED Status
    led_t led_state = host_keyboard_led_state();
    oled_write_P(led_state.num_lock ? PSTR("NUM:@") : PSTR("NUM:_"), false);
    oled_write_P(led_state.caps_lock ? PSTR("CAP:@") : PSTR("CAP:_"), false);
    oled_write_P(led_state.scroll_lock ? PSTR("SCR:@") : PSTR("SCR:_"), false);
    //oled_write_P(PSTR("-----JMODLEFT "), false);
    //oled_write_P(PSTR("-----JYMOD"), false);
    //oled_write_P(PSTR("LEFT "), false);
    oled_write_P(PSTR("-----"), false);
    
    switch(jMode2){
      case _MOUSE:
        oled_write_P(PSTR("MOUSE"), false);
        break;
      case _SCROLL:
        oled_write_P(PSTR("SCROL"), false);
        break;
      case _JOYSL:
        oled_write_P(PSTR(" JOYL"), false);
        break;
      case _JOYSR:
        oled_write_P(PSTR(" JOYR"), false);
        break;
      case _JOYSZ:
        oled_write_P(PSTR(" JOYZ"), false);
        break;
      case _WASD:
        oled_write_P(PSTR(" WASD"), false);
        break;
      case _UWASD:
        oled_write_P(PSTR("UWASD"), false);
        break;
      case _3DPAN:
        oled_write_P(PSTR("3DPAN"), false);
        break;
      case _3DROT:
        oled_write_P(PSTR("3DROT"), false);
        break;
      default:
        oled_write_P(PSTR(" XXX "), false);
        break;
    }

    switch(jMode1){
      case _MOUSE:
        oled_write_P(PSTR("MOUSE"), false);
        break;
      case _SCROLL:
        oled_write_P(PSTR("SCROL"), false);
        break;
      case _JOYSL:
        oled_write_P(PSTR(" JOYL"), false);
        break;
      case _JOYSR:
        oled_write_P(PSTR(" JOYR"), false);
        break;
      case _JOYSZ:
        oled_write_P(PSTR(" JOYZ"), false);
        break;
      case _WASD:
        oled_write_P(PSTR(" WASD"), false);
        break;
      case _UWASD:
        oled_write_P(PSTR("UWASD"), false);
        break;
      case _3DPAN:
        oled_write_P(PSTR("3DPAN"), false);
        break;
      case _3DROT:
        oled_write_P(PSTR("3DROT"), false);
        break;
      case _GAMING:
        oled_write_P(PSTR("GAMIG"), false);
        break;
      default:
        oled_write_P(PSTR(" XXX "), false);
        break;
    }
    oled_write_P(PSTR("-----"), false);
    //oled_write_P(PSTR("UWASD"), false);
    //oled_write_P(PSTR("UWASD"), false);
    switch(jMode4){
      case _MOUSE:
        oled_write_P(PSTR("MOUSE"), false);
        break;
      case _SCROLL:
        oled_write_P(PSTR("SCROL"), false);
        break;
      case _JOYSL:
        oled_write_P(PSTR(" JOYL"), false);
        break;
      case _JOYSR:
        oled_write_P(PSTR(" JOYR"), false);
        break;
      case _JOYSZ:
        oled_write_P(PSTR(" JOYZ"), false);
        break;
      case _WASD:
        oled_write_P(PSTR(" WASD"), false);
        break;
      case _UWASD:
        oled_write_P(PSTR("UWASD"), false);
        break;
      case _3DPAN:
        oled_write_P(PSTR("3DPAN"), false);
        break;
      case _3DROT:
        oled_write_P(PSTR("3DROT"), false);
        break;
      default:
        oled_write_P(PSTR(" XXX "), false);
        break;
    }

    switch(jMode3){
      case _MOUSE:
        oled_write_P(PSTR("MOUSE"), false);
        break;
      case _SCROLL:
        oled_write_P(PSTR("SCROL"), false);
        break;
      case _JOYSL:
        oled_write_P(PSTR(" JOYL"), false);
        break;
      case _JOYSR:
        oled_write_P(PSTR(" JOYR"), false);
        break;
      case _JOYSZ:
        oled_write_P(PSTR(" JOYZ"), false);
        break;
      case _WASD:
        oled_write_P(PSTR(" WASD"), false);
        break;
      case _UWASD:
        oled_write_P(PSTR("UWASD"), false);
        break;
      case _3DPAN:
        oled_write_P(PSTR("3DPAN"), false);
        break;
      case _3DROT:
        oled_write_P(PSTR("3DROT"), false);
        break;
      case _GAMING:
        oled_write_P(PSTR("GAMIG"), false);
        break;
      default:
        oled_write_P(PSTR(" XXX "), false);
        break;
    }
    //oled_write_P(PSTR("----- CPI "), false);
    oled_write_P(PSTR("-----"), false);


    switch(nMCPI){
      case 0:
        oled_write_P(PSTR("M 400"), false);
        //oled_write_P(PSTR(" 125 "), false);
        break;
      case 1:
        oled_write_P(PSTR("M 800"), false);
        //oled_write_P(PSTR(" 500 "), false);
        break;
      case 2:
        oled_write_P(PSTR("M1600"), false);
        //oled_write_P(PSTR(" 1000"), false);
        break;
      case 3:
        oled_write_P(PSTR("M3200"), false);
        //oled_write_P(PSTR(" 1375"), false);
        break;
      default:
        oled_write_P(PSTR("M 400"), false);
        //oled_write_P(PSTR(" 125 "), false);
        break;
    }

    switch(nSCPI){
      case 0:
        oled_write_P(PSTR("S 400"), false);
        //oled_write_P(PSTR(" 125 "), false);
        break;
      case 1:
        oled_write_P(PSTR("S 800"), false);
        //oled_write_P(PSTR(" 500 "), false);
        break;
      case 2:
        oled_write_P(PSTR("S1600"), false);
        //oled_write_P(PSTR(" 1000"), false);
        break;
      case 3:
        oled_write_P(PSTR("S3200"), false);
        //oled_write_P(PSTR(" 1375"), false);
        break;
      default:
        oled_write_P(PSTR("S 400"), false);
        //oled_write_P(PSTR(" 125 "), false);
        break;
    }
    //oled_write_P(PSTR(" 3200"), false);

#if 0
    if(is_drag_scroll_m){
        oled_write_P(PSTR(" Scl "), false);
    }else{
        oled_write_P(PSTR(" Mx"), false);
        oled_write_char((user_config.v99_dpi_master+48), false);
        oled_write_P(PSTR("\n"), false);
    }
    //oled_write_P(is_drag_scroll_m ? PSTR(" Scl  ") : PSTR(" Mx  "), false);
    //oled_write_P(PSTR(" Mx2 "), false);
    oled_write_P(PSTR("RIGHT"), false);
    if(is_drag_scroll_s){
        oled_write_P(PSTR(" Scl "), false);
    }else{
        oled_write_P(PSTR(" Mx"), false);
        oled_write_char((user_config.v99_dpi_slave+48), false);
        oled_write_P(PSTR("\n"), false);
    }

enum jmodes {
    _MOUSE = 0,
    _SCROLL,
    _JOYS,
    _WASD
};


#endif
    //oled_write_P(is_drag_scroll_s ? PSTR(" Scl  ") : PSTR(" Mx  "), false);
    //oled_write_P(PSTR(" Scl "), false);
    //oled_write_P(V99_X_TRANSFORM_M, false);
    //oled_write_char((get_highest_layer(layer_state)+48), false);
    oled_write_P(PSTR("-----"), false);
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
        return OLED_ROTATION_270;  // flips the display 270 degrees if offhand
    return rotation;
}


bool oled_task_user(void){
  if(is_keyboard_master()){
    if(f_oledchange){
      f_oledchange = false;
      render_status(); // Renders the current keyboard state (layer, lock, caps, scroll, etc)
    }
  }
  return false;
}


#else

static void render_logo(void) {
    static const char PROGMEM qmk_logo[] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0x00
    };

    oled_write_P(qmk_logo, false);
}

bool oled_task_user(void) {
    render_logo();
    return false;
}

#endif
#endif

#if 1
void eeconfig_init_user(void) {  // EEPROM is getting reset!

  bool f_conf_change = false;
  user_config.raw = 0;


  user_config.raw = eeconfig_read_user();
  if(user_config.jMode1 > _3DPAN || user_config.jMode1 < 0 ){
    user_config.jMode1 = _UWASD;
    f_conf_change = true;
  }
  //jMode = user_config.jMode;

  if(user_config.jMode2 > _3DPAN || user_config.jMode2 < 0){
    user_config.jMode2 = _UWASD;
    f_conf_change = true;
  }

  if(user_config.jMode3 > _3DPAN || user_config.jMode3 < 0){
    user_config.jMode3 = _UWASD;
    f_conf_change = true;
  }

  if(user_config.jMode4 > _3DPAN || user_config.jMode4 < 0){
    user_config.jMode4 = _UWASD;
    f_conf_change = true;
  }

  if(user_config.jMode1 == user_config.jMode2)
  {
    if(user_config.jMode2 >= _WASD)
    user_config.jMode2 = _UWASD;
    //user_config.jMode2++;
    f_conf_change = true;
  }
  //jMode2 = user_config.jMode2;

  if((user_config.nMCPI > 3) || (user_config.nMCPI < 0) ){
    user_config.nMCPI = 1;
    nMCPI = 1;
    f_conf_change = true;
  }
  
  if((user_config.nSCPI > 3) || (user_config.nSCPI < 0) ){
    user_config.nSCPI = 1;
    nSCPI = 1;
    f_conf_change = true;
  }

  if(f_conf_change){
    f_conf_change = false;
    eeconfig_update_user(user_config.raw); // Write default value to EEPROM now    
  }
  //pointing_device_driver_set_cpi(CPIn[nMCPI]);
  //pointing_device_set_cpi_on_side(false, CPIn[nSCPI]); //right side cpi
  f_oledchange = true;

}
#endif

#if 1
void matrix_init_user(void){
# ifdef CONSOLE_ENABLE
  dprintf("init MS XY transform value \n");
# endif
  eeconfig_init_user();
}

#if 1

void housekeeping_task_user(void) {
    static bool f_slavejoyinit = false;
    static bool f_slaveconnected = false;
    static uint32_t last_sync = 0;
    slave_to_master_t s2mbuf = { 0, 0, 0, 0};
    
    if (is_keyboard_master()) { //slave to master


      if(!f_slaveconnected){

        if(timer_elapsed32(last_sync) > 1000){ //1sec
          
          f_slaveconnected = true;
          last_sync = timer_read32();
        }

      }else{

          //if (timer_elapsed32(last_sync) > 50) {  // 50ms
        //if (timer_elapsed32(last_sync) > 1000) {  // 1000ms
        if (timer_elapsed32(last_sync) > 1) {  // 1ms,1khz
            last_sync = timer_read32();

            if (transaction_rpc_recv(JOYSYNC, sizeof(s2mbuf), &s2mbuf)) {
                s2m = s2mbuf;
                uprint("Slave sync success!!!!!!!!!!!\n");

                if(!f_slavejoyinit){    
                  
                  xOrigin3 = s2m.jx1;

                  if(xOrigin3>400 && xOrigin3<600){

                    f_slavejoyinit = true;

                    //xOrigin3 = s2m.jx1;
                    yOrigin3 = s2m.jy1;
                    xOrigin4 = s2m.jx2;
                    yOrigin4 = s2m.jy2;

                    uprintf("Slave connected!\n");
                    uprintf("receiver jx3 origin = %d\n",xOrigin3);
                    uprintf("receiver jy3 origin = %d\n",yOrigin3);
                    uprintf("receiver jx4 origin = %d\n",xOrigin4);
                    uprintf("receiver jy4 origin = %d\n",yOrigin4);

                  }                  
                }

                showreceiver();
            } else {
                dprint("Slave sync failed!\n");
                uprint("Slave sync failed!\n");
            }
        }
      }    
    }
}

void user_joys_slave_handler(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {

    slave_to_master_t *s2mout = (slave_to_master_t*)out_data;
      s2mout->jy1 = analogReadPin(JSH);
      s2mout->jx1 = analogReadPin(JSV);
      s2mout->jy2 = analogReadPin(JSH2);
      s2mout->jx2 = analogReadPin(JSV2);

      //s2mout->jx1 = 5;
      //s2mout->jy1 = 6;
      //s2mout->jx2 = 7;
      //s2mout->jy2 = 8;
}
#endif

void modeflagSet(uint8_t jmode){
  if(jmode == _MOUSE){f_JMOUS = true;}
  else if(jmode == _JOYSL){f_JOYSL = true;}
  else if(jmode == _JOYSR){f_JOYSR = true;}
  else if(jmode == _JOYSZ){f_JOYSZ = true;}
  else if(jmode == _3DROT){f_3DROT = true;}
  else if(jmode == _3DPAN){f_3DPAN = true;}
}

void keyboard_post_init_user(void) {
    //uint16_t slaveCPI;
    //debug_enable=true;
    //debug_matrix=true;
    transaction_register_rpc(JOYSYNC, user_joys_slave_handler);

    user_config.raw = eeconfig_read_user();
    jMode1 = user_config.jMode1;
    jMode2 = user_config.jMode2;
    jMode3 = user_config.jMode3;
    jMode4 = user_config.jMode4;
    nMCPI = user_config.nMCPI;
    nSCPI = user_config.nSCPI;

    modeflagSet(jMode1);
    modeflagSet(jMode2);
    modeflagSet(jMode3);
    modeflagSet(jMode4);

    if(is_keyboard_master()){   
      pointing_device_set_cpi_on_side(true, CPIn[nMCPI]); //left side cpi
      pointing_device_set_cpi_on_side(false, CPIn[nSCPI]); //right side cpi
    }
    uprintf("Debug console connected!!!!!\n");
    //pointing_device_driver_set_cpi(CPIn[nMCPI]);

}
#endif