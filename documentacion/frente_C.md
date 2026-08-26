# Frente C — Segmentación del entorno y generación de rutas

## Objetivo

El Frente C analiza visualmente el entorno, clasifica zonas según
su nivel de seguridad y genera rutas entre puntos utilizando un
algoritmo A* ponderado.

## Entorno actual

La versión actual se ejecuta en Google Colab.

## Tecnologías

- Python
- NumPy
- OpenCV
- Pillow
- Transformers
- PyTorch
- SegFormer

## Modelo de segmentación

Se utiliza:

`nvidia/segformer-b0-finetuned-ade-512-512`

El modelo realiza segmentación semántica del escenario.

## Clasificación del entorno

El procesamiento genera distintas máscaras:

- `mapa_seguro`
- `mapa_precaucion`
- `mapa_inseguro`
- `mapa_estructura`

Las zonas consideradas transitables se utilizan posteriormente
para generar puntos candidatos y rutas.

## Generación de puntos

Se generan puntos sobre las componentes conectadas de las áreas
permitidas, manteniendo una distancia mínima entre ellos.

El número de puntos puede variar entre ejecuciones.

## Mapa de costos

Los costos utilizados actualmente son:

| Zona | Costo |
|---|---:|
| Segura / verde | 1 |
| Precaución / amarilla | 15 |
| Insegura | 999999 |
| Estructura | 999999 |

## Generación de rutas

Se utiliza A* ponderado con movimiento horizontal, vertical y
diagonal.

Las rutas obtenidas se simplifican posteriormente utilizando
`cv2.approxPolyDP()`.

Para evitar superposición excesiva entre rutas, las zonas
utilizadas son penalizadas en un mapa de costos dinámico.

## Salidas

El código genera, entre otros:

- `puntos_separados.jpg`
- `ruta_final.jpg`

## Estado de integración

El desarrollo actual funciona de manera independiente en Google
Colab.

La integración automática con Frente B todavía requiere:

1. implementar o definir un endpoint que reciba el JSON de Frente B;
2. acordar cómo utilizar `gato_x` y `gato_y`;
3. convertir coordenadas normalizadas a las coordenadas utilizadas
   por el algoritmo de rutas;
