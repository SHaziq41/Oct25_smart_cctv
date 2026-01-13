# Oct25_smart_cctv


# Smart Surveillance and Monitoring System (oneM2M-based)

This project implements a **secure smart surveillance and monitoring system** using IoT devices, the **oneM2M standard (MOBIUS)**, and **Node-RED**.  
The system focuses on **access-controlled CCTV viewing**, **motion detection**, and **centralized IoT data management**.

##  Team Members
1. Shaiful Haziq Hidzmi bin Saipol Azmi
2. Muhammad Uwais bin Mohd Imran
3. Sharifah Nur Qistina Nabila binti Syed Abd Rani
4. Nur Aishah Kamaliah binti Mezlan
---

##  Project Overview

The system monitors an environment using:
- An **ESP32 with ultrasonic sensor** for distance detection
- An **LED alert mechanism** for visual warning
- An **ESP32-CAM** for live CCTV streaming

IoT data is managed using **oneM2M (MÖBIUS)**, while **Node-RED** serves as the application layer to handle:
- User access requests
- Admin approval / denial
- Dashboard visualization
- Alert notifications

CCTV access is **blocked by default** and only unlocked after **admin approval**.

---

##  System Architecture

**Main components:**
- **Edge Devices**
  - ESP32 + Ultrasonic Sensor + LED
  - ESP32-CAM (CCTV)
- **Middleware**
  - oneM2M platform (MÖBIUS)
- **Application Layer**
  - Node-RED Dashboard
- **Testing Tool**
  - Postman (REST API testing)

---

##  Hardware Setup

- ESP32 microcontroller
- Ultrasonic distance sensor
- LED (alert indicator)
- ESP32-CAM module
- Local Area Network (LAN)

**Functionality:**
- Ultrasonic sensor detects objects within a danger threshold
- LED flashes when alert condition is met
- ESP32 sends distance and alert data to MÖBIUS
- ESP32-CAM provides live video stream

---

##  oneM2M (MÖBIUS) Usage

MOBIUS is used as the **oneM2M Common Services Entity (CSE)**.

**Resource structure:**
- Application Entity (AE): `SmartSurveillance`
- Containers:
  - `distance` – ultrasonic sensor readings
  - `alarm` – alert status (0 / 1)
- ContentInstances store individual sensor and alert values

Data is transmitted using RESTful HTTP requests.

---

##  API Testing (Postman)

Postman is used to test and validate oneM2M REST APIs.

**Example request:**

POST /Mobius/Smartsurveillance/distance


**Payload:**
```json
{
  "m2m:cin": {
    "con": "75.3"
  }
}



