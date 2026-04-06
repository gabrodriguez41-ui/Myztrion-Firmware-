# Myztrion Firmware

Myztrion es una solución de automatización basada en **Raspberry Pi Pico / RP2040** que permite usar la placa como un **PLC programable desde Python**. El proyecto combina un **firmware en C/CMake** que corre en la Pico con una **librería host en Python** que detecta el dispositivo por USB, genera comandos automáticamente a partir del firmware y facilita el control de entradas, salidas, ADC, PWM y motores paso a paso.

La idea principal es simple: conectar la Pico al computador, instanciar `Myztrion()` desde Python y comenzar a interactuar con el hardware en pocos pasos.

---

## Tabla de contenido

- [Características principales](#características-principales)
- [Arquitectura del proyecto](#arquitectura-del-proyecto)
- [Estructura actual del repositorio](#estructura-actual-del-repositorio)
- [Funciones disponibles](#funciones-disponibles)
- [Requisitos](#requisitos)
- [Instalación y puesta en marcha](#instalación-y-puesta-en-marcha)
- [Compilación del firmware](#compilación-del-firmware)
- [Uso desde Python](#uso-desde-python)
- [Ejemplos incluidos](#ejemplos-incluidos)
- [Flujo interno de comunicación](#flujo-interno-de-comunicación)
- [Aplicaciones como PLC](#aplicaciones-como-plc)
- [Estado actual del repositorio](#estado-actual-del-repositorio)
- [Mejoras recomendadas](#mejoras-recomendadas)
- [Licencia](#licencia)

---

## Características principales

- Control de **GPIO digital** como entrada, salida, alta impedancia y resistencias internas.
- Secuencias rápidas de salida con `gpio_out_seq` para generar patrones temporizados.
- Lectura de **ADC** con soporte de transmisión por bloques.
- Adquisición continua con enfoque **asíncrono**, usando callbacks.
- Soporte de **PWM** para generación de señales y control básico de servos.
- Soporte para **motores paso a paso**, incluyendo inicialización, movimiento y consulta de estado.
- Comunicación por **USB CDC** entre la Raspberry Pi Pico y el programa en Python.
- Descubrimiento automático del dispositivo conectado.
- Generación automática de la API de Python a partir del análisis del firmware en C.
- Enfoque pensado para **automatización, laboratorio, prototipado y control tipo PLC** de bajo costo.

---

## Arquitectura del proyecto

Myztrion se compone de tres bloques principales:

### 1. Firmware embebido
El firmware está implementado en **C** y se compila con **CMake** usando el **Raspberry Pi Pico SDK**. Corre directamente sobre la Pico y expone una tabla de comandos binarios que el host puede invocar.

### 2. Librería host en Python
La librería `Myztrion.py` se encarga de:

- detectar la placa conectada por USB,
- validar compatibilidad de firmware,
- analizar el firmware para construir métodos dinámicos,
- enviar comandos binarios,
- recibir reportes,
- despachar respuestas síncronas o callbacks asíncronos.

### 3. Backend USB desacoplado
El archivo `usb_backend_process.py` mueve la recepción y el envío USB a un **proceso separado**, con hilos internos dedicados. Esto reduce los problemas derivados del **GIL de Python** cuando se trabaja con tasas altas de adquisición.

---

## Estructura actual del repositorio

La estructura visible actual del repositorio es plana en la raíz, con los archivos principales del firmware, la librería Python y los ejemplos en el mismo nivel:

```text
.
├── CMakeLists.txt
├── LICENSE
├── Myztrion.c
├── Myztrion.h
├── Myztrion.py
├── README.md
├── c_code_parser.py
├── pico_sdk_import.cmake
├── usb_backend_process.py
├── hello_world_myztrion.py
├── example_ADC_async_myztrion.py
├── example_ADC_sync_plot_myztrion.py
├── example_gpio_on_change_myztrion.py
├── example_gpio_seq_myztrion.py
├── example_pwm_myztrion.py
├── example_stepper_myztrion.py
└── example_steppers_groupedXY_myztrion.py
```

### Descripción de archivos

#### `Myztrion.c`
Archivo principal del firmware. Define la tabla de comandos, el ciclo principal, la lógica de recepción de paquetes, la transmisión de reportes y el arranque general del sistema.

#### `Myztrion.h`
Cabecera principal del firmware. Contiene definiciones globales, incluida la versión del firmware.

#### `CMakeLists.txt`
Script de compilación para construir el firmware con el SDK de Raspberry Pi Pico.

#### `Myztrion.py`
Interfaz principal del lado del host. Aquí se crea la clase `Myztrion`, se detecta el dispositivo y se construye la API de comandos disponible para el usuario.

#### `c_code_parser.py`
Analiza el firmware en C para extraer la tabla de comandos, estructuras y documentación. Su propósito es **autogenerar los métodos de Python** de manera consistente con el firmware.

#### `usb_backend_process.py`
Gestiona la comunicación USB en un proceso aparte para mejorar robustez y rendimiento.

#### `hello_world_myztrion.py`
Ejemplo básico con interfaz gráfica Tkinter. Permite comprobar conexión e interactuar con el LED integrado.

#### `example_ADC_async_myztrion.py`
Ejemplo avanzado de adquisición ADC por bloques, usando callbacks y visualización.

#### `example_ADC_sync_plot_myztrion.py`
Ejemplo de lectura ADC de forma más directa y graficación simple.

#### `example_gpio_on_change_myztrion.py`
Ejemplo de monitoreo de cambios de estado en un GPIO con callback.

#### `example_gpio_seq_myztrion.py`
Ejemplo de secuencias de salida digital temporizadas.

#### `example_pwm_myztrion.py`
Ejemplo de configuración PWM, útil para generación de señal o control de servos.

#### `example_stepper_myztrion.py`
Ejemplo individual de inicialización y movimiento de un motor paso a paso.

#### `example_steppers_groupedXY_myztrion.py`
Ejemplo de movimiento coordinado de dos steppers, pensado para ejes agrupados tipo XY.

---

## Funciones disponibles

A partir de la tabla de mensajes del firmware, el sistema expone estas funciones principales:

- `identify`
- `gpio_out`
- `gpio_in`
- `gpio_on_change`
- `gpio_highz`
- `gpio_pull`
- `gpio_out_seq`
- `adc`
- `adc_stop`
- `pwm_configure_pair`
- `pwm_set_value`
- `stepper_init`
- `stepper_move`
- `stepper_status`

### Resumen funcional

#### Identificación
Permite consultar la identidad del dispositivo y verificar que la comunicación está activa.

#### Entradas y salidas digitales
Se puede configurar el estado de los GPIO, leer entradas, activar resistencias pull y poner pines en alta impedancia.

#### Secuencias digitales
`gpio_out_seq` permite aplicar patrones binarios con retardos muy cortos, útil para señales de control o pruebas rápidas.

#### ADC
La función `adc` permite adquirir muestras analógicas por bloques. El diseño del proyecto apunta a adquisición rápida y continua, con procesamiento asíncrono en el host.

#### PWM
Se pueden configurar pares PWM y cambiar su valor. Esto sirve tanto para señales periódicas como para control básico de actuadores tipo servo.

#### Steppers
Las funciones para stepper permiten inicializar el motor, moverlo a una posición y consultar su estado. Los ejemplos muestran además uso de finales de carrera y coordinación de varios ejes.

---

## Requisitos

### Hardware

- Raspberry Pi Pico o compatible.
- Cable USB de datos.
- Sensores, actuadores o drivers, según el caso de uso.
- Para motores paso a paso: driver externo adecuado.
- Para señales analógicas: acondicionamiento según el rango de tensión requerido.

### Software para firmware

- CMake 3.12 o superior.
- Raspberry Pi Pico SDK.
- Toolchain compatible para compilar firmware de la Pico.

### Software para Python

- Python 3.
- `pyserial`
- `tkinter` para algunos ejemplos.
- `numpy` y `matplotlib` para ejemplos de ADC con gráficas.

Instalación sugerida:

```bash
pip install pyserial numpy matplotlib
```

---

## Instalación y puesta en marcha

### 1. Clonar el repositorio

```bash
git clone https://github.com/gabrodriguez41-ui/Myztrion-Firmware-.git
cd Myztrion-Firmware-
```

### 2. Compilar y cargar el firmware en la Raspberry Pi Pico

Compila el firmware con el SDK de Pico y copia el binario/UF2 resultante a la placa en modo bootloader.

### 3. Conectar la placa al computador

Una vez cargado el firmware, conecta la Pico por USB. La librería Python buscará un dispositivo compatible por VID/PID de Raspberry Pi.

### 4. Ejecutar un ejemplo

```bash
python hello_world_myztrion.py
```

O bien:

```bash
python example_pwm_myztrion.py
python example_stepper_myztrion.py
python example_ADC_async_myztrion.py
```

---

## Compilación del firmware

El archivo `CMakeLists.txt` muestra que el proyecto:

- usa `project(Myztrion)`,
- compila `Myztrion.c`,
- habilita `stdio` por USB,
- desactiva `stdio` por UART,
- enlaza módulos de hardware del SDK,
- está configurado por defecto para la tarjeta `pico`,
- deja comentada la opción `pico2` para una placa más nueva.

### Flujo típico de compilación

```bash
mkdir build
cd build
cmake ..
make -j4
```

> Ajusta este flujo según tu entorno y según cómo tengas instalado el Pico SDK.

---

## Uso desde Python

### Conexión básica

```python
import Myztrion

rp = Myztrion.Myztrion()
print(rp.identify())
```

### Encender el LED integrado

```python
import Myztrion

rp = Myztrion.Myztrion()
rp.gpio_out(gpio=25, value=1)
```

### Leer ADC

```python
import Myztrion

rp = Myztrion.Myztrion()
rv = rp.adc(channel_mask=1, blocksize=100, clkdiv=480)
print(rv)
```

### Configurar PWM

```python
import Myztrion

rp = Myztrion.Myztrion()
rp.pwm_configure_pair(gpio=14, wrap_value=65535, clkdiv=50, clkdiv_int_frac=0)
rp.pwm_set_value(gpio=14, value=100)
```

### Inicializar y mover un stepper

```python
import Myztrion

rp = Myztrion.Myztrion()
zero = rp.stepper_init(0, dir_gpio=12, step_gpio=13, endswitch_gpio=19, inertia=190)
result = rp.stepper_move(0, to=1000000, speed=260)
print(result)
```

---

## Ejemplos incluidos

### `hello_world_myztrion.py`
Muestra una interfaz sencilla con Tkinter. Comprueba conexión con `identify()` y permite prender o apagar el LED integrado.

### `example_pwm_myztrion.py`
Usa PWM con parámetros orientados a un pequeño servo. Es un buen punto de partida para salidas periódicas o posicionamiento simple.

### `example_gpio_on_change_myztrion.py`
Demuestra eventos asíncronos por cambio de estado en un GPIO. También sirve como prueba de rendimiento para reportes de borde.

### `example_gpio_seq_myztrion.py`
Muestra cómo emitir patrones binarios consecutivos con tiempos muy pequeños entre etapas. Esto es útil en automatización rápida o pruebas digitales.

### `example_ADC_sync_plot_myztrion.py`
Realiza una captura ADC y la representa gráficamente de forma simple.

### `example_ADC_async_myztrion.py`
Es el ejemplo más completo para adquisición analógica. Usa callbacks, separación por canales y graficación posterior. También ilustra el uso de DMA y procesamiento paralelo desde el host.

### `example_stepper_myztrion.py`
Inicializa motores paso a paso con pines de dirección, paso y final de carrera; luego ejecuta movimientos y consulta estados.

### `example_steppers_groupedXY_myztrion.py`
Coordina dos steppers como si fueran un sistema XY. El ejemplo usa callbacks para lanzar el siguiente movimiento solo cuando el grupo ha terminado.



