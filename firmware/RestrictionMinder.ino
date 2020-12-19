#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"

#define OLED_RESET                        4
#define SCREEN_WIDTH                      128 // OLED display width, in pixels
#define SCREEN_HEIGHT                     32 // OLED display height, in pixels

#define SETUP_BUTTON_PRESSED_CHECK_TIME   1500
#define RENDER_TIME                       200

#define OK_BUTTON_PIN                     9
#define RIGHT_BUTTON_PIN                  10
#define LEFT_BUTTON_PIN                   11
#define DOWN_BUTTON_PIN                   12
#define UP_BUTTON_PIN                     13

#define BUZZER_PIN                        A0
#define WARNING_ENABLE_PIN                A1

enum SetType
{
  ST_NONE = -1,
  ST_MONTH = 0,
  ST_DAY = 1,
  ST_HOUR = 2,
  ST_MINUTE = 3,
  ST_YEAR = 4
};

enum RestrictTailNumberType
{
  RTNT_NONE = -1,
  RTNT_0_5 = 0,
  RTNT_1_6,
  RTNT_2_7,
  RTNT_3_8,
  RTNT_4_9,
  RTNT_ODD_NUMBER,
  RTNT_EVEN_NUMBER
};

/*--  文字:  年  --*/
static const unsigned char PROGMEM str_nian[] =
{
  0x10, 0x00, 0x10, 0x00, 0x1F, 0xFC, 0x20, 0x80, 0x20, 0x80, 0x40, 0x80, 0x1F, 0xF8, 0x10, 0x80,
  0x10, 0x80, 0x10, 0x80, 0xFF, 0xFE, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80
};
/*--  文字:  月  --*/
static const unsigned char PROGMEM str_yue[] =
{
  0x00, 0x00, 0x1F, 0xF8, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x1F, 0xF8, 0x10, 0x08, 0x10, 0x08,
  0x10, 0x08, 0x1F, 0xF8, 0x10, 0x08, 0x10, 0x08, 0x20, 0x08, 0x20, 0x08, 0x40, 0x28, 0x80, 0x10
};
/*--  文字:  日  --*/
static const unsigned char PROGMEM str_ri[] =
{
  0x00, 0x00, 0x1F, 0xF0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0xF0,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0xF0, 0x10, 0x10
};
/*--  文字:  周  --*/
static const unsigned char PROGMEM str_zhou[] =
{
  0x00, 0x00, 0x3F, 0xF8, 0x21, 0x08, 0x21, 0x08, 0x2F, 0xE8, 0x21, 0x08, 0x21, 0x08, 0x3F, 0xF8,
  0x20, 0x08, 0x27, 0xC8, 0x24, 0x48, 0x24, 0x48, 0x27, 0xC8, 0x40, 0x08, 0x40, 0x28, 0x80, 0x10
};
/*--  文字:  一二三四五六日  --*/
static const unsigned char PROGMEM str_zhoushu[7][32] =
{
  { 0x00, 0x00, 0x1F, 0xF0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0xF0,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0xF0, 0x10, 0x10
  },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFE,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  },
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  },
  { 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xF8,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00
  },
  { 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFC, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
    0x48, 0x44, 0x48, 0x3C, 0x50, 0x04, 0x60, 0x04, 0x40, 0x04, 0x7F, 0xFC, 0x40, 0x04, 0x00, 0x00
  },
  { 0x00, 0x00, 0x7F, 0xFC, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x3F, 0xF0, 0x04, 0x10,
    0x04, 0x10, 0x04, 0x10, 0x04, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0xFF, 0xFE, 0x00, 0x00
  },
  { 0x02, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x40, 0x04, 0x20, 0x08, 0x10, 0x08, 0x08, 0x10, 0x08, 0x20, 0x04, 0x40, 0x04, 0x00, 0x00
  }
};
/*--  文字:  单  --*/
static const unsigned char PROGMEM str_dan[] =
{
  0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x3F, 0xF8, 0x21, 0x08, 0x21, 0x08, 0x3F, 0xF8, 0x21, 0x08,
  0x21, 0x08, 0x3F, 0xF8, 0x01, 0x00, 0x01, 0x00, 0xFF, 0xFE, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00
};
/*--  文字:  双  --*/
static const unsigned char PROGMEM str_shuang[] =
{
  0x00, 0x00, 0x00, 0x00, 0xFD, 0xFC, 0x04, 0x84, 0x44, 0x84, 0x44, 0x84, 0x28, 0x88, 0x28, 0x88,
  0x10, 0x50, 0x10, 0x50, 0x28, 0x20, 0x28, 0x20, 0x44, 0x50, 0x44, 0x88, 0x81, 0x04, 0x02, 0x02
};
/*--  文字:  不  --*/
static const unsigned char PROGMEM str_bu[] =
{
  0x00, 0x00, 0x7F, 0xFC, 0x00, 0x80, 0x00, 0x80, 0x01, 0x00, 0x01, 0x00, 0x03, 0x40, 0x05, 0x20,
  0x09, 0x10, 0x11, 0x08, 0x21, 0x04, 0x41, 0x04, 0x81, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00
};
/*--  文字:  尾  --*/
static const unsigned char PROGMEM str_wei[] =
{
  0x00, 0x00, 0x3F, 0xFC, 0x20, 0x04, 0x20, 0x04, 0x3F, 0xFC, 0x20, 0x00, 0x20, 0xF0, 0x2F, 0x00,
  0x21, 0x00, 0x21, 0xF0, 0x2F, 0x00, 0x21, 0x00, 0x21, 0xF8, 0x5F, 0x02, 0x41, 0x02, 0x80, 0xFE
};
/*--  文字:  号  --*/
static const unsigned char PROGMEM str_hao[] =
{
  0x00, 0x00, 0x1F, 0xF0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0xF0, 0x00, 0x00, 0xFF, 0xFE,
  0x08, 0x00, 0x10, 0x00, 0x1F, 0xF0, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0xA0, 0x00, 0x40
};
/*--  文字:  限  --*/
static const unsigned char PROGMEM str_xian[] =
{
  0x00, 0x00, 0x7B, 0xF8, 0x4A, 0x08, 0x52, 0x08, 0x53, 0xF8, 0x62, 0x08, 0x52, 0x08, 0x4B, 0xF8,
  0x4A, 0x44, 0x4A, 0x48, 0x6A, 0x30, 0x52, 0x20, 0x42, 0x10, 0x42, 0x88, 0x43, 0x06, 0x42, 0x00
};
/*--  文字:  行  --*/
static const unsigned char PROGMEM str_xing[] =
{
  0x08, 0x00, 0x09, 0xFC, 0x10, 0x00, 0x20, 0x00, 0x48, 0x00, 0x08, 0x00, 0x13, 0xFE, 0x30, 0x20,
  0x50, 0x20, 0x90, 0x20, 0x10, 0x20, 0x10, 0x20, 0x10, 0x20, 0x10, 0x20, 0x10, 0xA0, 0x10, 0x40
};
static const unsigned char str_xxType[5][3] =
{
  "05", "16", "27", "38", "49"
};

