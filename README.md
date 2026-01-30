# Soyabell 🥛🔔

**Smart Notification for Soyabella Milk Maker**

Soyabell is an ESP32-based IoT device that listens for the specific "beep" frequency of a Soyabella milk maker and sends an SMS notification when your milk is ready. It utilizes FFT (Fast Fourier Transform) for precise audio analysis and connects to your home WiFi to trigger alerts via the VoIP.ms API.

---

## ✨ Features

* **Audio Recognition:** Uses an I2S microphone and FFT to detect the specific 2360Hz beep pattern of the Soyabella machine.
* **WiFi Connectivity:** Connects to your home network to send alerts.
* **Captive Portal Configuration:** No hardcoding! If WiFi fails or on first boot, it launches a configuration hotspot (`Soyabell-Setup`) where you can enter your WiFi credentials, API keys, and phone numbers.
* **SMS Notifications:** Sends a text message ("Soymilk is Ready") to your phone via VoIP.ms when the cycle is complete.
* **Visual Status:** Uses the onboard LED to indicate detection (blinking) and status.

---

## 🛠️ Hardware Requirements

* **ESP32 Development Board** (e.g., ESP32 DevKit V1)
* **I2S Microphone** (e.g., INMP441)
    * **SD** -> GPIO 32
    * **WS** -> GPIO 33
    * **SCK** -> GPIO 25
* **USB Power Supply**

---

## 🚀 Installation & Setup

### 1. Build and Flash
1.  Clone this repository.
2.  Open the project in **PlatformIO** (VS Code).
3.  Ensure the `ESP32WiFi` library is accessible (in `lib/` or installed globally).
4.  Build and Upload to your ESP32.

### 2. Configuration (First Run)
1.  Power on the Soyabell.
2.  On your phone or computer, search for a WiFi network named **`Soyabell-Setup`**.
3.  Connect to it. A "Sign In to Network" window should appear (or go to `192.168.4.1` in your browser).
4.  Enter the following details:
    * **WiFi SSID & Password**
    * **VoIP.ms API User & Password**
    * **Source Number (DID)**
    * **Destination Number** (Your cell phone)
    * **Time Zone**
5.  Click **Save**. The device will restart and connect to your WiFi.

---

## 🧠 How It Works

1.  **Audio Sampling:** The ESP32 continuously samples audio at 16kHz using the I2S microphone.
2.  **Frequency Analysis:** An onboard FFT (Fast Fourier Transform) analyzes the audio spectrum.
3.  **Detection Logic:**
    * It looks for a strong peak at **2360Hz** (±100Hz).
    * It checks the Signal-to-Noise Ratio (SNR) to ignore background noise.
    * It counts sequential "beeps" to confirm the pattern (debouncing).
4.  **Trigger:** Once the threshold (5 beeps) is reached, it triggers the `sendSms()` function.
5.  **Cooldown:** The system sleeps for 60 seconds after a notification to prevent spamming.

---

## 📚 Dependencies

This project relies on the following libraries:
* **ESP32WiFi** (Local framework for WiFi/Config)
* **arduinoFFT** (for audio analysis)
* **ESPAsyncWebServer** & **AsyncTCP**

---

## 📜 License

This project is open-source.
