def create():  # добавляет однострочные заметки
    h=input()
    file.write(h+'\n')
def delete():
    try:
        line_number_to_delete = int(input("Введите номер строки для удаления (начиная с 0): "))
    except ValueError:
        print("Ошибка: Индекс должен быть целым числом.")
        return

    file.seek(0)
    lines = file.readlines()

    if 0 <= line_number_to_delete < len(lines):
        del lines[line_number_to_delete]

        file.seek(0)
        file.truncate(0)
        file.writelines(lines)
    else:
        print("Ошибка: Неверный номер строки.")

def search():  # ищет заметку
    pass
def close():
    print('До свидания')
    exit()
def show():  # выводит все
    pass


def interface():
    print('Здарова чувак,я менеджер твоих заметок')
    while True:
        print('''Вот список команд:
        1)create
        2)delete
        3)search
        4)close
        5)show
        Введи номер выбраной команды''')
        answer = input()
        match answer:
            case '1':
                create()
            case '2':
                delete()
            case '3':
                search()
            case '4':
                close()
            case '5':
                show()
            case _:
                print("Дурачок,неправильный ввод,введи нормально")
                continue

file = open('managerofprojects.txt', 'r+',encoding='utf8')
interface()