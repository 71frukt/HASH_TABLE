import subprocess
import json
import time
import math
import numpy as np


def read_perf_json(filepath):
    """Чтение JSONL-формата, который генерирует perf"""
    data = []
    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if line:
                try:
                    # Заменяем европейские десятичные запятые на точки
                    fixed_line = line.replace(',00', '.00').replace(',000000', '.000000')
                    data.append(json.loads(fixed_line))
                except json.JSONDecodeError as e:
                    print(f"Ошибка парсинга строки: {line}\n{str(e)}")
    return data

def run_perf(command, iterations=10):
    cycles_data = []
    
    for i in range(iterations):
        try:
            # Запускаем perf stat
            subprocess.run(
                ["taskset", "-c", "0-7", "nice", "-n", "-20", "perf", "stat", "-j", "-e", "cpu_core/cycles/", "-o", "perf_tmp.json"] + command,
                check=True,
                stderr=subprocess.PIPE,
                stdout=subprocess.PIPE
            )
            
            # Читаем и парсим JSONL
            data = read_perf_json("perf_tmp.json")
            
            # Извлекаем значения тактов
            core_cycles = None
            
            for item in data:
                if item.get("event") == "cpu_core/cycles/u":
                    core_cycles = float(item["counter-value"].replace(',', '.'))
            
            if core_cycles is not None:
                cycles_data.append(core_cycles)
                print(f"Запуск {i+1}: тактов P-cores: {int(core_cycles):,}")
            else:
                print(f"Запуск {i+1}: не удалось извлечь данные о тактах")
                
        except subprocess.CalledProcessError as e:
            print(f"Ошибка perf stat: {e.stderr.decode()}")
        except Exception as e:
            print(f"Ошибка обработки: {str(e)}")
        
        # time.sleep(0.5)
    
    rmse = 0
    average_cycles = np.mean(cycles_data)

    if cycles_data:
        mean = np.mean(cycles_data)
        # 2. Вычисляем квадраты отклонений от среднего
        squared_errors = [(x - mean) ** 2 for x in cycles_data]
        # 3. Находим среднее квадратов ошибок
        mean_squared_error = np.mean(squared_errors)
        # 4. Берём квадратный корень
        rmse = math.sqrt(mean_squared_error) / mean

    else:
        print("Нет валидных данных для расчёта погрешности")

    if cycles_data:
        print("\nРезультаты:")
        print(f"Все измерения: {cycles_data}")
        print(f"Среднее значение тактов: {average_cycles:,.0f}")
        print(f"Среднеквадратическая погрешность: {rmse:,.5f}")

        with open("perf_results.json", "w") as f:
            json.dump({
                "iterations": iterations,
                "cycles_data": cycles_data,
                "average_cycles": average_cycles,
                "rmse": rmse,
                "timestamp": time.strftime("%Y-%m-%d %H:%M:%S")
            }, f, indent=2)
    else:
        print("Нет валидных данных для анализа")

if __name__ == "__main__":
    run_perf(["./hash_table/build/hash_table"])