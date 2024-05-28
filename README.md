# ecg_digital_filter

Implementacion de un filtro digital para un ECG en un RP2040

## Instrucciones para plotter

Este repo incluye una interfaz para ver en "tiempo real" lo muestreado por el microcontrolador y el resultado de la FFT y filtro digital.

### Requisitos

Tener instalado `python 3` con su version mas reciente de `pip`. Podemos instalar Python desde su [pagina](https://www.python.org/) y luego, en una consola escribir:

```bash
python3 -m pip install --upgrade pip
python3 -m pip --version
```

## Entorno virtual

Para esta interfaz, vamos a hacer uso de un entorno virtual para instalar algunos paquetes. Para eso, vamos a abrir una terminal dentro de este repositorio y entrar a `plotter`:

```bash
cd plotter
```

Para crear el entorno virtual podemos escribir:

```bash
python3 -m venv .env
```

Luego, activamos el entorno virtual con:

```bash
source .env/bin/activate
```

Y finalmente, instalamos los paquetes con:

```bash
python3 -m pip install -r requirements.txt
```

Ahora, podemos usar la interfaz corriendo:

```bash
python3 plotter_app.py
```

Cuando hayamos terminado, podemos desactivar el entorno virtual con:

```bash
deactivate
```

### Uso del plotter

Una vez que esté corriendo la interfaz, requeriremos que este conectado el microcontrolador a algún puerto de la computadora. Si éste se encuentra, debemos seleccionarlo del menú desplegable y luego comenzara a mostrarse la información recibida.