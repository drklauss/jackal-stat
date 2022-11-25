#include <Arduino.h>
#include <avr/power.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>
#include <GyverButton.h>

#define LED_R 6 // питание светодиода
#define LED_G 9 // питание светодиода
#define LED_B 10 // питание светодиода
#define BTN 2
#define BUZ 5

#define WHITE_MAN 0      // индекс белого в списке раздумий
#define YELLOW_MAN 1     // индекс желтого в списке раздумий
#define BLACK_MAN 2      // индекс черного в списке раздумий
#define RED_MAN 3        // индекс красного в списке раздумий
#define SLOWPOKE_TIME 30 // время, через которое напомнить тугодуму о необходимости ходить

bool isPause = false;     // признак паузы
int totalSteps = 0;       // общее кол-во шагов за игру
int thinkingTime = 0;     // время, которое думает текущий игрок
int thinking = WHITE_MAN; // кто сейчас ходит

int durations[4] = {0, 0, 0, 0};                                 // длительность раздумий
const char *names[4] = {"Белый", "Желтый", "Черный", "Красный"}; // список названий

// "Всю жизнь будишь палубу драить!",
// "Тысяча вонючих китов!!!",
// "Прощелыга подкильная, не видать тебе сокровищ!",
// "Чтоб осьминог выпил весь ром в твоих кишках!"
// "Три тысячи акул мне в глотку!",
// "Недоумок палубный!!!",
// "Разрази меня гром!!!",
// "Кошку в пятки!",
// "Фок-грот-брамсель мне в левое ухо!",
// "Греметь тебе вечность якорями!",
// "Мачту тебе в зад!",
// "Чтобы морская болезнь высосала тебе мозги!!!",
// "Бушприт твою в компАс!",
// "Портовая крыса!",
// "Кальмарьи кишки!",
// "Полных парусов и сухого пороха!",
// "Три тысячи чертей на румбу!!!",
// "В пасть морскому дьяволу!!",
// "Абордажный лом с хреном во все дыры 333 раза",
// "Червь Гальюнный!",
// "Зелень подкильная!",
// "Сейчас Задницу порву на щупальца осьминога!!!",
// "Греметь тебе вечность якорями!",
// "Что ещё за пресноводных моллюсков притащило?!",
// "Потный пожиратель рыбьих потрахов!",
// "Тысяча вонючих китов!!!",
// "Чтоб мне сблевать ядовитой медузой!!!",
// "Нам Смекаешь?!",
// "Спятил?Слава Богу, нормальный не пошёл бы на это",
// "Всем ни с места! Я обронил мозги",
// "Ты капитан? Нынче назначают кого ни попадя!",
// "Все это видели? Ибо я отказываюсь это повторять!",
// "Если ты ждал подходящего момента...то это был он",
// "Куда это вечно исчезает ром?",

const unsigned char pirate[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc,
    0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x00, 0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f,
    0xff, 0xff, 0xf0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x07, 0xff, 0xff, 0xfc, 0x00,
    0x00, 0x67, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x73, 0xff, 0xff, 0xff, 0x00, 0x00, 0xf9, 0xff, 0xff,
    0xff, 0x00, 0x01, 0xfc, 0xff, 0xff, 0xff, 0x00, 0x01, 0xfe, 0x3f, 0xff, 0xff, 0x00, 0x01, 0xff,
    0x1f, 0xff, 0xfe, 0x00, 0x03, 0xff, 0x87, 0xff, 0xfc, 0x40, 0x03, 0xff, 0xe1, 0xff, 0xf8, 0xc0,
    0x03, 0xff, 0xf8, 0x7f, 0xe1, 0xc0, 0x07, 0xf9, 0xfe, 0x00, 0x03, 0xe0, 0x07, 0xf0, 0xff, 0x80,
    0x07, 0xe0, 0x07, 0xf0, 0x7f, 0xc0, 0x07, 0xe0, 0x07, 0xf0, 0x03, 0xc0, 0x07, 0xe0, 0x07, 0xf0,
    0x03, 0xc0, 0x07, 0xe0, 0x07, 0xf8, 0x07, 0xc0, 0x0f, 0xe0, 0x01, 0xfc, 0x0f, 0xe0, 0x0f, 0x80,
    0x00, 0x7f, 0xff, 0xf0, 0x1e, 0x00, 0x00, 0x3f, 0xfe, 0x78, 0x3c, 0x00, 0x00, 0x1f, 0xfe, 0x3f,
    0xf8, 0x00, 0x00, 0x0f, 0xfe, 0x3f, 0xf0, 0x00, 0x00, 0x07, 0xfc, 0x3f, 0xe0, 0x00, 0x00, 0x03,
    0xfc, 0x1f, 0xe0, 0x00, 0x00, 0x03, 0xfc, 0x1f, 0xc0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xc0, 0x00,
    0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
    0x00, 0x00, 0x00, 0x01, 0xc0, 0x01, 0x80, 0x00, 0x00, 0x01, 0xc0, 0x73, 0x80, 0x00, 0x00, 0x01,
    0xc0, 0x7b, 0xc0, 0x00, 0x00, 0x01, 0xc0, 0x73, 0x80, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x60, 0x00, 0x00, 0x07, 0xc0, 0x03, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff,
    0xf0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00,
    0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

Adafruit_SSD1306 display(128, 64, &Wire, -1);
GButton btn(BTN);

/**
 * Функция преобразования русских букв
 * */
String utf8rus(String source)
{
  int i, k;
  String target;
  unsigned char n;
  char m[2] = {'0', '\0'};
  k = source.length();
  i = 0;
  while (i < k)
  {
    n = source[i];
    i++;
    if (n >= 0xC0)
    {
      switch (n)
      {
      case 0xD0:
      {
        n = source[i];
        i++;
        if (n == 0x81)
        {
          n = 0xA8;
          break;
        }
        if (n >= 0x90 && n <= 0xBF)
          n = n + 0x30;
        break;
      }
      case 0xD1:
      {
        n = source[i];
        i++;
        if (n == 0x91)
        {
          n = 0xB8;
          break;
        }
        if (n >= 0x80 && n <= 0x8F)
          n = n + 0x70;
        break;
      }
      }
    }
    m[0] = n;
    target = target + String(m);
  }
  return target;
}

void isr()
{
  btn.tick();
}

/**
 * Зажигает нужный светодиод в зависимости от того, чей ход
 * */
void flashLED(int led)
{
  analogWrite(LED_R, 0);
  analogWrite(LED_G, 0);
  analogWrite(LED_B, 0);
  switch (led)
  {
  case WHITE_MAN:
    analogWrite(LED_R, 25);
    analogWrite(LED_G, 25);
    analogWrite(LED_B, 25);
    break;
  case YELLOW_MAN:
    analogWrite(LED_R, 25);
    analogWrite(LED_G, 25);
    analogWrite(LED_B, 0);
    break;
  case BLACK_MAN:
    analogWrite(LED_R, 10);
    analogWrite(LED_G, 0);
    analogWrite(LED_B, 25);
    break;
  case RED_MAN:
    analogWrite(LED_R, 25);
    analogWrite(LED_G, 0);
    analogWrite(LED_B, 0);
    break;
  }
}
/**
 * Инициализация
 */
void setup()
{
  clock_prescale_set(clock_div_2); // делит частоту на 2, все таймеры работают в 2 раха медленнее
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);
  attachInterrupt(0, isr, CHANGE);
  btn.setDebounce(20);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    for (;;)
      ;
  }
  display.clearDisplay();
  display.dim(true);
  display.setTextSize(1);
  display.cp437(true);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  flashLED(WHITE_MAN);
}

