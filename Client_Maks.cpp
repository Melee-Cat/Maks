#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <map>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define IP_ADDR "127.0.0.1"
#define PORT 7777
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)
using namespace std;
SOCKET Connection;
string answer;
string username, password;

void SendMsg(string msg) //Функция отправки сообщений
{
	int msg_size = msg.size();
	send(Connection, (char*)&msg_size, sizeof(int), NULL);
	send(Connection, msg.c_str(), msg_size, NULL);
}

void SendInt(int value) //Функция отправки целочисленных сообщени
{
	send(Connection, (char*)&value, sizeof(int), NULL);
}

string RecvMsg() //Функция приема строковых сообщений
{
	int msg_size = 0;
	recv(Connection, (char*)&msg_size, sizeof(int), NULL);
	char* msg = new char[msg_size + 1];
	msg[msg_size] = '\0';
	recv(Connection, msg, msg_size, NULL);
	string str_msg = msg;
	delete[] msg;
	return str_msg;
}

void RecvFile()//Получение файла с сервера
{
	string Name, Extension, Path = "D:\\Recv\\";//Переменная для хранения названия файла
	cout << "Введите название получаемого файла (Пример: Test): ";
	cin >> Name;
	cout << "Введите расширение получаемого файла (Пример: txt): ";
	cin >> Extension;
	string FullPath = Path + Name + "." + Extension;
	SendMsg(Name);
	SendMsg(Extension);
	answer = RecvMsg();
	if (answer == "File_found") {
		ofstream File;
		File.open(FullPath, ios::binary);//Создаем файл для записи
		int SizeBuff = 0, SizeFile = 0, choice = 0;//Переменная для хранения буфера
		recv(Connection, (char*)&SizeFile, sizeof(int), NULL);//Получение размера файла

		// Переменная для хранения общего размера получаемого файла
		int TotalFileSize = SizeFile;

		do {
			recv(Connection, (char*)&SizeBuff, sizeof(int), NULL);//Получение от сервера размера передаваемого буфера
			char* Buff = new char[SizeBuff]; //Выделение динамической памяти под буфер
			recv(Connection, Buff, SizeBuff, NULL);//Получение данных с сервера и запись в буфер

			// Запись буфера в файл
			File.write(Buff, SizeBuff);

			// Обновление оставшегося размера файла
			SizeFile -= SizeBuff;

			// Освобождение выделенной памяти
			delete[] Buff;
		} while (SizeFile > 0);

		// Проверка, что весь файл был получен
		if (SizeFile == 0) {
			cout << "Файл " << Name << " успешно принят и сохранен в " << Path << endl;
		}
		else {
			cout << "Ошибка при получении файла " << Name << endl;
		}

		File.close();
	}
	else if (answer == "File_not_found")
	{
		cout << Name << " не существует" << endl;
	}
}

int main(int argc, const char* argv[])
{
	setlocale(LC_ALL, "");
	//cout << "CLIENT (Parshin 20PT1)" << endl;
	cout << "SERVER (Nagodkin 20PT1)" << endl;
	WSAData wsaData;//Инициализация библиотеки Winsock
	WORD DDLVersions = MAKEWORD(2, 1);// Установка версии библиотеки
	WSAStartup(DDLVersions, &wsaData);// 1-ый параметр - версия; 2-ой параметр - ссылка на структуру
	WSAData;
	if (WSAStartup(DDLVersions, &wsaData) != 0) {// Проверка подключения библиотеки
		cout << "WSAStart error" << endl;
		return 0;
	}
	Connection = socket(AF_INET, SOCK_STREAM, 0);// Установка значений сокета
	if (Connection < 0) // Если значения сокета не установлены
	{
		cout << "Socket() error" << endl;
		return 0;
	}
	SOCKADDR_IN address;//Связывание сокета с локальным адресом
	int sizeOFaddress = sizeof(address);//Размер адреса
	address.sin_port = htons(PORT); // заполнение структуры sockaddr_in. Указание адреса и порта сервера
	address.sin_family = AF_INET;// Записываем идентификатор домена
	address.sin_addr.s_addr = 0;//не забываем о сетевом порядке, IP адрес хоста
	HOSTENT* hst;
	if (inet_addr(IP_ADDR) != INADDR_NONE)//преобразование IP адреа из символьного в сетевой формат
		address.sin_addr.s_addr = inet_addr(IP_ADDR);// Устанавливаем сетевой порядок, IP адрес хоста
	else
		if (hst = gethostbyname(IP_ADDR))// попытка получить IP адрес по доменному имени сервера
			((unsigned long*)&address.sin_addr)[0] = ((unsigned long**)hst->h_addr_list)[0][0];
		else
		{
			closesocket(Connection); //закрываем сокет
			WSACleanup();//Деиницилизация библиотеки Winsock
			cout << "Invalid address" << endl;
			return 0;
		}
	if (connect(Connection, (sockaddr*)&address, sizeof(address)))//адрес сервера получен–пытаемся установить соединение
	{
		closesocket(Connection);//закрываем сокет
		WSACleanup();//Деиницилизация библиотеки Winsock
		cout << "Connect error" << endl;
		return 0;
	}
	int nclients = 0;//Переменная для хранения количества подключенных клиентов
	recv(Connection, (char*)&nclients, sizeof(int), NULL);//Получение от сервера число подключенных клиентов
	if (nclients < 100) {//Если меньше одного, то
		string SendMess;
		string Check;
		string Check_log;
		string login;
		string password;
		cout << "Введите логин: ";
		cin >> login;//Ввод логина
		SendMsg(login);//Отправка логина на сервер
		cout << "Введите пароль: ";
		cin >> password;//Ввод пароля
		SendMsg(password);//Отправка пароля на сервер
		Check_log = RecvMsg();//Получение ответа от сервера
		if (Check_log == "Authentication_was_successful") { //Аутентификация, прихдит ответ от сервера
			cout << "Аутентификация прошла успешно" << endl;
		}
		else {
			cout << "Аутентификация не удалась" << endl;
			closesocket(Connection);//закрываем сокет
			WSACleanup();//Деиницилизация библиотеки Winsock
			return 0;
		}
		int NFiles;
		if (login == "admin" && password == "admin") {
			cout << "Введите число передаваемых файлов: ";
			cin >> NFiles;//Если авторизация произошла под админкой, то вводится число передаваемых файлов
			SendInt(NFiles);//Отправка числа передаваемых файлов на сервер
		}
		do {
			cout << "Введите Recv для получения файла с сервера (для выхода введите Exit): ";
			cin >> SendMess;
			int size = SendMess.size();
			send(Connection, (char*)&size, sizeof(int), NULL);
			send(Connection, SendMess.c_str(), size, NULL);
			Check = RecvMsg();
			if (Check == "File_sent") {
				if (SendMess == "Recv")
				{
					RecvFile();
				}
			}
			else {
				cout << "Ошибка: Превышено число получаемых файлов с сервера!" << endl;
			}
		} while (SendMess != "Exit");
		cout << "Выполнение отключения" << endl;
		closesocket(Connection);//закрываем сокет
		WSACleanup();//Деиницилизация библиотеки WinsockA
	}
	else {
		cout << "Сервер переполнен" << endl;
		closesocket(Connection);//закрываем сокет
		WSACleanup();//Деиницилизация библиотеки Winsock
		return 0;
	}
	return 0;

}
