# AutoTurel C++ High-Performance Edition 🎯

[RU] Высокопроизводительное баллистическое ядро на C++, предназначенное для мгновенных расчетов наведения в системах автоматизированного огня. Проект учитывает сложные физические факторы и оптимизирован для работы в реальном времени (на 6-8 раз быстрее версии на C#).

[EN] High-performance C++ ballistic engine designed for instant guidance calculations in automated fire systems. The project accounts for complex physical factors and is optimized for real-time applications (6-8x faster than the C# version).

---

## 📐 Система координат / Coordinate System
[RU] В программе используется авиационный/инженерный стандарт:
[EN] The program uses the aviation/engineering standard:

*   **X+** : Вперед (Основная дистанция) / Forward (Main distance)
*   **Y+** : Влево (Поперечное смещение) / Left (Lateral offset)
*   **Z+** : Вверх (Высота) / Up (Altitude)

> [RU] При Yaw = 0° ствол смотрит строго вдоль оси X.
> [EN] When Yaw = 0°, the gun points strictly along the X axis.

---

## 🚀 Физические возможности / Physics Features
- **Standard Drag Functions**: Встроены точные таблицы G1 и G7 с интерполяцией.
- **Dynamic Atmosphere**: Формула Тетенса для влажности, расчет плотности воздуха и скорости звука от высоты.
- **Earth Effects**: Учет кривизны Земли (радиальная гравитация) и силы Кориолиса.
- **Ballistics**: Деривация (гироскопический увод), влияние ветра по 3 осям и баллистический коэффициент.
- **Targeting**: Упреждение для маневрирующих целей (скорость + ускорение).
- **Dual Mode**: Поддержка настильной и навесной (минометной) стрельбы.

---

## 💻 Полный пример вызова / Full Argument Call Example

[RU] Пример запуска со всеми доступными аргументами одновременно:
[EN] Example of a launch with all available arguments simultaneously:

```bash
AutoTurel.exe ^
  --target-pos 2500,100,50 ^
  --target-vel -10,15,2 ^
  --target-acc 0,1,0 ^
  --gun-offset 0,0.5,-0.2 ^
  --wind 5,-2,0 ^
  --v0 825.0 ^
  --bc 0.45 ^
  --drag-model G7 ^
  --deriv-k 0.0001 ^
  --temp 25.0 ^
  --hum 85.0 ^
  --press 101325 ^
  --lat 55.75 ^
  --north-angle 0.0 ^
  --iters 30 ^
  --max-time 180.0 ^
  --high-angle
```

---

## ⚙️ Список аргументов / Arguments List


| Аргумент / Argument | Пример | Описание / Description |
| :--- | :--- | :--- |
| `--target-pos` | `1000,0,0` | [RU] Цель (X,Y,Z) / [EN] Target Position |
| `--target-vel` | `0,20,-5` | [RU] Скорость цели / [EN] Target Velocity |
| `--target-acc` | `0,2,0` | [RU] Ускорение цели / [EN] Target Acceleration |
| `--gun-offset` | `0,1,-0.5` | [RU] Смещение пушки / [EN] Gun-to-Camera offset |
| `--wind` | `5,0,0` | [RU] Ветер (X,Y,Z) / [EN] Wind vector |
| `--v0` | `800` | [RU] Нач. скорость (м/с) / [EN] Muzzle velocity |
| `--bc` | `0.5` | [RU] Баллистический коэф. / [EN] Ballistic coefficient |
| `--drag-model` | `G7` | [RU] Модель (G1/G7) / [EN] Drag model |
| `--temp` | `15.0` | [RU] Температура (°C) / [EN] Temperature |
| `--hum` | `50.0` | [RU] Влажность (%) / [EN] Humidity |
| `--press` | `101325` | [RU] Давление (Па) / [EN] Pressure |
| `--deriv-k` | `0.0001` | [RU] Коэф. деривации / [EN] Derivation coefficient |
| `--lat` | `55.0` | [RU] Широта / [EN] Latitude |
| `--north-angle` | `0.0` | [RU] Угол севера / [EN] North-to-X angle |
| `--iters` | `15` | [RU] Число итераций / [EN] Iteration count |
| `--max-time` | `150` | [RU] Лимит полета (с) / [EN] Max flight time |
| `--high-angle` | *(flag)* | [RU] Навесной режим / [EN] Enable high-angle mode |

---

## 📊 Формат вывода / Output Format
[RU] Результат выводится в консоль одной строкой:
[EN] The result is output as a single line:

`Pitch;Yaw;Miss;AngMiss;TimeMs`

---

## 🛠 Компиляция / Compilation
[RU] Сборка через G++ с максимальной оптимизацией:
[EN] Build via G++ with maximum optimization:
```bash
g++ -O3 AutoTurel.cpp -o AutoTurel.exe
```
