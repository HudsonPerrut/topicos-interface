import serial
from flask import Flask, jsonify, render_template_string

app = Flask(__name__)

# Configuração da porta serial (ajuste conforme necessário)
SERIAL_PORT = '/dev/ttyUSB0'  # Substitua pela porta correta, por exemplo, COM3 no Windows
BAUD_RATE = 115200

# Inicializar a comunicação serial
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

@app.route('/')
def index():
    return render_template_string('''
        <!DOCTYPE html>
        <html lang="pt-br">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Temperatura</title>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                    margin: 0;
                    background-color: #f0f0f0;
                }
                .container {
                    display: flex;
                    justify-content: space-around;
                    align-items: center;
                    width: 80%;
                    max-width: 1000px;
                }
                .thermometer-wrapper {
                    display: flex;
                    flex-direction: column;
                    align-items: center;
                    margin: 0 20px; /* Espaço entre os dois termômetros */
                }
                .thermometer {
                    position: relative;
                    width: 80px;
                    height: 300px;
                    background-color: #ddd;
                    border-radius: 40px;
                    overflow: hidden;
                }
                .thermometer .mercury {
                    position: absolute;
                    bottom: 0;
                    width: 100%;
                    background-color: #ff4d4d;
                    text-align: center;
                    color: white;
                    font-weight: bold;
                }
                .thermometer .bulb {
                    position: absolute;
                    bottom: -40px;
                    left: -40px;
                    width: 120px;
                    height: 120px;
                    background-color: #ff4d4d;
                    border-radius: 50%;
                    z-index: -1;
                }
                .temperature {
                    font-size: 24px;
                    font-weight: bold;
                    margin-top: 10px; /* Espaço entre o termômetro e a temperatura */
                }
            </style>
            <script>
                let lastTemp = null;  // Variável para armazenar o último valor de temperatura

                function fetchTemperature() {
                    fetch('/temperature')
                    .then(response => response.json())
                    .then(data => {
                        const temp = parseFloat(data.temp);
                        const temp2 = parseFloat(data.temp2);
                        if (!isNaN(temp)) {  // Verifica se temp é um número válido
                            lastTemp = temp;
                            const mercury = document.querySelector('.mercury');
                            mercury.style.height = (temp * 2) + '%';  // Ajuste conforme a escala desejada
                            document.querySelector('.temperature').textContent = temp + ' °C';
                        }
                        if (!isNaN(temp2)) {  // Verifica se temp2 é um número válido
                            lastTemp2 = temp2;
                            const temp2Mercury = document.querySelector('.temp2-mercury');
                            temp2Mercury.style.height = (temp2 * 2) + '%';  // Ajuste conforme a escala desejada
                            document.querySelector('.temp2').textContent = temp2 + ' °C';
                        }
                    });
                }

                setInterval(fetchTemperature, 5000);  // Atualiza a cada 5 segundos

                // Atualiza a temperatura inicial na carga da página
                document.addEventListener('DOMContentLoaded', fetchTemperature);
            </script>
        </head>
        <body>
            <div class="container">
                <div class="thermometer-wrapper">
                    <div class="thermometer">
                        <div class="temp1-mercury mercury" style="height: 0%;"></div>
                        <div class="bulb"></div>
                    </div>
                    <div class="temperature temp1">Carregando...</div>
                </div>
                <div class="thermometer-wrapper">
                    <div class="thermometer">
                        <div class="temp2-mercury mercury" style="height: 0%;"></div>
                        <div class="bulb"></div>
                    </div>
                    <div class="temperature temp2">Carregando...</div>
                </div>
            </div>
        </body>
        </html>
    ''')

@app.route('/temperature')
def get_temperature():
    temp = None
    temp2 = None

    if ser.in_waiting > 0:
        # Ler uma linha da porta serial
        while temp == None or temp2 == None:
            data = ser.readline().strip()

            # Converter o vetor de bytes em uma string
            string_data = data.decode('utf-8')
            end = string_data[1] + string_data[2]
            if end == '26':
                if len(string_data) > 9:
                    temp = string_data[6] + string_data[7] + '.' + string_data[8] + string_data[9]
            elif end == '42':
                if len(string_data) > 9:
                    temp2 = string_data[6] + string_data[7] + '.' + string_data[8] + string_data[9]

    print(temp, temp2)
    return jsonify(temp=temp, temp2=temp2)

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000)
