# Projet UWB : Contrôleur de Déplacement

[![Arduino](https://img.shields.io/badge/Arduino-Portenta%20UWB%20Shield-blue?logo=arduino)](https://store.arduino.cc/products/portenta-uwb-shield) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Implémentation simple d'un **protocole UWB** pour le cours **Objets Communiquants**. Utilise **1x Portenta UWB Shield** comme contrôleur (maître) et **2x Arduino Stella UWB** comme ancres fixes pour détecter les déplacements : **haut, bas, droite, gauche** via triangulation de distances [file:1][file:2].

Démo basique : Le Portenta mesure les distances aux deux Stellas et déduit la position/mouvement en 2D simplifiée.

## 🎯 Fonctionnalités

- Mesure Two-Way Ranging (TWR) précise (~10cm) entre Portenta et Stellas.
- Triangulation 1D/2D : Position relative via distances à deux ancres fixes (ex: espacées de 50cm).
- Détection mouvements : 
  - Droite/Gauche : Différence distances Stella A vs B.
  - Haut/Bas : Évolution temporelle des moyennes (moyenne glissante sur 10 échantillons).
- Feedback LED/Serial : Voyants verts/rouges pour proximité, debug distances en temps réel [file:1].
- Adresses MAC fixes : Portenta `0x4142` (src), Stellas `0x1111` et `0x2222` (dst).

## 🛠 Matériel Requis

| Composant | Quantité | Rôle | Adresse MAC |
|-----------|----------|------|-------------|
| Portenta H7 + UWB Shield | 1 | Contrôleur mobile (main/tag) | `0x41,0x42` [file:1] |
| Arduino Stella UWB | 2 | Ancres fixes (table) | Stella A: `0x11,0x11`<br>Stella B: `0x22,0x22` [file:2] |

- Distance max testée : ~2m (précision UWB optimale <1m).
- Alim : USB pour tous.

## 🚀 Installation

1. **Flasher les Stellas (ancres)** :
   - Uploadez `sketch-stella-controller.ino` sur Stella A (ID `0x1111`) et Stella B (ID `0x2222`).
   - Bibliothèque : `StellaUWB.h` (pré-installée Arduino IDE).

2. **Flasher Portenta (contrôleur)** :
   - Uploadez `sketch-portenta-controlee.ino` sur Portenta + Shield.
   - Bibliothèque : `PortentaUWBShield.h`.

3. **Test** :
   ```
   Positionnez Stellas fixes (50cm apart).
   Ouvrez Serial Monitor (115200 baud).
   Déplacez Portenta : Observez distances et LEDs.
   Exemple output :
   Distance cm: 45.2 | Average: 48.1 | Mouvement: DROITE
   ```