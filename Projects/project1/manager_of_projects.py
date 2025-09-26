def create():#добавляет однострочные заметки
    pass
def delete():#удаляет
    pass
def search():#ищет заметку
    pass
def close():
    print('Досвидание')
    exit()
    pass
def show():#выводит все
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
        answer=input()
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
interface()
file=open('managerofprojects.txt','wr')
