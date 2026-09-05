# 🔋 Power Path Management Circuit  
## LiPo Charger + Boost Converter with Load Sharing (5V Output)

---

## 📌 Description
This module implements a **power path management system** that allows a device (e.g., Arduino, ESP32) to be powered while simultaneously charging a LiPo battery.

### ✅ Key Features
- Safe battery charging  
- Automatic load sharing between USB input and battery  
- Stable 5V output using a boost converter  

---

## ⚙️ How It Works
- The **TP4056 module** charges the LiPo battery from a 5V USB input  
- A **P-channel MOSFET (DMP1045U)** controls power flow  

### 🔌 Operation Modes
**When USB is connected:**
- Load is powered directly from USB  
- Battery charges simultaneously  

**When USB is disconnected:**
- Battery supplies power to the load  

- The **MT3608 boost converter** steps battery voltage up to a stable **5V output**

---

## 🧩 Components Used
- TP4056 LiPo Charger Module (with protection)  
- LiPo Battery (3.7V)  
- DMP1045U P-Channel MOSFET  
- SB220 Schottky Diode  
- 10kΩ Resistor  
- MT3608 Boost Converter Module  

---

## 📁 Files Included
- **schematics/** → Circuit diagram files  
- **pcb/** → PCB design files (Gerber, layout)  

---

## 🔌 Input & Output
- **Input:** 5V (USB)  
- **Battery:** 3.7V LiPo  
- **Output:** Regulated 5V  

---

## ⚠️ Notes
- Adjust **Rprog** on TP4056 to set charging current  
- Adjust **MT3608** to get exact 5V output before connecting load  
- Ensure proper heat dissipation for high current usage  

---

## 🧠 Applications
- Arduino & Embedded Systems  
- Portable electronics  
- IoT devices with battery backup  

---

## 👨‍💻 Author
**Lasindu Viduranga**  
📧 wplvidu@gmail.com  

**Team Member**  
Rescue Mesh