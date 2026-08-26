# Frente B — Procesamiento de imágenes y detección del gato

## Objetivo

El Frente B recibe las imágenes JPEG capturadas por la ESP32-CAM
del Frente A y realiza el procesamiento visual necesario para
detectar la presencia, posición y movimiento aproximado del gato.

## Tecnologías

- Python
- FastAPI
- Uvicorn
- OpenCV
- NumPy
- Ultralytics YOLOv8
- Requests

## Modelo de detección

Se utiliza:

`yolov8n.pt`

El procesamiento se restringe a la clase correspondiente a gato
y utiliza una confianza mínima de 0.35.

## Entrada desde Frente A

Método:

`POST`

Endpoint:

`/frame`

Puerto:

`8000`

Contenido:

`image/jpeg`

La imagen se recibe en formato binario y posteriormente se
redimensiona internamente a 640×480 píxeles.

## Procesamiento

Para cada imagen recibida:

1. Se decodifica el JPEG mediante OpenCV.
2. Se redimensiona la imagen a 640×480.
3. Se ejecuta YOLO para detectar gatos.
4. En caso de múltiples detecciones, se selecciona la de mayor confianza.
5. Se calcula el centro de la caja delimitadora.
6. Las coordenadas X e Y se normalizan entre 0 y 1.
7. Se compara la posición actual con la detección anterior para
   estimar velocidad y dirección.
8. Se genera un JSON con el resultado.

## Respuesta con detección

```json
{
  "gato_detectado": true,
  "gato_x": 0.5203,
  "gato_y": 0.4916,
  "velocidad": 0.0,
  "direccion": "desconocida",
  "confianza": 0.8187
}
