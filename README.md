# Proyecto TDA — Sistema integrado ESP32-CAM, visión y planificación

## Descripción general

Este repositorio reúne el desarrollo e integración de los tres frentes del proyecto de Tecnología Digital Aplicada.

El sistema combina captura de imágenes mediante una ESP32-CAM, procesamiento visual para detectar al gato y generación de rutas o decisiones de movimiento. Cada frente aborda una parte específica del sistema y se comunica con los demás mediante interfaces definidas.

## Frentes del proyecto

### Frente A — Dispositivo físico y actuadores

Responsable de:

- Captura de imágenes mediante ESP32-CAM y cámara OV2640.
- Comunicación Wi-Fi con el Frente B.
- Control del mecanismo PAN-TILT mediante servomotores.
- Control del módulo láser/emisor.
- Ejecución de órdenes provenientes del sistema.

### Frente B — Detección y procesamiento visual

Responsable de:

- Recepción de imágenes JPEG desde el Frente A.
- Detección del gato mediante YOLOv8.
- Obtención de coordenadas normalizadas.
- Estimación de dirección, velocidad y confianza.
- Generación del JSON utilizado para la integración con los demás frentes.

### Frente C — Análisis del entorno y planificación

Responsable de:

- Segmentación semántica del entorno.
- Clasificación de zonas seguras, de precaución y restringidas.
- Generación de puntos candidatos.
- Construcción de mapas de costo.
- Generación de rutas mediante A* ponderado.

## Flujo general

ESP32-CAM (Frente A)
→ imagen JPEG
→ Frente B
→ detección y coordenadas
→ Frente C
→ planificación / decisión
→ Frente A
→ actuación mediante PAN-TILT

## Estado actual

- Captura JPEG mediante ESP32-CAM: validada.
- Comunicación Frente A → Frente B: validada.
- Procesamiento y detección en Frente B: implementado.
- Segmentación y generación de rutas en Frente C: implementada.
- Integración automática Frente B → Frente C: pendiente de validación.
- Integración Frente C → Frente A: pendiente.
- Control físico PAN-TILT: en desarrollo.

## Organización del repositorio

- `firmware/`: firmware correspondiente al Frente A.
- `documentacion/`: documentación técnica e interfaces.
- `documentacion/referencias_integracion/`: códigos de referencia desarrollados por los Frentes B y C.
- `Primeros avances TDA.pdf`: material correspondiente a etapas iniciales del proyecto.

## Nota

El desarrollo principal de este repositorio corresponde al Frente A. Los códigos de los Frentes B y C se incorporan como referencia para documentar y validar la integración completa del sistema.
