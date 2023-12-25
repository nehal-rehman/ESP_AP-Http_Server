# ESP_AP-Http_Server

Code that runs on ESP32 using ESP-IDF C used IDF 4.4.5, python v3.7.

The firmware consists of a server with the web UI.

The UI have:
• 2 input fields, for example First and Last Name;
• A button (Save/Send)

The IP address of ESP32 ise static for the AP mode (192.168.0.1).
After pressing the button sending of the input field values needs to be done using POST with the string format.

As a result:
- A server, which is available when connecting to the access point of the ESP32, using the static IP address.
- When clicked on the button need to send the POST request with the values of the field (First name, last name) in string and have their values printed in the terminal.
