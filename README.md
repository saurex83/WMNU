Мысли:
Начальный поиск сети это вообще отдельный модуль, который запускаеться netif.
модуль напрямую работает с радио.

Протокол синхронизации работет в режиме слэв и мастер. настройку получает из netif.

добавить стандартные приложения для мониторинга состояния узла и сбора статистики, а также удаленное управление. Передаваемые данные стандартищовать.
в режиме точки доступа приложения собирают данные но не передают.

в режиме точки доступа использовать динамические данные с уничтожением после чтения в уарт.
после приема данных от узла, структура с принятой информацией и метаданными.
после чтения по уарт, она уничтожается.

узел при старте можно завести в командный режим по отладочному уарт. запустить тесты оборудования, передачу и прием данных.
так же сменить ключ сети.

протокол маршрутизации. если у получателя открыто нескольк входящих окон,
выбираем окно случайным образом.

ядро рандома иницилизируем белым шумом.
делает это радио при иницилизации.

точка доступа. командный процессор уарт работает в режиме запрос-ответ.
прием данных в кольцевой буфер дма и прерывание по /n
