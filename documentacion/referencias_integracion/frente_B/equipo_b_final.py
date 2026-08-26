import cv2
import numpy as np
import os
import math
import time
import requests

from fastapi import FastAPI, Request, HTTPException
from ultralytics import YOLO
import uvicorn


# ============================================================
# CONFIGURACION
# ============================================================

MODELO = "yolov8n.pt"
CLASE_GATO = 15
CONFIANZA_MINIMA = 0.35

CARPETA_CAPTURAS = "ultimas_5_capturas"
MAX_CAPTURAS = 5

ANCHO_FRAME = 640
ALTO_FRAME = 480

# C esta corriendo en el mismo PC
URL_EQUIPO_C = "http://127.0.0.1:9000/datos-gato"

os.makedirs(CARPETA_CAPTURAS, exist_ok=True)

modelo = YOLO(MODELO)

app = FastAPI()

contador = 0

ultimo_x = None
ultimo_y = None
ultimo_tiempo = None

velocidad = 0.0
direccion = "desconocida"


# ============================================================
# DIRECCION
# ============================================================

def calcular_direccion(dx, dy):

    dx = float(dx)
    dy = float(dy)

    if abs(dx) < 0.01 and abs(dy) < 0.01:
        return "quieto"

    if abs(dx) > abs(dy):
        return "derecha" if dx > 0 else "izquierda"

    return "abajo" if dy > 0 else "arriba"


# ============================================================
# PROCESAR IMAGEN
# ============================================================

def procesar_frame(frame):

    global ultimo_x
    global ultimo_y
    global ultimo_tiempo
    global velocidad
    global direccion

    alto, ancho = frame.shape[:2]

    resultados = modelo.track(
        frame,
        persist=True,
        classes=[CLASE_GATO],
        conf=CONFIANZA_MINIMA,
        verbose=False
    )

    mejor_caja = None
    mejor_confianza = 0.0

    if resultados and resultados[0].boxes is not None:

        for caja in resultados[0].boxes:

            conf = float(caja.conf[0])

            if conf > mejor_confianza:
                mejor_confianza = conf
                mejor_caja = caja


    # ========================================================
    # GATO DETECTADO
    # ========================================================

    if mejor_caja is not None:

        x1, y1, x2, y2 = mejor_caja.xyxy[0].cpu().numpy()

        x1 = float(x1)
        y1 = float(y1)
        x2 = float(x2)
        y2 = float(y2)

        cx = (x1 + x2) / 2.0
        cy = (y1 + y2) / 2.0

        gato_x = float(cx / ancho)
        gato_y = float(cy / alto)

        ahora = float(time.time())

        if (
            ultimo_x is not None
            and ultimo_y is not None
            and ultimo_tiempo is not None
        ):

            dt = float(ahora - ultimo_tiempo)

            if dt > 0:

                dx = float(gato_x - ultimo_x)
                dy = float(gato_y - ultimo_y)

                velocidad = float(
                    math.sqrt(dx ** 2 + dy ** 2) / dt
                )

                direccion = calcular_direccion(dx, dy)

        ultimo_x = float(gato_x)
        ultimo_y = float(gato_y)
        ultimo_tiempo = float(ahora)


        # ====================================================
        # DIBUJAR RECUADRO
        # ====================================================

        cv2.rectangle(
            frame,
            (int(x1), int(y1)),
            (int(x2), int(y2)),
            (0, 255, 0),
            2
        )

        textos = [
            "Gato: SI",
            f"Confianza: {mejor_confianza:.2f}",
            f"X: {gato_x:.2f} Y: {gato_y:.2f}",
            f"Direccion: {direccion}",
            f"Velocidad: {velocidad:.3f}"
        ]

        y = 25

        for texto in textos:

            cv2.putText(
                frame,
                texto,
                (10, y),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (0, 255, 0),
                2
            )

            y += 25


        resultado = {
            "gato_detectado": True,
            "gato_x": float(round(gato_x, 4)),
            "gato_y": float(round(gato_y, 4)),
            "velocidad": float(round(velocidad, 4)),
            "direccion": str(direccion),
            "confianza": float(round(mejor_confianza, 4))
        }


    # ========================================================
    # GATO NO DETECTADO
    # ========================================================

    else:

        cv2.putText(
            frame,
            "GATO NO DETECTADO",
            (10, 30),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.7,
            (0, 0, 255),
            2
        )

        resultado = {
            "gato_detectado": False,
            "gato_x": None,
            "gato_y": None,
            "velocidad": 0.0,
            "direccion": "desconocida",
            "confianza": 0.0
        }

    return frame, resultado


