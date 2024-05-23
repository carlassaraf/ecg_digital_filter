import numpy as np
import dearpygui.dearpygui as dpg
import serial.tools.list_ports
import time

class ECGPlotter():

    def __init__(self, width, height):

        self._width = width
        self._heigth = height

        # Configurar DearPyGui
        dpg.create_context()
        dpg.create_viewport(title='ECG Plotter', width=width, height=height)

        # Puerto serial seleccionado
        self._port = None

        # Crear la ventana de selección de puerto serial
        with dpg.window(label="App", tag="app_window"):

            with dpg.child_window(tag="serial_window", width=self._width, height=(self._heigth // 4)):
                dpg.add_text("Seleccione un puerto serial:")
                
                # Listar los puertos seriales disponibles y crear el combo box
                ports = [port.device for port in serial.tools.list_ports.comports()]

                if ports:
                    dpg.add_combo(ports, label="Puertos", callback=self._port_selected_callback, tag="serial_combo")
                else:
                    dpg.add_text("No se encontraron puertos seriales.")

            # Configuro una ventana para el ploteo de la FFT
            with dpg.child_window(tag="fft_window", width=self._width, height=(2 * self._heigth // 4)):
                with dpg.plot(label="FFT Plot", height=-1, width=-1):
                    dpg.add_plot_axis(dpg.mvXAxis, label="Frequency [Hz]")
                    y_axis = dpg.add_plot_axis(dpg.mvYAxis, label="Magnitude [V]")
                    
                    dpg.add_line_series([], [], label="FFT (real)", parent=y_axis, tag="fft_real")
                    dpg.add_line_series([], [], label="FFT (filtrada)", parent=y_axis, tag="fft_filtered")

        # Conectar el callback de redimensionamiento
        dpg.set_viewport_resize_callback(self._resize_window_callback)

        dpg.setup_dearpygui()
        dpg.show_viewport()

        # Configurar el tamaño inicial de la ventana
        self._resize_window_callback(None, None)


    def run(self):
        # Actualizar el gráfico cada 100 ms
        while dpg.is_dearpygui_running():
            self._update_plot()
            dpg.render_dearpygui_frame()
            time.sleep(0.1)

        dpg.cleanup_dearpygui()


    def _update_plot(self):
        """
        Actualiza la informacion del ploteo
        """
        # Obtengo los puntos de la FFT
        freqs, fft_magnitude = self._generate_fft_data()
        # Actualizar los valores del gráfico
        dpg.set_value('fft_real', [freqs.tolist(), fft_magnitude.tolist()])
        # Obtengo los puntos de la FFT
        freqs, fft_magnitude = self._generate_fft_data()
        # Actualizar los valores del gráfico
        dpg.set_value('fft_filtered', [freqs.tolist(), fft_magnitude.tolist()])

    
    def _resize_window_callback(self, sender, app_data):
        """
        Callback para ajustar el tamaño de la ventana 
        """
        # Obtengo las dimensiones del viewport y las configuro para la ventana
        width, height = dpg.get_viewport_client_width(), dpg.get_viewport_client_height()
        dpg.set_item_width("serial_window", width)
        dpg.set_item_height("serial_window", height // 4)
        dpg.set_item_width("fft_window", width)
        dpg.set_item_height("fft_window", 3 * height // 4)

    
    def _port_selected_callback(self, sender, app_data):
        """
        Obtiene el valor seleccionado del menu desplegable
        """
        self._port = app_data
        # Minimozo y muestro la otra ventana
        dpg.configure_item("serial_window", minimized=True)
        dpg.configure_item("fft_window", minimized=False)

    
    def _generate_fft_data(self, samples: int = 1024, sample_rate: int = 1000) -> tuple:
        """
        Genera muestra aleatoria de datos para la FFT

        Args: 
            samples (int): Cantidad de muestras
            sample_rate (float): Frecuencia de muestreo

        Return:
            Tupla con el array de frecuencias y magnitudes
        """

        t = np.linspace(0, 1, samples, False)  # 1 second
        signal = np.random.normal(0, 1, samples)  # señal aleatoria
        fft_result = np.fft.fft(signal)
        freqs = np.fft.fftfreq(samples, 1/sample_rate)
        
        # Filtrar solo las frecuencias positivas y menores a 100 Hz
        idx = np.where((freqs >= 0) & (freqs <= 100))
        freqs = freqs[idx]
        fft_magnitude = np.abs(fft_result[idx])
        
        return freqs, fft_magnitude
