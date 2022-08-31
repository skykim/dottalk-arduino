#include <SoftwareSerial.h>

//Bluetooth setting
const int Tx = 11; 
const int Rx = 10;
const int baudrate = 9600;
SoftwareSerial BtSerial(Tx,Rx);

//Braille setting
const int braille_digits = 6;
bool first_character = true;
bool braille_read = false;
bool braille_write = true;
int state = 0;
int digpin[3] = {7,8,9};
int braille[braille_digits] = {1, 0, 0, 0, 0, 0};

//Braille characters
int alphabet[27][braille_digits] = 
{{1, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0}, {1, 0, 0, 1, 0, 0},
 {1, 1, 1, 0, 0, 0}, {1, 1, 1, 1, 0, 0}, {1, 0, 1, 1, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 1, 1, 1, 0, 0},
 {1, 0, 0, 0, 1, 0}, {1, 0, 1, 0, 1, 0}, {1, 1, 0, 0, 1, 0}, {1, 1, 0, 1, 1, 0}, {1, 0, 0, 1, 1, 0},
 {1, 1, 1, 0, 1, 0}, {1, 1, 1, 1, 1, 0}, {1, 0, 1, 1, 1, 0}, {0, 1, 1, 0, 1, 0}, {0, 1, 1, 1, 1, 0},
 {1, 0, 0, 0, 1, 1}, {1, 0, 1, 0, 1, 1}, {0, 1, 1, 1, 0, 1}, {1, 1, 0, 0, 1, 1}, {1, 1, 0, 1, 1, 1},
 {1, 0, 0, 1, 1, 1}, {0, 0, 0, 0, 0, 0}};

int braille_output[26][braille_digits] = 
{{0, 1, 1, 1, 1, 1}, {0, 1, 0, 1, 1, 1}, {0, 0, 1, 1, 1, 1}, {0, 0, 1, 0, 1, 1}, {0, 1, 1, 0, 1, 1},
 {0, 0, 0, 1, 1, 1}, {0, 0, 0, 0, 1, 1}, {0, 1, 0, 0, 1, 1}, {1, 0, 0, 1, 1, 1}, {1, 0, 0, 0, 1, 1},
 {0, 1, 1, 1, 0, 1}, {0, 1, 0, 1, 0, 1}, {0, 0, 1, 1, 0, 1}, {0, 0, 1, 0, 0, 1}, {0, 1, 1, 0, 0, 1},
 {0, 0, 0, 1, 0, 1}, {0, 0, 0, 0, 0, 1}, {0, 1, 0, 0, 0, 1}, {1, 0, 0, 1, 0, 1}, {1, 0, 0, 0, 0, 1},
 {0, 1, 1, 1, 0, 0}, {0, 1, 0, 1, 0, 0}, {1, 0, 0, 0, 1, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 1, 0, 0, 0},
 {0, 1, 1, 0, 0, 0}};


void setup()
{
  pinMode(digpin[0], OUTPUT);
  pinMode(digpin[1], OUTPUT);
  pinMode(digpin[2], OUTPUT);
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(4,INPUT);
  pinMode(6,INPUT);
  pinMode(12,OUTPUT);
  BtSerial.begin(baudrate);
  Serial.begin(baudrate);
}

