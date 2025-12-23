# Wokwi Simulator

## 1. Wokwi Simulator VS Code Extension installieren
https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode

## 2 Wokwiki License Key  einrichten:
https://docs.wokwi.com/vscode/getting-started \
"press F1 and select "Wokwi: Request a new License" 

## 3. ESPHome firmeware kompilieren
```bash
esphome compile examples/wokwi-sim/wokwi-sim.yaml 
```

## 4. HM-MOD-RPI Wokwiki custom chip kompilieren
```bash
clang --target=wasm32-unknown-wasi -nostartfiles -Wl,--import-memory -Wl,--export-table -Wl,--no-entry -Werror  -o  examples/wokwi-sim/custom_chip/hm-mod-rpi.chip.wasm examples/wokwi-sim/custom_chip/hm-mod-rpi.chip.c
```

## 4. Simulation starten
diagram.json  öffnen und simulation strten






