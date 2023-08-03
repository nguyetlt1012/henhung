#include <IRremote.hpp>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define ONE_WIRE_BUS 04
#define BLOCK_SIZE 4
#define rows 32           // SCREEN_WIDTH / BLOCK_SIZE
#define cols 16           // SCREEN_HEIGHT / BLOCK_SIZE
#define MAX_SNAKE_SIZE 50 // Assuming only of size 50 - low memory

const int IR_RECEIVE_PIN = 13;
const int pinLed1 = 18;
const int pinLed2 = 5;
const int pinLed3 = 19;

// snake head
uint8_t snake_x = BLOCK_SIZE * 12;
uint8_t snake_y = BLOCK_SIZE * 6;
// snake body
uint8_t snake_body[MAX_SNAKE_SIZE][2];
uint8_t snake_length = 2;
// snake moving directions
struct
{
  const uint8_t LEFT = 0, UP = 1, RIGHT = 2, DOWN = 3;
} Direction;
// current snake direction
uint8_t dir = Direction.RIGHT;

// food
uint8_t food_x = BLOCK_SIZE * 10, food_y = BLOCK_SIZE * 10;
int8_t velocity_x = 0, velocity_y = 0;

// game over
bool game_over = false;
bool playing = false;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long lastTime = millis();
//=================================================
// current direction input from joystick
void new_food_position()
{
  food_x = random(0, rows) * BLOCK_SIZE;
  food_y = random(0, cols) * BLOCK_SIZE;
  for (uint8_t i = 0; i < snake_length; i++)
  {
    if (snake_body[i][0] == food_x && snake_body[i][1] == food_y)
      new_food_position();
  }
}
void update()
{

  // snake head move
  snake_x += velocity_x * BLOCK_SIZE;
  snake_y += velocity_y * BLOCK_SIZE;

  // snake eats food
  if (snake_x == food_x && snake_y == food_y)
  {
    snake_body[snake_length][0] = food_x;
    snake_body[snake_length][1] = food_y;
    new_food_position();
    snake_length++;
    // play buzzer
    // tone(BUZZER, BUZZER_FREQ, BUZZER_DUR);
  }

  // change direction
  if (dir == Direction.UP && velocity_y != 1)
  {
    velocity_x = 0;
    velocity_y = -1;
  }
  else if (dir == Direction.DOWN && velocity_y != -1)
  {
    velocity_x = 0;
    velocity_y = 1;
  }
  else if (dir == Direction.LEFT && velocity_x != 1)
  {
    velocity_x = -1;
    velocity_y = 0;
  }
  else if (dir == Direction.RIGHT && velocity_x != -1)
  {
    velocity_x = 1;
    velocity_y = 0;
  }

  // snake body move
  for (int8_t i = snake_length - 1; i > 0; i--)
  {
    snake_body[i][0] = snake_body[i - 1][0];
    snake_body[i][1] = snake_body[i - 1][1];
  }
  snake_body[0][0] = snake_x;
  snake_body[0][1] = snake_y;

  // game over condition
  if (snake_x >= rows * BLOCK_SIZE || snake_x <= 0 || snake_y >= cols * BLOCK_SIZE || snake_y <= 0)
  {
    game_over = true;
  }
  for (uint8_t i = 0; i < snake_length; i++)
  {
    if (i == 0)
      continue;
    if (snake_body[i][0] == snake_x && snake_body[i][1] == snake_y)
      game_over = true;
  }
}
uint8_t get_direction(uint8_t data)
{

  Serial.print("snake");

  if (data == 3125149440)
    return Direction.LEFT;
  if (data == 3108437760)
    return Direction.UP;
  if (data == 3141861120)
    return Direction.RIGHT;
  if (data == 3208707840)
    return Direction.DOWN;

  return dir;
}

void wellcome_screen()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(5, 20);
  display.print("SNAKE GAME");

  display.setCursor(10, 40);
  display.setTextSize(1);
  display.print("(press any button)");
  display.display();
}
void gameover_screen()
{
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(18, 20);
  display.print("!! GAME OVER !!");

  display.setTextSize(1);
  display.setCursor(40, 38);
  Serial.print(snake_length);
  display.print("score: " + String(snake_length));
  display.display();
}
void draw_snake()
{
  for (uint8_t i = 0; i < snake_length; i++)
  {
    display.fillRect(snake_body[i][0], snake_body[i][1], BLOCK_SIZE, BLOCK_SIZE, WHITE);
  }
}
void render()
{
  display.clearDisplay();
  // border
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
  // food
  display.fillRect(food_x, food_y, BLOCK_SIZE, BLOCK_SIZE, WHITE);
  // snake
  draw_snake();
  display.display();
}

void setup()
{
  Serial.begin(9600);
  Serial.print("hello");
  pinMode(pinLed1, OUTPUT);
  pinMode(pinLed2, OUTPUT);
  pinMode(pinLed3, OUTPUT);

  digitalWrite(pinLed1, HIGH);
  digitalWrite(pinLed2, HIGH);
  digitalWrite(pinLed3, HIGH);
  // Khởi động bộ thu
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  // setup ui
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  wellcome_screen();
}
//=================================================
void loop()
{
  if (IrReceiver.decode())
  {
    // Serial.print("xoo");
    uint32_t dataRemote = IrReceiver.decodedIRData.decodedRawData;
    if (dataRemote > 0)
    {
      Serial.println(dataRemote);
      if (millis() - lastTime > 250)
      {
        switch (dataRemote)
        {
        case 4077715200: // so1
          digitalWrite(pinLed1, !digitalRead(pinLed1));
          break;
        case 3877175040: // so2
          digitalWrite(pinLed2, !digitalRead(pinLed2));
          break;
        case 2707357440: // so3
          digitalWrite(pinLed3, !digitalRead(pinLed3));
          break;
        case 4144561920: // phím 4 Tắt hết
          digitalWrite(pinLed1, LOW);
          digitalWrite(pinLed2, LOW);
          digitalWrite(pinLed3, LOW);
          break;
        case 3810328320: // Phím 5 Bật hết
          digitalWrite(pinLed1, HIGH);
          digitalWrite(pinLed2, HIGH);
          digitalWrite(pinLed3, HIGH);
          break;
        case 3158572800:
          playing = true;
          // update();
          // render();
          break;
        case 3141861120: //prev
          dir = Direction.LEFT;
          break;
        case 3108437760: //ch
          dir = Direction.UP;
          break;
        case 3208707840: //next
          dir = Direction.RIGHT;
          break;
        case 3125149440: //ch-
          dir = Direction.DOWN;
          break;
        }
      }
    }
    lastTime = millis();
    IrReceiver.resume(); // Cho phép nhận giá trị tiếp theo
  }
  if (playing)
  {
    if (game_over)
    {
      gameover_screen();
      game_over = false;
      snake_length = 1;
      snake_x = BLOCK_SIZE * 12;
      snake_y = BLOCK_SIZE * 6;
      dir = Direction.RIGHT;
      velocity_x = 1;
      velocity_y = 0;
      new_food_position();
      game_over = false;
      playing = false;
    }
    else{
      update();
      delay(100);
      render();
      delay(100);
    }
  }
}
