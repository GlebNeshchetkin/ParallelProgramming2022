# ParallelProgramming2022
### Лабораторная работа 1. Выполнили: Нещеткин Глеб, Жаравина Полина, 19ПМИ-1.
Файл **matrix_multiplication.c** содержит код для трех методов умножения матриц: разбиение по строкам, столбцам и блокам. Программа проводит вычислительный эксперимент для разного числа потоков: от 1 до 128. Программа принимает число - размер квадратных перемножаемых матриц (элементы матрицы генерируются внутри). Ограничения: принимаемое числа должно быть степенью числа 2 и быть не меньше 128.\
**Результаты выполненного эксперимента для матриц размером 1024x1024:**
![Screenshot 2022-12-04 224240](https://user-images.githubusercontent.com/71218745/205511830-4bbe3dc7-0ad4-46c9-a1b5-a20701458a8e.png)
![Screenshot 2022-12-04 225522](https://user-images.githubusercontent.com/71218745/205512469-b2a863fe-4268-4479-b77a-6ec2628e4cc2.png)
![Screenshot 2022-12-04 230346](https://user-images.githubusercontent.com/71218745/205512844-10bc2d73-f088-40f6-816a-fd2af37b28ce.png)


**Вывод:** Лучшие результаты по времени показывает параллельная реализация перемножения матриц по строкам. 