void loop()
{
  int button = digitalRead(6);
  delay(100);

  if (button==0)
  { 
    if (braille_write == true)
    {
      if(state==0)
      {
        state = 1;
        braille[0]= digitalRead(2); 
        braille[2]= digitalRead(3);  
        braille[4]= digitalRead(4);  
        delay(150); 
      }
      else if(state==1)
      {
        state = 0;
        braille[1]= digitalRead(2); 
        braille[3]= digitalRead(3);  
        braille[5]= digitalRead(4);   
        delay(150);
        
        char letter = (char)(compare(braille, alphabet) - 32);

        if (isValidChar(letter) == true) //진동은 짧게 출력
        {
          setVibrationDuration(200);

          if (letter == '[') // 문자전달 시 점자가 000000이고 "/"전달하고 진동이 길게 출력
          {
            BtSerial.println("/");
            setVibrationDuration(1500);
          }
          else if (letter != '[') //영문자만 폰으로 전달
          {
            BtSerial.println(letter);
          }
        }
        Serial.println(letter);
      }
    }
  }
  if (BtSerial.available() == true) // 점자를 폰에서 받을 때
  { 
    braille_write = false; // 점자 쓰기 모드를 false으로 설정
    
    if(first_character == true)
    {
      setVibrationDuration(-1); // 문자를 폰에서 처음 받을 경우 진동이 계속 발생
      first_character = false;
    }
    
    if (button==0) // 버튼을 누르면 
    {
      braille_read = true; // 점자 읽기 모드로 변환
      setVibrationDuration(0); // 진동모터가 꺼짐
      setAllBrailleDown(); // 점자가 읽기 모드로 변환되어 점자블럭이 내려감.
      delay(500);
    }
    if (braille_read == true) // 점자가 읽기 모드일때 
    {   
      int ch = BtSerial.read(); // 폰에서 전달하는 문자 하나를 읽음.
      
      if (isValidChar(ch) == true) // 영어 문자인지 확인 후
      {
        Serial.write(ch);
        display(ch); // 점자 출력 함수가 수행
      }
      else if (ch == '/') // '/'를 인식하면
      {
        delay(2000);
        Serial.write(ch);    
        Serial.println("");
    
        setAllBrailleUp(); // 점자 블럭이 올라옴.
    
        first_character = true; // 최초문자 전달 변수를 0으로 설정
        braille_read = false; // 점자 읽기 모드를 false로 설정
        braille_write = true; // 점자 쓰기 모드를 true로 설정
      }
    }
  }
}

void setVibrationDuration(int delay_ms)
{
  if (delay_ms == -1)
  {
    digitalWrite(12, HIGH);
  }
  else if (delay_ms == 0)
  {
    digitalWrite(12, LOW);
  }
  else
  {
    digitalWrite(12, HIGH);
    delay(delay_ms);
    digitalWrite(12, LOW);
  }
}

void setAllBrailleUp()
{
  for(int loop_index = 0; loop_index < 3; loop_index++)
  {
    digitalWrite(digpin[loop_index], LOW); // 점자 블럭이 올라옴.
  }
}

void setAllBrailleDown()
{
  for(int loop_index = 0; loop_index < 3; loop_index++)
  {
    digitalWrite(digpin[loop_index], HIGH); // 점자 블럭이 내려감.
  }
}

bool isValidChar(uint8_t ch)
{
  if ((ch >= 'a') && (ch <= 'z'))
    return true;
  else if ((ch >= 'A') && (ch <= 'Z'))
    return true;
  else
    return false;  
}

int getIndexOfChar(uint8_t ch)
{
  int index = -1;
  
  if ((ch >= 'a') && (ch <= 'z'))
    index = ch - 'a';
  else if ((ch >= 'A') && (ch <= 'Z'))
    index = ch - 'A';

  return index;
}

void display(uint8_t ch)
{
  int index = getIndexOfChar(ch);

  if(index >= 0)
  {
    delay(300);
    digitalWrite(digpin[0],braille_output[index][0]);
    digitalWrite(digpin[1],braille_output[index][2]);
    digitalWrite(digpin[2],braille_output[index][4]);

    setVibrationDuration(300);
    
    digitalWrite(digpin[0],HIGH);
    digitalWrite(digpin[1],HIGH);
    digitalWrite(digpin[2],HIGH);
    delay(300);

    digitalWrite(digpin[0],braille_output[index][1]);
    digitalWrite(digpin[1],braille_output[index][3]);
    digitalWrite(digpin[2],braille_output[index][5]);

    setVibrationDuration(300);

    digitalWrite(digpin[0],HIGH);
    digitalWrite(digpin[1],HIGH);
    digitalWrite(digpin[2],HIGH);
    delay(700);
  }
}
  
int compare(int *input, int (*data)[braille_digits])
{
  int letter = -1;
  const int a_ascii_base = 97;
  
  for(int loop_index = 0; loop_index < 27; loop_index++) 
  {
    if (isSameDigits(input, alphabet[loop_index]) == true)
    {
      letter = loop_index;
    }
  }
  Serial.println(a_ascii_base + letter);
  return (a_ascii_base + letter);
}

bool isSameDigits(int *input, int *model) 
{
  for(int loop_index = 0; loop_index < braille_digits; loop_index++)
  {
    if(input[loop_index] != model[loop_index])
    {
      return false;
    }
  }
  return true;
}
