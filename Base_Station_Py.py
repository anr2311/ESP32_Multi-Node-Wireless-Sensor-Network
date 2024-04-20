import serial
import thingspeak
import time
from flask import Flask, render_template
from datetime import datetime

app = Flask(__name__)

# Define the serial port and baud rate
serial_port = 'COM3'  # Adjust the port based on your ESP32's serial connection
baud_rate = 115200  # Adjust the baud rate if necessary

temperature_global = 0
altitude_global = 0
pressure_global = 0
person_count_global = 0
light_intensity_global = 0
sound_intensity_global = 0

fan_suggestion_global = 0
light_suggestion_global = 0
television_suggestion_global = 0
AC_suggestion_global = 0

count = 0

# Sample data for the first set of values
environmental_data = {
    "Temperature (°C)": temperature_global,
    "Altitude (m)": altitude_global,
    "Pressure (Pa)": pressure_global,
    "Human Presence": "Yes" if person_count_global == 1 else "No",
    "Light Intensity (%)": light_intensity_global,
    "Sound Intensity (%)": sound_intensity_global
}

# Sample data for the second set of values
status_data = {
    "Fan": "OFF" if fan_suggestion_global == 0 else "ON",
    "Lights": "OFF" if light_suggestion_global == 0 else "ON",
    "Television": "OFF" if television_suggestion_global == 0 else "ON",
    "A/C": "OFF" if AC_suggestion_global == 0 else "ON"
}

@app.route('/')
def display_table():
    return render_template('table.html', environmental_data=environmental_data, status_data=status_data)

if __name__ == '__main__':

    try:
        # Open the serial port
        ser = serial.Serial(serial_port, baud_rate)

        channel = thingspeak.Channel(id=123, api_key='ABCD') # Replace with required ThingSpeak Channel ID and API Write Key
        
        # Read data from the serial port indefinitely   
        while True:
            now = datetime.now()

            # Format the current time to only include hours
            current_hours = int(now.strftime("%H"))

            # Read one line of data from the serial port
            line = ser.readline().decode().strip()
            
            # Split the line by ':'
            parts = line.split(':')
            
            # Check if the line contains "Temperature"
            if len(parts) == 2 and parts[0].strip() == "Temperature":
                try:
                    # Parse the temperature value
                    temperature = float(parts[1].strip())
                    
                    # Print the temperature value
                    print("Temperature as recorded in PySerial:", temperature)

                    temperature_global = temperature
                    
                    response = channel.update({'field1': temperature})
                    
                except ValueError:
                    print("Error parsing temperature value")

            # Check if the line contains "Altitude"
            if len(parts) == 2 and parts[0].strip() == "Altitude":
                try:
                    # Parse the altitude value
                    altitude = float(parts[1].strip())
                    
                    # Print the altitude value
                    print("Altitude as recorded in PySerial:", altitude)

                    altitude_global = altitude

                    response = channel.update({'field3': altitude})
                    
                except ValueError:
                    print("Error parsing altitude value")

            # Check if the line contains "Pressure"
            if len(parts) == 2 and parts[0].strip() == "Pressure":
                try:
                    # Parse the pressure value
                    pressure = float(parts[1].strip())
                    
                    # Print the pressure value
                    print("Pressure as recorded in PySerial:", pressure)

                    pressure_global = pressure

                    response = channel.update({'field2': pressure})
                    
                except ValueError:
                    print("Error parsing pressure value")

            # Check if the line contains "Person in Room"
            if len(parts) == 2 and parts[0].strip() == "Person":
                try:
                    # Parse the person_count value
                    person_count = int(parts[1].strip())
                    
                    # Print the person_count value
                    print("Person in Room as recorded in PySerial:", person_count)

                    person_count_global = person_count

                    response = channel.update({'field6': person_count})
                    
                except ValueError:
                    print("Error parsing Person in Room value")                
                        

            # Check if the line contains "Sound Intensity"
            if len(parts) == 2 and parts[0].strip() == "Sound":
                try:
                    # Parse the pressure value
                    sound_intensity = float(parts[1].strip())
                    
                    # Print the pressure value
                    print("Sound Intensity as recorded in PySerial:", sound_intensity)

                    sound_intensity_global = sound_intensity

                    response = channel.update({'field5': sound_intensity})
                    
                except ValueError:
                    print("Error parsing Sound Intensity value")

            # Check if the line contains "Light Intensity"
            if len(parts) == 2 and parts[0].strip() == "Light":
                try:
                    # Parse the pressure value
                    light_intensity = float(parts[1].strip())
                    
                    # Print the pressure value
                    print("Light Intensity as recorded in PySerial:", light_intensity)

                    light_intensity_global = light_intensity

                    response = channel.update({'field4': light_intensity})
                    
                except ValueError:
                    print("Error parsing Light Intensity value")
            
            # Print the received data
            print(line)

            # Update the thingspeak fields
            print("Data updated to thingspeak server")

            environmental_data["Temperature (°C)"] = temperature_global
            environmental_data["Altitude (m)"] = altitude_global
            environmental_data["Pressure (Pa)"] = pressure_global
            environmental_data["Human Presence"] = person_count_global
            environmental_data["Light Intensity (%)"] = light_intensity_global
            environmental_data["Sound Intensity (%)"] = sound_intensity_global

            # lights suggestion
            if current_hours > 18 and person_count_global < 1 and light_intensity_global > 70:
                light_suggestion_global = 0
            elif current_hours > 18 and person_count_global > 0 and light_intensity_global < 30:
                light_suggestion_global = 1
            else:
                light_suggestion_global = 0

            # television suggestion
            if person_count_global < 1 and sound_intensity_global > 30:
                television_suggestion_global = 0
            elif person_count_global > 0:
                television_suggestion_global = 1
            else:
                television_suggestion_global = 0

            # fan suggestion
            if person_count_global > 0 and temperature_global > 30:
                fan_suggestion_global = 1
            else:
                fan_suggestion_global = 0

            # AC suggestion
            if person_count_global > 0 and temperature_global > 40:
                AC_suggestion_global = 1
            else:
                AC_suggestion_global = 0

            status_data["A/C"] = "ON" if AC_suggestion_global == 1 else "OFF"
            status_data["Fan"] = "ON" if fan_suggestion_global == 1 else "OFF"
            status_data["Lights"] = "ON" if light_suggestion_global == 1 else "OFF"
            status_data["Television"] = "ON" if television_suggestion_global == 1 else "OFF"

            # Run the Flask app on localhost and loop indefinitely
            count = count + 1

            # Wait for some serial data to accumulate
            if count == 30:
                count = 0
                app.run(host='0.0.0.0', port=8080)

    except serial.SerialException as e:
        print("Serial port error:", e)