static const int tailNumber[5][2] =
{
  {0, 5}, {1, 6}, {2, 7}, {3, 8}, {4, 9}
};

RTC_DS1307 rtc;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SetType                 m_setType             = ST_NONE;    //时间设置类型
RestrictTailNumberType  m_restrictType        = RTNT_NONE;  //当日限行类型
RestrictTailNumberType  m_tailNumberType      = RTNT_NONE;  //尾号类型
int                     m_tailNumber          = -1;         //限行尾号

long  m_lastRenderTime              = 0;

int   m_year                        = 0;
int   m_month                       = 0;
int   m_day                         = 0;
int   m_hour                        = 0;
int   m_minute                      = 0;
int   m_dayOfTheWeek                = 0;

long  m_lastSetupButtonReleaseTime  = 0;
bool  m_checkSetButton              = true;
bool  m_checkDirButton              = true;


bool restrictCheck(int _tailNumber, RestrictTailNumberType *_rtntType)
{
  //北京限行规则
  //每13周限行尾号轮换一次，周末及法定节假日不限行。
  //自2018年4月9日至2018年7月8日，周一至五限行尾号顺序为：4和9、5和0、1和6、2和7、3和8；
  //工作日(法定节假日除外)早7点至晚8点，限行机动车（含临时号牌）禁止在五环路（不含）以内道路行驶。
  bool oddEvenRestrict = false;
  if (digitalRead(8) == LOW) oddEvenRestrict = true;
  if (oddEvenRestrict)
  {
    if (m_day % 2 == 0) *_rtntType = RTNT_EVEN_NUMBER;
    else *_rtntType = RTNT_ODD_NUMBER;
    if (_tailNumber % 2 == m_day % 2)return true;
    else return false;
  } else
  {
    if (m_dayOfTheWeek == 0 || m_dayOfTheWeek == 6)
    {
      *_rtntType = RTNT_NONE;
      return false;
    }
  }
  int restrictTypePoint = 4 + 100;
  DateTime baseDate = DateTime(2018, 4, 9, 0, 0, 0);
  DateTime now = rtc.now();
  uint32_t turnSeconds = 7862400;//13 * 7 * 24 * 60 * 60;
  int turnTime = (now.unixtime() - baseDate.unixtime()) / turnSeconds; //从2018年4月9日零点到现在共轮换几次
  restrictTypePoint -= turnTime;
  restrictTypePoint %= 5;
  int restrictType = (restrictTypePoint + m_dayOfTheWeek - 1) % 5;
  *_rtntType = restrictType;
  if (_tailNumber % 5 == restrictType) return true;
  return false;
}


