# AquariumSun
Прошивка для аквариумного светильника

Прошивка для модуля WeMos D1 WiFi Uno, на основе ESP8266.

======================================================================================================================================
Функции, которые реализует данная прошивка:

- синхронизация времени модуля с мировым временем через сервер time.nist.gov
- управление яркостью светодиодов
- сохранение параметров работы в EEPROM память
- управление воздушным компрессором и балоном CO2 через реле (пины D12 и D11 соответственно)

======================================================================================================================================
Логика работы:

Светильник имитирует реальную смену освещения в течении суток. Сутки работы светильника условно можно разделить на 4 фазы:
- день
- сумерки
- ночь
- рассвет

В течении дневной фазы светодиоды работают на максимальной яркости, с небольшими изменениями (незначительное увеличение яркости с начала фазы к середине и уменьшение к исходному состоянию в конце фазы).
Фазы сумерек и рассвета являются переходными и работают зеркально (рассвет увеличивает яркость с нуля на почти максимальное состояние, закат наоборот). При этом включение света происходит с левых светодиодов постепенно в сторону правых, выключение наоборот.
В ночной фазе имитируется лунный свет. Работают только белые светодиоды, которые зажигаются поочерёдно слева направо в течении всей ночи, имитируя движение луны по ночному небу.

======================================================================================================================================
Структура прошивки:

Прошивка состоит из нескольких модулей
- модуль управления светодиодами - формирует массив яркостей светодиодов
- модуль WiFi - отвечает за синхронизацию времени и обработку HTTP запросов от пользователя (изменение параметров работы светильника)
- модуль EEPROM - отвечает за сохранение параметров в долговременную память контроллера и загрузку после включения прибора

Модуль управления светодиодами в свою очередь разбит на несколько абстрактных уровней. Перечень начиная от низкоуровневых к высокоуровневым:

1. PCA9685. Отображает соединение с PWM драйвером. Можно абстрактно рассматривать как светодиод. Основные методы setPin и getPin, позволяют менять режим работы одного конкретного светодиода. Заимствован у AdaFruit и переделан под свои нужды.
2. LedCluster. Содержит в себе массив из четырех экземпляров PCA9685. Абстракция RGBW светодиода. Основной метод setNewBright, при помощи которого можно задавать цвет и яркость абстрактного светодиода, сообщая RGB код и значение яркости.
3. LedCandle. Связывает все светодиоды в одну абстрактную сущность. Принимает входные данные в виде текущей фазы суток и фрагмента текущей фазы. Основываясь на этих данных формирует массив RGBW цветов экземпляров LedCluster.
4. LightController. Основываясь на текущем времени и сохранённых настройках вычисляет текущую фазу суток и её фрагмент.

======================================================================================================================================
Управление параметрами светильника:

Настройка режима работы происходит при помощи веб-интерфейса. При http get запросе на ip-адрес устройства, выдаётся html страница с настройками режима работы.
Возможные настройки:
- колонка "Time settings":
  Day phase duration - длительность дневной фазы освещения. Формат "HH:MM";
  Day phase end time - время окончания дневной фазы. Формат "HH:MM";
  Phase change duration - длительность фазы перехода между дневной и ночной фазами. Формат "HH:MM";
- колонка "Led settings":
  Red maximum - максимальное значение яркости красных светодиодов. Диапазон значений: 0-255; 
  Green maximum - максимальное значение яркости зелёных светодиодов. Диапазон значений: 0-255;
  Blue maximum - максимальное значение яркости синих светодиодов. Диапазон значений: 0-255;
  White maximum - максимальное значение яркости белых светодиодов. Диапазон значений: 0-255;
  Night maximum - максимальное значение яркости белых светодиодов во время ночной фазы. Диапазон значений: 0-255;
- колонка "Other settings":
  PWM frequency - не фунционирует
  Control mode - не фунционирует

======================================================================================================================================
Структура сведодиодного светильника:

1. Модуль ESP8266. Содержит главную логику управления, хранит настройки, синхронизирует время, формирует массив яркостей светодиодов и передаёт массив к PWM-драйверам по интерфейсу I2C.
2. 16-канальные PWM драйверы PCA9685 (две штуки). Принимают массив из 16 скважностей, по одному значению скважности на каждый канал (всего 32 канала в сумме на оба драйвера) и включают соответствующие драйверы светодиодов на нужную яркость.
3. Силовые драйверы светодиодов. Принимают управляющий PWM сигнал и выдают нужный ток на светодиоды.
4. Светдоиоды. 41 штука, 5 разных цветов: W-белый, R-красный, G-зелёный, B-синий, Y-жёлтый. Некоторые объеденены в каналы последовательно по несколько штук. Всего 32 канала.

Для удобства управления светодиоды объединены в группы RGBW. В каждой группе есть светодиоды красного, зелёного, синего и белого цветов. Таким образом, можно формировать оттенок каждой группы и яркость дополнительного белого светодиода. Отдельно стоит обратить внимание на последнюю группу 7, в неё вошли оставшиеся светодиоды R0, R7, W7, Y1.
Драйверы светдиодов (далее по тексту - светодиоды) пронумерованы двумя способами:
- число в скобках (например (3)) обозначает канал PWM драйвера. Так как используется два PWM драйвера по 16 каналов в каждом, номера каналов с 0 по 15 относятся к первому драйверу, номера с 16 по 31 - ко второму. Например, светодиод (11) - 11 канал первого драйвера, светодиод 20 - 4 канал второго драйвера.
- буква и число (например, R1) обозначает цвет и номер группы светодиодов. Например, в 4 группе присутствуют светодиоды R4, G4, B4, W4.

Соответствие между номером канала и названием конкретного светодиода находится в начале файла 03-LedCandle.h 

======================================================================================================================================
Схема расположения светодиодов на светильнике:
__________________________________________________________________________________________
|     | (1) | (2) | (5) | (6) |(11) |(12) |(15) |(17) |(20) |(21) |(25) |(27) |(30) |     |
|     | W1  | B1  | W2  | B2  | W3  | B3  | W4  | B4  | W5  | B5  | W6  | B6  | W7  |     |
|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|
| (0) | (1) | (3) |(24) | (9) |(11) |(13) |(24) |(18) |(20) |(22) |(24) |(28) |(30) |(31) |
| R0  | W1  | R1  | Y1  | R2  | W3  | R3  | Y1  | R4  | W5  | R5  | Y1  | R6  | W7  | R7  |
|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|
|     | (1) | (4) | (5) |(10) |(11) |(14) |(15) |(19) |(20) |(23) |(25) |(29) |(30) |     |
|     | W1  | G1  | W2  | G2  | W3  | G3  | W4  | G4  | W5  | G5  | W6  | G6  | W7  |     |
|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|