/**
 *  Обработчик одинарного клика
 */
void handleClick()
{
  if (!btn.isClick())
  {
    return;
  }
  if (isPause)
  {
    return;
  }
  tone(BUZ, 4000, 50);
  // Переход хода
  if (thinking >= 3)
  {
    thinking = 0;
  }
  else
  {
    thinking++;
  }
  totalSteps++;

  flashLED(thinking);
}

/**
 *  Обработчик удерживания кнопки
 */
void handleHold()
{
  if (!btn.isHold())
  {
    return;
  }
  isPause = !isPause;
}

/**
 * Отображение паузы
 **/
void showPause()
{
  display.fillRect(0, 0, 128, 16, SSD1306_BLACK);
  display.setTextSize(2);
  display.setCursor(32, 0);
  display.print(utf8rus("Пауза"));
  display.setTextSize(1);
  display.display();
}
/**
 * Напоминание тугодуму о том, что пора ходить
 **/
void slowPoke()
{
  if (thinkingTime < SLOWPOKE_TIME)
  {
    return;
  }
  tone(BUZ, 3000, 30);
  delay(15);
  tone(BUZ, 3250, 30);
  delay(15);
  tone(BUZ, 3500, 30);
  delay(15);
  tone(BUZ, 3750, 30);
  delay(15);
  tone(BUZ, 4000, 30);
  thinkingTime = 0;
}

/**
 * Основная функция отображения статистики
 * */
void showMain()
{
  display.clearDisplay();
  display.drawBitmap(80, 16, pirate, 48, 48, WHITE);
  int y = 8;
  int h = 0;
  int t = 0;
  int m = 0;
  int s = 0;
  int total = 0;
  for (int i = 0; i < 4; i++)
  {
    display.setCursor(0, y * i + 16);
    display.print(utf8rus(names[i]));
    display.print(": ");

    t = durations[i];

    m = t / 60;
    if (m < 10)
    {
      display.print("0");
    }
    display.print(m);
    display.print(":");
    s = t - 60 * m;
    if (s < 10)
    {
      display.print("0");
    }
    display.print(s);
    total += t;
  }
  // Количество шагов
  display.setCursor(0, 0);
  display.print(utf8rus("Шагов: "));
  display.print(totalSteps);
  // Всего времени
  display.setCursor(0, 8);
  display.print(utf8rus("Всего: "));
  h = total / 3600;
  display.print("0");
  display.print(h);
  display.print(":");

  m = (total - (h * 3600)) / 60;
  if (m < 10)
  {
    display.print("0");
  }
  display.print(m);
  display.print(":");
  s = (total - (h * 3600)) - m * 60;
  if (s < 10)
  {
    display.print("0");
  }
  display.print(s);
  display.display();
}

/**
 * Основной цикл
 * */
void loop()
{
  static unsigned long diff = 0;
  btn.tick();

  handleClick();
  handleHold();

  if (millis() - diff < 500)
  {
    return;
  }
  static int lastThinking = 0;

  if (isPause)
  {
    showPause();
    delay(250);
    return;
  }
  showMain();
  slowPoke();

  if (lastThinking != thinking)
  {
    thinkingTime = 0;
  }else{
    thinkingTime++;
  }
  durations[thinking]++;
  lastThinking = thinking;
  diff = millis();
}
