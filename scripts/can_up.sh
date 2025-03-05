#!/bin/bash

# Проверяем, был ли передан аргумент
if [ "$#" -ne 1 ]; then
    echo "Используйте: $0 {up|down}"
    exit 1
fi

ACTION=$1

# Включаем или выключаем интерфейс CAN в зависимости от переданного аргумента
case "$ACTION" in
    up)
        echo "Настройка интерфейса can0 с битрейтом 1000000 и включение."
        sudo ip link set can0 type can bitrate 1000000
        sudo ip link set can0 up
        echo "Интерфейс can0 активирован."
        ip a
        ;;
    down)
        echo "Выключение интерфейса can0."
        sudo ip link set can0 down
        echo "Интерфейс can0 деактивирован."
        ip a
        ;;
    *)
        echo "Неверный аргумент: $ACTION. Используйте 'up' или 'down'."
        exit 1
        ;;
esac
