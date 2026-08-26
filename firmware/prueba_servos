
include <ESP32Servo.h>

Servo servoX; // Pin GPIO 12 (Izquierda / Derecha)
Servo servoY; // Pin GPIO 13 (Arriba / Abajo)

int tiempoPaso = 20; 

void moverLento(Servo &servo, int inicio, int fin) {
  if (inicio < fin) {
    for (int pos = inicio; pos <= fin; pos++) {
      servo.write(pos);
      delay(tiempoPaso);
    }
  } else {
    for (int pos = inicio; pos >= fin; pos--) {
      servo.write(pos);
      delay(tiempoPaso);
    }
  }
}

void setup() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  servoX.setPeriodHertz(50);
  servoY.setPeriodHertz(50);

  // Se asignan los pines seguros del ESP32-CAM: GPIO 12 y GPIO 13
  servoX.attach(12, 1000, 2000);
  servoY.attach(13, 1000, 2000);
}

void loop() {
  // Movimiento Eje X (Izquierda / Derecha)
  moverLento(servoX, 0, 180);
  delay(1000);
  moverLento(servoX, 180, 0);
  delay(1000);

  // Movimiento Eje Y (Arriba / Abajo)
  moverLento(servoY, 0, 180);
  delay(1000);
  moverLento(servoY, 180, 0);
  delay(1000);
}
