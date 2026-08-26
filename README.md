# Frente A — ESP32-CAM y mecanismo PAN-TILT

## Objetivo
Diseñar e implementar el dispositivo físico encargado de capturar imágenes, comunicarse con el Frente B y ejecutar movimientos mediante un mecanismo PAN-TILT.

## Hardware principal
- ESP32-CAM AI-Thinker
- Cámara OV2640
- 2 servomotores SG90/MG90S
- Módulo láser/emisor
- Fuente externa de 5 V
- Interruptor de paro de emergencia
- Capacitor de desacoplo

## Asignación actual de GPIO

| Función | GPIO |
|---|---:|
| Servo PAN | 12 |
| Servo TILT | 13 |
| Láser / emisor | 4 |

> GPIO 12 es un strapping pin, por lo que debe evitarse forzar un nivel incompatible durante el arranque.

## Estado actual

- [x] Diagrama eléctrico
- [x] Lista de conexiones
- [x] Firmware ESP32-CAM compila
- [x] Captura JPEG
- [x] Conexión Wi-Fi
- [x] Envío HTTP POST al Frente B
- [x] Recepción de JSON
- [ ] Integración física PAN-TILT
- [ ] Prueba de ambos servos
- [ ] Control del láser
- [ ] Paro de emergencia
- [ ] Calibración coordenadas → ángulos
- [ ] Recepción de órdenes finales
