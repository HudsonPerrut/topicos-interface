# -*- coding: utf-8 -*-

import serial
from flask import Flask, render_template, jsonify

# Configuração da porta serial (ajuste conforme necessário)
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 9600

# Inicializar a comunicação serial
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# Inicializar o Flask
app = Flask(__name__)

# Variável para armazenar o último valor lido da porta serial
latest_value = None

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/data')
def data():
    global latest_value
    if ser.in_waiting > 0:
        latest_value = ser.readline().decode('utf-8', errors='ignore').strip()
        print( latest_value)
    return jsonify(value=latest_value)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
