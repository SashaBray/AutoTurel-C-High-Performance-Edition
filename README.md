AutoTurel C++ High-Performance Edition 🎯
[RU] Высокопроизводительное баллистическое ядро на C++, предназначенное для мгновенных расчетов наведения в системах автоматизированного огня. Оптимизировано для работы в реальном времени.
[EN] High-performance C++ ballistic engine designed for instant guidance calculations in automated fire systems. Optimized for real-time applications.
📐 Система координат / Coordinate System
[RU] Используется инженерный стандарт:
[EN] Engineering standard used:
X+ : Вперед (Основная дистанция) / Forward (Main distance)
Y+ : Влево (Поперечное смещение) / Left (Lateral offset)
Z+ : Вверх (Высота) / Up (Altitude)
🚀 Физические возможности / Physics Features
Drag Models: Standard G1 & G7 drag functions with binary search interpolation.
Atmosphere: Tetens formula for humidity, dynamic air density, and speed of sound.
Earth Effects: Spherical Earth curvature (radial gravity) and Coriolis force.
External Ballistics: Gyroscopic drift (derivation) and wind influence (3-axis).
Targeting: Support for maneuvering targets (velocity + acceleration) with lead calculation.
💻 Полный пример вызова / Full Argument Call Example

[RU] Ниже представлен пример вызова функции со всеми доступными аргументами одновременно:
[EN] Below is an example of a function call using all available arguments simultaneously:

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

⚙️ Список аргументов / Arguments List
Аргумент / Argument	Пример / Example	Описание / Description
--target-pos	1000,0,0	[RU] Позиция цели (X,Y,Z) / [EN] Target Position
--target-vel	0,20,-5	[RU] Скорость цели (м/с) / [EN] Target Velocity
--target-acc	0,2,0	[RU] Ускорение цели (м/с²) / [EN] Target Acceleration
--gun-offset	0,1,-0.5	[RU] Смещение орудия от камеры / [EN] Gun-to-Camera offset
--wind	5,0,0	[RU] Вектор ветра (X,Y,Z) / [EN] Wind vector
--v0	800	[RU] Нач. скорость (м/с) / [EN] Muzzle velocity
--bc	0.5	[RU] Баллистический коэф. / [EN] Ballistic coefficient
--drag-model	G7	[RU] Модель (G1 или G7) / [EN] Drag model
--temp	15.0	[RU] Температура (°C) / [EN] Temperature
--hum	50.0	[RU] Влажность (%) / [EN] Humidity
--press	101325	[RU] Давление (Па) / [EN] Pressure
--deriv-k	0.0001	[RU] Коэф. деривации / [EN] Derivation coefficient
--lat	55.0	[RU] Широта (для Кориолиса) / [EN] Latitude
--north-angle	0.0	[RU] Угол севера от X / [EN] North-to-X angle
--iters	15	[RU] Число итераций / [EN] Iteration count
--max-time	150	[RU] Лимит полета (с) / [EN] Max flight time
--high-angle	(флаг)	[RU] Навесной режим / [EN] Enable high-angle mode
📊 Формат вывода / Output Format
[RU] Программа выводит результат в формате:
[EN] Program outputs data in the format:
Pitch;Yaw;Miss;AngMiss;TimeMs
Pitch: Elevation angle (degrees).
Yaw: Azimuth angle (degrees).
Miss: Final linear miss distance (meters).
AngMiss: Angular miss (degrees).
TimeMs: Computation time (milliseconds).
🛠 Компиляция / Compilation
[RU] Для максимальной производительности используйте флаг -O3:
[EN] Use -O3 flag for maximum performance:
bash
g++ -O3 AutoTurel.cpp -o AutoTurel.exe"# AutoTurel-C-High-Performance-Edition" 
