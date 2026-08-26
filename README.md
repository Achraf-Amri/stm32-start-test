# STM32 Start Test

Projet **CI/CD pour firmware STM32**, automatisant le cycle complet de développement jusqu'à la validation sur matériel réel.

Le pipeline intègre :

* Analyse statique avec **Cppcheck**
* Tests unitaires avec **Ceedling / Unity**
* Compilation ARM avec **Docker**
* Flash du STM32 via **ST-Link**
* Tests **Hardware-in-the-Loop (HIL)**
* Flash sans fil **OTA via ESP32**
* Archivage des artefacts
* Notifications Jenkins

---

## Architecture

### Flash classique

```text
Git Push
   ↓
Jenkins
   ↓
Tests + Analyse
   ↓
Compilation
   ↓
ST-Link
   ↓
STM32F401RE
   ↓
HIL Test
```

### Flash OTA

```text
Git Push
   ↓
Jenkins
   ↓
Tests + Analyse
   ↓
Compilation
   ↓
Wi-Fi
   ↓
ESP32
   ↓
UART / STM32 Bootloader
   ↓
STM32F401RE
   ↓
HIL Test
```

---

## Matériel

* **Nucleo-F401RE / STM32F401RE**
* **ESP32-WROOM-32** pour le flash OTA
* ST-Link intégré pour le flash classique
* Connexion Wi-Fi pour l'OTA

### Connexion ESP32 → STM32

| ESP32  | STM32 | Fonction   |
| ------ | ----- | ---------- |
| GPIO16 | PA9   | UART RX    |
| GPIO17 | PA10  | UART TX    |
| GPIO4  | BOOT0 | Bootloader |
| GPIO5  | NRST  | Reset      |
| GND    | GND   | Masse      |

---

## Structure

```text
stm32-start-test/
├── Core/                 # Firmware STM32
├── Drivers/              # HAL + CMSIS
├── src/                  # Logique métier
├── test/                 # Tests unitaires
├── hil_tests/            # Tests Hardware-in-the-Loop
├── esp32_ota/             # Flash OTA via ESP32
├── docs/                  # Documentation
├── Dockerfile
├── Jenkinsfile
├── Makefile
└── project.yml
```

---

## Prérequis

### Développement

* Git
* STM32CubeIDE / STM32CubeMX
* Python 3
* Docker

### CI/CD

* Jenkins
* Docker
* ARM GCC
* Cppcheck
* Ceedling / Unity
* ST-Link / `st-flash`

### OTA

* ESP32
* Wi-Fi
* Python `requests`

---

## Installation

```bash
git clone https://github.com/Achraf-Amri/stm32-start-test.git
cd stm32-start-test
```

Construire l'environnement Docker :

```bash
docker build -t stm32-build-env .
```

---

## Tests unitaires

```bash
docker run --rm \
  -v $(pwd):/project \
  stm32-build-env \
  sh -c "ceedling test:all"
```

---

## Analyse statique

```bash
cppcheck --enable=all --error-exitcode=1 ...
```

L'analyse est également exécutée automatiquement par Jenkins.

---

## Compilation

```bash
docker run --rm \
  -v $(pwd):/project \
  stm32-build-env \
  sh -c "make clean && make"
```

Le build génère notamment :

```text
.elf
.hex
.bin
```

---

## Flash STM32

### ST-Link

Le firmware peut être flashé directement avec le ST-Link intégré :

```bash
st-flash write firmware.bin 0x08000000
```

### OTA — ESP32

L'ESP32 permet de transmettre le firmware au **bootloader UART intégré du STM32** via Wi-Fi.

Le firmware est envoyé avec :

```bash
python3 esp32_ota/scripts/send_firmware.py \
    firmware.bin \
    http://<ESP32-IP>/flash
```

L'ESP32 reste connecté au STM32 et reçoit les nouveaux firmwares depuis Jenkins.

---

## Jenkins

Le pipeline Jenkins automatise :

```text
Checkout
   ↓
Static Analysis
   ↓
Unit Tests
   ↓
Build
   ↓
Flash
   ↓
HIL Test
   ↓
Artifacts
   ↓
Notifications
```

Le flash peut être effectué :

* via **ST-Link**
* ou via **ESP32 OTA**

selon la configuration du pipeline.

---

## HIL Test

Après le flash, un test matériel vérifie automatiquement le démarrage du firmware.

```bash
python3 hil_tests/test_uart_response.py
```

Résultat attendu :

```text
Reponse recue : 'BOOT_OK'
Test HIL : PASS
```

---

## État du projet

| Fonctionnalité             | Statut |
| -------------------------- | ------ |
| CI/CD Jenkins              | ✅      |
| Docker Build               | ✅      |
| Cppcheck                   | ✅      |
| Tests unitaires            | ✅      |
| Compilation ARM            | ✅      |
| Flash ST-Link              | ✅      |
| HIL                        | ✅      |
| ESP32 Wi-Fi                | ✅      |
| Communication UART         | ✅      |
| Synchronisation bootloader | ✅      |
| Flash OTA complet          | ⏳      |

---

## Documentation

La documentation détaillée est disponible dans :

```text
docs/
```

Notamment :

* `Phase3_Documentation.md`
* `Phase6_Documentation.md`

La documentation du module OTA se trouve dans :

```text
esp32_ota/
```

---

## Objectif

L'objectif du projet est d'obtenir un pipeline permettant de passer automatiquement de :

```text
git push
    ↓
Jenkins
    ↓
Build
    ↓
Flash
    ↓
Validation matérielle
```

avec la possibilité d'utiliser un **déploiement OTA via ESP32**, afin de supprimer la manipulation physique du câble ST-Link.

---

## Auteur

**Achraf Amri**

GitHub :
https://github.com/Achraf-Amri/stm32-start-test

## Licence

Projet à usage pédagogique et portfolio personnel.