# ============================================================
# ENVIAR RESULTADO A EQUIPO C
# ============================================================

def enviar_a_c(resultado):

    try:

        respuesta = requests.post(
            URL_EQUIPO_C,
            json=resultado,
            timeout=3
        )

        if respuesta.status_code == 200:

            print()
            print("B -> C: datos enviados correctamente")

            try:
                print("Respuesta C:")
                print(respuesta.json())
            except:
                pass

        else:

            print()
            print(
                "B -> C: C respondio con error:",
                respuesta.status_code
            )

            print(respuesta.text)


    except requests.exceptions.ConnectionError:

        print()
        print(
            "B -> C: no se pudo conectar con C."
        )

        print(
            "Comprueba que equipo_c.py este ejecutandose."
        )


    except requests.exceptions.Timeout:

        print()
        print(
            "B -> C: tiempo de espera agotado."
        )


    except Exception as e:

        print()
        print(
            "B -> C: error inesperado:",
            e
        )


# ============================================================
# GUARDAR Y MOSTRAR ULTIMAS 5
# ============================================================

def mostrar_y_guardar(frame):

    global contador

    posicion = contador % MAX_CAPTURAS
    numero = posicion + 1

    ruta = os.path.join(
        CARPETA_CAPTURAS,
        f"captura_{numero}.jpg"
    )

    cv2.imwrite(ruta, frame)

    vista = cv2.resize(
        frame,
        (400, 300)
    )

    nombre = f"Captura {numero}"

    cv2.imshow(
        nombre,
        vista
    )

    posiciones = [
        (0, 0),
        (410, 0),
        (820, 0),
        (205, 340),
        (615, 340)
    ]

    x, y = posiciones[posicion]

    cv2.moveWindow(
        nombre,
        x,
        y
    )

    cv2.waitKey(1)

    contador += 1


# ============================================================
# ENDPOINT B
# ============================================================

@app.get("/")
def inicio():

    return {
        "equipo": "B",
        "estado": "activo",
        "equipo_c": URL_EQUIPO_C
    }


@app.post("/frame")
async def recibir_frame(request: Request):

    datos = await request.body()

    if not datos:

        raise HTTPException(
            status_code=400,
            detail="No se recibio imagen"
        )


    imagen_np = np.frombuffer(
        datos,
        dtype=np.uint8
    )


    frame = cv2.imdecode(
        imagen_np,
        cv2.IMREAD_COLOR
    )


    if frame is None:

        raise HTTPException(
            status_code=400,
            detail="Imagen invalida"
        )


    # Todas las imagenes quedan del mismo tamaño

    frame = cv2.resize(
        frame,
        (ANCHO_FRAME, ALTO_FRAME)
    )


    # Procesar con YOLO

    frame_procesado, resultado = procesar_frame(
        frame
    )


    # Mostrar las ultimas 5

    mostrar_y_guardar(
        frame_procesado
    )


    # ========================================================
    # NUEVO:
    # MANDAR AUTOMATICAMENTE RESULTADO A C
    # ========================================================

    enviar_a_c(resultado)


    # Seguimos respondiendo tambien a A
    return resultado


# ============================================================
# EJECUTAR
# ============================================================

if __name__ == "__main__":

    print()
    print("==============================")
    print(" EQUIPO B ACTIVO")
    print("==============================")

    print()
    print("Esperando imagenes de A...")

    print()
    print(
        "Resultados se enviaran a:"
    )

    print(
        URL_EQUIPO_C
    )

    print()


    uvicorn.run(
        app,
        host="0.0.0.0",
        port=8000
    )
