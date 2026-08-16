#include <BluetoothSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>

// Configurar Bluetooth
BluetoothSerial SerialBT;

// Definir LCD (dirección 0x27 o 0x3F según el módulo)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definir comunicación con DFPlayer Mini en ESP32 (usando Serial2)
HardwareSerial mySerial(2);
DFRobotDFPlayerMini myDFPlayer;

int volume = 15;          // Nivel de volumen inicial
int currentTrack = 1;     // Pista actual
String inputBuffer = "";  // Buffer para recibir números por Bluetooth
bool isPlaying = true;    // Estado de reproducción (true = reproduciendo, false = pausado)

const char* trackNames[] = {
  "Como le llevo serenata - Enjambre",
  "Lovely Night - LaLaLand",
  "Amenaza - Enjambre",
  "Camara de Faltas - Enjambre",
  "DoDoDo - Dansu"
};
const int totalTracks = sizeof(trackNames) / sizeof(trackNames[0]);

void setup() {
  Serial.begin(115200);
  SerialBT.begin("blancoEsp");  // Nombre del dispositivo Bluetooth
  Serial.println("Bluetooth iniciado, esperando conexión...");

  // Iniciar comunicación con DFPlayer Mini
  mySerial.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17 en ESP32
  if (myDFPlayer.begin(mySerial)) {
    Serial.println("DFPlayer Mini conectado!");
    myDFPlayer.volume(volume);
    myDFPlayer.play(currentTrack);
  } else {
    Serial.println("No se pudo conectar con DFPlayer Mini!");
  }

  // Configurar LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Reproduciendo:");
  lcd.setCursor(0, 1);
  lcd.print(trackNames[currentTrack - 1]);
}

unsigned long lastUpdate = 0;
unsigned long interval = 3000;  // Intervalo de 3 segundos

void loop() {
  // Leer caracteres recibidos por Bluetooth
  if (SerialBT.available()) {
    char receivedChar = SerialBT.read();
    Serial.print("Caracter recibido: ");
    Serial.println(receivedChar);

    if (isDigit(receivedChar)) {
      inputBuffer += receivedChar;
    } 
    else if (receivedChar == '\n' || receivedChar == '\r') {
      if (inputBuffer.length() > 0) {
        int newVolume = inputBuffer.toInt();
        inputBuffer = "";  
        if (newVolume >= 0 && newVolume <= 100) {
          volume = map(newVolume, 0, 100, 0, 30);
          myDFPlayer.volume(volume);
          Serial.print("Nuevo volumen: ");
          Serial.println(volume);
          lcd.setCursor(0, 0);
          lcd.print("Volumen: ");
          lcd.print(newVolume);
          lcd.print("   ");  
        }
      }
    } 
    else {
      switch (receivedChar) {
        case 'P':  // Play
          if (!isPlaying) {  
            myDFPlayer.start(); // Reanuda desde la pausa en lugar de reiniciar la pista
            isPlaying = true;
            actualizarLCD();
          }
          inputBuffer = "";
          break;
        case 'p':  // Pause
          if (isPlaying) {  
            myDFPlayer.pause();
            isPlaying = false;
            lcd.setCursor(0, 0);
            lcd.print("Estado: Pausa ");
            lcd.setCursor(0, 1);
            lcd.print("                "); // Limpiar segunda línea
          }
          inputBuffer = "";
          break;
        case 'S':  // Siguiente canción
          currentTrack = (currentTrack % totalTracks) + 1;
          myDFPlayer.play(currentTrack);
          isPlaying = true;
          actualizarLCD();
          inputBuffer = "";
          break;
        case 'A':  // Canción anterior
          currentTrack = (currentTrack - 2 + totalTracks) % totalTracks + 1;
          myDFPlayer.play(currentTrack);
          isPlaying = true;
          actualizarLCD();
          inputBuffer = "";
          break;
      }
    }
  }

  // Actualizar el LCD solo si está reproduciendo
  if (millis() - lastUpdate > interval && isPlaying) {
    lastUpdate = millis();
    actualizarLCD();
  }
}


void actualizarLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Reproduciendo:");
  lcd.setCursor(0, 1);
  lcd.print(trackNames[currentTrack - 1]);
}
