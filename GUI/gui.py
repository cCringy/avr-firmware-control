import tkinter as tk
from tkinter import ttk, messagebox
import serial
from serial.tools import list_ports

BAUD = 9600

# Alle Pins des ATmega328P, gruppiert nach Port
AVAILABLE_PINS = (
    [f"PB{i}" for i in range(8)] +   # PB0–PB7
    [f"PC{i}" for i in range(7)] +   # PC0–PC6
    [f"PD{i}" for i in range(8)]     # PD0–PD7
)

def get_ports():
    return [p.device for p in list_ports.comports()]

def refresh_ports():
    ports = get_ports()
    port_combo["values"] = ports
    if ports:
        port_combo.current(0)
    else:
        port_var.set("")

def send_settings():
    port = port_var.get()
    if not port:
        messagebox.showwarning("Kein Port", "Bitte einen COM-Port auswählen.")
        return

    cmd = (
        f"SET TEMP {temp_var.get()} LIGHT {light_var.get()} PID {pid_var.get()}\n"
        f"SET PIN RELAY {relay_pin_var.get()}\n"
        f"SET PIN SERVO {servo_pin_var.get()}\n"
        f"SET PIN LDR {ldr_pin_var.get()}\n"
        f"SET PIN DHT {dht_pin_var.get()}\n"
    )
    try:
        with serial.Serial(port, BAUD, timeout=1) as ser:
            ser.write(cmd.encode())
    except serial.SerialException as e:
        messagebox.showerror("Verbindungsfehler", f"Konnte nicht senden:\n{e}")

root = tk.Tk()
root.title("Room Automation Config")

temp_var = tk.DoubleVar(value=22.0)
light_var = tk.IntVar(value=500)
pid_var = tk.DoubleVar(value=1.0)
port_var = tk.StringVar()

relay_pin_var = tk.StringVar(value="PB1")
servo_pin_var = tk.StringVar(value="PB2")
ldr_pin_var = tk.StringVar(value="PC0")
dht_pin_var = tk.StringVar(value="PD2")

# --- Port-Auswahl ---
ttk.Label(root, text="COM-Port").grid(row=0, column=0, padx=5, pady=5, sticky="w")
port_combo = ttk.Combobox(root, textvariable=port_var, state="readonly", width=15)
port_combo.grid(row=0, column=1, sticky="w")
ttk.Button(root, text="Aktualisieren", command=refresh_ports).grid(row=0, column=2, padx=5)

# --- Settings ---
ttk.Label(root, text="Zieltemperatur (°C)").grid(row=1, column=0, padx=5, pady=5, sticky="w")
ttk.Entry(root, textvariable=temp_var).grid(row=1, column=1)

ttk.Label(root, text="Lichtschwelle (LDR)").grid(row=2, column=0, padx=5, pady=5, sticky="w")
ttk.Entry(root, textvariable=light_var).grid(row=2, column=1)

ttk.Label(root, text="PID Gain").grid(row=3, column=0, padx=5, pady=5, sticky="w")
ttk.Entry(root, textvariable=pid_var).grid(row=3, column=1)

# --- Pin-Zuordnung ---
ttk.Separator(root, orient="horizontal").grid(row=4, column=0, columnspan=3, sticky="ew", pady=8)
ttk.Label(root, text="Pin-Zuordnung", font=("", 9, "bold")).grid(row=5, column=0, columnspan=3, sticky="w", padx=5)

ttk.Label(root, text="Relay").grid(row=6, column=0, padx=5, pady=3, sticky="w")
ttk.Combobox(root, textvariable=relay_pin_var, values=AVAILABLE_PINS, state="readonly", width=8).grid(row=6, column=1, sticky="w")

ttk.Label(root, text="Servo (PID-Aktuator)").grid(row=7, column=0, padx=5, pady=3, sticky="w")
ttk.Combobox(root, textvariable=servo_pin_var, values=AVAILABLE_PINS, state="readonly", width=8).grid(row=7, column=1, sticky="w")

ttk.Label(root, text="LDR (ADC)").grid(row=8, column=0, padx=5, pady=3, sticky="w")
ttk.Combobox(root, textvariable=ldr_pin_var, values=AVAILABLE_PINS, state="readonly", width=8).grid(row=8, column=1, sticky="w")

ttk.Label(root, text="DHT11").grid(row=9, column=0, padx=5, pady=3, sticky="w")
ttk.Combobox(root, textvariable=dht_pin_var, values=AVAILABLE_PINS, state="readonly", width=8).grid(row=9, column=1, sticky="w")

ttk.Button(root, text="Senden", command=send_settings).grid(row=10, column=0, columnspan=3, pady=10)

# Beim Start einmal Ports laden
refresh_ports()

root.mainloop()