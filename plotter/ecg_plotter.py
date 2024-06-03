import dearpygui.dearpygui as dpg
import serial.tools.list_ports
import serial
import time
import json

class ECGPlotter():

    def __init__(self, width, height):

        self._width = width
        self._heigth = height

        # Configurar DearPyGui
        dpg.create_context()
        dpg.create_viewport(title='ECG Plotter', width=width, height=height)

        # Puerto serial seleccionado
        self._port = None

        # Datos para mostrar
        self._freqs = [0.0]
        self._fft_real= [0.0]
        self._fft_filtered = [0.0]
        self._time = [0.0]
        self._ifft_real = [0.0]
        self._ifft_filtered = [0.0]

        # Crear la ventana de selección de puerto serial
        with dpg.window(label="App", tag="app_window"):

            with dpg.child_window(tag="serial_window", width=self._width, height=(self._heigth // 5)):
                dpg.add_text("Seleccione un puerto serial:")
                
                # Listar los puertos seriales disponibles y crear el combo box
                ports = [port.device for port in serial.tools.list_ports.comports()]

                if ports:
                    dpg.add_combo(ports, label="Puertos", callback=self._port_selected_callback, tag="serial_combo")
                else:
                    dpg.add_text("No se encontraron puertos seriales.")

                dpg.add_text("", tag="serial_status")

            # Configuro una ventana para el ploteo de la FFT
            with dpg.child_window(tag="fft_window", width=self._width, height=(3 * self._heigth // 7)):
                with dpg.plot(label="FFT Plot", height=-1, width=-1, tag="fft_plot"):
                    x_axis = dpg.add_plot_axis(dpg.mvXAxis, label="Frequency [Hz]", tag="freq_axis")
                    y_axis = dpg.add_plot_axis(dpg.mvYAxis, label="Magnitude [V]", tag="fft_axis")
                    
                    dpg.add_line_series([], [], label="FFT (real)", parent=y_axis, tag="fft_real")
                    dpg.add_line_series([], [], label="FFT (filtrada)", parent=y_axis, tag="fft_filtered")

                    # Fijar los límites de los ejes
                    dpg.set_axis_limits("freq_axis", 0, 250)
                    dpg.set_axis_limits("fft_axis", 0, 2)

                    # Muestro la etiqueta
                    dpg.add_plot_legend(parent="fft_plot")
            

            # Configuro una ventana para el ploteo de la FFT
            with dpg.child_window(tag="ifft_window", width=self._width, height=(3 * self._heigth // 7)):
                with dpg.plot(label="IFFT Plot", height=-1, width=-1, tag="ifft_plot"):
                    x_axis = dpg.add_plot_axis(dpg.mvXAxis, label="Time [s]", tag="time_axis")
                    y_axis = dpg.add_plot_axis(dpg.mvYAxis, label="Magnitude [V]", tag="ifft_axis")
                    
                    dpg.add_line_series([], [], label="IFFT (real)", parent=y_axis, tag="ifft_real")
                    dpg.add_line_series([], [], label="IFFT (filtrada)", parent=y_axis, tag="ifft_filtered")

                    # Fijar los límites de los ejes
                    dpg.set_axis_limits("ifft_axis", -2.5, 2.5)

                    # Muestro la etiqueta
                    dpg.add_plot_legend(parent="ifft_plot")

        # Conectar el callback de redimensionamiento
        dpg.set_viewport_resize_callback(self._resize_window_callback)

        dpg.setup_dearpygui()
        dpg.show_viewport()

        # Configurar el tamaño inicial de la ventana
        self._resize_window_callback(None, None)


    def run(self):
        # Actualizar el gráfico cada 100 ms
        while dpg.is_dearpygui_running():
            if self._port:
                if self._port.in_waiting > 0:
                    # Leo y decodifico el JSON
                    data = self._port.readline().decode().strip()
                    data = json.loads(data)
                    # Veo si hay datos
                    self._freqs = data.get("freqs", self._freqs)
                    self._fft_real = data.get("fft_real", self._fft_real)
                    self._fft_filtered = data.get("fft_filtered", self._fft_filtered)
                    self._time = data.get("time", self._time)
                    self._ifft_real = data.get("ifft_real", self._ifft_real)
                    self._ifft_filtered = data.get("ifft_filtered", self._ifft_filtered)

            self._update_plot()
            self._refresh_ports()
            dpg.render_dearpygui_frame()

            time.sleep(0.1)

        dpg.cleanup_dearpygui()


    def _update_plot(self):
        """
        Actualiza la informacion del ploteo
        """
        # Actualizar los valores del gráfico
        dpg.set_value("fft_real", [self._freqs, self._fft_real])
        dpg.set_value("fft_filtered", [self._freqs, self._fft_filtered])
        dpg.set_value("ifft_real", [self._time, self._ifft_real])
        dpg.set_value("ifft_filtered", [self._time, self._ifft_filtered])

    
    def _resize_window_callback(self, sender, app_data):
        """
        Callback para ajustar el tamaño de la ventana 
        """
        # Obtengo las dimensiones del viewport y las configuro para la ventana
        width, height = dpg.get_viewport_client_width(), dpg.get_viewport_client_height()
        dpg.set_item_width("serial_window", width)
        dpg.set_item_height("serial_window", height // 7)
        dpg.set_item_width("fft_window", width)
        dpg.set_item_height("fft_window", 3 * height // 7)
        dpg.set_item_width("ifft_window", width)
        dpg.set_item_height("ifft_window", 3 * height // 7)

    
    def _refresh_ports(self):
        # Listar los puertos seriales disponibles y crear el combo box
        ports = [port.device for port in serial.tools.list_ports.comports()]
        if ports:
            dpg.configure_item("serial_combo", items=ports)
        else:
            dpg.configure_item("serial_combo", items=["No se encontraron puertos seriales."])

    
    def _port_selected_callback(self, sender, app_data):
        """
        Obtiene el valor seleccionado del menu desplegable
        """
        if self._port:
            self._port.close()
            self._port = None
        else:
            try:
                self._port = serial.Serial(app_data, 115200)
                dpg.set_value(item="serial_status", value=f"Puerto {app_data} conectado con exito!")
            except:
                dpg.set_value(item="serial_status", value="Error conectando al puerto!")
