# Asignación de GPIO

## Pines utilizados

| GPIO | Uso |
|---:|---|
| 12 | Servo PAN |
| 13 | Servo TILT |
| 4 | Láser / emisor |

## Observaciones

GPIO 12 es un strapping pin de la ESP32. Se utiliza en esta versión del prototipo, pero debe evitarse que el periférico fuerce un nivel lógico incompatible durante el arranque.

GPIO 4 también está asociado al flash integrado de la ESP32-CAM. En la versión final se utiliza como señal de control del emisor.

La tarjeta microSD no se utiliza en esta versión.