void setup() {
  Serial.begin(9600);//串口初始化
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");  //时钟模块初始化
    while (1);
  }
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));  //显示模块初始化
    while (1);
  }
  display.setTextSize(2);
  display.clearDisplay();
  for (int i = 2; i <= 13; i++) //2-8 拨码开关 9-13 按钮
  {
    pinMode(i, INPUT_PULLUP);
  }
}

void loop() {
  tailNumberCheck(&m_tailNumber, &m_tailNumberType);
  if (restrictCheck(m_tailNumber, &m_restrictType))
  {
    if (digitalRead(WARNING_ENABLE_PIN) == HIGH)
    {
      tone(BUZZER_PIN, 880);
      delay(200);
      noTone(BUZZER_PIN);
    }
  } else
  {
    noTone(BUZZER_PIN);
  }
  buttonAction();
  if (renderCheck())
  {
    if (m_setType == ST_NONE)
    {
      DateTime now = rtc.now();
      m_year = now.year();
      m_month = now.month();
      m_day = now.day();
      m_hour = now.hour();
      m_minute = now.minute();
      m_dayOfTheWeek = now.dayOfTheWeek();
    }
    render();
  }
}

bool renderCheck()
{
  if (millis() - m_lastRenderTime > RENDER_TIME)
  {
    m_lastRenderTime = millis();
    return true;
  }
  return false;
}

void render()
{
  display.clearDisplay();
  dateDisplay(m_year, m_month, m_day , m_dayOfTheWeek , m_hour, m_minute, m_setType);
  restrictDisplay(m_tailNumber, m_restrictType, m_setType);
  display.display();
}

void buttonAction()
{
  if (digitalRead(OK_BUTTON_PIN) == HIGH)
  {
    m_lastSetupButtonReleaseTime = millis();
    m_checkSetButton = true;
  } else
  {
    if (m_setType != ST_NONE)
    {
      if (m_checkSetButton)
      {
        m_setType = ST_NONE;
        rtc.adjust(DateTime(m_year, m_month, m_day, m_hour, m_minute, 0));
      }
    } else
    {
      if (millis() - m_lastSetupButtonReleaseTime > SETUP_BUTTON_PRESSED_CHECK_TIME)
      {
        m_setType = ST_MONTH;
        m_checkSetButton = false;
      }
    }
  }
  if (m_setType != ST_NONE)
  {
    if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
      if ( m_checkDirButton)
      {
        m_setType = m_setType >= 4 ? 0 : m_setType + 1;
        m_checkDirButton = false;
      }
    }
    else if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
      if ( m_checkDirButton)
      {
        m_setType = m_setType <= 0 ? 4 : m_setType - 1;
        m_checkDirButton = false;
      }
    } else if (digitalRead(DOWN_BUTTON_PIN) == LOW) {
      if ( m_checkDirButton)
      {
        timeSet(false);
        m_checkDirButton = false;
      }
    } else if (digitalRead(UP_BUTTON_PIN) == LOW) {
      if ( m_checkDirButton)
      {
        timeSet(true);
        m_checkDirButton = false;
      }
    }
    else m_checkDirButton = true;
  }
}

