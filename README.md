# ⛴️ Ferry Tour Simulation – Multithreaded OS Project

A multithreaded ferry simulation system developed in **C using POSIX threads**, this project models vehicle transportation between two shores. Each vehicle behaves greedily, competing for ferry access under synchronization constraints using **mutexes** and **semaphores**. The ferry adheres to complex scheduling rules, providing a powerful OS-conceptual educational tool.

The project was collaboratively developed by [@berkinyl](https://github.com/berkinyl), [@silanazaslan](https://github.com/silanazaslan), and [@erenyurtcu](https://github.com/erenyurtcu) as part of the **Operating Systems** course at Manisa Celal Bayar University.

---

## 🎯 Project Objective

To build a concurrent, rule-based simulation that demonstrates key OS concepts through real-world modeling:

- ✅ POSIX thread creation and lifecycle management  
- ✅ Mutual exclusion via `pthread_mutex_t`  
- ✅ Semaphore-controlled access via toll booths (`sem_t`)  
- ✅ Greedy vehicle behavior and race-condition handling  
- ✅ Real-time visualization with a printer thread  
- ✅ Trip-level logging and performance statistics  

---

## 🔧 System Components

📁 **Modules**

| File(s)         | Description                                     |
|----------------|-------------------------------------------------|
| `main.c`        | Initializes system and launches all threads     |
| `vehicle.c/h`   | Vehicle thread logic and boarding behavior      |
| `ferry.c/h`     | Core ferry logic, trip management, trip logs    |
| `visual.c/h`    | Real-time ASCII-based system visualization      |
| `trip_log.txt`  | Summary log of ferry trips                      |

🧵 **Threads Used**

- **30 Vehicle Threads** (12 Cars, 10 Minibuses, 8 Trucks)  
- **1 Ferry Thread**  
- **1 Printer Thread**

---

## 📊 Features

- 🚌 **Vehicle Types with Capacity Units**
  - Car → 1 unit
  - Minibus → 2 units
  - Truck → 3 units

- 🚦 **4 Toll Booths (2 per side)**  
  Managed by binary semaphores to serialize vehicle access.

- 🔁 **Round Trip Scheduling**
  - Enforced rules:  
    - 🕳️ **First return trip is empty**  
    - 🧹 **Final trip ensures no vehicle is left**

- 🧠 **Edge Case Handling**
  - Capacity fragmentation
  - Starvation risk
  - Simultaneous access attempts
  - Timeout-based departures

---

## 🖥️ Real-Time Visualization

A dedicated printer thread displays live system state in the terminal every 0.5s:

```
| Side-A |   Ferry   | Side-B |
|  C1    |  MB5 TR2  |   TR3  |
→→→ MOVING TO SIDE-B
```

🔍 Vehicles color-coded by type  
📦 Capacity and direction displayed  
🧵 Thread-safe via `print_mutex`

---

## ⏱️ Performance Metrics

At simulation end, the following stats are printed and logged:

- ⏳ Average waiting times (Side-A / Side-B)  
- 🚢 Total ferry travel time  
- 📈 Ferry capacity utilization per trip  
- ⚖️ Starvation observations for large vehicles  
- 🗂️ Vehicle trip log and summary table

---

## 🛠️ How to Run

### 🔨 Compile
```bash
make
```

### ▶️ Run
```bash
./ferry
```

### 🧹 Clean build
```bash
make clean
```

---

## 📽️ Demo

🎥 [Watch the Simulation Video](https://drive.google.com/file/d/1t4ywSNJVH5eeZe7VeOEYtBeKmUSiVzDp/view?usp=sharing)

---

## 🚀 Future Improvements

- ⏱️ Priority-based scheduling for fairness  
- ⚖️ Starvation-preventive boarding algorithms  
- 🧠 Configurable input (JSON/YAML)  
- 📊 Dashboard with GUI visualization  
- 🧪 Test automation for performance benchmarking  
- 🛳️ Multi-ferry system scalability  

---

## 🧑‍🏫 Educational Scope

This project simulates fundamental operating system concepts through an engaging ferry metaphor, making it suitable for:

- OS course assignments  
- Concurrency labs and workshops  
- Real-time systems teaching models

---

## 📄 License

This project is licensed under the MIT License. See `LICENSE` file for details.