bool isRuinian(int _year)
{
  if (_year % 100 == 0)
  {
    if (_year % 400 == 0)return true;
    else return false;
  } else
  {
    if (_year % 4 == 0) return true;
    else return false;
  }
}
void timeSet(bool up)
{
  int dayCountArr[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  switch (m_setType)
  {
    case ST_MONTH:
      {
        if (up)m_month = m_month >= 12 ? 1 : m_month + 1;
        else m_month = m_month <= 1 ? 12 : m_month - 1;
        int dayCount = dayCountArr[m_month];
        if (isRuinian(m_year) && m_month == 2) dayCount++;
        if (m_day > dayCount)m_day = dayCount;
      }
      break;
    case ST_DAY:
      {
        int dayCount = dayCountArr[m_month];
        if (isRuinian(m_year) && m_month == 2) dayCount++;
        if (up)m_day = m_day >= dayCount ? 1 : m_day + 1;
        else m_day = m_day <= 1 ? dayCount : m_day - 1;
      }
      break;
    case ST_HOUR:
      {
        if (up)m_hour = m_hour >= 24 ? 0 : m_hour + 1;
        else m_hour = m_hour <= 0 ? 24 : m_hour - 1;
      }
      break;
    case ST_MINUTE:
      {
        if (up)m_minute = m_minute >= 59 ? 0 : m_minute + 1;
        else m_minute = m_minute <= 0 ? 59 : m_minute - 1;
      }
      break;
    case ST_YEAR:
      {
        if (up)m_year = m_year >= 2099 ? 2000 : m_year + 1;
        else m_year = m_year <= 2000 ? 2099 : m_year - 1;
      }
      break;
    default:;
  }
}

void tailNumberCheck(int *_tailNumber, RestrictTailNumberType *_tailNumberType)
{
  RestrictTailNumberType tailNumberType = RTNT_NONE;
  for (int i = 0; i < 5; i++)
  {
    if (digitalRead(i + 2) == LOW)
    {
      tailNumberType = i;
      break;
    }
  }
  *_tailNumberType = tailNumberType;
  if (tailNumberType == RTNT_NONE) {
    *_tailNumber = -1;
    return;
  }
  if (digitalRead(7) == LOW)  *_tailNumber = tailNumber[tailNumberType][1];
  else *_tailNumber = tailNumber[tailNumberType][0];
}

String dateNumberFormat(int _num)
{
  if (_num < 10) return "0" + String(_num);
  else return String(_num);
}

void dateDisplay(int _y, int _M, int _d, int _w, int _h, int _m , SetType _setType)
{
  int left = 0, top = 0;
  int setupOffset = 0;
  if (_setType != ST_NONE) setupOffset = 48;
  char dateChar[11];
  String dateString = dateNumberFormat(_M) + dateNumberFormat(_d) + dateNumberFormat(_h) + dateNumberFormat(_m) + dateNumberFormat(_y % 100);
  dateString.toCharArray(dateChar, 11);
  display.setTextColor(SSD1306_WHITE);
  display.drawBitmap(left + 24 + setupOffset, top + 0, str_yue, 16, 16, 1);
  display.drawBitmap(left + 64 + setupOffset, top + 0, str_ri, 16, 16, 1);
  if (_setType == ST_NONE)
  {
    display.drawBitmap(left + 96, top + 0, str_zhou, 16, 16, 1);
    display.drawBitmap(left + 112, 0, str_zhoushu[_w], 16, 16, 1);
  } else
  {
    display.drawBitmap(left - 16 + setupOffset, top + 0, str_nian, 16, 16, 1);
    display.setCursor(left + 60, top + 17);     // Start at top-left corner
    display.write(':');
  }
  int numberOffset[5][2] = {{0, 1}, {41, 1}, { -12, 17}, {24, 17}, { -40, 1}};
  for (int t = 0; t <= 4; t++)
  {
    if (_setType == ST_NONE && t > 1) break;
    if (m_setType == t)
    {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else
    {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(left + setupOffset + numberOffset[t][0], top + numberOffset[t][1]);
    for (int16_t i = 0; i <= 1; i++) {
      display.write(dateChar[t * 2 + i]);
    }
  }
}
void restrictDisplay(int _tailNumber, int _restrictType, SetType _setType)
{
  if (_setType != ST_NONE) return;
  int left = 0;
  int top = 16;
  display.drawBitmap(left + 0, top + 0, str_wei, 16, 16, 1);
  display.drawBitmap(left + 16, top + 0, str_hao, 16, 16, 1);
  display.setCursor(left + 32, top + 1);
  char tailNumberChar[2];
  String(_tailNumber).toCharArray(tailNumberChar, 2);
  display.write(tailNumberChar[0]);
  switch (_restrictType)
  {
    case RTNT_0_5:
    case RTNT_1_6:
    case RTNT_2_7:
    case RTNT_3_8:
    case RTNT_4_9:
      {
        display.setCursor(left + 67, top + 1);
        display.write(str_xxType[_restrictType][0]);
        display.setCursor(left + 83, top + 1);
        display.write(str_xxType[_restrictType][1]);
      }
      break;
    case RTNT_ODD_NUMBER:
      {
        display.drawBitmap(left + 65, top + 0, str_dan, 16, 16, 1);
        display.drawBitmap(left + 81, top + 0, str_hao, 16, 16, 1);
      }
      break;
    case RTNT_EVEN_NUMBER:
      {
        display.drawBitmap(left + 65, top + 0, str_shuang, 16, 16, 1);
        display.drawBitmap(left + 81, top + 0, +str_hao, 16, 16, 1);
      }
      break;
    case RTNT_NONE:
      {
        display.drawBitmap(left + 81, top + 0, +str_bu, 16, 16, 1);
      }
      break;
  }
  display.drawBitmap(left + 97, top + 0, str_xian, 16, 16, 1);
  display.drawBitmap(left + 113, top + 0, str_xing, 16, 16, 1);
}
